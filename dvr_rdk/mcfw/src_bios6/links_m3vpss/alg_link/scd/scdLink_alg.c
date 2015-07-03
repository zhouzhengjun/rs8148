/*******************************************************************************
 *                                                                             *
 * Copyright (c) 2009 Texas Instruments Incorporated - http://www.ti.com/      *
 *                        ALL RIGHTS RESERVED                                  *
 *                                                                             *
 ******************************************************************************/

#include "scdLink_priv.h"

Int32 AlgLink_scdAlgIsValidCh(AlgLink_ScdObj * pObj, UInt32 chId)
{
    if(chId < ALG_LINK_SIMCOP_SCD_MAX_CH
        &&
       chId < pObj->inQueInfo->numCh
        &&
        pObj->chObj[chId].enableScd==TRUE
    )
    {
        return TRUE;
    }

    return FALSE;
}



Int32 AlgLink_scdAlgResetStatistics(AlgLink_ScdObj * pObj)
{
    AlgLink_ScdChObj *pChObj;
    Int32 chId;

    for(chId=0; chId<ALG_LINK_SIMCOP_SCD_MAX_CH; chId++)
    {
        pChObj = &pObj->chObj[chId];

        pChObj->inFrameRecvCount = 0;
        pChObj->inFrameSkipCount = 0;
        pChObj->inFrameProcessCount = 0;
    }

    pObj->statsStartTime = Utils_getCurTimeInMsec();

    return 0;
}

Int32 AlgLink_scdAlgPrintStatistics(AlgLink_ScdObj * pObj, Bool resetAfterPrint)
{
    UInt32 chId;
    AlgLink_ScdChObj *pChObj;
    UInt32 elaspedTime;

    elaspedTime = Utils_getCurTimeInMsec() - pObj->statsStartTime; // in msecs
    elaspedTime /= 1000; // convert to secs

    Vps_printf( " \n"
            " *** (SIMCOP) SCD Statistics *** \n"
            " \n"
            " Elasped Time           : %d secs\n"
            " Total Fields Processed : %d \n"
            " Total Fields FPS       : %d FPS\n"
            " \n"
            " \n"
            " CH  | In Recv In Skip In Process \n"
            " Num | FPS     FPS     FPS        \n"
            " ---------------------------------\n",
            elaspedTime,
            pObj->processFrameCount,
            pObj->processFrameCount * 100 / (pObj->totalTime / 10)
                    );

    for (chId = 0; chId < pObj->inQueInfo->numCh; chId++)
    {
        pChObj = &pObj->chObj[chId];

        Vps_printf( " %3d | %7d %7d %10d\n",
            chId,
            pChObj->inFrameRecvCount/elaspedTime,
            pChObj->inFrameSkipCount/elaspedTime,
            pChObj->inFrameProcessCount/elaspedTime
            );
    }

    Vps_printf( " \n");

    if(resetAfterPrint)
    {
        AlgLink_scdAlgResetStatistics(pObj);
    }
    return FVID2_SOK;
}

