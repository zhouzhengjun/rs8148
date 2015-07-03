/*******************************************************************************
 *                                                                             *
 * Copyright (c) 2009 Texas Instruments Incorporated - http://www.ti.com/      *
 *                        ALL RIGHTS RESERVED                                  *
 *                                                                             *
 ******************************************************************************/
#include <ti/sysbios/hal/Cache.h>
#include "scdLink_priv.h"
#include "scd_ti.h"
#include "ti/sdo/fc/dskt2/dskt2.h"
#include "ti/sdo/fc/rman/rman.h"
#include <mcfw/interfaces/common_def/ti_vsys_common_def.h>
#include <mcfw/src_bios6/utils/utils_mem.h>

static UInt8 gScratchId = 1;
static Int32 AlgLink_ScdalgSetChScdPrm(AlgLink_ScdchPrm    *pScdChPrm,
                            AlgLink_ScdChParams * params);

Int32 AlgLink_ScdalgCreate(AlgLink_ScdObj * pObj)
{
    Int32               status, chId, scdChId;
    SCD_createPrm       algCreatePrm;
    SCD_chPrm           chDefaultParams[16];
    AlgLink_ScdChParams *pChLinkPrm;
    AlgLink_ScdchPrm    *pScdChPrm;
    IALG_Fxns           *algFxns = (IALG_Fxns *)&SCD_TI;
  //  IRES_Fxns *resFxns = &SCD_TI_IRES;

#ifdef SYSTEM_DEBUG_SCD
    Vps_printf(" %d: SCD    : Create in progress !!!\n",
               Utils_getCurTimeInMsec());
#endif

    pObj->totalFrameCount = 0;

    if(pObj->createArgs.numBufPerCh == 0)
        pObj->createArgs.numBufPerCh = ALG_LINK_SCD_MAX_OUT_FRAMES_PER_CH;

    if(pObj->createArgs.numBufPerCh > ALG_LINK_SCD_MAX_OUT_FRAMES_PER_CH)
    {
        Vps_printf("\n SCDLINK: WARNING: User is asking for %d buffers per CH. But max allowed is %d. \n"
            " Over riding user requested with max allowed \n\n",
            pObj->createArgs.numBufPerCh, ALG_LINK_SCD_MAX_OUT_FRAMES_PER_CH
            );

        pObj->createArgs.numBufPerCh = ALG_LINK_SCD_MAX_OUT_FRAMES_PER_CH;

    }



#ifdef SYSTEM_DEBUG_SCD
    Vps_printf(" %d: SCD    : Max WxH = %d x %d, Max CH = %d, FPS = %d !!!\n",
           Utils_getCurTimeInMsec(),
            pObj->createArgs.maxWidth,
            pObj->createArgs.maxHeight,
            //pObj->createArgs.startChNoForSCD,
            pObj->createArgs.numValidChForSCD,
            pObj->createArgs.outputFrameRate
        );
#endif

    if( pObj->createArgs.numValidChForSCD > pObj->inQueInfo->numCh)
    {
#ifdef SYSTEM_DEBUG_SCD
        Vps_printf(" %d: SCD    : Create ERROR - SCD channels < InQueue Channels !!!\n",
               Utils_getCurTimeInMsec());
#endif

        return FVID2_EFAIL;
    }

    algCreatePrm.maxWidth    = pObj->createArgs.maxWidth;
    algCreatePrm.maxHeight   = pObj->createArgs.maxHeight;
    algCreatePrm.maxStride   = pObj->createArgs.maxStride;
    algCreatePrm.maxChannels = pObj->createArgs.numValidChForSCD;
    algCreatePrm.numSecs2WaitB4Init		= pObj->createArgs.numSecs2WaitB4Init;
    algCreatePrm.numSecs2WaitB4FrmAlert = pObj->createArgs.numSecs2WaitB4FrmAlert;
    algCreatePrm.numSecs2WaitAfterFrmAlert = pObj->createArgs.numSecs2WaitAfterFrmAlert;
    algCreatePrm.fps                    = (SCD_Fps) pObj->createArgs.outputFrameRate;
    algCreatePrm.chDefaultParams        = (SCD_chPrm *)&chDefaultParams[0];

    for(chId=0; chId < algCreatePrm.maxChannels; chId++)
    {
        SCD_chPrm			* chl	= &(algCreatePrm.chDefaultParams[chId]);
        AlgLink_ScdChParams	* chPrm = &(pObj->createArgs.chDefaultParams[chId]);

        if ((chPrm->mode == SCD_DETECTMODE_MONITOR_BLOCKS) ||
             (chPrm->mode == SCD_DETECTMODE_MONITOR_BLOCKS_FRAME))
        {
          Vps_printf("Setting block configuration data \n");
          chl->blkConfig = (SCD_blkChngConfig *)chPrm->blkConfig;
    /*
          for (blk=0; blk < chPrm->blkNumBlksInFrame; blk++)
          {
            chl->blkConfig[blk].sensitivity = (SCD_Sensitivity) chPrm->blkConfig[blk].sensitivity;
            chl->blkConfig[blk].monitored	= (UInt32) chPrm->blkConfig[blk].monitored;
          }
    */
        }
        else
        {
          chl->blkConfig = NULL;
        }

        // The remaining parameter values filled in here do not really matter as
        // they will be over-written by calls to SCD_TI_setPrms. We'll fill in
        // just a few
        chl->chId	= chPrm->chId;
        chl->mode	= (SCD_Mode)chPrm->mode;
        chl->width	= pObj->createArgs.maxWidth;
        chl->height = pObj->createArgs.maxHeight;
        chl->stride = pObj->createArgs.maxStride;
        chl->curFrame = NULL;
        chl->frmSensitivity = (SCD_Sensitivity)chPrm->frmSensitivity;
        chl->frmIgnoreLightsON = chPrm->frmIgnoreLightsON;
        chl->frmIgnoreLightsOFF = chPrm->frmIgnoreLightsOFF;
        chl->frmEdgeThreshold   = chPrm->frmEdgeThreshold;
    }

    /* Create algorithm instance and get algo handle  */
    pObj->algHndl = DSKT2_createAlg((Int)gScratchId,
            (IALG_Fxns *)algFxns, NULL,(IALG_Params *)&algCreatePrm);

    if(pObj->algHndl == NULL)
    {
        #ifdef SYSTEM_DEBUG_SCD
        Vps_printf(" %d: SCD    : Create ERROR !!!\n",
               Utils_getCurTimeInMsec());
        #endif

        return FVID2_EFAIL;
    }
#if 0
    /* Assign resources to the algorithm */
    status = RMAN_assignResources((IALG_Handle)pObj->algHndl, resFxns, gScratchId);
    if (status != IRES_OK) {
        return FVID2_EFAIL;
    }
#endif
    for(scdChId = 0;  scdChId<pObj->createArgs.numValidChForSCD; scdChId++)
    {
        pScdChPrm = &pObj->chParams[scdChId];

        pChLinkPrm = &pObj->createArgs.chDefaultParams[scdChId];

        pObj->chObj[scdChId].frameSkipCtx.firstTime       = TRUE;
        pObj->chObj[scdChId].frameSkipCtx.inputFrameRate  = pObj->createArgs.inputFrameRate;
        pObj->chObj[scdChId].frameSkipCtx.outputFrameRate = pObj->createArgs.outputFrameRate;
        pObj->chObj[scdChId].scdChStat                    = ALG_LINK_SCD_DETECTOR_UNAVAILABLE;
        pObj->chObj[scdChId].chId                         = pChLinkPrm->chId;


        pScdChPrm->chId = pChLinkPrm->chId;
        pScdChPrm->chBlkConfigUpdate = FALSE;


        status = AlgLink_ScdalgSetChScdPrm(pScdChPrm, pChLinkPrm);
        UTILS_assert(status==0);

        pScdChPrm->width  = pObj->inQueInfo->chInfo[pScdChPrm->chId].width +
            pObj->inQueInfo->chInfo[pScdChPrm->chId].startX;
        pScdChPrm->height = pObj->inQueInfo->chInfo[pScdChPrm->chId].height +
            pObj->inQueInfo->chInfo[pScdChPrm->chId].startY;
        pScdChPrm->stride = pObj->inQueInfo->chInfo[pScdChPrm->chId].pitch[0];

#ifdef SYSTEM_DEBUG_SCD
        Vps_printf(" %d: SCD    : %d: %d x %d, In FPS = %d, Out FPS = %d!!!\n",
                   Utils_getCurTimeInMsec(),
                    pScdChPrm->chId,
                    pScdChPrm->width,
                    pScdChPrm->height,
                    pObj->chObj[scdChId].frameSkipCtx.inputFrameRate,
                    pObj->chObj[scdChId].frameSkipCtx.outputFrameRate
            );
#endif

    }
    AlgLink_ScdresetStatistics(pObj);


#ifdef SYSTEM_DEBUG_SCD
    Vps_printf(" %d: SCD    : Create Done !!!\n",
               Utils_getCurTimeInMsec());
#endif

 return FVID2_SOK;
}

