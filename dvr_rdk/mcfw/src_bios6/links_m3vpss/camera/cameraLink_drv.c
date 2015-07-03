/** ==================================================================
 *  @file   cameraLink_drv.c
 *
 *  @path   /ti/psp/examples/common/iss/chains/links/camera/
 *
 *  @desc   This  File contains.
 * ===================================================================
 *  Copyright (c) Texas Instruments Inc 2011, 2012
 *
 *  Use of this software is controlled by the terms and conditions found
 *  in the license agreement under which this software has been supplied
 * ===================================================================*/

#include "cameraLink_priv.h"
#include <ti/psp/platforms/iss_platform.h>
#include <ti/psp/devices/iss_sensorDriver.h>
#define SYSTEM_USE_SENSOR
#define SYSTEM_DEBUG_CAMERA
Iss_SensorVideoModeParams sensorVideoModeArgs;
char *gCameraLink_portName[] = {
    "VP0 PortA", "VP0 PortB", "VP1 PortA", "VP1 PortB",
};

char *gCameraLink_ifName[] = {
    " 8-bit", "16-bit", "24-bit",
};

char *gCameraLink_modeName[] = {
    "Non-mux Embedded Sync", "Line-mux Embedded Sync",
        "Pixel-mux Embedded Sync", "Non-mux Discrete Sync",
};

/* Is YUV422I format */
/* ===================================================================
 *  @func     CameraLink_isYUV422IFormat
 *
 *  @desc     Function does the following
 *
 *  @modif    This function modifies the following structures
 *
 *  @inputs   This function takes the following inputs
 *            <argument name>
 *            Description of usage
 *            <argument name>
 *            Description of usage
 *
 *  @outputs  <argument name>
 *            Description of usage
 *
 *  @return   Return value of this function if any
 *  ==================================================================
 */
Int32 CameraLink_isYUV422IFormat(Int32 format)
{




    if((format == FVID2_DF_YUV422I_UYVY)&&
       (format == FVID2_DF_YUV422I_YUYV)&&
	   (format == FVID2_DF_YUV422I_YVYU)&&
	   (format == FVID2_DF_YUV422I_VYUY))
	{
       return 1;
	}
	else
	{
        return 0;
	}
}

/* driver callback */
/* ===================================================================
 *  @func     CameraLink_drvCallback
 *
 *  @desc     Function does the following
 *
 *  @modif    This function modifies the following structures
 *
 *  @inputs   This function takes the following inputs
 *            <argument name>
 *            Description of usage
 *            <argument name>
 *            Description of usage
 *
 *  @outputs  <argument name>
 *            Description of usage
 *
 *  @return   Return value of this function if any
 *  ==================================================================
 */
Int32 CameraLink_drvCallback(FVID2_Handle handle, Ptr appData, Ptr reserved)
{
    CameraLink_Obj *pObj = (CameraLink_Obj *) appData;

    Utils_tskSendCmd(&pObj->tsk, SYSTEM_CMD_NEW_DATA);
    pObj->cbCount++;

    return FVID2_SOK;
}

/* Create Sensor for MT9J003 */
/* ===================================================================
 *  @func     CameraLink_drvCreateSensor
 *
 *  @desc     Function does the following
 *
 *  @modif    This function modifies the following structures
 *
 *  @inputs   This function takes the following inputs
 *            <argument name>
 *            Description of usage
 *            <argument name>
 *            Description of usage
 *
 *  @outputs  <argument name>
 *            Description of usage
 *
 *  @return   Return value of this function if any
 *  ==================================================================
 */
Int32 CameraLink_drvCreateSensor(CameraLink_Obj * pObj, UInt16 instId)
{
    CameraLink_VipInstParams *pInstPrm;

    CameraLink_InstObj *pInst;

    Iss_CaptCreateParams *pVipCreateArgs;

    pInstPrm = &pObj->createArgs.vipInst[instId];
    pInst = &pObj->instObj[instId];

    pVipCreateArgs = &pInst->createArgs;

    pInst->sensorCreateArgs.deviceI2cInstId = Iss_platformGetI2cInstId();
    pInst->sensorCreateArgs.numDevicesAtPort = 1;
    pInst->sensorCreateArgs.deviceI2cAddr[0]
        = Iss_platformGetSensorI2cAddr(pInstPrm->SensorId, pInstPrm->vipInstId);
    pInst->sensorCreateArgs.deviceResetGpio[0] = ISS_SENSOR_GPIO_NONE;
    pInst->sensorCreateArgs.InputStandard = pInstPrm->standard;

    if ((pInstPrm->SensorId == FVID2_ISS_SENSOR_MT9J003_DRV)
        &&
        (pInstPrm->vipInstId == ISS_CAPT_INST_VP ||
         pInstPrm->vipInstId == ISS_CAPT_INST_VP))
    {
        Iss_platformSelectSensor(pInstPrm->SensorId, pInstPrm->vipInstId);
    }


    pInst->sensorVideoModeArgs.videoIfMode = pVipCreateArgs->videoIfMode;
    pInst->sensorVideoModeArgs.videoDataFormat = pVipCreateArgs->inDataFormat;
    pInst->sensorVideoModeArgs.standard = pInstPrm->standard;
    // pInst->sensorVideoModeArgs.videoCaptureMode =
    // pVipCreateArgs->videoCaptureMode;
    pInst->sensorVideoModeArgs.videoSystem =
        ISS_SENSOR_VIDEO_SYSTEM_AUTO_DETECT;
    pInst->sensorVideoModeArgs.videoAutoDetectTimeout = BIOS_WAIT_FOREVER;
    sensorVideoModeArgs.standard = pInstPrm->standard;

    pInst->SensorHandle = FVID2_create(pInstPrm->SensorId,
                                       0,
                                       &pInst->sensorCreateArgs,
                                       &pInst->sensorCreateStatus, NULL);

    UTILS_assert( pInst->SensorHandle != NULL);
    return FVID2_SOK;
}

/* ===================================================================
 *  @func     CameraLink_drvInstSetFrameSkip
 *
 *  @desc     Function does the following
 *
 *  @modif    This function modifies the following structures
 *
 *  @inputs   This function takes the following inputs
 *            <argument name>
 *            Description of usage
 *            <argument name>
 *            Description of usage
 *
 *  @outputs  <argument name>
 *            Description of usage
 *
 *  @return   Return value of this function if any
 *  ==================================================================
 */