Int32 AlgLink_scdAlgChCreate(AlgLink_ScdObj * pObj)
{
    AlgLink_ScdChObj *pChObj;
    AlgLink_ScdCreateParams *pCreateArgs;
    AlgLink_ScdChParams *pChPrm;
    System_LinkChInfo *pChInfo;
    UInt32 blockId, chId, i;

    pCreateArgs = &pObj->scdCreateParams;

    for(chId=0; chId<ALG_LINK_SIMCOP_SCD_MAX_CH; chId++)
    {
        pChObj = &pObj->chObj[chId];

        memset(pChObj, 0, sizeof(*pChObj));

        pChObj->enableScd = FALSE;
    }

    for(i=0; i<pCreateArgs->numValidChForSCD; i++)
    {
        pChPrm = &pCreateArgs->chDefaultParams[i];

        chId =  pChPrm->chId;

        pChObj = &pObj->chObj[chId];

        pChInfo = &pObj->inQueInfo->chInfo[chId];

        pChObj->enableScd = TRUE;

        pChObj->skipInitialFrames = TRUE;
        pChObj->startTime         = 0;

        pChObj->isTiledMode = FALSE;

        if(pChInfo->memType==SYSTEM_MT_TILEDMEM)
            pChObj->isTiledMode = TRUE;

        pChObj->frameSkipContext.inputFrameRate = pCreateArgs->inputFrameRate;
        pChObj->frameSkipContext.outputFrameRate = pCreateArgs->outputFrameRate;
        pChObj->frameSkipContext.firstTime = TRUE;

        pChObj->chId = pChPrm->chId;

        pChObj->scdStatus     = ALG_LINK_SCD_DETECTOR_UNAVAILABLE;
        pChObj->prevScdStatus = ALG_LINK_SCD_DETECTOR_UNAVAILABLE;

        pChObj->algProcessPrm.chanId                = chId;
        /* need 32 pixel alignment for SCD */
        pChObj->algProcessPrm.width                 = SystemUtils_floor(pChInfo->width, 16);
        pChObj->algProcessPrm.height                = pChInfo->height;
        pChObj->algProcessPrm.pitch                 = pChInfo->pitch[0];
        pChObj->algProcessPrm.mode                  = SCD_DETECTMODE_MONITOR_FULL_FRAME;

        pChObj->algProcessPrm.frmSensitivity        = (SCD_Sensitivity)pChPrm->frmSensitivity;
        pChObj->algProcessPrm.frmIgnoreLightsON     = pChPrm->frmIgnoreLightsON;
        pChObj->algProcessPrm.frmIgnoreLightsOFF    = pChPrm->frmIgnoreLightsOFF;

        pChObj->algProcessPrm.fps                   = SCD_FPS_05;
        pChObj->algProcessPrm.pAlgImageBuf          = &pChObj->algTmpImageBufs;
        pChObj->algProcessPrm.inAddr                = NULL;
        pChObj->algProcessPrm.prevInAddr            = NULL;

        pChObj->algProcessPrm.numSecs2WaitAfterFrmAlert = pCreateArgs->numSecs2WaitAfterFrmAlert;

        pChObj->algInitMeanVarMHIPrm.chanId         = chId;
        pChObj->algInitMeanVarMHIPrm.width          = pChInfo->width;
        pChObj->algInitMeanVarMHIPrm.height         = pChInfo->height;
        pChObj->algInitMeanVarMHIPrm.pitch          = pChInfo->pitch[0];
        pChObj->algInitMeanVarMHIPrm.pAlgImageBuf   = &pChObj->algTmpImageBufs;
        pChObj->algInitMeanVarMHIPrm.inAddr         = NULL;

        for(blockId=0; blockId<pObj->algPerChMemAllocPrm.numMemBlocks; blockId++)
        {

            Vps_printf(" %d: SCD: CH%d: MEM REQUEST %d: of size %d B (align=%d)\n",
                Utils_getCurTimeInMsec(),
                chId,
                blockId,
                pObj->algPerChMemAllocPrm.memBlockSize[blockId],
                pObj->algPerChMemAllocPrm.memBlockAlign[blockId]
                );

            pChObj->memBlockAddr[blockId] =
                    Utils_memAlloc(
                        pObj->algPerChMemAllocPrm.memBlockSize[blockId],
                        pObj->algPerChMemAllocPrm.memBlockAlign[blockId]
                    );
            UTILS_assert(pChObj->memBlockAddr[blockId]!=NULL);

            Vps_printf(" %d: SCD: CH%d: MEM ALLOC %d: @ 0x%08x of size %d B (align=%d)\n",
                Utils_getCurTimeInMsec(),
                chId,
                blockId,
                pChObj->memBlockAddr[blockId],
                pObj->algPerChMemAllocPrm.memBlockSize[blockId],
                pObj->algPerChMemAllocPrm.memBlockAlign[blockId]
                );
        }

        pChObj->algTmpImageBufs.pBkgrdMeanSQ8_7     = pChObj->memBlockAddr[0];
        pChObj->algTmpImageBufs.pBkgrdVarianceSQ12_3= pChObj->memBlockAddr[1];
        pChObj->algTmpImageBufs.pMHIimageUQ8_0      = pChObj->memBlockAddr[2];
        pChObj->algTmpImageBufs.pUpdateMaskMHIUQ8_0 = pChObj->memBlockAddr[3];
    }

    return FVID2_SOK;
}