Int32 AlgLink_ScdalgDelete(AlgLink_ScdObj * pObj)
{
    Int32 scratchId = gScratchId;
    //IRES_Status status;
    //IRES_Fxns * resFxns = &SCD_TI_IRES;

#ifdef SYSTEM_DEBUG_SCD
    Vps_printf(" %d: SCD    : Delete in progress !!!\n",
               Utils_getCurTimeInMsec());
#endif

    if(pObj->algHndl == NULL)
        return FVID2_EFAIL;

   /*
    * Deactivate All Resources

    RMAN_deactivateAllResources((IALG_Handle)pObj->algHndl, resFxns, scratchId);
    */
    /* Deactivate algorithm */
    DSKT2_deactivateAlg(scratchId, (IALG_Handle)pObj->algHndl);

    /*
    * Free resources assigned to this algorihtm

    status = RMAN_freeResources((IALG_Handle)pObj->algHndl, resFxns, scratchId);

    if (IRES_OK != status) {
     return SWOSD_EFAIL;
    }
   */

    DSKT2_freeAlg(scratchId, (IALG_Handle)pObj->algHndl);

#ifdef SYSTEM_DEBUG_SCD
    Vps_printf(" %d: SCD    : Delete Done !!!\n",
               Utils_getCurTimeInMsec());
#endif

    return FVID2_SOK;
}

