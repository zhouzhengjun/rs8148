/*******************************************************************************
 *                                                                             *
 * Copyright (c) 2009 Texas Instruments Incorporated - http://www.ti.com/      *
 *                        ALL RIGHTS RESERVED                                  *
 *                                                                             *
 ******************************************************************************/

#include <avcap/common/chains.h>

/*
H265/h264 compare demostration case
                      Capture (YUV422I) 1CH 1080P 24fps
                          |
                          |
                        NSF (YUV420SP)
                          |
						DUP ---------------------ipcFrameOutVpssToHost---------ipcFrameInHost
						  |	                                                         |
						  |															 |
						ENCH264                                                  ENCH265
						  |															 |
						  |															 |
						DECH264													 DECH265
                          |															 |
						  |															 |
                      SW Mosaic -----------------ipcFrameInVpssFromHost--------ipcFrameOutHost
                      (DEIH YUV422I)
                          |
                          |
                          |
                    Off-Chip HDMI
                      1080p60
					  
H265/AAC-LC encoder production case
                      Capture (YUV422I) 1CH 1080P 24fps
                          |
                          |
                        NSF (YUV420SP)
						  |
						  |
						DUP ---------------------ipcFrameOutVpssToHost---------ipcFrameInHost
						  |
						  |
                      SW Mosaic (for debug)
                      (DEIH YUV422I)
                          |
                          |
                          |
                    Off-Chip HDMI(for debug)
                      1080p60						    
*/
	void framecallback(void *ctx)
	{
	int i;
    Int status;
	VIDFrame_BufList  vidBufList;
		
	//	printf("frame in callback called\n");
		    vidBufList.numFrames = 0;
		    IpcFramesInLink_getFullVideoFrames(SYSTEM_HOST_LINK_ID_IPC_FRAMES_IN_0,&vidBufList);	
			if (vidBufList.numFrames)
		    {
				for(i=0;i<vidBufList.numFrames;i++)
				{
			//			printf("got frame from que,width=%d, height=%d\n",vidBufList.frames[i].frameWidth,vidBufList.frames[i].frameHeight );
				}			
				status =IpcFramesInLink_putEmptyVideoFrames(SYSTEM_HOST_LINK_ID_IPC_FRAMES_IN_0,&vidBufList);
			}
		OSA_waitMsecs(100);	
	}	
	
static SystemVideo_Ivahd2ChMap_Tbl systemVid_encDecIvaChMapTbl =
{
    .isPopulated = 1,
    .ivaMap[0] =
    {
        .EncNumCh  = 1,
        .EncChList = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13 , 14, 15},
        .DecNumCh  = 1,
        .DecChList = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13 , 14, 15},
    },

};