Int32 CameraLink_drvInstSetFrameSkip(CameraLink_Obj * pObj, UInt16 instId,
                                     UInt32 frameSkipMask)
{
    Iss_CaptFrameSkip frameSkip;

    UInt16 outId, chId;

    Int32 status;

    CameraLink_InstObj *pInst;

    Iss_CaptCreateParams *pVipCreateArgs;

    pInst = &pObj->instObj[instId];
    pVipCreateArgs = &pInst->createArgs;

    /*
     * set frame skip using a IOCTL if enabled
     */
    for (outId = 0; outId < pVipCreateArgs->numStream; outId++)
    {
        for (chId = 0; chId < pVipCreateArgs->numCh; chId++)
        {

            frameSkip.channelNum = pVipCreateArgs->channelNumMap[outId][chId];
            if (outId == 0)
            {
                frameSkip.frameSkipMask[outId] = frameSkipMask;
				frameSkip.frameSkipMaskHigh[outId] = frameSkipMask;
            }
            else if (outId == 1)
            {
                frameSkip.frameSkipMask[outId] = 0x2AAAAAAA;
				frameSkip.frameSkipMaskHigh[outId] = 0x2AAAAAAA;
            }
            status = FVID2_control(pInst->cameraVipHandle,
                                   IOCTL_ISS_CAPT_SET_FRAME_SKIP,
                                   &frameSkip, NULL);
            UTILS_assert( status == FVID2_SOK);
        }
    }

    return status;
}

/* Create camera driver */
/* ===================================================================
 *  @func     CameraLink_drvCreateInst
 *
 *  @desc     Function does the following
 *
 *  @modif    This function modifies the following structures
 *
 *  @inputs   This function takes the following inputs
 *            <argument name>
 *            Description of usage
 *            <argument name>
 *            Description of usage
 *
 *  @outputs  <argument name>
 *            Description of usage
 *
 *  @return   Return value of this function if any
 *  ==================================================================
 */
Int32 CameraLink_drvCreateInst(CameraLink_Obj * pObj, UInt16 instId)
{
    CameraLink_VipInstParams *pInstPrm;

    CameraLink_InstObj *pInst;

    Iss_CaptCreateParams *pVipCreateArgs;

    Iss_CaptOutInfo *pVipOutPrm;

    CameraLink_OutParams *pOutPrm;

    System_LinkChInfo *pQueChInfo;

    UInt16 queId, queChId, outId, chId;

    FVID2_ScanFormat inScanFormat;

    pInstPrm = &pObj->createArgs.vipInst[instId];
    pInst = &pObj->instObj[instId];
    pVipCreateArgs = &pInst->createArgs;

    pInst->instId = pInstPrm->vipInstId;

    CameraLink_drvInitCreateArgs(pVipCreateArgs, pInstPrm);

    pVipCreateArgs->inDataFormat = pInstPrm->inDataFormat;

    inScanFormat = FVID2_SF_PROGRESSIVE;
    if (instId == 0)
    {
        pObj->maxWidth = 1920;
        pObj->maxHeight = 1080;
    }

    if (pVipCreateArgs->inDataFormat == FVID2_DF_RGB24_888)
    {
        pVipCreateArgs->videoCaptureMode =
            ISS_CAPT_VIDEO_CAPTURE_MODE_SINGLE_STREAM;
        pVipCreateArgs->videoIfMode = ISS_CAPT_BAYER_12BIT;
    }
    else
    {
        pVipCreateArgs->videoCaptureMode =
            ISS_CAPT_VIDEO_CAPTURE_MODE_SINGLE_STREAM;
        pVipCreateArgs->videoIfMode = ISS_CAPT_BAYER_12BIT;
    }
    pVipCreateArgs->numCh = 1;

    Vps_printf(" %d: CAMERA: %s camera mode is [%s, %s] !!! \n",
               Clock_getTicks(),
               gCameraLink_portName[pInstPrm->vipInstId],
               gCameraLink_ifName[pVipCreateArgs->videoIfMode],
               gCameraLink_modeName[pVipCreateArgs->videoCaptureMode]);

    pVipCreateArgs->periodicCallbackEnable = TRUE;

    pVipCreateArgs->numStream = pInstPrm->numOutput;

    for (outId = 0; outId < pVipCreateArgs->numStream; outId++)
    {
        pVipOutPrm = &pVipCreateArgs->outStreamInfo[outId];

        pOutPrm = &pInstPrm->outParams[outId];

        pVipOutPrm->dataFormat = pOutPrm->dataFormat;

        pVipOutPrm->memType = ISS_NONTILEDMEM;

        if (pObj->createArgs.tilerEnable &&
            (pOutPrm->dataFormat == FVID2_DF_YUV420SP_UV ||
             pOutPrm->dataFormat == FVID2_DF_YUV422SP_UV))
        {
            pVipOutPrm->memType = ISS_TILEDMEM;
        }

        pVipOutPrm->pitch[0] = VpsUtils_align(pObj->maxWidth, ISS_BUFFER_ALIGNMENT * 2);

        if(CameraLink_isYUV422IFormat(pVipOutPrm->dataFormat) == 1)
            pVipOutPrm->pitch[0] *= 2;
        if (pVipOutPrm->dataFormat == FVID2_DF_RGB24_888)
            pVipOutPrm->pitch[0] *= 3;
        pVipOutPrm->pitch[1] = pVipOutPrm->pitch[0];

        if (CameraLink_drvIsDataFormatTiled(pVipCreateArgs, outId))
        {
            pVipOutPrm->pitch[0] = VPSUTILS_TILER_CNT_8BIT_PITCH;
            pVipOutPrm->pitch[1] = 0;
        }

        pVipOutPrm->pitch[2] = 0;

        pVipOutPrm->scEnable = pOutPrm->scEnable;

        for (chId = 0; chId < pVipCreateArgs->numCh; chId++)
        {
            queId = pOutPrm->outQueId;
            queChId = pObj->info.queInfo[queId].numCh;

            pQueChInfo = &pObj->info.queInfo[queId].chInfo[queChId];

            pQueChInfo->dataFormat = (FVID2_DataFormat) pVipOutPrm->dataFormat;

            if (pVipOutPrm->scEnable)
            {
                pQueChInfo->width = pOutPrm->scOutWidth;
                pQueChInfo->height = pOutPrm->scOutHeight;
            }
            else
            {
                pQueChInfo->width = pOutPrm->scOutWidth;
                pQueChInfo->height = pOutPrm->scOutHeight;
                pQueChInfo->pitch[0] = pOutPrm->scOutWidth;
                if(CameraLink_isYUV422IFormat(pVipOutPrm->dataFormat) == 1)
                    pQueChInfo->pitch[0] *= 2;
                if (pVipOutPrm->dataFormat == FVID2_DF_RGB24_888)
                    pQueChInfo->pitch[0] *= 3;
            }

            pQueChInfo->pitch[1] = pQueChInfo->pitch[0];
            pQueChInfo->pitch[2] = 0;
            pQueChInfo->scanFormat = inScanFormat;

            pObj->info.queInfo[queId].numCh++;

            pVipCreateArgs->channelNumMap[outId][chId] =
                CameraLink_makeChannelNum(queId, queChId);
        }
    }

    memset(&pInst->cbPrm, 0, sizeof(pInst->cbPrm));

    pInst->cbPrm.appData = pObj;

    if (instId == 0)
        pInst->cbPrm.cbFxn = CameraLink_drvCallback;

    pObj->enableCheckOverflowDetect = FALSE;

    pInst->cameraVipHandle = FVID2_create(FVID2_ISS_CAPT_DRV,
                                          pInst->instId,
                                          pVipCreateArgs,
                                          &pInst->createStatus, &pInst->cbPrm);

    UTILS_assert( pInst->cameraVipHandle != NULL);

    CameraLink_drvAllocAndQueueFrames(pObj, pInst);

    CameraLink_drvInstSetFrameSkip(pObj, instId, 0);

 //   pInst->SensorHandle = NULL;

#ifdef SYSTEM_USE_SENSOR
    CameraLink_drvCreateSensor(pObj, instId);
#endif

    return FVID2_SOK;
}