Int32 AlgLink_scdAlgAllocMem(AlgLink_ScdObj * pObj)
{
    UInt32 blockId;
    Int32 status;

    status = SCD_getAllocInfo(&pObj->algObj, &pObj->algMemAllocPrm, &pObj->algPerChMemAllocPrm);
    UTILS_assert(status==FVID2_SOK);

    for(blockId=0; blockId<pObj->algMemAllocPrm.numMemBlocks; blockId++)
    {

        Vps_printf(" %d: SCD: MEM REQUEST %d: of size %d B (align=%d)\n",
            Utils_getCurTimeInMsec(),
            blockId,
            pObj->algMemAllocPrm.memBlockSize[blockId],
            pObj->algMemAllocPrm.memBlockAlign[blockId]
            );

        pObj->algMemAllocPrm.memBlockAddr[blockId] =
                Utils_memAlloc(
                    pObj->algMemAllocPrm.memBlockSize[blockId],
                    pObj->algMemAllocPrm.memBlockAlign[blockId]
                );
        UTILS_assert(pObj->algMemAllocPrm.memBlockAddr[blockId]!=NULL);

        Vps_printf(" %d: SCD: MEM ALLOC %d: @ 0x%08x of size %d B (align=%d)\n",
            Utils_getCurTimeInMsec(),
            blockId,
            pObj->algMemAllocPrm.memBlockAddr[blockId],
            pObj->algMemAllocPrm.memBlockSize[blockId],
            pObj->algMemAllocPrm.memBlockAlign[blockId]
            );
    }

    status = SCD_setAllocInfo(&pObj->algObj, &pObj->algMemAllocPrm);
    UTILS_assert(status==FVID2_SOK);

    return FVID2_SOK;
}

Int32 AlgLink_scdAlgCreate(AlgLink_ScdObj * pObj)
{
	Int32 status;
    SCD_CreatePrm *pAlgCreatePrm;

    pAlgCreatePrm = &pObj->algCreatePrm;

    AlgLink_scdAlgResetStatistics(pObj);

    pObj->processFrameCount  = 0;
    pObj->totalTime  = 0;

    pAlgCreatePrm->maxWidth  = pObj->scdCreateParams.maxWidth;
    pAlgCreatePrm->maxHeight = pObj->scdCreateParams.maxHeight;
    pAlgCreatePrm->maxPitch  = pObj->scdCreateParams.maxStride;

    Vps_printf(" %d: SCD: Opening algorithm ... !!!\n",
        Utils_getCurTimeInMsec()
        );

    status = SCD_open(&pObj->algObj, pAlgCreatePrm);

    UTILS_assert(status==FVID2_SOK);

    Vps_printf(" %d: SCD: Opening algorithm ... DONE !!!\n",
        Utils_getCurTimeInMsec()
        );

    AlgLink_scdAlgAllocMem(pObj);
    AlgLink_scdAlgChCreate(pObj);

	return FVID2_SOK;
}

Int32 AlgLink_scdAlgFreeMem(AlgLink_ScdObj * pObj)
{
    UInt32 blockId;
    Int32 status;

    for(blockId=0; blockId<pObj->algMemAllocPrm.numMemBlocks; blockId++)
    {
        status = Utils_memFree(
                pObj->algMemAllocPrm.memBlockAddr[blockId],
                pObj->algMemAllocPrm.memBlockSize[blockId]
                );
        UTILS_assert(status==FVID2_SOK);
    }

    return FVID2_SOK;
}