Int32 AlgLink_ScdalgProcess(AlgLink_ScdObj *pScdAlgLinkObj, UInt32 chId, AlgLink_ScdResult * scdResultBuff)
{
    SCD_Status      chanStatus;
    SCD_Result      scdResult;
    UInt32          chanID;
    SCD_chPrm       chanParam;
    AlgLink_ScdchPrm * chPrm;
    AlgLink_ScdChObj * chObj;
    UInt32         blkIdx;

    /* Activate the Algorithm */
    DSKT2_activateAlg(gScratchId, (IALG_Handle)pScdAlgLinkObj->algHndl);

    chanID = pScdAlgLinkObj->chParams[chId].chId;
    chPrm = &(pScdAlgLinkObj->chParams[chId]);
    chObj = &(pScdAlgLinkObj->chObj[chId]);

    chanParam.chId	                = chPrm->chId;
    chanParam.mode	                = (SCD_Mode)chPrm->mode;
    chanParam.width	              = chPrm->width;
    chanParam.height               = chPrm->height;
    chanParam.stride               = chPrm->stride;
    chanParam.curFrame             = chPrm->curFrame;
    chanParam.frmSensitivity       = (SCD_Sensitivity)chPrm->frmSensitivity;
    chanParam.frmIgnoreLightsON    = chPrm->frmIgnoreLightsON;
    chanParam.frmIgnoreLightsOFF   = chPrm->frmIgnoreLightsOFF;
    chanParam.frmEdgeThreshold     = chPrm->frmEdgeThreshold;

    if(chPrm->chBlkConfigUpdate == TRUE)
    {
        chanParam.blkConfig = (SCD_blkChngConfig *) chPrm->blkConfig;
    }
    else
    {
        chanParam.blkConfig = NULL;
    }
    chanStatus = SCD_TI_setPrms(pScdAlgLinkObj->algHndl, &chanParam, chanID);

    if(chanStatus != SCD_NO_ERROR)
    {
#ifdef SYSTEM_DEBUG_SCD
        Vps_printf(" %d: SCD    : ERROR: Alg Set Params (chanID = %d) - 0x%08X !!!\n",
                  Utils_getCurTimeInMsec(), chanID, chanStatus);
#endif
        /* Deactivate algorithm */
        DSKT2_deactivateAlg(gScratchId, (IALG_Handle)pScdAlgLinkObj->algHndl);
        return FVID2_EFAIL;
    }


    scdResult.frmResult = SCD_DETECTOR_NO_TAMPER;
    scdResult.blkResult = (SCD_blkChngMeta *)(scdResultBuff->blkResult);

    chanStatus = SCD_TI_process(pScdAlgLinkObj->algHndl, chanID, &scdResult);


    /* Deactivate algorithm */
    DSKT2_deactivateAlg(gScratchId, (IALG_Handle)pScdAlgLinkObj->algHndl);

    if(chanStatus != SCD_NO_ERROR)
    {
#ifdef SYSTEM_DEBUG_SCD
        Vps_printf(" %d: SCD    : ERROR: Alg Process (chanID = %d) !!!\n",
                  Utils_getCurTimeInMsec(), chanID );
#endif

        return FVID2_EFAIL;
    }

    scdResultBuff->frmResult = (AlgLink_ScdOutput) scdResult.frmResult;
    scdResultBuff->chId = chanID;

//    if(pScdAlgLinkObj->createArgs.chDefaultParams[chId].mode & SCD_DETECTMODE_MONITOR_BLOCKS)

    /* Motion detect event notification. */
    if(pScdAlgLinkObj->chParams[chId].mode & SCD_DETECTMODE_MONITOR_BLOCKS)
    {
        UInt32 numBlkChg;
        UInt32 monitoredBlk;
        UInt32 numHorzBlks, numVertBlks, numBlksInFrame, blkHeight;

        monitoredBlk = 0;
        numBlkChg = 0;
        numHorzBlks     = ((pScdAlgLinkObj->chParams[chId].width + 0x1F ) & (~0x1F)) / 32;  /* Rounding to make divisible by 32 */
        if((pScdAlgLinkObj->chParams[chId].height%ALG_LINK_SCD_BLK_HEIGHT_MIN) == 0)/* For Block height is divisible by 10 */
           blkHeight = ALG_LINK_SCD_BLK_HEIGHT_MIN;
        else   /* For Block height is divisible by 12 */
           blkHeight = ALG_LINK_SCD_BLK_HEIGHT;
        numVertBlks    = pScdAlgLinkObj->chParams[chId].height / blkHeight;

        numBlksInFrame = numHorzBlks * numVertBlks;

        /* Logic  to see how many blocks of total enabled blocks experienced change.
         * For each block, algorithm returns no. of pixels changed in the current
         * frame. This is compared against thresold determined using SCD sensitivity .
         * if changed pixels are more than the calculated thresold, block is marked as changed
         * i.e. motion is detected in the block.
         * Minimum value of thresold is 20% and then it is incrmented by 10% for
         * each sensitivity level change. Thresold can vary from 20% - 100%
         * At max sensitivity, thresold would be 20%. That means if 20% pixels
         * are changed block is marked as changed.
         * At minimu sensitivity, thresold would be 100%. That means if 100% pixels
         * are changed block is marked as changed */

        for(blkIdx = 0; blkIdx < numBlksInFrame; blkIdx++)
        {
            SCD_blkChngConfig * blockConfig;

            blockConfig = &pScdAlgLinkObj->chParams[chId].blkConfig[blkIdx];
            scdResultBuff->blkConfig[blkIdx].monitored   = blockConfig->monitored;
            scdResultBuff->blkConfig[blkIdx].sensitivity = blockConfig->sensitivity;
            if(blockConfig->monitored == 1)
            {
                UInt32 threshold;

                monitoredBlk++;
                threshold = MOTION_DETECTION_SENSITIVITY(ALG_LINK_SCD_BLK_WIDTH, blkHeight) +
                                  (MOTION_DETECTION_SENSITIVITY_STEP * (SCD_SENSITIVITY_MAX - blockConfig->sensitivity));

                if(scdResultBuff->blkResult[blkIdx].numPixelsChanged > threshold)
                {
                    numBlkChg++;
                }
            }
        }

        /* Logic  to notify A8-host about motion detection.
         * Currently, if atleast 1 block is detected as changed A8 is notified.
         * User can use commented logic to chnage this behavior. */
        /* if((monitoredBlk > 0) && (numBlkChg > (NUM_BLOCK_MOTION_DETECTION_THRESOLD(monitoredBlk)))) */

        if(pScdAlgLinkObj->createArgs.enableMotionNotify)
        {
           if((monitoredBlk > 0) && (numBlkChg > 0))
           {
#ifdef SYSTEM_DEBUG_SCD_RT
               Vps_printf(" %d: SCD    : Motion Detected (chanID = %d),  !!!\n",
                      Utils_getCurTimeInMsec(), chanID );
#endif
               System_linkControl(SYSTEM_LINK_ID_HOST, VSYS_EVENT_MOTION_DETECT, scdResultBuff, sizeof(AlgLink_ScdResult), FALSE);

           }
        }

    }

    chObj->scdChStat = ALG_LINK_SCD_DETECTOR_UNAVAILABLE;

    if(scdResult.frmResult == SCD_DETECTOR_NO_TAMPER)
        chObj->scdChStat = ALG_LINK_SCD_DETECTOR_NO_CHANGE;
    else if(scdResult.frmResult == SCD_DETECTOR_TAMPER)
        chObj->scdChStat = ALG_LINK_SCD_DETECTOR_CHANGE;


    if(pScdAlgLinkObj->createArgs.enableTamperNotify)
    {

    /* Tamper detect event notification. */
        if(scdResult.frmResult == SCD_DETECTOR_TAMPER)
        {
            AlgLink_ScdChStatus pChStatus;

            pChStatus.frmResult = (UInt32) ALG_LINK_SCD_DETECTOR_CHANGE;
            pChStatus.chId      = chObj->chId;


#ifdef SYSTEM_DEBUG_SCD_RT
            Vps_printf(" %d: SCD    : Tamper Detected (chanID = %d) !!!\n",
                      Utils_getCurTimeInMsec(), chanID );
#endif

            System_linkControl(SYSTEM_LINK_ID_HOST, VSYS_EVENT_TAMPER_DETECT, &pChStatus, sizeof(AlgLink_ScdChStatus), FALSE);
        }
    }

    return FVID2_SOK;
}

