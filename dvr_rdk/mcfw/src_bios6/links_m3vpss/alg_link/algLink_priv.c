/*******************************************************************************
 *                                                                             *
 * Copyright (c) 2009 Texas Instruments Incorporated - http://www.ti.com/      *
 *                        ALL RIGHTS RESERVED                                  *
 *                                                                             *
 ******************************************************************************/

#include "algLink_priv.h"
#include <mcfw/src_bios6/utils/utils_mem.h>

static Int32 AlgLink_createOutObj(AlgLink_Obj * pObj)
{
    Int32 status;

    status = Utils_bufCreate(&pObj->framesOutBufQue, FALSE, FALSE);
    UTILS_assert(status == FVID2_SOK);


    pObj->info.numQue = ALG_LINK_MAX_OUT_QUE;

    /* SCD is NOT integrated as of now */
    pObj->info.queInfo[ALG_LINK_SCD_OUT_QUE] = pObj->inQueInfo;

    /* frames out info is exactly same as input info */
    pObj->info.queInfo[ALG_LINK_FRAMES_OUT_QUE] = pObj->inQueInfo;

    return (status);
}



Int32 AlgLink_algCreate(AlgLink_Obj * pObj, AlgLink_CreateParams * pPrm)
{
    Int32 status;

    Vps_printf(" %d: ALG : Create in progress !!!\n", Utils_getCurTimeInMsec());

    UTILS_MEMLOG_USED_START();
    memcpy(&pObj->createArgs, pPrm, sizeof(*pPrm));
    memcpy(&pObj->osdAlg.osdChCreateParams, &pPrm->osdChCreateParams, (sizeof(AlgLink_OsdChCreateParams) * ALG_LINK_OSD_MAX_CH));
    memcpy(&pObj->scdAlg.scdCreateParams, &pPrm->scdCreateParams, sizeof(AlgLink_ScdCreateParams));

    Vps_printf(" %d: ALG : Prev Link ID = %08x !!!\n", Utils_getCurTimeInMsec(), pPrm->inQueParams.prevLinkId);

    status = System_linkGetInfo(pPrm->inQueParams.prevLinkId, &pObj->inTskInfo);


    UTILS_assert(status == FVID2_SOK);

    UTILS_assert(pPrm->inQueParams.prevLinkQueId < pObj->inTskInfo.numQue);

    memcpy(&pObj->inQueInfo,
           &pObj->inTskInfo.queInfo[pPrm->inQueParams.prevLinkQueId],
           sizeof(pObj->inQueInfo));

    UTILS_assert(pObj->inQueInfo.numCh <= ALG_LINK_OSD_MAX_CH);

    pObj->osdAlg.inQueInfo = &pObj->inQueInfo;

    if (pObj->createArgs.enableOSDAlg)
    {
        status = AlgLink_osdAlgCreate(&pObj->osdAlg);
        UTILS_assert(status == FVID2_SOK);
    }


    if (pObj->createArgs.enableUVAlg)
    {
		status = Utils_dmaCreateCh(&pObj->dmaUV, UTILS_DMA_DEFAULT_EVENT_Q, 16, FALSE);
		UTILS_assert(status == FVID2_SOK);
    }

    pObj->scdAlg.inQueInfo = &pObj->inQueInfo;

    if (pObj->createArgs.enableSCDAlg)
    {
        status = AlgLink_scdAlgCreate(&pObj->scdAlg);
        UTILS_assert(status == FVID2_SOK);
    }

    AlgLink_createOutObj(pObj);

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

    Vps_printf(" %d: ALG : Delete in progress !!!\n", Utils_getCurTimeInMsec());


    if (pObj->createArgs.enableOSDAlg)
    {
        status = AlgLink_osdAlgDelete(&pObj->osdAlg);
        UTILS_assert(status == FVID2_SOK);
    }
    if (pObj->createArgs.enableUVAlg)
    {
		status = Utils_dmaDeleteCh(&pObj->dmaUV);
		UTILS_assert(status == FVID2_SOK);
		
    }

    if (pObj->createArgs.enableSCDAlg)
    {
        status = AlgLink_scdAlgDelete(&pObj->scdAlg);
        UTILS_assert(status == FVID2_SOK);
    }

    status = Utils_bufDelete(&pObj->framesOutBufQue);
    UTILS_assert(status == FVID2_SOK);

    Vps_printf(" %d: ALG : Delete Done !!!\n", Utils_getCurTimeInMsec());

	return FVID2_SOK;
}
//Utils_dmaCopyUV(Utils_DmaChObj *pObj, Utils_DmaCopyUV *pInfo, UInt32 numTransfers)

Int32 AlgLink_UVAlgProcessFrames(Utils_TskHndl *pTsk, AlgLink_Obj * pObj, FVID2_FrameList *pFrameList)
{
	int i;
	System_LinkChInfo *chInfo;
	System_LinkQueInfo *qInfo;
	FVID2_Frame *pFrame;
	Utils_DmaCopyUV copyInfo[16];
	//
	if (pFrameList->numFrames)
	{
		for(i=0;i<pFrameList->numFrames;i++)
		{
			
			pFrame=pFrameList->frames[i];
			chInfo=&(pObj->inQueInfo.chInfo[pFrame->channelNum]);
			copyInfo[i].destAddr=Utils_memAlloc(chInfo->width*chInfo->height*2, 4);
			copyInfo[i].srcAddr=pFrame->addr[0][0]+chInfo->width*chInfo->height;
			copyInfo[i].width = chInfo->width/2;
			copyInfo[i].height = chInfo->height/2;
		}
		Utils_dmaCopyUV(&pObj->dmaUV, copyInfo, pFrameList->numFrames);
		//copy UV back
		
		for(i=0;i<pFrameList->numFrames;i++)
		{
			pFrame=pFrameList->frames[i];
			chInfo=&(pObj->inQueInfo.chInfo[pFrame->channelNum]);
			Utils_memFree(copyInfo[i].destAddr,chInfo->width*chInfo->height*2);
		}		
	}
}

Int32 AlgLink_algProcessData(Utils_TskHndl *pTsk, AlgLink_Obj * pObj)
{
    UInt32 status;
    System_LinkInQueParams *pInQueParams;

    FVID2_FrameList frameList;

    pInQueParams = &pObj->createArgs.inQueParams;
    System_getLinksFullFrames(pInQueParams->prevLinkId,
                              pInQueParams->prevLinkQueId, &frameList);

    if (frameList.numFrames)
    {
        // do SCD
        if (pObj->createArgs.enableSCDAlg)
        {
            AlgLink_scdAlgProcessFrames(pTsk, &pObj->scdAlg, &frameList);
        }

        // do SW OSD
        if (pObj->createArgs.enableOSDAlg)
        {
            AlgLink_osdAlgProcessFrames(pTsk, &pObj->osdAlg, &frameList);
        }
		
        if (pObj->createArgs.enableUVAlg)
        {
            AlgLink_UVAlgProcessFrames(pTsk, pObj, &frameList);
        }

        status = Utils_bufPutFull(&pObj->framesOutBufQue,
                    &frameList
                    );
        UTILS_assert(status == FVID2_SOK);

        /* Send-out the output bitbuffer */
        System_sendLinkCmd(pObj->createArgs.outQueParams[ALG_LINK_FRAMES_OUT_QUE].nextLink,
                           SYSTEM_CMD_NEW_DATA);

    }

    return FVID2_SOK;
}