Int32 AlgLink_scdAlgChDelete(AlgLink_ScdObj * pObj)
{
    AlgLink_ScdChObj *pChObj;
    UInt32 blockId, chId;
    Int32 status;

    for(chId=0; chId<ALG_LINK_SIMCOP_SCD_MAX_CH; chId++)
    {
        pChObj = &pObj->chObj[chId];

        if(pChObj->enableScd==FALSE)
            continue;

        for(blockId=0; blockId<pObj->algPerChMemAllocPrm.numMemBlocks; blockId++)
        {
            status = Utils_memFree(
                    pChObj->memBlockAddr[blockId],
                    pObj->algPerChMemAllocPrm.memBlockSize[blockId]
                    );
            UTILS_assert(status==FVID2_SOK);
        }
    }

    return FVID2_SOK;
}

Int32 AlgLink_scdAlgDelete(AlgLink_ScdObj * pObj)
{
    Int32 status;

    status = SCD_close(&pObj->algObj);
    UTILS_assert(status==FVID2_SOK);

    AlgLink_scdAlgChDelete(pObj);
    AlgLink_scdAlgFreeMem(pObj);

	return FVID2_SOK;
}

Int32 AlgLink_scdAlgRtPrmUpdate(AlgLink_ScdObj * pObj, AlgLink_ScdChObj *pChObj, FVID2_Frame *pFrame)
{
    System_FrameInfo *pFrameInfo;
    System_LinkChInfo *pChInfo;

    pFrameInfo = (System_FrameInfo *)pFrame->appData;

    if(pFrameInfo==NULL)
        return FVID2_EFAIL;

    if(pFrameInfo->rtChInfoUpdate==FALSE)
        return FVID2_SOK;

    pChInfo = &pFrameInfo->rtChInfo;

    pChObj->algProcessPrm.width                 = pChInfo->width;
    pChObj->algProcessPrm.height                = pChInfo->height;
    pChObj->algProcessPrm.pitch                 = pChInfo->pitch[0];
    pChObj->algProcessPrm.inAddr                = NULL;
    pChObj->algProcessPrm.prevInAddr            = NULL;

    pChObj->algInitMeanVarMHIPrm.width          = pChInfo->width;
    pChObj->algInitMeanVarMHIPrm.height         = pChInfo->height;
    pChObj->algInitMeanVarMHIPrm.pitch          = pChInfo->pitch[0];
    pChObj->algInitMeanVarMHIPrm.inAddr         = NULL;

    pChObj->isTiledMode = FALSE;

    if(pChInfo->memType==SYSTEM_MT_TILEDMEM)
        pChObj->isTiledMode = TRUE;

    return FVID2_SOK;
}

Int32 AlgLink_scdAlgDoTmpFrameInit(AlgLink_ScdObj * pObj, AlgLink_ScdChObj *pChObj, FVID2_Frame *pFrame)
{
    Int32 status = FVID2_SOK;
    UInt32 curTime;

    if(pChObj->algInitMeanVarMHIPrm.inAddr==NULL)
    {
        if(pChObj->isTiledMode)
        {
            pChObj->algInitMeanVarMHIPrm.inAddr = (Ptr)Utils_tilerAddr2CpuAddr( (UInt32)pFrame->addr[0][0] );
        }
        else
        {
            pChObj->algInitMeanVarMHIPrm.inAddr = pFrame->addr[0][0];
        }

        #ifdef SYSTEM_DEBUG_SCD_RT
        Vps_rprintf(" %d: SCD: CH%d: SCD_initMeanVarMHI() ... !!!\n",
                Utils_getCurTimeInMsec(),
                pFrame->channelNum
                );
        #endif

        curTime = Utils_getCurTimeInMsec();

        /* first frame  */
        status = SCD_initMeanVarMHI(&pObj->algObj, &pChObj->algInitMeanVarMHIPrm);

        curTime = Utils_getCurTimeInMsec() - curTime;

        pObj->totalTime += curTime;

        #ifdef SYSTEM_DEBUG_SCD_RT
        Vps_rprintf(" %d: SCD: CH%d: SCD_initMeanVarMHI() ... DONE !!!\n",
                Utils_getCurTimeInMsec(),
                pFrame->channelNum
                );
        #endif

        if(status!=SCD_NO_ERROR)
        {
            Vps_printf(" %d: SCD: CH%d: ERROR (%d) in SCD_initMeanVarMHI() !!!\n",
                Utils_getCurTimeInMsec(),
                pFrame->channelNum,
                status
                );
        }
    }

    return status;
}