Int32 AlgLink_ScdalgProcessData(AlgLink_ScdObj * pObj, FVID2_FrameList *frameList, Utils_BitBufHndl *bufOutQue)
{
    UInt32 frameId, curTime;
    FVID2_Frame *pFrame;
    Int32 chIdx, status;
    Int32 frameFound;
    Bitstream_Buf *pOutBuf;
    Bitstream_BufList outBitBufList;
    System_FrameInfo *pInFrameInfo;

    outBitBufList.numBufs = 0;
    outBitBufList.appData = NULL;

    AlgLink_ScdChObj *pChObj;

    pObj->totalFrameCount += frameList->numFrames;

    status = FVID2_EFAIL;
    if (frameList->numFrames)
    {
        for(frameId=0; frameId< frameList->numFrames; frameId++)
        {
            Bool doFrameDrop;
            pFrame = frameList->frames[frameId];
            //chIdx = pFrame->channelNum - pObj->createArgs.startChNoForSCD;

            frameFound = 0;
            for(chIdx = 0; chIdx < pObj->createArgs.numValidChForSCD; chIdx++)
            {
                if(pFrame->channelNum == pObj->createArgs.chDefaultParams[chIdx].chId)
                {
                  frameFound = 1;
                  break;
                }
            }

            if (frameFound == 0)
              continue;

            pChObj = &pObj->chObj[chIdx];

            pChObj->inFrameRecvCount++;
            doFrameDrop = Utils_doSkipFrame(&pChObj->frameSkipCtx);
            if(doFrameDrop == FALSE)
            {
                pOutBuf = NULL;
                status = Utils_bitbufGetEmptyBuf(bufOutQue,
                                                 &pOutBuf,
                                                 0, //pObj->outObj.ch2poolMap[chIdx], /*Need to change later.*/
                                                 BIOS_NO_WAIT);

                if(!((status == FVID2_SOK) && (pOutBuf)))
                {
                   doFrameDrop = TRUE;
                }
            }
            if(doFrameDrop == FALSE)
            {
                pObj->chParams[chIdx].curFrame = pFrame->addr[0][0];

                pInFrameInfo = (System_FrameInfo *) pFrame->appData;

                pOutBuf->lowerTimeStamp = (UInt32)(pInFrameInfo->ts64 & 0xFFFFFFFF);
                pOutBuf->upperTimeStamp = (UInt32)(pInFrameInfo->ts64 >> 32);
                pOutBuf->channelNum = pFrame->channelNum;
                pOutBuf->fillLength = sizeof(AlgLink_ScdResult);
                pOutBuf->frameWidth = pObj->chParams[chIdx].width;
                pOutBuf->frameHeight = pObj->chParams[chIdx].height;

                curTime = Utils_getCurTimeInMsec();

//                AlgLink_ScdalgProcess(pObj, pFrame->channelNum, (AlgLink_ScdResult *) pOutBuf->addr);
                AlgLink_ScdalgProcess(pObj, chIdx, (AlgLink_ScdResult *) pOutBuf->addr);
                Cache_wb(pOutBuf->addr, sizeof(AlgLink_ScdResult), Cache_Type_ALL,TRUE);
                pObj->chParams[chIdx].chBlkConfigUpdate = FALSE;

                pChObj->inFrameProcessTime += (Utils_getCurTimeInMsec() - curTime);
                pChObj->inFrameProcessCount++;
                outBitBufList.bufs[outBitBufList.numBufs] = pOutBuf;
                outBitBufList.numBufs++;

            }
            else
            {
                pChObj->inFrameUserSkipCount++;

            }
        }
    }

    if (outBitBufList.numBufs)
    {
        status = Utils_bitbufPutFull(bufOutQue,
                                     &outBitBufList);
        UTILS_assert(status == FVID2_SOK);
        status = FVID2_SOK;
    }
    else
    {
        status = FVID2_EFAIL;
    }

    return status;
}