/* Create camera link
 *
 * This creates - camera driver - video decoder driver - allocate and queues
 * frames to the camera driver - DOES NOT start the camera ONLY make it ready
 * for operation */
/* ===================================================================
 *  @func     CameraLink_drvCreate
 *
 *  @desc     Function does the following
 *
 *  @modif    This function modifies the following structures
 *
 *  @inputs   This function takes the following inputs
 *            <argument name>
 *            Description of usage
 *            <argument name>
 *            Description of usage
 *
 *  @outputs  <argument name>
 *            Description of usage
 *
 *  @return   Return value of this function if any
 *  ==================================================================
 */
Int32 CameraLink_drvCreate(CameraLink_Obj * pObj,
                           CameraLink_CreateParams * pPrm)
{
    Int32 status;

    UInt32 queId, instId;

#ifdef SYSTEM_DEBUG_CAMERA
    Vps_printf(" %d: CAMERA: Create in progress !!!\n", Clock_getTicks());
#endif

    memcpy(&pObj->createArgs, pPrm, sizeof(*pPrm));

    pObj->cameraDequeuedFrameCount = 0;
    pObj->cameraQueuedFrameCount = 0;
    pObj->cbCount = 0;
    pObj->cbCountServicedCount = 0;
    pObj->prevFrameCount = 0;
    pObj->totalCpuLoad = 0;
    pObj->resetCount = 0;
    pObj->resetTotalTime = 0;
    pObj->prevResetTime = 0;
    pObj->isPalMode = FALSE;

    pObj->brightness = 0x1c;                               /* TUNED for
                                                            * specific
                                                            * scene's, to
                                                            * make black
                                                            * blacker */
    pObj->contrast = 0x89;                                 /* TUNED for
                                                            * specific
                                                            * scene's, to
                                                            * make black
                                                            * blacker */
    pObj->saturation = 128;                                /* default */

    memset(pObj->cameraFrameCount, 0, sizeof(pObj->cameraFrameCount));

    pObj->info.numQue = CAMERA_LINK_MAX_OUT_QUE;

    for (queId = 0; queId < CAMERA_LINK_MAX_OUT_QUE; queId++)
    {
        status = Utils_bufCreate(&pObj->bufQue[queId], FALSE, FALSE);
        UTILS_assert( status == FVID2_SOK);

        pObj->info.queInfo[queId].numCh = 0;
    }

    /*
     * Create global VP camera handle, used for dequeue,
     * queue from all active cameras
     */
    pObj->fvidHandleVipAll = FVID2_create(FVID2_ISS_CAPT_DRV,
                                          ISS_CAPT_INST_ALL, NULL, NULL, NULL);
    UTILS_assert( pObj->fvidHandleVipAll != NULL);

    if (CAMERA_LINK_TMP_BUF_SIZE)
    {
        pObj->tmpBufAddr = Utils_memAlloc(CAMERA_LINK_TMP_BUF_SIZE, 32);
    }

    for (instId = 0; instId < pPrm->numVipInst; instId++)
    {
        CameraLink_drvCreateInst(pObj, instId);
    }

#ifdef SYSTEM_DEBUG_CAMERA
    Vps_printf(" %d: CAMERA: Create Done !!!\n", Clock_getTicks());
#endif

    return status;
}

/* ===================================================================
 *  @func     CameraLink_drvDetectVideo
 *
 *  @desc     Function does the following
 *
 *  @modif    This function modifies the following structures
 *
 *  @inputs   This function takes the following inputs
 *            <argument name>
 *            Description of usage
 *            <argument name>
 *            Description of usage
 *
 *  @outputs  <argument name>
 *            Description of usage
 *
 *  @return   Return value of this function if any
 *  ==================================================================
 */
Int32 CameraLink_drvDetectVideo(CameraLink_Obj * pObj, UInt32 timeout)
{
    UInt32 instId;

    CameraLink_InstObj *pInst;

#ifdef SYSTEM_DEBUG_CAMERA
    Vps_printf(" %d: CAMERA: Detect video in progress !!!\n", Clock_getTicks());
#endif

    for (instId = 0; instId < pObj->createArgs.numVipInst; instId++)
    {
        pInst = &pObj->instObj[instId];

        if (pInst->SensorHandle == NULL)
            return FVID2_SOK;

        pInst->sensorVideoModeArgs.videoAutoDetectTimeout = timeout;
    }

#ifdef SYSTEM_DEBUG_CAMERA
    Vps_printf(" %d: CAMERA: Detect video Done !!!\n", Clock_getTicks());
#endif

    return FVID2_SOK;
}

/* ===================================================================
 *  @func     CameraLink_drvOverflowDetectAndReset
 *
 *  @desc     Function does the following
 *
 *  @modif    This function modifies the following structures
 *
 *  @inputs   This function takes the following inputs
 *            <argument name>
 *            Description of usage
 *            <argument name>
 *            Description of usage
 *
 *  @outputs  <argument name>
 *            Description of usage
 *
 *  @return   Return value of this function if any
 *  ==================================================================
 */