Int32 AlgLink_scdAlgDoProcessFrame(AlgLink_ScdObj * pObj, AlgLink_ScdChObj *pChObj, FVID2_Frame *pFrame)
{
    Int32 status = FVID2_SOK;
    UInt32 curTime;
    Bool statusChangeDetected = FALSE;

    if(pChObj->isTiledMode)
    {
        pChObj->algProcessPrm.inAddr = (Ptr)Utils_tilerAddr2CpuAddr( (UInt32)pFrame->addr[0][0] );
    }
    else
    {
        pChObj->algProcessPrm.inAddr = pFrame->addr[0][0];
    }

    if(pChObj->algProcessPrm.prevInAddr==NULL)
        pChObj->algProcessPrm.prevInAddr = pChObj->algProcessPrm.inAddr;

    #ifdef SYSTEM_DEBUG_SCD_RT
    Vps_rprintf(" %d: SCD: CH%d: SCD_process() ... !!!\n",
            Utils_getCurTimeInMsec(),
            pFrame->channelNum
            );
    #endif

    curTime = Utils_getCurTimeInMsec();

    status = SCD_process(&pObj->algObj, &pChObj->algProcessPrm, &pChObj->algProcessStatus);

    curTime = Utils_getCurTimeInMsec() - curTime;

    #ifdef SYSTEM_DEBUG_SCD_RT
    Vps_rprintf(" %d: SCD: CH%d: SCD_process() ... DONE !!!\n",
            Utils_getCurTimeInMsec(),
            pFrame->channelNum
            );
    #endif

    if(pChObj->algProcessStatus.status!=SCD_NO_ERROR)
    {
        Vps_printf(" %d: SCD: CH%d: ERROR (%d) in SCD_process() !!!\n",
            Utils_getCurTimeInMsec(),
            pFrame->channelNum,
            status
            );
    }

    pChObj->scdStatus = ALG_LINK_SCD_DETECTOR_UNAVAILABLE;

    if(pChObj->algProcessStatus.output==SCD_DETECTOR_NO_CHANGE)
        pChObj->scdStatus = ALG_LINK_SCD_DETECTOR_NO_CHANGE;
    else
    if(pChObj->algProcessStatus.output==SCD_DETECTOR_CHANGE)
        pChObj->scdStatus = ALG_LINK_SCD_DETECTOR_CHANGE;


    if(pChObj->scdStatus==ALG_LINK_SCD_DETECTOR_CHANGE)
    {
        if(pChObj->prevScdStatus!=ALG_LINK_SCD_DETECTOR_CHANGE)
        {
            statusChangeDetected = TRUE;

            Vps_rprintf(" %d: SCD: CH%d: Tamper Detected !!!\n",
                Utils_getCurTimeInMsec(),
                pFrame->channelNum
                );

        }
    }
    if(pChObj->scdStatus==ALG_LINK_SCD_DETECTOR_NO_CHANGE)
    {
        if(pChObj->prevScdStatus!=ALG_LINK_SCD_DETECTOR_NO_CHANGE)
        {
            statusChangeDetected = TRUE;

            Vps_rprintf(" %d: SCD: CH%d: NO Tamper Detected !!!\n",
                Utils_getCurTimeInMsec(),
                pFrame->channelNum
                );
        }
    }

    pChObj->prevScdStatus = pChObj->scdStatus;

    pObj->totalTime += curTime;

    pChObj->algProcessPrm.prevInAddr = pChObj->algProcessPrm.inAddr;


    if(statusChangeDetected)
    {
        if(pObj->scdCreateParams.enableTamperNotify)
        {
            AlgLink_ScdChStatus chStatus;

            chStatus.chId = pChObj->chId;
            chStatus.frmResult = pChObj->scdStatus;

            System_linkControl(SYSTEM_LINK_ID_HOST, VSYS_EVENT_TAMPER_DETECT, &chStatus, sizeof(AlgLink_ScdChStatus), FALSE);
        }
    }

    return status;
}