IpcFramesInCbFcn gFrameCB=framecallback;
void *gFrameCBCtx=NULL;
void Chains_CreateGraph(Chains_Ctrl *chainsCfg)
{

}
Void Chains_multiChCaptureNsfDei(Chains_Ctrl *chainsCfg)
{
	Device_Sii9135Handle sii9135Handle;
    CaptureLink_CreateParams    capturePrm;
    NsfLink_CreateParams        nsfPrm;
    SwMsLink_CreateParams       swMsPrm;
    DisplayLink_CreateParams    displayPrm;
    IpcLink_CreateParams     ipcOutVpssPrm;
    IpcLink_CreateParams     ipcInVpssPrm;
    IpcLink_CreateParams     ipcOutVideoPrm;
    IpcLink_CreateParams     ipcInVideoPrm;
    EncLink_CreateParams     encPrm;
    DecLink_CreateParams     decPrm;	
    IpcBitsOutLinkHLOS_CreateParams   ipcBitsOutHostPrm;
    IpcBitsOutLinkRTOS_CreateParams   ipcBitsOutVideoPrm;
    IpcBitsInLinkHLOS_CreateParams    ipcBitsInHostPrm;
    IpcBitsInLinkRTOS_CreateParams    ipcBitsInVideoPrm;	
//	MergeLink_CreateParams		mergePrm;
	DupLink_CreateParams		dupPrm;
	IpcFramesOutLinkRTOS_CreateParams  ipcFramesOutVpssToHostPrm;
	IpcFramesInLinkHLOS_CreateParams   ipcFramesInHostPrm;
//	IpcFramesOutLinkHLOS_CreateParams  ipcFramesOutHostPrm;
//	IpcFramesInLinkRTOS_CreateParams   ipcFramesInVpssFromHostPrm;		

    CaptureLink_VipInstParams *pCaptureInstPrm;
    CaptureLink_OutParams     *pCaptureOutPrm;

    UInt32 captureId, nsfId, swMsId, displayId, dupId, m3out, a8in;
    UInt32 vipInstId;
    UInt32 encId, decId;
    UInt32 ipcOutVpssId, ipcInVpssId;
    UInt32 ipcOutVideoId, ipcInVideoId;
    UInt32 ipcBitsOutVideoId, ipcBitsInHostId;
    UInt32 ipcBitsInVideoId, ipcBitsOutHostId;	
	Int32                                i;	
    Bool switchCh;
    Bool switchLayout;
    System_LinkInfo bitsProducerLinkInfo;

    char ch;
#if 0
	System_linkControl(
		SYSTEM_LINK_ID_M3VPSS,
		SYSTEM_M3VPSS_CMD_RESET_VIDEO_DEVICES,
		NULL,
		0,
		TRUE
		);
#endif		
    captureId   = SYSTEM_LINK_ID_CAPTURE;
    nsfId       = SYSTEM_LINK_ID_NSF_0;
    swMsId      = SYSTEM_LINK_ID_SW_MS_MULTI_INST_0;
    displayId   = SYSTEM_LINK_ID_DISPLAY_0;
	dupId = SYSTEM_VPSS_LINK_ID_DUP_0;
	m3out = SYSTEM_VPSS_LINK_ID_IPC_FRAMES_OUT_1;
	a8in = SYSTEM_HOST_LINK_ID_IPC_FRAMES_IN_0;
    encId  = SYSTEM_LINK_ID_VENC_0;
    decId  = SYSTEM_LINK_ID_VDEC_0;
    ipcOutVpssId = SYSTEM_VPSS_LINK_ID_IPC_OUT_M3_0;
    ipcInVideoId = SYSTEM_VIDEO_LINK_ID_IPC_IN_M3_0;
    ipcOutVideoId= SYSTEM_VIDEO_LINK_ID_IPC_OUT_M3_0;
    ipcInVpssId  = SYSTEM_VPSS_LINK_ID_IPC_IN_M3_0;
    ipcBitsOutVideoId = SYSTEM_VIDEO_LINK_ID_IPC_BITS_OUT_0;
    ipcBitsInHostId   = SYSTEM_HOST_LINK_ID_IPC_BITS_IN_0;
    ipcBitsOutHostId  = SYSTEM_HOST_LINK_ID_IPC_BITS_OUT_0;
    ipcBitsInVideoId  = SYSTEM_VIDEO_LINK_ID_IPC_BITS_IN_0;


    System_linkControl(
        SYSTEM_LINK_ID_M3VIDEO,
        SYSTEM_COMMON_CMD_SET_CH2IVAHD_MAP_TBL,
        &systemVid_encDecIvaChMapTbl,
        sizeof(SystemVideo_Ivahd2ChMap_Tbl),
        TRUE
    );
    Chains_ipcBitsInit();
    CaptureLink_CreateParams_Init(&capturePrm);
    SwMsLink_CreateParams_Init(&swMsPrm);
	DisplayLink_CreateParams_Init(&displayPrm);
	NsfLink_CreateParams_Init(&nsfPrm);
	CHAINS_INIT_STRUCT(IpcFramesOutLinkRTOS_CreateParams ,ipcFramesOutVpssToHostPrm);
	CHAINS_INIT_STRUCT(IpcFramesInLinkHLOS_CreateParams  ,ipcFramesInHostPrm);
    CHAINS_INIT_STRUCT(DecLink_CreateParams, decPrm);
    CHAINS_INIT_STRUCT(EncLink_CreateParams, encPrm);
    CHAINS_INIT_STRUCT(IpcLink_CreateParams,ipcOutVpssPrm);
    CHAINS_INIT_STRUCT(IpcLink_CreateParams,ipcInVpssPrm);
    CHAINS_INIT_STRUCT(IpcLink_CreateParams,ipcOutVideoPrm);
    CHAINS_INIT_STRUCT(IpcLink_CreateParams,ipcInVideoPrm);	
    CHAINS_INIT_STRUCT(IpcBitsOutLinkHLOS_CreateParams,ipcBitsOutHostPrm);
    CHAINS_INIT_STRUCT(IpcBitsOutLinkRTOS_CreateParams,ipcBitsOutVideoPrm);
    CHAINS_INIT_STRUCT(IpcBitsInLinkHLOS_CreateParams,ipcBitsInHostPrm);
    CHAINS_INIT_STRUCT(IpcBitsInLinkRTOS_CreateParams,ipcBitsInVideoPrm);
	
    capturePrm.numVipInst = 1;
    capturePrm.outQueParams[0].nextLink = dupId;
    capturePrm.tilerEnable              = FALSE;
    capturePrm.enableSdCrop             = FALSE;
    for(vipInstId=0; vipInstId<capturePrm.numVipInst; vipInstId++)
    {
        pCaptureInstPrm                     = &capturePrm.vipInst[vipInstId];
        pCaptureInstPrm->vipInstId          = (SYSTEM_CAPTURE_INST_VIP1_PORTA+vipInstId)%SYSTEM_CAPTURE_INST_MAX;
 //       pCaptureInstPrm->videoDecoderId     = SYSTEM_DEVICE_VID_DEC_SII9135_DRV;
        pCaptureInstPrm->inDataFormat       = SYSTEM_DF_YUV422P;
        pCaptureInstPrm->standard           = SYSTEM_STD_1080P_60;
        pCaptureInstPrm->numOutput          = 1;

        pCaptureOutPrm                      = &pCaptureInstPrm->outParams[0];
        pCaptureOutPrm->dataFormat          = SYSTEM_DF_YUV422I_YUYV;
        pCaptureOutPrm->scEnable            = FALSE;
        pCaptureOutPrm->scOutWidth          = 0;
        pCaptureOutPrm->scOutHeight         = 0;
        pCaptureOutPrm->outQueId            = 0;
    }
#if 1
    {
        Int32 status = 0;
        Device_VideoDecoderChipIdParams      vidDecChipIdArgs;
        Device_VideoDecoderChipIdStatus      vidDecChipIdStatus;
        VCAP_VIDEO_SOURCE_STATUS_PARAMS_S    videoStatusArgs;
        VCAP_VIDEO_SOURCE_CH_STATUS_S        videoStatus;

        Device_VideoDecoderCreateParams      createArgs;
        Device_VideoDecoderCreateStatus      createStatusArgs;

        Device_VideoDecoderVideoModeParams                 vidDecVideoModeArgs;


        /* Initialize and create video decoders */
        Device_sii9135Init();

        memset(&createArgs, 0, sizeof(Device_VideoDecoderCreateParams));

        createArgs.deviceI2cInstId    = 0;
        createArgs.numDevicesAtPort   = 1;
        createArgs.deviceI2cAddr[0]   = Device_getVidDecI2cAddr(
                                                             DEVICE_VID_DEC_SII9135_DRV,
                                                             0);
        createArgs.deviceResetGpio[0] = DEVICE_VIDEO_DECODER_GPIO_NONE;

        sii9135Handle = Device_sii9135Create(          DEVICE_VID_DEC_SII9135_DRV,
                                                         0, // instId - need to change
                                                         &(createArgs),
                                                         &(createStatusArgs));


        vidDecChipIdArgs.deviceNum = 0;

        status = Device_sii9135Control(sii9135Handle,
                                           IOCTL_DEVICE_VIDEO_DECODER_GET_CHIP_ID,
                                           &vidDecChipIdArgs,
                                           &vidDecChipIdStatus);
        if (status >= 0)
        {
                videoStatusArgs.channelNum = 0;

                status = Device_sii9135Control(sii9135Handle,
                                               IOCTL_DEVICE_VIDEO_DECODER_GET_VIDEO_STATUS,
                                               &videoStatusArgs, &videoStatus);

                if (videoStatus.isVideoDetect)
                {
                    printf(" VCAP: SII9135 (0x%02x): Detected video (%dx%d@%dHz, %d) !!!\n",
                               createArgs.deviceI2cAddr[0],
                               videoStatus.frameWidth,
                               videoStatus.frameHeight,
                               1000000 / videoStatus.frameInterval,
                               videoStatus.isInterlaced);
                }
                else
                {
                    printf(" VCAP: SII9135 (0x%02x):  NO Video Detected !!!\n", createArgs.deviceI2cAddr[0]);
                }
        }
        else
        {
                printf(" VCAP: SII9135 (0x%02x): Device not found !!!\n", createArgs.deviceI2cAddr[0]);
		}
		/* Configure video decoder */

        memset(&vidDecVideoModeArgs,0, sizeof(Device_VideoDecoderVideoModeParams));
		
        vidDecVideoModeArgs.videoIfMode        = DEVICE_CAPT_VIDEO_IF_MODE_16BIT;
        vidDecVideoModeArgs.videoDataFormat    = SYSTEM_DF_YUV422P;
        vidDecVideoModeArgs.standard           = SYSTEM_STD_1080P_60;
        vidDecVideoModeArgs.videoCaptureMode   = DEVICE_CAPT_VIDEO_CAPTURE_MODE_SINGLE_CH_NON_MUX_EMBEDDED_SYNC;
        vidDecVideoModeArgs.videoSystem        = DEVICE_VIDEO_DECODER_VIDEO_SYSTEM_NONE;
        vidDecVideoModeArgs.videoCropEnable    = FALSE;
        vidDecVideoModeArgs.videoAutoDetectTimeout = -1;

        status = Device_sii9135Control(sii9135Handle,IOCTL_DEVICE_VIDEO_DECODER_SET_VIDEO_MODE,&vidDecVideoModeArgs,NULL);

    }
#endif

	dupPrm.inQueParams.prevLinkId = captureId;
	dupPrm.inQueParams.prevLinkQueId= 0;
	dupPrm.outQueParams[0].nextLink = nsfId;
	dupPrm.outQueParams[1].nextLink = m3out;
   	dupPrm.numOutQue				   = 2;
   	dupPrm.notifyNextLink			   = TRUE;

	//dup1
	ipcFramesOutVpssToHostPrm.baseCreateParams.inQueParams.prevLinkId = dupId;
	ipcFramesOutVpssToHostPrm.baseCreateParams.inQueParams.prevLinkQueId = 1;		
    ipcFramesOutVpssToHostPrm.baseCreateParams.noNotifyMode = TRUE;
    ipcFramesOutVpssToHostPrm.baseCreateParams.notifyNextLink = FALSE;
    ipcFramesOutVpssToHostPrm.baseCreateParams.notifyPrevLink = TRUE;		
	ipcFramesOutVpssToHostPrm.baseCreateParams.outQueParams[0].nextLink=a8in;
	
	ipcFramesInHostPrm.baseCreateParams.inQueParams.prevLinkId = m3out;
	ipcFramesInHostPrm.baseCreateParams.inQueParams.prevLinkQueId = 0;
    ipcFramesInHostPrm.baseCreateParams.noNotifyMode = TRUE;
    ipcFramesInHostPrm.baseCreateParams.notifyNextLink = FALSE;
    ipcFramesInHostPrm.baseCreateParams.notifyPrevLink = FALSE;
    ipcFramesInHostPrm.baseCreateParams.outQueParams[0].nextLink = SYSTEM_LINK_ID_INVALID;
    ipcFramesInHostPrm.exportOnlyPhyAddr = TRUE;

    ipcFramesInHostPrm.cbCtx = gFrameCBCtx;
    ipcFramesInHostPrm.cbFxn = gFrameCB;

	//encode then decode for demo
	//dup0
    nsfPrm.bypassNsf                = TRUE;
    nsfPrm.tilerEnable              = FALSE;
    nsfPrm.inQueParams.prevLinkId   = dupId;
    nsfPrm.inQueParams.prevLinkQueId= 0;
    nsfPrm.numOutQue                = 1;
    nsfPrm.outQueParams[0].nextLink = ipcOutVpssId;
		
    ipcOutVpssPrm.inQueParams.prevLinkId    = nsfId;
    ipcOutVpssPrm.inQueParams.prevLinkQueId = 0;
    ipcOutVpssPrm.numOutQue                 = 1;
    ipcOutVpssPrm.outQueParams[0].nextLink     = ipcInVideoId;
    ipcOutVpssPrm.notifyNextLink            = TRUE;
    ipcOutVpssPrm.notifyPrevLink            = TRUE;
    ipcOutVpssPrm.noNotifyMode              = FALSE;

    ipcInVideoPrm.inQueParams.prevLinkId    = ipcOutVpssId;
    ipcInVideoPrm.inQueParams.prevLinkQueId = 0;
    ipcInVideoPrm.numOutQue                 = 1;
    ipcInVideoPrm.outQueParams[0].nextLink     = encId;
    ipcInVideoPrm.notifyNextLink            = TRUE;
    ipcInVideoPrm.notifyPrevLink            = TRUE;
    ipcInVideoPrm.noNotifyMode              = FALSE;

    for (i=0; i<1; i++) {
        encPrm.chCreateParams[i].format     = IVIDEO_H264HP;
        encPrm.chCreateParams[i].profile    = IH264_HIGH_PROFILE;
        encPrm.chCreateParams[i].dataLayout = IVIDEO_FIELD_SEPARATED;
        encPrm.chCreateParams[i].fieldMergeEncodeEnable  = FALSE;
        encPrm.chCreateParams[i].maxBitRate = 100 * 1000;
        encPrm.chCreateParams[i].enableHighSpeed = 1;
        encPrm.chCreateParams[i].defaultDynamicParams.intraFrameInterval = 50;
        encPrm.chCreateParams[i].encodingPreset = XDM_USER_DEFINED;
        encPrm.chCreateParams[i].enableAnalyticinfo = 0;
        encPrm.chCreateParams[i].enableWaterMarking = 0;
        encPrm.chCreateParams[i].rateControlPreset =IVIDEO_LOW_DELAY;
	
        encPrm.chCreateParams[i].defaultDynamicParams.inputFrameRate = 30;
        encPrm.chCreateParams[i].defaultDynamicParams.targetBitRate =(100 * 1000);
        encPrm.chCreateParams[i].defaultDynamicParams.interFrameInterval = 1;
        encPrm.chCreateParams[i].defaultDynamicParams.mvAccuracy =IVIDENC2_MOTIONVECTOR_QUARTERPEL;
        encPrm.chCreateParams[i].defaultDynamicParams.rcAlg = IH264_RATECONTROL_PRC_LOW_DELAY ;
        encPrm.chCreateParams[i].defaultDynamicParams.qpMin = 10;
        encPrm.chCreateParams[i].defaultDynamicParams.qpMax = 40;
        encPrm.chCreateParams[i].defaultDynamicParams.qpInit = -1;
        encPrm.chCreateParams[i].defaultDynamicParams.vbrDuration = 0;
        encPrm.chCreateParams[i].defaultDynamicParams.vbrSensitivity = 0;
    }
    encPrm.inQueParams.prevLinkId   = ipcInVideoId;
    encPrm.inQueParams.prevLinkQueId= 0;
    encPrm.outQueParams.nextLink = ipcBitsOutVideoId;
    encPrm.numBufPerCh[0] = 4;

    ipcBitsOutVideoPrm.baseCreateParams.inQueParams.prevLinkId = encId;
    ipcBitsOutVideoPrm.baseCreateParams.inQueParams.prevLinkQueId = 0;
    ipcBitsOutVideoPrm.baseCreateParams.numOutQue                 = 1;
    ipcBitsOutVideoPrm.baseCreateParams.outQueParams[0].nextLink = ipcBitsInHostId;
    Chains_ipcBitsInitCreateParams_BitsOutRTOS(&ipcBitsOutVideoPrm,TRUE);

    ipcBitsInHostPrm.baseCreateParams.inQueParams.prevLinkId = ipcBitsOutVideoId;
    ipcBitsInHostPrm.baseCreateParams.inQueParams.prevLinkQueId = 0;
    Chains_ipcBitsInitCreateParams_BitsInHLOS(&ipcBitsInHostPrm);
	
    System_linkCreate (captureId, &capturePrm, sizeof(capturePrm));
 //   System_linkControl(captureId, CAPTURE_LINK_CMD_CONFIGURE_VIP_DECODERS, NULL, 0, TRUE);
	System_linkCreate(dupId, &dupPrm, sizeof(dupPrm));	
    System_linkCreate(nsfId     , &nsfPrm, sizeof(nsfPrm));
    System_linkCreate(ipcOutVpssId , &ipcOutVpssPrm , sizeof(ipcOutVpssPrm) );
    System_linkCreate(ipcInVideoId , &ipcInVideoPrm , sizeof(ipcInVideoPrm) );
    System_linkCreate(encId, &encPrm, sizeof(encPrm));
    System_linkCreate(ipcBitsOutVideoId, &ipcBitsOutVideoPrm, sizeof(ipcBitsOutVideoPrm));
    System_linkCreate(ipcBitsInHostId, &ipcBitsInHostPrm, sizeof(ipcBitsInHostPrm));


    System_linkGetInfo(ipcBitsInHostId,&bitsProducerLinkInfo);
    OSA_assert(bitsProducerLinkInfo.numQue = 1);
    ipcBitsOutHostPrm.baseCreateParams.numOutQue                = 1;
    ipcBitsOutHostPrm.baseCreateParams.outQueParams[0].nextLink = ipcBitsInVideoId;
    Chains_ipcBitsInitCreateParams_BitsOutHLOS(&ipcBitsOutHostPrm,&bitsProducerLinkInfo.queInfo[0]);
    ipcBitsInVideoPrm.baseCreateParams.inQueParams.prevLinkId = ipcBitsOutHostId;
    ipcBitsInVideoPrm.baseCreateParams.inQueParams.prevLinkQueId = 0;
    ipcBitsInVideoPrm.baseCreateParams.numOutQue                 = 1;
    ipcBitsInVideoPrm.baseCreateParams.outQueParams[0].nextLink = decId;
    Chains_ipcBitsInitCreateParams_BitsInRTOS(&ipcBitsInVideoPrm,TRUE);
											  
    for (i=0; i<1; i++) {
        decPrm.chCreateParams[i].format          = IVIDEO_H264HP;
        decPrm.chCreateParams[i].profile         = IH264VDEC_PROFILE_ANY;
        decPrm.chCreateParams[i].targetMaxWidth  = 1920;
        decPrm.chCreateParams[i].targetMaxHeight = 1080;
        decPrm.chCreateParams[i].fieldMergeDecodeEnable  = FALSE;
        decPrm.chCreateParams[i].numBufPerCh = 4;
        decPrm.chCreateParams[i].defaultDynamicParams.targetFrameRate = 30;
        decPrm.chCreateParams[i].defaultDynamicParams.targetBitRate =(100 * 1000);
    }
    decPrm.inQueParams.prevLinkId = encId;
    decPrm.inQueParams.prevLinkQueId = 0;
    decPrm.outQueParams.nextLink     = ipcOutVideoId;

    ipcOutVideoPrm.inQueParams.prevLinkId    = decId;
    ipcOutVideoPrm.inQueParams.prevLinkQueId = 0;
    ipcOutVideoPrm.numOutQue                   = 1;
    ipcOutVideoPrm.outQueParams[0].nextLink     = ipcInVpssId;
    ipcOutVideoPrm.notifyNextLink            = TRUE;
    ipcOutVideoPrm.notifyPrevLink            = TRUE;
    ipcOutVideoPrm.noNotifyMode              = FALSE;

    ipcInVpssPrm.inQueParams.prevLinkId    = ipcOutVideoId;
    ipcInVpssPrm.inQueParams.prevLinkQueId = 0;
    ipcInVpssPrm.numOutQue                    = 1;
    ipcInVpssPrm.outQueParams[0].nextLink     = swMsId;
    ipcInVpssPrm.notifyNextLink            = TRUE;
    ipcInVpssPrm.notifyPrevLink            = TRUE;
    ipcInVpssPrm.noNotifyMode              = FALSE;
	
    swMsPrm.inQueParams.prevLinkId = ipcInVpssId;
    swMsPrm.numSwMsInst = 1;
//    swMsPrm.swMsInstId[0] = SYSTEM_SW_MS_SC_INST_DEI_SC;
	swMsPrm.swMsInstId[0] = SYSTEM_SW_MS_SC_INST_DEI_SC_NO_DEI;
    swMsPrm.inQueParams.prevLinkQueId = 0;
    swMsPrm.outQueParams.nextLink     = displayId;
    swMsPrm.maxInputQueLen            = 4;
    swMsPrm.maxOutRes                 = chainsCfg->displayRes[SYSTEM_DC_VENC_HDMI];
	swMsPrm.numOutBuf                 = 8;
    swMsPrm.lineSkipMode              = TRUE;
    swMsPrm.layoutPrm.outputFPS       = 60;

    Chains_swMsGenerateLayoutParams(0, 2, &swMsPrm);


    displayPrm.inQueParams[0].prevLinkId    = swMsId;
    displayPrm.inQueParams[0].prevLinkQueId = 0;
    displayPrm.displayRes                = swMsPrm.maxOutRes;

    Chains_displayCtrlInit(chainsCfg->displayRes);


    System_linkCreate(decId, &decPrm, sizeof(decPrm));
    System_linkCreate(ipcOutVideoId, &ipcOutVideoPrm, sizeof(ipcOutVideoPrm));
    System_linkCreate(ipcInVpssId  , &ipcInVpssPrm  , sizeof(ipcInVpssPrm)  );	
	System_linkCreate(m3out, &ipcFramesOutVpssToHostPrm, sizeof(ipcFramesOutVpssToHostPrm));
	System_linkCreate(a8in, &ipcFramesInHostPrm, sizeof(ipcFramesInHostPrm));
    System_linkCreate(swMsId   , &swMsPrm, sizeof(swMsPrm));
    System_linkCreate(displayId, &displayPrm, sizeof(displayPrm));

    Chains_memPrintHeapStatus();

    {

        /* Start taking CPU load just before starting of links */
        Chains_prfLoadCalcEnable(TRUE, FALSE, FALSE);
		
        System_linkStart(displayId);
        System_linkStart(swMsId   );	
        System_linkStart(encId);
        System_linkStart(decId);
        System_linkStart(nsfId    );		
        System_linkStart(dupId   );	
        System_linkStart(m3out   );		
        System_linkStart(a8in   );			
        System_linkStart(captureId);
	    Device_sii9135Control(sii9135Handle,DEVICE_CMD_START,NULL,NULL);
        while(1)
        {
            switchLayout = FALSE;
            switchCh     = FALSE;

            ch = Chains_menuRunTime();

            if(ch=='0')
                break;
            if(ch=='p')
                System_linkControl(captureId, CAPTURE_LINK_CMD_PRINT_ADV_STATISTICS, NULL, 0, TRUE);
            if(ch=='i')
            {
                System_linkControl(encId,
                       ENC_LINK_CMD_PRINT_IVAHD_STATISTICS, NULL, 0, TRUE);
            }				
            if(ch=='s')
               switchLayout = TRUE;
            if(ch=='c')
               switchCh = TRUE;

            Chains_swMsSwitchLayout(&swMsId, &swMsPrm, switchLayout, switchCh, 1);
        }
	    Device_sii9135Control(sii9135Handle,DEVICE_CMD_STOP,NULL,NULL);	
        System_linkStop(captureId);
		printf(" CHAINS  : capture stoped !!! \r\n");			
        System_linkStop(dupId   );	
        System_linkStop(m3out   );		
        System_linkStop(a8in   );
        System_linkStop(nsfId    );
	//	printf(" CHAINS  : nsf stoped !!! \r\n");	
        System_linkStop(encId);
		        Chains_ipcBitsStop();
        System_linkStop(decId);	
        System_linkStop(swMsId    );
        System_linkStop(displayId);
	
    }

    System_linkDelete(captureId);
    System_linkDelete(nsfId    );
    System_linkDelete(dupId    );
    System_linkDelete(m3out    );
    System_linkDelete(a8in    );	
    System_linkDelete(ipcOutVpssId );
    System_linkDelete(ipcInVideoId );
    System_linkDelete(encId);
    System_linkDelete(decId);
    System_linkDelete(ipcOutVideoId);
    System_linkDelete(ipcInVpssId  );	
    System_linkDelete(swMsId   );
    System_linkDelete(displayId);

    Chains_displayCtrlDeInit();
	
    Device_sii9135Delete(sii9135Handle, NULL);
    Device_sii9135DeInit();	
    /* Print the HWI, SWI and all tasks load */
    /* Reset the accumulated timer ticks */
    Chains_prfLoadCalcEnable(FALSE, TRUE, FALSE);
	    Chains_ipcBitsExit();
}