Int32 CameraLink_drvOverflowDetectAndReset(CameraLink_Obj * pObj,
                                           Bool doForceReset)
{
    Iss_CaptOverFlowStatus overFlowStatus;

    UInt32 curTime;

    FVID2_control(pObj->fvidHandleVipAll,
                  IOCTL_ISS_CAPT_CHECK_OVERFLOW, NULL, &overFlowStatus);

    if (doForceReset)
    {
        overFlowStatus.isPortOverFlowed[ISS_CAPT_INST_VP] = TRUE;
        overFlowStatus.isPortOverFlowed[ISS_CAPT_INST_VP] = TRUE;
        overFlowStatus.isPortOverFlowed[ISS_CAPT_INST_VP] = TRUE;
        overFlowStatus.isPortOverFlowed[ISS_CAPT_INST_VP] = TRUE;
    }

    if (overFlowStatus.isPortOverFlowed[ISS_CAPT_INST_VP]
        ||
        overFlowStatus.isPortOverFlowed[ISS_CAPT_INST_VP]
        ||
        overFlowStatus.isPortOverFlowed[ISS_CAPT_INST_VP]
        || overFlowStatus.isPortOverFlowed[ISS_CAPT_INST_VP])
    {
        pObj->resetCount++;
        curTime = Clock_getTicks();
    }

    if (overFlowStatus.isPortOverFlowed[ISS_CAPT_INST_VP]
        || overFlowStatus.isPortOverFlowed[ISS_CAPT_INST_VP])
    {
        // System_lockVip(SYSTEM_VP_0);

        Vps_rprintf
            (" %d: CAMERA: Overflow detected on VP0, Total Resets = %d\n",
             Clock_getTicks(), pObj->resetCount);
    }
    if (overFlowStatus.isPortOverFlowed[ISS_CAPT_INST_VP]
        || overFlowStatus.isPortOverFlowed[ISS_CAPT_INST_VP])
    {
        // System_lockVip(SYSTEM_VP_1);

        Vps_rprintf
            (" %d: CAMERA: Overflow detected on VP1, Total Resets = %d\n",
             Clock_getTicks(), pObj->resetCount);
    }

    FVID2_control(pObj->fvidHandleVipAll,
                  IOCTL_ISS_CAPT_RESET_AND_RESTART, &overFlowStatus, NULL);

    if (overFlowStatus.isPortOverFlowed[ISS_CAPT_INST_VP]
        ||
        overFlowStatus.isPortOverFlowed[ISS_CAPT_INST_VP]
        ||
        overFlowStatus.isPortOverFlowed[ISS_CAPT_INST_VP]
        || overFlowStatus.isPortOverFlowed[ISS_CAPT_INST_VP])
    {
        curTime = Clock_getTicks() - curTime;
        pObj->resetTotalTime += curTime;
    }

    return FVID2_SOK;
}

/* ===================================================================
 *  @func     CameraLink_drvPrintRtStatus
 *
 *  @desc     Function does the following
 *
 *  @modif    This function modifies the following structures
 *
 *  @inputs   This function takes the following inputs
 *            <argument name>
 *            Description of usage
 *            <argument name>
 *            Description of usage
 *
 *  @outputs  <argument name>
 *            Description of usage
 *
 *  @return   Return value of this function if any
 *  ==================================================================
 */
Int32 CameraLink_drvPrintRtStatus(CameraLink_Obj * pObj, UInt32 frameCount,
                                  UInt32 elaspedTime)
{
    UInt32 fps = (frameCount * 100) / (elaspedTime / 10);

    Vps_rprintf
        (" %d: CAMERA: Fields = %d (fps = %d), Total Resets = %d (Avg %d ms per reset)\r\n",
         Clock_getTicks(), frameCount, fps, pObj->resetCount,
         pObj->resetTotalTime / pObj->resetCount);

//    System_displayUnderflowPrint(TRUE, FALSE);

    return 0;
}

/* ===================================================================
 *  @func     CameraLink_drvProcessData
 *
 *  @desc     Function does the following
 *
 *  @modif    This function modifies the following structures
 *
 *  @inputs   This function takes the following inputs
 *            <argument name>
 *            Description of usage
 *            <argument name>
 *            Description of usage
 *
 *  @outputs  <argument name>
 *            Description of usage
 *
 *  @return   Return value of this function if any
 *  ==================================================================
 */
Int32 CameraLink_drvProcessData(CameraLink_Obj * pObj)
{
    UInt32 frameId, queId, streamId, queChId, elaspedTime;

    FVID2_FrameList frameList;

    FVID2_Frame *pFrame;

    volatile UInt32 sendMsgToTsk = 0, tmpValue;

    Int32 status;

    pObj->cbCountServicedCount++;

    System_displayUnderflowCheck(FALSE);

    for (streamId = 0; streamId < CAMERA_LINK_MAX_OUTPUT_PER_INST; streamId++)
    {
        /*
         * Deque frames for all active handles
         */
        FVID2_dequeue(pObj->fvidHandleVipAll,
                      &frameList, streamId, BIOS_NO_WAIT);

        if (frameList.numFrames)
        {
            for (frameId = 0; frameId < frameList.numFrames; frameId++)
            {
                pFrame = frameList.frames[frameId];

                queId = CameraLink_getQueId(pFrame->channelNum);
                queChId = CameraLink_getQueChId(pFrame->channelNum);

                UTILS_assert( queId < CAMERA_LINK_MAX_OUT_QUE);
                UTILS_assert(
                          queChId < CAMERA_LINK_MAX_CH_PER_OUT_QUE);

                pObj->cameraDequeuedFrameCount++;
                pObj->cameraFrameCount[queId][queChId]++;

                tmpValue = (UInt32) pFrame->reserved;
#if 0
                if (tmpValue > 0)
                {
                    Vps_printf
                        (" %d: CAMERA: Dequeued frame more than once (%d,%d, %08x) \n",
                         Clock_getTicks(), queId, queChId, pFrame->addr[0][0]);
                }
#endif
                tmpValue++;
                pFrame->reserved = (Ptr) tmpValue;

                pFrame->perFrameCfg = NULL;
                pFrame->channelNum = queChId;

                sendMsgToTsk |= (1 << queId);

                status = Utils_bufPutFullFrame(&pObj->bufQue[queId], pFrame);
                UTILS_assert( status == FVID2_SOK);
            }

#ifdef SYSTEM_DEBUG_CAMERA_RT
            Vps_printf(" %d: CAMERA: Dequeued %d frames !!!\n",
                       Clock_getTicks(), frameList.numFrames);
#endif
        }
    }

    elaspedTime = Clock_getTicks() - pObj->startTime;

    if ((elaspedTime - pObj->prevTime) > 60 * 1000)
    {
        CameraLink_getCpuLoad();

        CameraLink_drvPrintRtStatus(pObj,
                                    pObj->cameraDequeuedFrameCount -
                                    pObj->prevFrameCount,
                                    elaspedTime - pObj->prevTime);

        pObj->prevFrameCount = pObj->cameraDequeuedFrameCount;
        pObj->prevTime = elaspedTime;
    }

    for (queId = 0; queId < CAMERA_LINK_MAX_OUT_QUE; queId++)
    {
        if (sendMsgToTsk & 0x1)
        {
            /* send data available message to next tsk */
            System_sendLinkCmd(pObj->createArgs.outQueParams[queId].nextLink,
                               SYSTEM_CMD_NEW_DATA);
        }

        sendMsgToTsk >>= 1;
        if (sendMsgToTsk == 0)
            break;
    }

    if (pObj->enableCheckOverflowDetect)
    {
        CameraLink_drvOverflowDetectAndReset(pObj, FALSE);
    }

    pObj->exeTime = Clock_getTicks() - pObj->startTime;

    return FVID2_SOK;
}