Int32 AlgLink_ScdalgSetChblkUpdate(AlgLink_ScdObj * pObj,
                            AlgLink_ScdChblkUpdate * params)
{
    AlgLink_ScdchPrm *pScdChPrm;
    Int32 channelUpdated, idx;

    channelUpdated = -1;
    // Search through ID list to get the proper channel no.

    pObj->chParams[params->chId].chBlkConfigUpdate = TRUE;

    pScdChPrm = &pObj->chParams[params->chId];
    for(idx = 0; idx < params->numValidBlock; idx++)
    {
        SCD_blkChngConfig * blkConfig;
        blkConfig = (SCD_blkChngConfig *) ((UInt32)(pScdChPrm->blkConfig) + \
                       (sizeof(SCD_blkChngConfig) * params->blkConfig[idx].blockId));

        blkConfig->monitored = params->blkConfig[idx].monitorBlock;
        blkConfig->sensitivity = (SCD_Sensitivity) params->blkConfig[idx].sensitivity;

        channelUpdated = 0;
    }

    return (channelUpdated);
}

Int32 AlgLink_ScdalgSetChScdSensitivity(AlgLink_ScdObj * pObj,
                            AlgLink_ScdChParams * params)
{
    AlgLink_ScdchPrm *pScdChPrm;
    Int32 channelUpdated, idx;

    channelUpdated = -1;
    // Search through ID list to get the proper channel no.
    for(idx = 0; idx < pObj->createArgs.numValidChForSCD; idx++)
    {
      pScdChPrm = &pObj->chParams[idx];
      if(params->chId == pScdChPrm->chId)
      {
        pScdChPrm->frmSensitivity  = (SCD_Sensitivity) params->frmSensitivity;
        channelUpdated = 0;
        break;
      }
    }

    return (channelUpdated);
}

