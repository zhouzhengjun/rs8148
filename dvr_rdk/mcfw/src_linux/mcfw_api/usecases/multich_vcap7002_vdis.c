/*******************************************************************************
 *                                                                             *
 * Copyright (c) 2013 BroadEng - http://www.broadeng.net						  *
 *                        ALL RIGHTS RESERVED                                  *
 *                                                                             *
 ******************************************************************************/
/*
                        Capture (YUV422I) 1CH 720P@60fps
                          |
                          |
                  GRPX0   |
				       |    |
                  On-Chip HDMI
                    1080p60   
*/

#include "multich_common.h"
#include "multich_ipcbits.h"

/* =============================================================================
 * Externs
 * =============================================================================
 */

#define     NUM_CAPTURE_DEVICES          (1)

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


/* =============================================================================
 * Use case code
 * =============================================================================
 */

Void MultiCh_createVcap7002Vdis()
{
    CaptureLink_CreateParams    capturePrm;
    DisplayLink_CreateParams    displayPrm[VDIS_DEV_MAX];
    IpcLink_CreateParams        ipcOutVpssPrm;
    IpcLink_CreateParams        ipcInVpssPrm;
    IpcLink_CreateParams        ipcOutVideoPrm;
    IpcLink_CreateParams        ipcInVideoPrm;
    EncLink_CreateParams        encPrm;
    DecLink_CreateParams        decPrm;
    IpcBitsOutLinkHLOS_CreateParams   ipcBitsOutHostPrm;
    IpcBitsOutLinkRTOS_CreateParams   ipcBitsOutVideoPrm;
    IpcBitsInLinkHLOS_CreateParams    ipcBitsInHostPrm[2];
    AlgLink_CreateParams              dspAlgPrm;

    IpcBitsInLinkRTOS_CreateParams    ipcBitsInVideoPrm;
    CaptureLink_VipInstParams         *pCaptureInstPrm;
    CaptureLink_OutParams             *pCaptureOutPrm;
    IpcFramesInLinkRTOS_CreateParams  ipcFramesInDspPrm;
    IpcFramesOutLinkRTOS_CreateParams ipcFramesOutVpssPrm;
    IpcBitsOutLinkRTOS_CreateParams   ipcBitsOutDspPrm;
    static SwMsLink_CreateParams       swMsPrm[VDIS_DEV_MAX];
    VCAP_VIDDEC_PARAMS_S vidDecVideoModeArgs[NUM_CAPTURE_DEVICES];
	MergeLink_CreateParams      mergePrm[2];
    DupLink_CreateParams        dupPrm[2];
    System_LinkInfo                   bitsProducerLinkInfo;

    UInt32 mergeId[2];
    UInt32 dupId[2];

    UInt32 ipcOutVpssId, ipcInVpssId;
    UInt32 ipcOutVideoId, ipcInVideoId;

    UInt32 i;
    Bool   enableOsdAlgLink = gVsysModuleContext.vsysConfig.enableOsd;
	Bool   enableEncDec = gVsysModuleContext.vsysConfig.enableEncode && gVsysModuleContext.vsysConfig.enableDecode;
	if(enableOsdAlgLink)
		printf("OSD enabled!==================================\n");
	
    MULTICH_INIT_STRUCT(IpcLink_CreateParams,ipcOutVpssPrm);
    MULTICH_INIT_STRUCT(IpcLink_CreateParams,ipcInVpssPrm);
    MULTICH_INIT_STRUCT(IpcLink_CreateParams,ipcOutVideoPrm);
    MULTICH_INIT_STRUCT(IpcLink_CreateParams,ipcInVideoPrm);
    MULTICH_INIT_STRUCT(IpcBitsOutLinkHLOS_CreateParams,ipcBitsOutHostPrm);
    MULTICH_INIT_STRUCT(IpcBitsOutLinkRTOS_CreateParams,ipcBitsOutVideoPrm);
    MULTICH_INIT_STRUCT(IpcBitsOutLinkRTOS_CreateParams,ipcBitsOutDspPrm);

    MULTICH_INIT_STRUCT(IpcBitsInLinkHLOS_CreateParams,ipcBitsInHostPrm[0]);
    MULTICH_INIT_STRUCT(IpcBitsInLinkHLOS_CreateParams,ipcBitsInHostPrm[1]);
    MULTICH_INIT_STRUCT(IpcBitsInLinkRTOS_CreateParams,ipcBitsInVideoPrm);
    MULTICH_INIT_STRUCT(DecLink_CreateParams, decPrm);
    MULTICH_INIT_STRUCT(IpcFramesInLinkRTOS_CreateParams,ipcFramesInDspPrm);
    MULTICH_INIT_STRUCT(IpcFramesOutLinkRTOS_CreateParams,ipcFramesOutVpssPrm);
    MULTICH_INIT_STRUCT(EncLink_CreateParams, encPrm);
    for (i = 0; i < VDIS_DEV_MAX; i++)
    {
        MULTICH_INIT_STRUCT(DisplayLink_CreateParams,displayPrm[i]);
        MULTICH_INIT_STRUCT(SwMsLink_CreateParams ,swMsPrm[i]);
    }

    MultiCh_detectBoard();

    System_linkControl(
        SYSTEM_LINK_ID_M3VPSS,
        SYSTEM_M3VPSS_CMD_RESET_VIDEO_DEVICES,
        NULL,
        0,
        TRUE
        );
	
    System_linkControl(
        SYSTEM_LINK_ID_M3VIDEO,
        SYSTEM_COMMON_CMD_SET_CH2IVAHD_MAP_TBL,
        &systemVid_encDecIvaChMapTbl,
        sizeof(SystemVideo_Ivahd2ChMap_Tbl),
        TRUE
    );

    gVcapModuleContext.captureId    = SYSTEM_LINK_ID_CAPTURE;
    gVcapModuleContext.nullSrcId    = SYSTEM_VPSS_LINK_ID_NULL_SRC_0;
    gVencModuleContext.encId        = SYSTEM_LINK_ID_VENC_0;
    gVdecModuleContext.decId        = SYSTEM_LINK_ID_VDEC_0;
    gVdisModuleContext.displayId[0] = SYSTEM_LINK_ID_DISPLAY_0; // ON CHIP HDMI
    ipcOutVpssId = SYSTEM_VPSS_LINK_ID_IPC_OUT_M3_0;
    ipcInVideoId = SYSTEM_VIDEO_LINK_ID_IPC_IN_M3_0;
    ipcOutVideoId= SYSTEM_VIDEO_LINK_ID_IPC_OUT_M3_0;
    ipcInVpssId  = SYSTEM_VPSS_LINK_ID_IPC_IN_M3_0;
    gVencModuleContext.ipcBitsOutRTOSId  = SYSTEM_VIDEO_LINK_ID_IPC_BITS_OUT_0;
    gVencModuleContext.ipcBitsInHLOSId   = SYSTEM_HOST_LINK_ID_IPC_BITS_IN_0;
    gVdecModuleContext.ipcBitsOutHLOSId  = SYSTEM_HOST_LINK_ID_IPC_BITS_OUT_0;
    gVdecModuleContext.ipcBitsInRTOSId  = SYSTEM_VIDEO_LINK_ID_IPC_BITS_IN_0;
    gVdisModuleContext.swMsId[0]      = SYSTEM_LINK_ID_SW_MS_MULTI_INST_0;
    mergeId[0]   = SYSTEM_VPSS_LINK_ID_MERGE_0;
    mergeId[1]   = SYSTEM_VPSS_LINK_ID_MERGE_1;
    dupId[0]     = SYSTEM_VPSS_LINK_ID_DUP_0;
    dupId[1]     = SYSTEM_VPSS_LINK_ID_DUP_1;
	if(enableOsdAlgLink)
	{
		gVcapModuleContext.dspAlgId[0] = SYSTEM_LINK_ID_ALG_0  ;
		gVcapModuleContext.ipcFramesOutVpssId[0] = SYSTEM_VPSS_LINK_ID_IPC_FRAMES_OUT_0;
		gVcapModuleContext.ipcFramesInDspId[0] = SYSTEM_DSP_LINK_ID_IPC_FRAMES_IN_0;

	}

    CaptureLink_CreateParams_Init(&capturePrm);
    capturePrm.numVipInst    			= 1;
	capturePrm.outQueParams[0].nextLink = dupId[0];
    capturePrm.tilerEnable              = FALSE;
    capturePrm.enableSdCrop             = FALSE;
	capturePrm.numBufsPerCh				= 4;
	capturePrm.maxBlindAreasPerCh		= 4;
	
    pCaptureInstPrm                     = &capturePrm.vipInst[0];
    pCaptureInstPrm->vipInstId          = SYSTEM_CAPTURE_INST_VIP0_PORTA;
    pCaptureInstPrm->videoDecoderId     = SYSTEM_DEVICE_VID_DEC_TVP7002_DRV;
    pCaptureInstPrm->inDataFormat       = SYSTEM_DF_YUV422P;
    pCaptureInstPrm->standard           = SYSTEM_STD_720P_60;
    pCaptureInstPrm->numOutput          = 1;

    pCaptureOutPrm                      = &pCaptureInstPrm->outParams[0];
    pCaptureOutPrm->dataFormat          = SYSTEM_DF_YUV420SP_UV;
    pCaptureOutPrm->scEnable            = FALSE;
    pCaptureOutPrm->scOutWidth          = 0;
    pCaptureOutPrm->scOutHeight         = 0;
    pCaptureOutPrm->outQueId            = 0;
	
    for(i = 0; i < 1; i++)
    {
        vidDecVideoModeArgs[i].videoIfMode        = DEVICE_CAPT_VIDEO_IF_MODE_8BIT;
        vidDecVideoModeArgs[i].videoDataFormat    = SYSTEM_DF_YUV420SP_UV;
        vidDecVideoModeArgs[i].standard           = SYSTEM_STD_720P_60;
        vidDecVideoModeArgs[i].videoCaptureMode   =
                    DEVICE_CAPT_VIDEO_CAPTURE_MODE_SINGLE_CH_NON_MUX_EMBEDDED_SYNC;
        vidDecVideoModeArgs[i].videoSystem        =
                                      DEVICE_VIDEO_DECODER_VIDEO_SYSTEM_AUTO_DETECT;
        vidDecVideoModeArgs[i].videoCropEnable    = FALSE;
        vidDecVideoModeArgs[i].videoAutoDetectTimeout = -1;
    }

    Vcap_configVideoDecoder(vidDecVideoModeArgs, 1);


#ifndef SYSTEM_USE_VIDEO_DECODER
	capturePrm.isPalMode = Vcap_isPalMode();
#endif
	System_linkCreate(gVcapModuleContext.captureId, &capturePrm, sizeof(capturePrm));

	dupPrm[0].inQueParams.prevLinkId	   = gVcapModuleContext.captureId;
	dupPrm[0].inQueParams.prevLinkQueId	   = 0;
	dupPrm[0].numOutQue					   = 2;
	dupPrm[0].outQueParams[0].nextLink 	   = mergeId[1];
	dupPrm[0].outQueParams[1].nextLink	   = mergeId[0];
	dupPrm[0].notifyNextLink			   = TRUE;
	System_linkCreate(dupId[0], &dupPrm[0], sizeof(dupPrm[0]));

	mergePrm[1].numInQue = 1;
	mergePrm[1].inQueParams[0].prevLinkId = dupId[0];
	mergePrm[1].inQueParams[0].prevLinkQueId = 0;
	mergePrm[1].notifyNextLink = TRUE;
	if(enableOsdAlgLink)
	{
		mergePrm[1].outQueParams.nextLink = gVcapModuleContext.ipcFramesOutVpssId[0];
	}
	else
	{
		if(enableEncDec)
			mergePrm[1].outQueParams.nextLink	   = ipcOutVideoId;
		else
			mergePrm[1].outQueParams.nextLink	   = mergeId[0];
	}
	System_linkCreate(mergeId[1], &mergePrm[1], sizeof(mergePrm[1]));

    if(enableOsdAlgLink)
    {
		int chId;
		
        /* Redirect to DSP for OSD / SCD */
        ipcFramesOutVpssPrm .baseCreateParams.inQueParams.prevLinkId   = mergeId[1];
        ipcFramesOutVpssPrm.baseCreateParams.inQueParams.prevLinkQueId = 0;
		
		if(enableEncDec)
	        ipcFramesOutVpssPrm.baseCreateParams.outQueParams[0].nextLink  = ipcOutVpssId;
		else
			ipcFramesOutVpssPrm.baseCreateParams.outQueParams[0].nextLink  = mergeId[0];
        ipcFramesOutVpssPrm.baseCreateParams.processLink               = gVcapModuleContext.ipcFramesInDspId[0];
        ipcFramesOutVpssPrm.baseCreateParams.notifyPrevLink            = TRUE;
        ipcFramesOutVpssPrm.baseCreateParams.notifyNextLink            = TRUE;
        ipcFramesOutVpssPrm.baseCreateParams.notifyProcessLink         = TRUE;
        ipcFramesOutVpssPrm.baseCreateParams.noNotifyMode              = FALSE;
        ipcFramesOutVpssPrm.baseCreateParams.numOutQue                 = 1;

        ipcFramesInDspPrm.baseCreateParams.inQueParams.prevLinkId      = gVcapModuleContext.ipcFramesOutVpssId[0];
        ipcFramesInDspPrm.baseCreateParams.inQueParams.prevLinkQueId   = 0;
        ipcFramesInDspPrm.baseCreateParams.outQueParams[0].nextLink    = gVcapModuleContext.dspAlgId[0];
        ipcFramesInDspPrm.baseCreateParams.notifyPrevLink              = TRUE;
        ipcFramesInDspPrm.baseCreateParams.notifyNextLink              = TRUE;
        ipcFramesInDspPrm.baseCreateParams.noNotifyMode                = FALSE;
        ipcFramesInDspPrm.baseCreateParams.numOutQue                   = 1;

        dspAlgPrm.inQueParams.prevLinkId = gVcapModuleContext.ipcFramesInDspId[0];
        dspAlgPrm.inQueParams.prevLinkQueId = 0;
        dspAlgPrm.enableOSDAlg = TRUE;

        for(chId = 0; chId < ALG_LINK_OSD_MAX_CH; chId++)
        {
            AlgLink_OsdChWinParams * chWinPrm = &dspAlgPrm.osdChCreateParams[chId].chDefaultParams;

            /* set osd window max width and height */
            dspAlgPrm.osdChCreateParams[chId].maxWidth  = EXAMPLE_OSD_WIN_MAX_WIDTH;
            dspAlgPrm.osdChCreateParams[chId].maxHeight = EXAMPLE_OSD_WIN_MAX_HEIGHT;

            chWinPrm->chId = chId;
            chWinPrm->numWindows = 0;
        }
        dspAlgPrm.outQueParams[ALG_LINK_SCD_OUT_QUE].nextLink     = SYSTEM_LINK_ID_INVALID;
		
		System_linkCreate(gVcapModuleContext.ipcFramesOutVpssId[0], &ipcFramesOutVpssPrm, sizeof(ipcFramesOutVpssPrm));
		System_linkCreate(gVcapModuleContext.ipcFramesInDspId[0], &ipcFramesInDspPrm, sizeof(ipcFramesInDspPrm));
		System_linkCreate(gVcapModuleContext.dspAlgId[0] , &dspAlgPrm, sizeof(dspAlgPrm));
    }

	if(enableEncDec)
	{
		if(enableOsdAlgLink)
			ipcOutVpssPrm.inQueParams.prevLinkId	   = gVcapModuleContext.ipcFramesOutVpssId[0];
		else
			ipcOutVpssPrm.inQueParams.prevLinkId	   = mergeId[1];
		ipcOutVpssPrm.inQueParams.prevLinkQueId    = 0;

		ipcOutVpssPrm.numOutQue = 1;
	    ipcOutVpssPrm.outQueParams[0].nextLink     = ipcInVideoId;
	    ipcOutVpssPrm.notifyNextLink               = TRUE;
	    ipcOutVpssPrm.notifyPrevLink               = TRUE;
	    ipcOutVpssPrm.noNotifyMode                 = FALSE;
		System_linkCreate(ipcOutVpssId , &ipcOutVpssPrm , sizeof(ipcOutVpssPrm) );

	    ipcInVideoPrm.inQueParams.prevLinkId       = ipcOutVpssId;
	    ipcInVideoPrm.inQueParams.prevLinkQueId    = 0;
	    ipcInVideoPrm.numOutQue                    = 1;
	    ipcInVideoPrm.outQueParams[0].nextLink     = gVencModuleContext.encId;
	    ipcInVideoPrm.notifyNextLink               = TRUE;
	    ipcInVideoPrm.notifyPrevLink               = TRUE;
	    ipcInVideoPrm.noNotifyMode                 = FALSE;
		System_linkCreate(ipcInVideoId , &ipcInVideoPrm , sizeof(ipcInVideoPrm) );

	    {
	        EncLink_ChCreateParams *pLinkChPrm;
	        EncLink_ChDynamicParams *pLinkDynPrm;
	        VENC_CHN_DYNAMIC_PARAM_S *pDynPrm;
	        VENC_CHN_PARAMS_S *pChPrm;

	        EncLink_CreateParams_Init(&encPrm);

	        encPrm.numBufPerCh[0] = 4;
	        encPrm.numBufPerCh[1] = 4;

	        /* Primary Stream Params - D1 */
	        for (i=0; i<gVencModuleContext.vencConfig.numPrimaryChn; i++)
	        {
	            pLinkChPrm  = &encPrm.chCreateParams[i];
	            pLinkDynPrm = &pLinkChPrm->defaultDynamicParams;

	            pChPrm      = &gVencModuleContext.vencConfig.encChannelParams[i];
	            pDynPrm     = &pChPrm->dynamicParam;

	            pLinkChPrm->format                  = IVIDEO_H264HP;
	            pLinkChPrm->profile                 = gVencModuleContext.vencConfig.h264Profile[i];
	            pLinkChPrm->dataLayout              = IVIDEO_FIELD_SEPARATED;
	            pLinkChPrm->fieldMergeEncodeEnable  = FALSE;
	            pLinkChPrm->enableAnalyticinfo      = pChPrm->enableAnalyticinfo;
	            pLinkChPrm->enableWaterMarking      = pChPrm->enableWaterMarking;
	            pLinkChPrm->maxBitRate              = pChPrm->maxBitRate;
	            pLinkChPrm->encodingPreset          = pChPrm->encodingPreset;
	            pLinkChPrm->rateControlPreset       = pChPrm->rcType;
	            pLinkChPrm->enableSVCExtensionFlag  = pChPrm->enableSVCExtensionFlag;
	            pLinkChPrm->numTemporalLayer        = pChPrm->numTemporalLayer;

	            pLinkDynPrm->intraFrameInterval     = pDynPrm->intraFrameInterval;
	            pLinkDynPrm->targetBitRate          = pDynPrm->targetBitRate;
	            pLinkDynPrm->interFrameInterval     = 1;
	            pLinkDynPrm->mvAccuracy             = IVIDENC2_MOTIONVECTOR_QUARTERPEL;
	            pLinkDynPrm->inputFrameRate         = pDynPrm->inputFrameRate;
	            pLinkDynPrm->rcAlg                  = pDynPrm->rcAlg;
	            pLinkDynPrm->qpMin                  = pDynPrm->qpMin;
	            pLinkDynPrm->qpMax                  = pDynPrm->qpMax;
	            pLinkDynPrm->qpInit                 = pDynPrm->qpInit;
	            pLinkDynPrm->vbrDuration            = pDynPrm->vbrDuration;
	            pLinkDynPrm->vbrSensitivity         = pDynPrm->vbrSensitivity;
	        }

	        encPrm.inQueParams.prevLinkId    = ipcInVideoId;
	        encPrm.inQueParams.prevLinkQueId = 0;
	        encPrm.outQueParams.nextLink     = gVencModuleContext.ipcBitsOutRTOSId;
	    }
		System_linkCreate(gVencModuleContext.encId, &encPrm, sizeof(encPrm));

	    ipcBitsOutVideoPrm.baseCreateParams.inQueParams.prevLinkId = gVencModuleContext.encId;
	    ipcBitsOutVideoPrm.baseCreateParams.inQueParams.prevLinkQueId = 0;
	    ipcBitsOutVideoPrm.baseCreateParams.numOutQue                 = 1;
	    ipcBitsOutVideoPrm.baseCreateParams.outQueParams[0].nextLink = gVencModuleContext.ipcBitsInHLOSId;
	    MultiCh_ipcBitsInitCreateParams_BitsOutRTOS(&ipcBitsOutVideoPrm, TRUE);
		System_linkCreate(gVencModuleContext.ipcBitsOutRTOSId, &ipcBitsOutVideoPrm, sizeof(ipcBitsOutVideoPrm));

	    ipcBitsInHostPrm[0].baseCreateParams.inQueParams.prevLinkId = gVencModuleContext.ipcBitsOutRTOSId;
	    ipcBitsInHostPrm[0].baseCreateParams.inQueParams.prevLinkQueId = 0;
	    MultiCh_ipcBitsInitCreateParams_BitsInHLOS(&ipcBitsInHostPrm[0]);
		System_linkCreate(gVencModuleContext.ipcBitsInHLOSId, &ipcBitsInHostPrm[0], sizeof(ipcBitsInHostPrm[0]));	
		
		System_linkGetInfo(gVencModuleContext.ipcBitsInHLOSId,&bitsProducerLinkInfo);
		OSA_assert(bitsProducerLinkInfo.numQue == 1);
		ipcBitsOutHostPrm.baseCreateParams.outQueParams[0].nextLink = gVdecModuleContext.ipcBitsInRTOSId;
		
		printf ("\n\n========bitsProducerLinkInfo============\n");
		printf ("numQ %d, numCh %d\n",
						bitsProducerLinkInfo.numQue,
						bitsProducerLinkInfo.queInfo[0].numCh);
		{
			int i;
			for (i=0; i<bitsProducerLinkInfo.queInfo[0].numCh; i++)
			{
				printf ("Ch [%d] Width %d, Height %d\n",
					i,
					bitsProducerLinkInfo.queInfo[0].chInfo[i].width,
					bitsProducerLinkInfo.queInfo[0].chInfo[i].height
					);
			}
		}
		printf ("\n====================\n\n");
		
		if (bitsProducerLinkInfo.queInfo[0].numCh > gVencModuleContext.vencConfig.numPrimaryChn)
			bitsProducerLinkInfo.queInfo[0].numCh = gVencModuleContext.vencConfig.numPrimaryChn;
		
		printf ("Reducing bitsProducerLinkInfo.numCh to %d\n", bitsProducerLinkInfo.queInfo[0].numCh);
		
		MultiCh_ipcBitsInitCreateParams_BitsOutHLOS(&ipcBitsOutHostPrm,
												   &bitsProducerLinkInfo.queInfo[0]);
		ipcBitsOutHostPrm.numBufPerCh[0]  = 4;
		if(gVdecModuleContext.vdecConfig.forceUseDecChannelParams)
		{
			/* use channel info provided by user instead of from encoder */
			UInt32 chId;
			System_LinkChInfo *pChInfo;

			ipcBitsOutHostPrm.inQueInfo.numCh = gVdecModuleContext.vdecConfig.numChn;
			for(chId=0; chId<ipcBitsOutHostPrm.inQueInfo.numCh; chId++)
			{
				pChInfo = &ipcBitsOutHostPrm.inQueInfo.chInfo[chId];
		
				/* Not Used - Start */
				pChInfo->bufType		= 0;
				pChInfo->codingformat	= 0;
				pChInfo->dataFormat 	= 0;
				pChInfo->memType		= 0;
				pChInfo->startX 		= 0;
				pChInfo->startY 		= 0;
				pChInfo->pitch[0]		= 0;
				pChInfo->pitch[1]		= 0;
				pChInfo->pitch[2]		= 0;
				/* Not Used - End */
		
				pChInfo->width			= gVdecModuleContext.vdecConfig.decChannelParams[chId].maxVideoWidth;
				pChInfo->height 		= gVdecModuleContext.vdecConfig.decChannelParams[chId].maxVideoHeight;
				pChInfo->scanFormat 	= SYSTEM_SF_PROGRESSIVE;
			}
		}	
		System_linkCreate(gVdecModuleContext.ipcBitsOutHLOSId,&ipcBitsOutHostPrm,sizeof(ipcBitsOutHostPrm));

		ipcBitsInVideoPrm.baseCreateParams.inQueParams.prevLinkId		= gVdecModuleContext.ipcBitsOutHLOSId;
		ipcBitsInVideoPrm.baseCreateParams.inQueParams.prevLinkQueId	= 0;
		ipcBitsInVideoPrm.baseCreateParams.numOutQue					= 1;
		ipcBitsInVideoPrm.baseCreateParams.outQueParams[0].nextLink 	= gVdecModuleContext.decId;
		MultiCh_ipcBitsInitCreateParams_BitsInRTOS(&ipcBitsInVideoPrm, TRUE);
		System_linkCreate(gVdecModuleContext.ipcBitsInRTOSId,&ipcBitsInVideoPrm,sizeof(ipcBitsInVideoPrm));
		
		for (i=0; i<gVdecModuleContext.vdecConfig.numChn; i++) {
			decPrm.chCreateParams[i].format 				= IVIDEO_H264HP;
			decPrm.chCreateParams[i].profile				= IH264VDEC_PROFILE_ANY;
			decPrm.chCreateParams[i].fieldMergeDecodeEnable = FALSE;
			decPrm.chCreateParams[i].targetMaxWidth 		= ipcBitsOutHostPrm.inQueInfo.chInfo[i].width;
			decPrm.chCreateParams[i].targetMaxHeight		= ipcBitsOutHostPrm.inQueInfo.chInfo[i].height;
			decPrm.chCreateParams[i].defaultDynamicParams.targetFrameRate = gVdecModuleContext.vdecConfig.decChannelParams[i].dynamicParam.frameRate;
			decPrm.chCreateParams[i].defaultDynamicParams.targetBitRate   = gVdecModuleContext.vdecConfig.decChannelParams[i].dynamicParam.targetBitRate;
			/* Max ref frames is only 2 as this is closed loop decoder */
			decPrm.chCreateParams[i].dpbBufSizeInFrames 				  = 2;
			decPrm.chCreateParams[i].numBufPerCh = 4;
		}
		decPrm.inQueParams.prevLinkId	 = gVdecModuleContext.ipcBitsInRTOSId;
		decPrm.inQueParams.prevLinkQueId = 0;
		decPrm.outQueParams.nextLink  = ipcOutVideoId;
		decPrm.tilerEnable = FALSE;
		System_linkCreate(gVdecModuleContext.decId, &decPrm, sizeof(decPrm));
		
		ipcOutVideoPrm.inQueParams.prevLinkId	 = gVdecModuleContext.decId;
		ipcOutVideoPrm.inQueParams.prevLinkQueId = 0;
		ipcOutVideoPrm.numOutQue				 = 1;
		ipcOutVideoPrm.outQueParams[0].nextLink  = ipcInVpssId;
		ipcOutVideoPrm.notifyNextLink			 = TRUE;
		ipcOutVideoPrm.notifyPrevLink			 = TRUE;
		ipcOutVideoPrm.noNotifyMode 			 = FALSE;
		System_linkCreate(ipcOutVideoId, &ipcOutVideoPrm, sizeof(ipcOutVideoPrm));

		ipcInVpssPrm.inQueParams.prevLinkId    = ipcOutVideoId;
		ipcInVpssPrm.inQueParams.prevLinkQueId = 0;
		ipcInVpssPrm.numOutQue				   = 1;
		ipcInVpssPrm.outQueParams[0].nextLink  = mergeId[0];
		ipcInVpssPrm.notifyNextLink 		   = TRUE;
		ipcInVpssPrm.notifyPrevLink 		   = TRUE;	
		ipcInVpssPrm.noNotifyMode			   = FALSE;
		System_linkCreate(ipcInVpssId, &ipcInVpssPrm, sizeof(ipcInVpssPrm));
	}
	
	mergePrm[0].numInQue = 2;
	if(enableEncDec)
	{
		mergePrm[0].inQueParams[0].prevLinkId = ipcInVpssId;
		mergePrm[0].inQueParams[0].prevLinkQueId = 0;
	}
	else
	{
		mergePrm[0].inQueParams[0].prevLinkId =  gVcapModuleContext.ipcFramesOutVpssId[0];
		mergePrm[0].inQueParams[0].prevLinkQueId = 0;
	}
	mergePrm[0].inQueParams[1].prevLinkId = dupId[0];
	mergePrm[0].inQueParams[1].prevLinkQueId = 1;
	mergePrm[0].outQueParams.nextLink = gVdisModuleContext.swMsId[0];
	mergePrm[0].notifyNextLink = TRUE;
	
	swMsPrm[0].inQueParams.prevLinkId	 = mergeId[0];
	swMsPrm[0].inQueParams.prevLinkQueId = 0;
    swMsPrm[0].numSwMsInst = 1;
    swMsPrm[0].swMsInstId[0] = SYSTEM_SW_MS_SC_INST_DEI_SC_NO_DEI;
	swMsPrm[0].outQueParams.nextLink	 = gVdisModuleContext.displayId[0];
	swMsPrm[0].maxInputQueLen			 = 4; // KC: changed to 4 to make the display smooth
	swMsPrm[0].maxOutRes				 = gVdisModuleContext.vdisConfig.deviceParams[0].resolution;
	swMsPrm[0].numOutBuf				 = 8;
#ifdef  SYSTEM_USE_TILER
	swMsPrm[0].lineSkipMode = FALSE; // Double pitch not possible in tiler mode; so Line skip not possible
#else
	swMsPrm[0].lineSkipMode = TRUE; // Set to TRUE for Enable low cost scaling
#endif
	swMsPrm[0].enableLayoutGridDraw = gVdisModuleContext.vdisConfig.enableLayoutGridDraw;
	MultiCh_swMsGetDefaultLayoutPrm(VDIS_DEV_HDMI, &swMsPrm[0], TRUE);

	System_linkCreate(mergeId[0], &mergePrm[0], sizeof(mergePrm[0]));
	System_linkCreate(gVdisModuleContext.swMsId[0], &swMsPrm[0], sizeof(swMsPrm[0]));
	
	displayPrm[0].inQueParams[0].prevLinkId    = gVdisModuleContext.swMsId[0];
	displayPrm[0].inQueParams[0].prevLinkQueId = 0;
	displayPrm[0].displayRes				    = swMsPrm[0].maxOutRes;
	System_linkCreate(gVdisModuleContext.displayId[0], &displayPrm[0], sizeof(displayPrm[0]));

    MultiCh_memPrintHeapStatus();

}


