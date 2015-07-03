/*******************************************************************************
 *                                                                             *
 * Copyright (c) 2009 Texas Instruments Incorporated - http://www.ti.com/      *
 *                        ALL RIGHTS RESERVED                                  *
 *                                                                             *
 ******************************************************************************/

#include "algLink_priv.h"
#include <mcfw/src_bios6/utils/utils_mem.h>

//#define UTILS_SCD_GET_OUTBUF_SIZE()   (sizeof(SCD_Output) + (sizeof(SCD_blkChngMeta) * ALG_LINK_SCD_MAX_BLOCKS_IN_FRAME))

#define UTILS_SCD_GET_OUTBUF_SIZE()   (sizeof(AlgLink_ScdResult))

static Int32 AlgLink_createOutObj(AlgLink_Obj * pObj)
{
    AlgLink_OutObj *pOutObj;
    System_LinkChInfo *pOutChInfo;
    Int32 status;
    UInt32 bufIdx;
    Int i,j,queueId,chId;
    UInt32 totalBufCnt;

    for(queueId = 0; queueId < ALG_LINK_MAX_OUT_QUE; queueId++)
    {

        pOutObj = &pObj->outObj[queueId];

        pObj->outObj[queueId].numAllocPools = 1;

        pOutObj->buf_size[0] = UTILS_SCD_GET_OUTBUF_SIZE();
        pOutObj->buf_size[0] = 
          VpsUtils_align(pOutObj->buf_size[0], 
                         SharedRegion_getCacheLineSize(SYSTEM_IPC_SR_CACHED));

        status = Utils_bitbufCreate(&pOutObj->bufOutQue, TRUE, FALSE,
                                    pObj->outObj[queueId].numAllocPools);
        UTILS_assert(status == FVID2_SOK);

        totalBufCnt = 0;
        for (i = 0; i < pOutObj->numAllocPools; i++)
        {
            pOutObj->outNumBufs[i] = (pObj->scdAlg.createArgs.numValidChForSCD * pObj->scdAlg.createArgs.numBufPerCh);

            for (j = 0; j < pObj->scdAlg.createArgs.numValidChForSCD; j++)
            {
                pOutObj->ch2poolMap[j] =  i;
            }

            status = Utils_memBitBufAlloc(&(pOutObj->outBufs[totalBufCnt]),
                                          pOutObj->buf_size[i],
                                          pOutObj->outNumBufs[i]);
            UTILS_assert(status == FVID2_SOK);

            for (bufIdx = 0; bufIdx < pOutObj->outNumBufs[i]; bufIdx++)
            {
                UTILS_assert((bufIdx + totalBufCnt) < ALG_LINK_SCD_MAX_OUT_FRAMES);
                pOutObj->outBufs[bufIdx + totalBufCnt].allocPoolID = i;
                pOutObj->outBufs[bufIdx + totalBufCnt].doNotDisplay =
                    FALSE;
                status =
                    Utils_bitbufPutEmptyBuf(&pOutObj->bufOutQue,
                                            &pOutObj->outBufs[bufIdx +
                                                              totalBufCnt]);
                UTILS_assert(status == FVID2_SOK);
            }
            /* align size to minimum required frame buffer alignment */
            totalBufCnt += pOutObj->outNumBufs[i];
        }
    }
    pObj->info.numQue = ALG_LINK_MAX_OUT_QUE;

    for (queueId = 0u; queueId < ALG_LINK_MAX_OUT_QUE; queueId++)
    {
        pObj->info.queInfo[queueId].numCh = pObj->inQueInfo.numCh;
    }

    for (chId = 0u; chId < pObj->inQueInfo.numCh; chId++)
    {
        for (queueId = 0u; queueId < ALG_LINK_MAX_OUT_QUE; queueId++)
        {
            pOutChInfo = &pObj->info.queInfo[queueId].chInfo[chId];
            pOutChInfo->bufType = SYSTEM_BUF_TYPE_VIDBITSTREAM;
            pOutChInfo->codingformat = NULL;
            pOutChInfo->memType = NULL;
            pOutChInfo->scanFormat = pObj->inQueInfo.chInfo[chId].scanFormat;
            pOutChInfo->width = pObj->inQueInfo.chInfo[chId].width;
            pOutChInfo->height = pObj->inQueInfo.chInfo[chId].height;
        }
    }


    return (status);
}