Int32 AlgLink_ScdalgSetChScdMode(AlgLink_ScdObj * pObj,
                            AlgLink_ScdChParams * params)
{
    AlgLink_ScdchPrm *pScdChPrm;
    Int32 channelUpdated, idx;

    channelUpdated = -1;
    // Search through ID list to get the proper channel no.
    for(idx = 0; idx < pObj->createArgs.numValidChForSCD; idx++)
    {
      pScdChPrm = &pObj->chParams[idx];
      if(params->chId == pScdChPrm->chId)
      {
        pScdChPrm->mode = (SCD_Mode) params->mode;
        channelUpdated = 0;
        break;
      }
    }

    return (channelUpdated);
}

Int32 AlgLink_ScdalgSetChScdIgnoreLightsOn(AlgLink_ScdObj * pObj,
                            AlgLink_ScdChParams * params)
{
    AlgLink_ScdchPrm *pScdChPrm;
    Int32 channelUpdated, idx;

    channelUpdated = -1;
    // Search through ID list to get the proper channel no.
    for(idx = 0; idx < pObj->createArgs.numValidChForSCD; idx++)
    {
      pScdChPrm = &pObj->chParams[idx];
      if(params->chId == pScdChPrm->chId)
      {
        pScdChPrm->frmIgnoreLightsON = params->frmIgnoreLightsON;
        channelUpdated = 0;
        break;
      }
    }

    return (channelUpdated);
}