Bool AlgLink_scdAlgSkipInitialFrames(AlgLink_ScdObj * pObj, AlgLink_ScdChObj *pChObj)
{
    if(pChObj->skipInitialFrames)
    {
        UInt32 elaspedTime;

        if(pChObj->startTime==0)
        {
            pChObj->startTime = Utils_getCurTimeInMsec();
        }

        elaspedTime = Utils_getCurTimeInMsec() - pChObj->startTime;

        if(elaspedTime >= pObj->scdCreateParams.numSecs2WaitB4Init*1000)
            pChObj->skipInitialFrames = FALSE;
    }

    return pChObj->skipInitialFrames;
}

Int32 AlgLink_scdAlgProcessFrames(Utils_TskHndl *pTsk, AlgLink_ScdObj * pObj, FVID2_FrameList *pFrameList)
{
    Int32 status = FVID2_SOK;
    UInt32 frameId;
    FVID2_Frame *pFrame;
    AlgLink_ScdChObj *pChObj;
    Bool skipFrame;

    if(pObj->processFrameCount==0)
    {
        AlgLink_scdAlgResetStatistics(pObj);
    }

    for(frameId=0; frameId<pFrameList->numFrames; frameId++)
    {
        pFrame = pFrameList->frames[frameId];

        if(pFrame==NULL)
            continue;

        if(!AlgLink_scdAlgIsValidCh(pObj, pFrame->channelNum))
        {
            continue;
        }

        pChObj = &pObj->chObj[pFrame->channelNum];

        pChObj->inFrameRecvCount++;

        skipFrame = Utils_doSkipFrame(&pChObj->frameSkipContext);

        if(skipFrame)
        {
            pChObj->inFrameSkipCount++;
            continue;
        }

        pChObj->inFrameProcessCount++;
        pObj->processFrameCount++;

        #if 0
        AlgLink_scdAlgRtPrmUpdate(pObj, pChObj, pFrame);
        #endif

        if(AlgLink_scdAlgSkipInitialFrames(pObj, pChObj))
            continue;

        status = AlgLink_scdAlgDoTmpFrameInit(pObj, pChObj, pFrame);
        if(status!=FVID2_SOK)
            continue;

        AlgLink_scdAlgDoProcessFrame(pObj, pChObj, pFrame);
    }

    return status;
}


Int32 AlgLink_scdAlgGetAllChFrameStatus(AlgLink_ScdObj * pObj, AlgLink_ScdAllChFrameStatus *pPrm)
{
    AlgLink_ScdChObj *pChObj;
    UInt32 chId;

    pPrm->numCh = 0;

    for(chId=0; chId<ALG_LINK_SIMCOP_SCD_MAX_CH; chId++)
    {
        pChObj = &pObj->chObj[chId];

        if(pChObj->enableScd==TRUE)
        {
            pPrm->chanFrameResult[pPrm->numCh].chId = pChObj->chId;
            pPrm->chanFrameResult[pPrm->numCh].frmResult = pChObj->scdStatus;

            pPrm->numCh++;

            if(pPrm->numCh >= ALG_LINK_SCD_MAX_CH)
            {
                break;
            }
        }
    }

    return FVID2_SOK;
}