/* ===================================================================
 *  @func     CameraLink_drvPutEmptyFrames
 *
 *  @desc     Function does the following
 *
 *  @modif    This function modifies the following structures
 *
 *  @inputs   This function takes the following inputs
 *            <argument name>
 *            Description of usage
 *            <argument name>
 *            Description of usage
 *
 *  @outputs  <argument name>
 *            Description of usage
 *
 *  @return   Return value of this function if any
 *  ==================================================================
 */
Int32 CameraLink_drvPutEmptyFrames(CameraLink_Obj * pObj,
                                   FVID2_FrameList * pFrameList)
{
    UInt32 frameId;

    FVID2_Frame *pFrame;

    System_FrameInfo *pFrameInfo;

    volatile UInt32 tmpValue;

    if (pFrameList->numFrames)
    {
        for (frameId = 0; frameId < pFrameList->numFrames; frameId++)
        {
            pFrame = pFrameList->frames[frameId];

            tmpValue = (UInt32) pFrame->reserved;
            tmpValue--;
            pFrame->reserved = (Ptr) tmpValue;

            pFrameInfo = (System_FrameInfo *) pFrame->appData;
            UTILS_assert( pFrameInfo != NULL);
            // pFrame->perFrameCfg = &pFrameInfo->cameraRtParams;
            pFrame->channelNum = pFrameInfo->cameraChannelNum;
        }

#ifdef SYSTEM_DEBUG_CAMERA_RT
        Vps_printf(" %d: CAMERA: Queued back %d frames !!!\n", Clock_getTicks(),
                   pFrameList->numFrames);
#endif

        pObj->cameraQueuedFrameCount += pFrameList->numFrames;

        FVID2_queue(pObj->fvidHandleVipAll, pFrameList, ISS_CAPT_STREAM_ID_ANY);
    }

    return FVID2_SOK;
}

/* ===================================================================
 *  @func     CameraLink_drvStart
 *
 *  @desc     Function does the following
 *
 *  @modif    This function modifies the following structures
 *
 *  @inputs   This function takes the following inputs
 *            <argument name>
 *            Description of usage
 *            <argument name>
 *            Description of usage
 *
 *  @outputs  <argument name>
 *            Description of usage
 *
 *  @return   Return value of this function if any
 *  ==================================================================
 */
Int32 CameraLink_drvStart(CameraLink_Obj * pObj)
{
    UInt32 instId;

    CameraLink_InstObj *pInstObj;

#ifdef SYSTEM_DEBUG_CAMERA
    Vps_printf(" %d: CAMERA: Start in progress !!!\n", Clock_getTicks());
#endif

    for (instId = 0; instId < pObj->createArgs.numVipInst; instId++)
    {
        pInstObj = &pObj->instObj[instId];

        /* video decoder */
        if (pInstObj->SensorHandle)
            FVID2_start(pInstObj->SensorHandle, NULL);

    }

#ifdef SYSTEM_DEBUG_CAMERA
    Vps_printf(" %d: CAMERA: Start Done !!!\n", Clock_getTicks());
#endif

    Task_sleep(100);

    pObj->prevTime = pObj->startTime = Clock_getTicks();

    for (instId = 0; instId < pObj->createArgs.numVipInst; instId++)
    {
        pInstObj = &pObj->instObj[instId];

        /* VP camera */
        FVID2_start(pInstObj->cameraVipHandle, NULL);
    }

    return FVID2_SOK;
}

/* ===================================================================
 *  @func     CameraLink_drvFlush
 *
 *  @desc     Function does the following
 *
 *  @modif    This function modifies the following structures
 *
 *  @inputs   This function takes the following inputs
 *            <argument name>
 *            Description of usage
 *            <argument name>
 *            Description of usage
 *
 *  @outputs  <argument name>
 *            Description of usage
 *
 *  @return   Return value of this function if any
 *  ==================================================================
 */
Int32 CameraLink_drvFlush(FVID2_Handle cameraVipHandle, char *portName)
{
    Int32 status;

    FVID2_FrameList frameList;

    do
    {
        status = FVID2_control(cameraVipHandle, NULL, NULL, &frameList);

#ifdef SYSTEM_DEBUG_CAMERA
        Vps_rprintf(" %d: CAMERA: %s: Flushed %d frames.\n",
                    Clock_getTicks(), portName, frameList.numFrames);
#endif

    } while (frameList.numFrames != 0 && status == FVID2_SOK);

    if (status != FVID2_SOK)
    {
#ifdef SYSTEM_DEBUG_CAMERA
        Vps_rprintf(" %d: CAMERA: %s: Flushing ... ERROR !!!\n",
                    Clock_getTicks(), portName);
#endif
    }

    return FVID2_SOK;
}

/* ===================================================================
 *  @func     CameraLink_drvStop
 *
 *  @desc     Function does the following
 *
 *  @modif    This function modifies the following structures
 *
 *  @inputs   This function takes the following inputs
 *            <argument name>
 *            Description of usage
 *            <argument name>
 *            Description of usage
 *
 *  @outputs  <argument name>
 *            Description of usage
 *
 *  @return   Return value of this function if any
 *  ==================================================================
 */
Int32 CameraLink_drvStop(CameraLink_Obj * pObj)
{
    UInt32 instId;

    CameraLink_InstObj *pInstObj;

    for (instId = 0; instId < pObj->createArgs.numVipInst; instId++)
    {
        pInstObj = &pObj->instObj[instId];

        /* VP camera */
        FVID2_stop(pInstObj->cameraVipHandle, NULL);
    }

    pObj->exeTime = Clock_getTicks() - pObj->startTime;

#ifdef SYSTEM_DEBUG_CAMERA
    Vps_printf(" %d: CAMERA: Stop in progress !!!\n", Clock_getTicks());
#endif

    for (instId = 0; instId < pObj->createArgs.numVipInst; instId++)
    {
        pInstObj = &pObj->instObj[instId];

        /* video decoder */
        if (pInstObj->SensorHandle)
            FVID2_stop(pInstObj->SensorHandle, NULL);
    }

#ifdef SYSTEM_DEBUG_CAMERA
    Vps_printf(" %d: CAMERA: Stop Done !!!\n", Clock_getTicks());
#endif

    return FVID2_SOK;
}