Int32 AlgLink_ScdalgSetChScdIgnoreLightsOff(AlgLink_ScdObj * pObj,
                            AlgLink_ScdChParams * params)
{
    AlgLink_ScdchPrm *pScdChPrm;
    Int32 channelUpdated, idx;

    channelUpdated = -1;
    // Search through ID list to get the proper channel no.
    for(idx = 0; idx < pObj->createArgs.numValidChForSCD; idx++)
    {
      pScdChPrm = &pObj->chParams[idx];
      if(params->chId == pScdChPrm->chId)
      {
        pScdChPrm->frmIgnoreLightsOFF = params->frmIgnoreLightsOFF;
        channelUpdated = 0;
        break;
      }
    }

    return (channelUpdated);
}

static Int32 AlgLink_ScdalgSetChScdPrm(AlgLink_ScdchPrm    *pScdChPrm,
                            AlgLink_ScdChParams * params)
{
    Int32 blkIdx;

    pScdChPrm->mode               = (SCD_Mode) params->mode;
    pScdChPrm->frmSensitivity     = (SCD_Sensitivity) params->frmSensitivity;
    pScdChPrm->frmIgnoreLightsOFF = params->frmIgnoreLightsOFF;
    pScdChPrm->frmIgnoreLightsON  = params->frmIgnoreLightsON;

    for(blkIdx = 0; blkIdx < ALG_LINK_SCD_MAX_BLOCKS_IN_FRAME; blkIdx++)
    {
        pScdChPrm->blkConfig[blkIdx].monitored   = params->blkConfig[blkIdx].monitored;
        pScdChPrm->blkConfig[blkIdx].sensitivity = (SCD_Sensitivity) params->blkConfig[blkIdx].sensitivity;
    }

    return 0;
}