Int32 AlgLink_algCreate(AlgLink_Obj * pObj, AlgLink_CreateParams * pPrm)
{
    Int32 status;

    Vps_printf(" %d: ALG : Create in progress !!!\n", Utils_getCurTimeInMsec());


    UTILS_MEMLOG_USED_START();
    memcpy(&pObj->createArgs, pPrm, sizeof(*pPrm));
    memcpy(&pObj->scdAlg.createArgs, &pPrm->scdCreateParams, sizeof(AlgLink_ScdCreateParams));
    memcpy(&pObj->osdAlg.osdChCreateParams, &pPrm->osdChCreateParams, (sizeof(AlgLink_OsdChCreateParams) * ALG_LINK_OSD_MAX_CH));
    status = System_linkGetInfo(pPrm->inQueParams.prevLinkId, &pObj->inTskInfo);


    UTILS_assert(status == FVID2_SOK);

    UTILS_assert(pPrm->inQueParams.prevLinkQueId < pObj->inTskInfo.numQue);

    memcpy(&pObj->inQueInfo,
           &pObj->inTskInfo.queInfo[pPrm->inQueParams.prevLinkQueId],
           sizeof(pObj->inQueInfo));

    UTILS_assert(pObj->inQueInfo.numCh <= ALG_LINK_OSD_MAX_CH);

    pObj->scdAlg.inQueInfo = &pObj->inQueInfo;

    pObj->osdAlg.inQueInfo = &pObj->inQueInfo;

    if (pObj->createArgs.enableOSDAlg)
    {
        status = AlgLink_OsdalgCreate(&pObj->osdAlg);
        UTILS_assert(status == FVID2_SOK);
    }

    if (pObj->createArgs.enableSCDAlg)
    {
        status = AlgLink_ScdalgCreate(&pObj->scdAlg);
        UTILS_assert(status == FVID2_SOK);
        AlgLink_createOutObj(pObj);
    }

    pObj->isCreated = ALG_LINK_STATE_ACTIVE;

    UTILS_MEMLOG_USED_END(pObj->memUsed);
    UTILS_MEMLOG_PRINT("ALGLINK",
                       pObj->memUsed,
                       UTILS_ARRAYSIZE(pObj->memUsed));
    Vps_printf(" %d: ALG : Create Done !!!\n", Utils_getCurTimeInMsec());
    return FVID2_SOK;
}

Int32 AlgLink_algDelete(AlgLink_Obj * pObj)
{
    Int32 status;
    Int32 i,outId,bitbuf_index;
    AlgLink_OutObj *pOutObj;

    Vps_printf(" %d: ALG : Delete in progress !!!\n", Utils_getCurTimeInMsec());


    if (pObj->createArgs.enableOSDAlg)
    {
        status = AlgLink_OsdalgDelete(&pObj->osdAlg);
        UTILS_assert(status == FVID2_SOK);
    }

    if (pObj->createArgs.enableSCDAlg)
    {
        status = AlgLink_ScdalgDelete(&pObj->scdAlg);
        UTILS_assert(status == FVID2_SOK);
        for (outId = 0; outId < ALG_LINK_MAX_OUT_QUE; outId++)
        {
            {
                pOutObj = &pObj->outObj[outId];

                status = Utils_bitbufDelete(&pOutObj->bufOutQue);
                UTILS_assert(status == FVID2_SOK);
                bitbuf_index = 0;
                for (i = 0; i < pOutObj->numAllocPools; i++)
                {
                    UTILS_assert((pOutObj->outBufs[bitbuf_index].bufSize ==
                                  pOutObj->buf_size[i]));
                    status = Utils_memBitBufFree(&pOutObj->outBufs[bitbuf_index],
                                        pOutObj->outNumBufs[i]);
                    UTILS_assert(status == FVID2_SOK);
                    bitbuf_index += pOutObj->outNumBufs[i];
                }
            }
        }
    }
    pObj->isCreated = ALG_LINK_STATE_INACTIVE;
    Vps_printf(" %d: ALG : Delete Done !!!\n", Utils_getCurTimeInMsec());

	return FVID2_SOK;
}


Int32 AlgLink_algProcessData(AlgLink_Obj * pObj)
{
    UInt32 frameId, status;
    System_LinkInQueParams *pInQueParams;
    FVID2_Frame *pFrame;

    FVID2_FrameList frameList;

    pInQueParams = &pObj->createArgs.inQueParams;
    System_getLinksFullFrames(pInQueParams->prevLinkId,
                              pInQueParams->prevLinkQueId, &frameList);

    if (frameList.numFrames)
    {
        /* SCD should be done first as it requires to operate on raw YUV */
        if (pObj->createArgs.enableSCDAlg)
        {
            status = AlgLink_ScdalgProcessData(&pObj->scdAlg, &frameList, &pObj->outObj[0].bufOutQue);

            if (status == FVID2_SOK)
            {
                /* Send-out the output bitbuffer */
                System_sendLinkCmd(pObj->createArgs.outQueParams[ALG_LINK_SCD_OUT_QUE].nextLink,
                                   SYSTEM_CMD_NEW_DATA);
            }

        }

        for(frameId=0; frameId<frameList.numFrames; frameId++)
        {
            pFrame = frameList.frames[frameId];

            if(pFrame->channelNum >= pObj->inQueInfo.numCh)
                continue;

            // do SW OSD
            if (pObj->createArgs.enableOSDAlg)
            {
                AlgLink_OsdalgProcessFrame(&pObj->osdAlg, pFrame);
            }
        }
        System_putLinksEmptyFrames(pInQueParams->prevLinkId,
                                   pInQueParams->prevLinkQueId, &frameList);
    }

    return FVID2_SOK;
}