/* ===================================================================
 *  @func     CameraLink_drvDelete
 *
 *  @desc     Function does the following
 *
 *  @modif    This function modifies the following structures
 *
 *  @inputs   This function takes the following inputs
 *            <argument name>
 *            Description of usage
 *            <argument name>
 *            Description of usage
 *
 *  @outputs  <argument name>
 *            Description of usage
 *
 *  @return   Return value of this function if any
 *  ==================================================================
 */
Int32 CameraLink_drvDelete(CameraLink_Obj * pObj)
{
    UInt32 instId;

    UInt32 queId;

    CameraLink_InstObj *pInstObj;

#ifdef SYSTEM_DEBUG_CAMERA
    CameraLink_drvPrintStatus(pObj);
#endif

#ifdef SYSTEM_DEBUG_CAMERA
    Vps_printf(" %d: CAMERA: Delete in progress !!!\n", Clock_getTicks());
#endif

    if (CAMERA_LINK_TMP_BUF_SIZE)
    {
        Utils_memFree(pObj->tmpBufAddr, CAMERA_LINK_TMP_BUF_SIZE);
    }

    for (instId = 0; instId < pObj->createArgs.numVipInst; instId++)
    {
        pInstObj = &pObj->instObj[instId];

        /* VP camera */
        FVID2_delete(pInstObj->cameraVipHandle, NULL);

        if (pInstObj->SensorHandle)
        {
            /* video decoder */
            FVID2_delete(pInstObj->SensorHandle, NULL);
        }

        CameraLink_drvFreeFrames(pObj, pInstObj);
    }

    FVID2_delete(pObj->fvidHandleVipAll, NULL);

    for (queId = 0; queId < CAMERA_LINK_MAX_OUT_QUE; queId++)
    {
        Utils_bufDelete(&pObj->bufQue[queId]);
    }
    Task_sleep(1);

#ifdef SYSTEM_DEBUG_CAMERA
    Vps_printf(" %d: CAMERA: Delete Done !!!\n", Clock_getTicks());
#endif

    return FVID2_SOK;
}

/*
 * Allocate and queue frames to driver
 *
 * pDrvObj - camera driver information */
/* ===================================================================
 *  @func     CameraLink_drvAllocAndQueueFrames
 *
 *  @desc     Function does the following
 *
 *  @modif    This function modifies the following structures
 *
 *  @inputs   This function takes the following inputs
 *            <argument name>
 *            Description of usage
 *            <argument name>
 *            Description of usage
 *
 *  @outputs  <argument name>
 *            Description of usage
 *
 *  @return   Return value of this function if any
 *  ==================================================================
 */
Int32 CameraLink_drvAllocAndQueueFrames(CameraLink_Obj * pObj,
                                        CameraLink_InstObj * pDrvObj)
{
    Int32 status;

    UInt16 streamId, chId, frameId, idx;

    Iss_CaptOutInfo *pOutInfo;

    FVID2_Frame *frames;

    System_FrameInfo *pFrameInfo;

    FVID2_FrameList frameList;

    FVID2_Format format;

    CameraLink_VipInstParams *pInstPrm;

    CameraLink_OutParams *pOutPrm;

    pInstPrm = &pObj->createArgs.vipInst[0];
    /*
     * init frameList for list of frames that are queued per CH to driver
     */
    frameList.perListCfg = NULL;
    frameList.reserved = NULL;

    /*
     * for every stream and channel in a camera handle
     */
    for (streamId = 0; streamId < pDrvObj->createArgs.numStream; streamId++)
    {
        for (chId = 0; chId < pDrvObj->createArgs.numCh; chId++)
        {

            pOutInfo = &pDrvObj->createArgs.outStreamInfo[streamId];
            pOutPrm = &pInstPrm->outParams[streamId];
            /*
             * base index for pDrvObj->frames[] and pDrvObj->frameInfo[]
             */
            idx =
                ISS_CAPT_CH_PER_PORT_MAX * CAMERA_LINK_FRAMES_PER_CH *
                streamId + CAMERA_LINK_FRAMES_PER_CH * chId;
            if (idx >= CAMERA_LINK_MAX_FRAMES_PER_HANDLE)
            {
                idx = 0u;
            }

            pFrameInfo = &pDrvObj->frameInfo[idx];
            frames = &pDrvObj->frames[idx];

            /* fill format with channel specific values */
            format.channelNum =
                pDrvObj->createArgs.channelNumMap[streamId][chId];
            format.width = pOutPrm->scOutWidth;            // pObj->maxWidth;
            format.height = pOutPrm->scOutHeight;          // pObj->maxHeight+CAMERA_LINK_HEIGHT_PAD_LINES;
            format.pitch[0] = pOutPrm->scOutWidth;         // pOutInfo->pitch[0];
            if(CameraLink_isYUV422IFormat(pOutInfo->dataFormat) == 1)
                format.pitch[0] *= 2;
            if (pOutInfo->dataFormat == FVID2_DF_RGB24_888)
                format.pitch[0] *= 3;
            format.pitch[1] = format.pitch[0];             // pOutInfo->pitch[0];
            format.pitch[2] = 0;
            format.fieldMerged[0] = FALSE;
            format.fieldMerged[1] = FALSE;
            format.fieldMerged[2] = FALSE;
            format.dataFormat = pOutInfo->dataFormat;
            format.scanFormat = FVID2_SF_PROGRESSIVE;
            format.bpp = FVID2_BPP_BITS8;                  /* ignored */

            if (format.dataFormat == FVID2_DF_RAW_VBI)
            {
                format.height = CAMERA_LINK_RAW_VBI_LINES;
            }

            /*
             * alloc memory based on 'format'
             * Allocated frame info is put in frames[]
             * CAMERA_LINK_APP_FRAMES_PER_CH is the number of buffers per channel to
             * allocate
             */
            if (CameraLink_drvIsDataFormatTiled(&pDrvObj->createArgs, streamId))
            {
                Utils_tilerFrameAlloc(&format, frames,
                                         CAMERA_LINK_FRAMES_PER_CH);
            }
            else
            {
                Utils_memFrameAlloc(&format, frames,
                                       CAMERA_LINK_FRAMES_PER_CH);
            }

            /*
             * Set rtParams for every frame in perFrameCfg
             */
            for (frameId = 0; frameId < CAMERA_LINK_FRAMES_PER_CH; frameId++)
            {
                // frames[frameId].perFrameCfg =
                // &pFrameInfo[frameId].cameraRtParams;
                frames[frameId].subFrameInfo = NULL;
				if(CameraLink_isYUV422IFormat(format.dataFormat) == 1)
				frames[frameId].addr[1][0] = frames[frameId].addr[0][0];
				else if((format.dataFormat == FVID2_DF_YUV420SP_VU) ||
							(format.dataFormat == FVID2_DF_YUV420SP_UV))
				{
					frames[frameId].addr[1][0] = frames[frameId].addr[0][0];
					frames[frameId].addr[1][1] = frames[frameId].addr[0][1];
				}
                frames[frameId].appData = &pFrameInfo[frameId];
                frames[frameId].reserved = NULL;

                pFrameInfo[frameId].cameraChannelNum =frames[frameId].channelNum;

                frameList.frames[frameId] = &frames[frameId];

#ifdef SYSTEM_VERBOSE_PRINTS
                if (pDrvObj->instId == 0 && streamId == 0 && chId == 0)
                {
                    Vps_rprintf(" %d: CAMERA: %d: 0x%08x, %d x %d, %08x B\n",
                                Clock_getTicks(),
                                frameId, frames[frameId].addr[0][0],
                                format.pitch[0] / 2, format.height,
                                format.height * format.pitch[0]);
                }
#endif
            }
#ifdef SYSTEM_VERBOSE_PRINTS
            if (pDrvObj->instId == 0 && streamId == 0 && chId == 0)
            {
                Vps_rprintf(" %d: CAMERA: 0x%08x %08x B\n",
                            Clock_getTicks(),
                            frames[0].addr[0][0],
                            format.height * format.pitch[0] * frameId);
            }
#endif

            /*
             * Set number of frame in frame list
             */
            frameList.numFrames = CAMERA_LINK_FRAMES_PER_CH;

            /*
             * queue the frames in frameList
             * All allocate frames are queued here as an example.
             * In general atleast 2 frames per channel need to queued
             * before starting camera,
             * else frame will get dropped until frames are queued
             */
            status =
                FVID2_queue(pDrvObj->cameraVipHandle, &frameList, streamId);
            UTILS_assert( status == FVID2_SOK);
        }
    }

    return FVID2_SOK;
}