Int32 AlgLink_ScdresetStatistics(AlgLink_ScdObj *pObj)
{
    UInt32 chId;
    AlgLink_ScdChObj *pChObj;

    for (chId = 0; chId < pObj->createArgs.numValidChForSCD; chId++)
    {
        pChObj = &pObj->chObj[chId];

        pChObj->inFrameRecvCount = 0;
        pChObj->inFrameUserSkipCount = 0;
        pChObj->inFrameProcessCount = 0;
        pChObj->inFrameProcessTime = 0;
    }

    pObj->totalFrameCount = 0;

    pObj->statsStartTime = Utils_getCurTimeInMsec();

    return 0;
}


Int32 AlgLink_scdAlgGetAllChFrameStatus(AlgLink_ScdObj * pObj, AlgLink_ScdAllChFrameStatus *pPrm)
{
    AlgLink_ScdChObj *pChObj;
    AlgLink_ScdCreateParams *pCreateArgs;
    UInt32 chId;

    pCreateArgs = &pObj->createArgs;

    pPrm->numCh = pCreateArgs->numValidChForSCD;

    for(chId=0; chId<pCreateArgs->numValidChForSCD; chId++)
    {
       pPrm->chanFrameResult[chId].frmResult = (UInt32)ALG_LINK_SCD_DETECTOR_UNAVAILABLE;
    }

    for(chId=0; chId<pCreateArgs->numValidChForSCD; chId++)
    {
        pChObj = &pObj->chObj[chId];

        pPrm->chanFrameResult[chId].chId      = pChObj->chId;
        pPrm->chanFrameResult[chId].frmResult = pChObj->scdChStat;
    }

    return FVID2_SOK;

}

Int32 AlgLink_ScdalgGetChResetChannel(AlgLink_ScdObj * pObj,
                            AlgLink_ScdChCtrl * params)
{
    SCD_Status      chanStatus;

    chanStatus = SCD_TI_resetChannel(
                                    pObj->algHndl,
                                    params->chId);

    if(chanStatus != SCD_NO_ERROR)
    {
#ifdef SYSTEM_DEBUG_SCD
        Vps_printf(" %d: SCD    : ERROR: Alg Process (chanID = %d) !!!\n",
                  Utils_getCurTimeInMsec(), params->chId );
#endif

        return FVID2_EFAIL;
    }
    else
    {
        Vps_printf(" %d: SCD    : chanID = %d Reset Done !!!\n",
                  Utils_getCurTimeInMsec(), params->chId);
    }
    return 0;
}

Int32 AlgLink_ScdprintStatistics (AlgLink_ScdObj *pObj, Bool resetAfterPrint)
{
    UInt32 chId;
    AlgLink_ScdChObj *pChObj;
    UInt32 elaspedTime;

    elaspedTime = Utils_getCurTimeInMsec() - pObj->statsStartTime; // in msecs
    elaspedTime /= 1000; // convert to secs

    Vps_printf( " \n"
            " *** SCD Statistics *** \n"
            " \n"
            " Elasped Time           : %d secs\n"
            " Total Fields Processed : %d \n"
            " Total Fields FPS       : %d FPS\n"
            " \n"
            " \n"
            " CH  | In Recv In Process In Skip In Process Time \n"
            " Num | FPS     FPS        FPS     per frame (msec)\n"
            " -------------------------------------------------\n",
            elaspedTime,
                    pObj->totalFrameCount,
            pObj->totalFrameCount / (elaspedTime)
                    );

    for (chId = 0; chId < pObj->createArgs.numValidChForSCD; chId++)
    {
        pChObj = &pObj->chObj[chId];

        Vps_printf( " %3d | %7d %10d %7d %14d\n",
            chId,
            pChObj->inFrameRecvCount/elaspedTime,
            pChObj->inFrameProcessCount/elaspedTime,
            pChObj->inFrameUserSkipCount/elaspedTime,
            (pChObj->inFrameProcessTime)/(pChObj->inFrameProcessCount)
            );
    }

    Vps_printf( " \n");

    if(resetAfterPrint)
    {
        AlgLink_ScdresetStatistics(pObj);
    }
    return FVID2_SOK;
}