Void MultiCh_deleteVcap7002Vdis()
{
    Bool enableOsdAlgLink = gVsysModuleContext.vsysConfig.enableOsd;
	Bool enableEncDec = gVsysModuleContext.vsysConfig.enableEncode && gVsysModuleContext.vsysConfig.enableDecode;
    UInt32 mergeId[2];
    UInt32 dupId[2];

    mergeId[0]	= SYSTEM_VPSS_LINK_ID_MERGE_0;
    mergeId[1] 	= SYSTEM_VPSS_LINK_ID_MERGE_1;

    dupId[0]  	= SYSTEM_VPSS_LINK_ID_DUP_0;
    dupId[1]  	= SYSTEM_VPSS_LINK_ID_DUP_1;

    gVdecModuleContext.ipcBitsOutHLOSId = SYSTEM_HOST_LINK_ID_IPC_BITS_OUT_0;
    gVdecModuleContext.ipcBitsInRTOSId = SYSTEM_VIDEO_LINK_ID_IPC_BITS_IN_0;

    gVcapModuleContext.captureId    = SYSTEM_LINK_ID_CAPTURE;
    gVcapModuleContext.nullSrcId    = SYSTEM_VPSS_LINK_ID_NULL_SRC_0;
    gVdisModuleContext.displayId[0] = SYSTEM_LINK_ID_DISPLAY_0;
    gVdisModuleContext.swMsId[0]    = SYSTEM_LINK_ID_SW_MS_MULTI_INST_0;

    System_linkDelete(gVcapModuleContext.captureId);
    System_linkDelete(gVcapModuleContext.nullSrcId);
    if(enableOsdAlgLink)
    {
        System_linkDelete(gVcapModuleContext.ipcFramesOutVpssId[0]);
        System_linkDelete(gVcapModuleContext.ipcFramesInDspId[0]);
        System_linkDelete(gVcapModuleContext.dspAlgId[0]);
    }

	if(enableEncDec)
	{
	    UInt32 ipcOutVpssId, ipcInVpssId;
    	UInt32 ipcOutVideoId, ipcInVideoId;

	    ipcOutVpssId = SYSTEM_VPSS_LINK_ID_IPC_OUT_M3_0;
    	ipcInVideoId = SYSTEM_VIDEO_LINK_ID_IPC_IN_M3_0;
	    ipcOutVideoId= SYSTEM_VIDEO_LINK_ID_IPC_OUT_M3_0;
		ipcInVpssId  = SYSTEM_VPSS_LINK_ID_IPC_IN_M3_0;

	    System_linkDelete(ipcOutVpssId );
    	System_linkDelete(ipcInVideoId );
		
    	System_linkDelete(gVencModuleContext.encId);
	    System_linkDelete(gVencModuleContext.ipcBitsOutRTOSId);
	    System_linkDelete(gVencModuleContext.ipcBitsInHLOSId);
	    System_linkDelete(gVdecModuleContext.ipcBitsOutHLOSId);
	    System_linkDelete(gVdecModuleContext.ipcBitsInRTOSId);
    	System_linkDelete(gVdecModuleContext.decId);

	    System_linkDelete(ipcOutVideoId);
    	System_linkDelete(ipcInVpssId  );
	}
	
    System_linkDelete(gVdisModuleContext.swMsId[0] );

    System_linkDelete(gVdisModuleContext.displayId[0]);

    System_linkDelete(dupId[0]);

    System_linkDelete(mergeId[0]);
    System_linkDelete(mergeId[1]);
	
    System_linkDelete(gVdisModuleContext.displayId[0]);

    /* Print the HWI, SWI and all tasks load */
    /* Reset the accumulated timer ticks */
    MultiCh_prfLoadCalcEnable(FALSE, TRUE, FALSE);

}