/*
 * Free allocated frames
 *
 * pDrvObj - camera driver information */
/* ===================================================================
 *  @func     CameraLink_drvFreeFrames
 *
 *  @desc     Function does the following
 *
 *  @modif    This function modifies the following structures
 *
 *  @inputs   This function takes the following inputs
 *            <argument name>
 *            Description of usage
 *            <argument name>
 *            Description of usage
 *
 *  @outputs  <argument name>
 *            Description of usage
 *
 *  @return   Return value of this function if any
 *  ==================================================================
 */
Int32 CameraLink_drvFreeFrames(CameraLink_Obj * pObj,
                               CameraLink_InstObj * pDrvObj)
{
    UInt32 idx;

    UInt16 streamId, chId;

    FVID2_Format format;

    FVID2_Frame *pFrames;

    Iss_CaptOutInfo *pOutInfo;

    UInt32 tilerUsed = FALSE;

    CameraLink_OutParams *pOutPrm;

    CameraLink_VipInstParams *pInstPrm;

    pInstPrm = &pObj->createArgs.vipInst[0];

    for (streamId = 0; streamId < pDrvObj->createArgs.numStream; streamId++)
    {
        for (chId = 0; chId < pDrvObj->createArgs.numCh; chId++)
        {
            pOutInfo = &pDrvObj->createArgs.outStreamInfo[streamId];
            pOutPrm = &pInstPrm->outParams[streamId];
            idx = ISS_CAPT_CH_PER_PORT_MAX *
                CAMERA_LINK_FRAMES_PER_CH * streamId +
                CAMERA_LINK_FRAMES_PER_CH * chId;

            if (idx >= CAMERA_LINK_MAX_FRAMES_PER_HANDLE)
            {
                idx = 0u;
            }

            pFrames = &pDrvObj->frames[idx];

            /* fill format with channel specific values */
            format.channelNum =
                pDrvObj->createArgs.channelNumMap[streamId][chId];
            format.width = pOutPrm->scOutWidth;
            format.height = pOutPrm->scOutHeight;
            format.pitch[0] = pOutPrm->scOutWidth;
            if(CameraLink_isYUV422IFormat(pOutInfo->dataFormat) == 1)
                format.pitch[0] *= 2;
            if (pOutInfo->dataFormat == FVID2_DF_RGB24_888)
                format.pitch[0] *= 3;
            format.pitch[1] = format.pitch[0];
            format.pitch[2] = 0;
            format.fieldMerged[0] = FALSE;
            format.fieldMerged[1] = FALSE;
            format.fieldMerged[2] = FALSE;
            format.dataFormat = pOutInfo->dataFormat;
            format.scanFormat = FVID2_SF_PROGRESSIVE;
            format.bpp = FVID2_BPP_BITS8;                  /* ignored */

            if (format.dataFormat == FVID2_DF_RAW_VBI)
            {
                format.height = CAMERA_LINK_RAW_VBI_LINES;
            }

            if (CameraLink_drvIsDataFormatTiled(&pDrvObj->createArgs, streamId))
            {
                /*
                 * cannot free tiled frames
                 */
                tilerUsed = TRUE;
            }
            else
            {
                /*
                 * free frames for this channel, based on pFormat
                 */
                Utils_memFrameFree(&format, pFrames,
                                      CAMERA_LINK_FRAMES_PER_CH);
            }
        }
    }

    if (tilerUsed)
    {
        SystemTiler_freeAll();
    }

    return FVID2_SOK;
}

/* ===================================================================
 *  @func     CameraLink_drvIsDataFormatTiled
 *
 *  @desc     Function does the following
 *
 *  @modif    This function modifies the following structures
 *
 *  @inputs   This function takes the following inputs
 *            <argument name>
 *            Description of usage
 *            <argument name>
 *            Description of usage
 *
 *  @outputs  <argument name>
 *            Description of usage
 *
 *  @return   Return value of this function if any
 *  ==================================================================
 */
UInt32 CameraLink_drvIsDataFormatTiled(Iss_CaptCreateParams * createArgs,
                                       UInt16 streamId)
{
    Iss_CaptOutInfo *pOutInfo;

    pOutInfo = &createArgs->outStreamInfo[streamId];

    if ((pOutInfo->dataFormat == FVID2_DF_YUV420SP_UV ||
         pOutInfo->dataFormat == FVID2_DF_YUV422SP_UV)
        && pOutInfo->memType == ISS_TILEDMEM)
    {
        return TRUE;
    }

    return FALSE;
}

/*
 * Init create arguments to default values
 *
 * createArgs - create arguments */
/* ===================================================================
 *  @func     CameraLink_drvInitCreateArgs
 *
 *  @desc     Function does the following
 *
 *  @modif    This function modifies the following structures
 *
 *  @inputs   This function takes the following inputs
 *            <argument name>
 *            Description of usage
 *            <argument name>
 *            Description of usage
 *
 *  @outputs  <argument name>
 *            Description of usage
 *
 *  @return   Return value of this function if any
 *  ==================================================================
 */
Int32 CameraLink_drvInitCreateArgs(Iss_CaptCreateParams * createArgs,
                                   CameraLink_VipInstParams * pInstPrm)
{
    UInt16 chId, StreamId;

    Iss_CaptOutInfo *pOutInfo;

    Iss_CaptScParams *pScParams[ISS_CAPT_STREAM_ID_MAX];

    CameraLink_OutParams *pOutPrm;

    memset(createArgs, 0, sizeof(*createArgs));

    createArgs->videoCaptureMode = ISS_CAPT_VIDEO_CAPTURE_MODE_SINGLE_STREAM;

    createArgs->videoIfMode = ISS_CAPT_BAYER_12BIT;

    createArgs->inDataFormat = FVID2_DF_YUV422I_UYVY; //FVID2_DF_YUV422I_VYUY;
    createArgs->periodicCallbackEnable = FALSE;
    createArgs->numCh = 1;
    createArgs->numStream = 2;

    for (StreamId = 0; StreamId < createArgs->numStream; StreamId++)
    {
        pOutPrm = &pInstPrm->outParams[StreamId];
        pScParams[StreamId] = &createArgs->scParams[StreamId];
        pScParams[StreamId]->inWidth = pOutPrm->scOutWidth;
        pScParams[StreamId]->inHeight = pOutPrm->scOutHeight;
        pScParams[StreamId]->inCropCfg.cropStartX = 0;
        pScParams[StreamId]->inCropCfg.cropStartY = 0;
        pScParams[StreamId]->inCropCfg.cropWidth = pScParams[StreamId]->inWidth;
        pScParams[StreamId]->inCropCfg.cropHeight =
            pScParams[StreamId]->inHeight;
        pScParams[StreamId]->outWidth = pScParams[StreamId]->inWidth;
        pScParams[StreamId]->outHeight = pScParams[StreamId]->inHeight;
        pScParams[StreamId]->scConfig = NULL;
        pScParams[StreamId]->scCoeffConfig = NULL;
        createArgs->pitch[StreamId] = pOutPrm->scOutWidth;
    }
    for (StreamId = 0; StreamId < ISS_CAPT_STREAM_ID_MAX; StreamId++)
    {
        pOutInfo = &createArgs->outStreamInfo[StreamId];

        pOutInfo->memType = ISS_NONTILEDMEM;

        pOutInfo->dataFormat = FVID2_DF_INVALID;

        pOutInfo->scEnable = FALSE;
        pOutInfo->subFrameModeEnable = FALSE;
        pOutInfo->numLinesInSubFrame = 0;
        pOutInfo->subFrameCb = NULL;

        if (StreamId == 0 || StreamId == 1)
        {
            pOutInfo->dataFormat = FVID2_DF_YUV422I_UYVY; //FVID2_DF_YUV422I_VYUY;
        }

        for (chId = 0; chId < ISS_CAPT_CH_PER_PORT_MAX; chId++)
        {
            createArgs->channelNumMap[StreamId][chId] =
                Iss_captMakeChannelNum(0, StreamId, chId);
        }
    }

    return 0;
}

/* ===================================================================
 *  @func     CameraLink_getCpuLoad
 *
 *  @desc     Function does the following
 *
 *  @modif    This function modifies the following structures
 *
 *  @inputs   This function takes the following inputs
 *            <argument name>
 *            Description of usage
 *            <argument name>
 *            Description of usage
 *
 *  @outputs  <argument name>
 *            Description of usage
 *
 *  @return   Return value of this function if any
 *  ==================================================================
 */
Int32 CameraLink_getCpuLoad()
{
    gCameraLink_obj.totalCpuLoad += Load_getCPULoad();
    gCameraLink_obj.cpuLoadCount++;

    return 0;
}

/* ===================================================================
 *  @func     CameraLink_drvPrintStatus
 *
 *  @desc     Function does the following
 *
 *  @modif    This function modifies the following structures
 *
 *  @inputs   This function takes the following inputs
 *            <argument name>
 *            Description of usage
 *            <argument name>
 *            Description of usage
 *
 *  @outputs  <argument name>
 *            Description of usage
 *
 *  @return   Return value of this function if any
 *  ==================================================================
 */
Int32 CameraLink_drvPrintStatus(CameraLink_Obj * pObj)
{
    UInt32 fps;

    FVID2_control(pObj->fvidHandleVipAll,
                  IOCTL_ISS_CAPT_PRINT_ADV_STATISTICS,
                  (Ptr) pObj->exeTime, NULL);

    fps = (pObj->cameraDequeuedFrameCount * 100) / (pObj->exeTime / 10);

    Vps_printf(" %d: CAMERA: Fields = %d (fps = %d, CPU Load = %d)\r\n",
               Clock_getTicks(),
               pObj->cameraDequeuedFrameCount,
               fps, pObj->totalCpuLoad / pObj->cpuLoadCount);

    Vps_printf(" %d: CAMERA: Num Resets = %d (Avg %d ms per reset)\r\n",
               Clock_getTicks(),
               pObj->resetCount, pObj->resetTotalTime / pObj->resetCount);

    return 0;
}

/* ===================================================================
 *  @func     CameraLink_drvSetColor
 *
 *  @desc     Function does the following
 *
 *  @modif    This function modifies the following structures
 *
 *  @inputs   This function takes the following inputs
 *            <argument name>
 *            Description of usage
 *            <argument name>
 *            Description of usage
 *
 *  @outputs  <argument name>
 *            Description of usage
 *
 *  @return   Return value of this function if any
 *  ==================================================================
 */
Int32 CameraLink_drvSetColor(CameraLink_Obj * pObj, Int32 contrast,
                             Int32 brightness, Int32 saturation, Int32 hue)
{
#if 0
    CameraLink_VipInstParams *pInstPrm;

    CameraLink_InstObj *pInst;

    Iss_VideoDecoderColorParams colorPrm;

    Int32 instId, chId, status;

    pObj->brightness += brightness;
    pObj->contrast += contrast;
    pObj->saturation += saturation;

    if (pObj->brightness < 0)
        pObj->brightness = 0;
    if (pObj->brightness > 255)
        pObj->brightness = 255;

    if (pObj->contrast < 0)
        pObj->contrast = 0;
    if (pObj->contrast > 255)
        pObj->contrast = 255;

    if (pObj->saturation < 0)
        pObj->saturation = 0;
    if (pObj->saturation > 255)
        pObj->saturation = 255;

    for (instId = 0; instId < pObj->createArgs.numVipInst; instId++)
    {
        pInstPrm = &pObj->createArgs.vipInst[instId];
        pInst = &pObj->instObj[instId];
    }
#endif
    return FVID2_SOK;
}
