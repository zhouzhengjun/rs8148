/*******************************************************************************
 *                                                                             *
 *      Copyright (c) 2013 BroadEng Beijing - http://www.broadeng.net/         *
 *                        ALL RIGHTS RESERVED                                  *
 *                                                                             * 
 *******************************************************************************
 *                                                                             *
 * Author:      Zhang Tao <zhangtao@broadeng.net>                              *
 * Version:     v1.0.0                                                         *
 *                                                                             *
 ******************************************************************************/

#include "bell_common.h"
#include "bell_ipcbits.h"
#include "bell_osd.h"

#define USE_CAPTURE_SII9135
#define MODE_CAP_MSC_LOCAL		0x0
#define MODE_CAP_MSC_HDMI_IN	0x1
#define MODE_CAP_MSC_REMOTE		0x2
#define MODE_CAP_MSC_HDMI_MAIN		0x4

/* =============================================================================
 * Externs
 * =============================================================================
 */

typedef enum {
    UI_AEWB_ID_NONE = 0,
    UI_AEWB_ID_APPRO,
    UI_AEWB_ID_TI,
    UI_AEWB_ID_MAXNUM = 3
} UI_AEWB_VENDOR;

typedef enum {
    UI_AEWB_OFF = 0,
    UI_AEWB_AE,
    UI_AEWB_AWB,
    UI_AEWB_AEWB,
    UI_AEWB_MODE_MAXNUM
} UI_AEWB_MODE;


#define     NUM_CAPTURE_DEVICES          (1)

#define LINK_IN_USE_NUM                 (128)
Int32 Links_in_use[LINK_IN_USE_NUM] = {SYSTEM_LINK_ID_INVALID};

static Void mark_as_in_use(Int32 link_id)
{
    Int32 i = 0;
    for(i = 0; i < LINK_IN_USE_NUM - 1; i++)
    {
        if(Links_in_use[i] == SYSTEM_LINK_ID_INVALID)
        {
            Links_in_use[i+1] = SYSTEM_LINK_ID_INVALID;
            Links_in_use[i]   = link_id;
            return;
        }
    }
    printf("Links_in_use array overflow!\n");
}

Void Bell_link_create(UInt32 id, Ptr prm, UInt32 argSize)
{
    System_linkCreate(id, prm, argSize);
    mark_as_in_use(id);
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

static void bell_link_join(Int32 pre_link_id, Int32 pre_link_que, System_LinkOutQueParams *pre_link, Int32 post_link_id, System_LinkInQueParams *post_link)
{
    pre_link->nextLink = post_link_id;
    post_link->prevLinkId = pre_link_id;
    post_link->prevLinkQueId = pre_link_que;
}
static void bell_captrue_config(CaptureLink_CreateParams *capturePrm, Int32 captrue_id)
{
    CaptureLink_VipInstParams   *pCaptureInstPrm;
    CaptureLink_OutParams       *pCaptureOutPrm;
    
    gVcapModuleContext.captureId = captrue_id;
    capturePrm->numVipInst               = 1;
    capturePrm->tilerEnable              = FALSE;
    capturePrm->enableSdCrop             = FALSE;
//    capturePrm->numBufsPerCh             = 4;
    capturePrm->maxBlindAreasPerCh     = 4;
    
    pCaptureInstPrm                     = &capturePrm->vipInst[0];
#ifdef USE_CAPTURE_SII9135
    pCaptureInstPrm->vipInstId          = SYSTEM_CAPTURE_INST_VIP1_PORTA;
    pCaptureInstPrm->videoDecoderId     = SYSTEM_DEVICE_VID_DEC_SII9135_DRV;
#else
    pCaptureInstPrm->vipInstId          = SYSTEM_CAPTURE_INST_VIP0_PORTA;
    pCaptureInstPrm->videoDecoderId     = SYSTEM_DEVICE_VID_DEC_TVP7002_DRV;
#endif
    pCaptureInstPrm->inDataFormat       = SYSTEM_DF_YUV422P;
#ifdef USE_CAPTURE_SII9135
    pCaptureInstPrm->standard           = SYSTEM_STD_1080P_24;
#else
    pCaptureInstPrm->standard           = SYSTEM_STD_720P_60;
#endif
    pCaptureInstPrm->numOutput          = 1;

    pCaptureOutPrm                      = &pCaptureInstPrm->outParams[0];
#ifdef USE_CAPTURE_SII9135
    pCaptureOutPrm->dataFormat          = SYSTEM_DF_YUV422I_YUYV;
#else
    pCaptureOutPrm->dataFormat          = SYSTEM_DF_YUV420SP_UV;
#endif
    pCaptureOutPrm->scEnable            = FALSE;
    pCaptureOutPrm->scOutWidth          = 0;
    pCaptureOutPrm->scOutHeight         = 0;
    pCaptureOutPrm->outQueId            = 0;

#ifndef SYSTEM_USE_VIDEO_DECODER
    capturePrm->isPalMode = Vcap_isPalMode();
#endif
    Bell_link_create(captrue_id, capturePrm, sizeof(*capturePrm));

    printf("%s(): gpio20 up\n",__FUNCTION__);
    system("echo out > /sys/class/gpio/gpio20/direction");
    system("echo 0 > /sys/class/gpio/gpio20/value");
}

static void bell_camera_config(CameraLink_CreateParams *cameraPrm, Int32 camera_id)
{    
    CameraLink_VipInstParams *pCameraInstPrm;
    CameraLink_OutParams *pCameraOutPrm;

    cameraPrm->numVipInst = 1;
    cameraPrm->tilerEnable = FALSE;
    //cameraPrm.captureMode = CAMERA_LINK_CAPMODE_ISIF;
		
	pCameraInstPrm = &cameraPrm->vipInst[0];
	pCameraInstPrm->SensorId = 0x10000512;
    pCameraInstPrm->vipInstId = SYSTEM_CAMERA_INST_VP_CSI2;
    pCameraInstPrm->inDataFormat = SYSTEM_DF_YUV422I_YUYV;

	pCameraInstPrm->standard = SYSTEM_STD_1080P_30;
	pCameraInstPrm->numOutput = 1;
	
    pCameraOutPrm = &pCameraInstPrm->outParams[0];            
    pCameraOutPrm->dataFormat = SYSTEM_DF_YUV420SP_UV;      
    pCameraOutPrm->scEnable = FALSE;
    pCameraOutPrm->scOutWidth = 1920;
    pCameraOutPrm->scOutHeight = 1080;
	pCameraOutPrm->outQueId = 0;
	Bell_link_create(camera_id, cameraPrm,sizeof(*cameraPrm));
}
static void bell_config_dup(DupLink_CreateParams *dupPrm, Int32 dup_id, Int32 numOutQue)
{
   	dupPrm->numOutQue				   = numOutQue;
   	dupPrm->notifyNextLink			   = TRUE;
   	Bell_link_create(dup_id, dupPrm, sizeof(*dupPrm));
}

static void bell_config_merge(MergeLink_CreateParams *mergePrm, Int32 merge_id, Int32 numInQue)
{
   	mergePrm->numInQue    			   = numInQue;
   	mergePrm->notifyNextLink			= TRUE;
   	Bell_link_create(merge_id, mergePrm, sizeof(*mergePrm));
}

static void bell_config_nsf(NsfLink_CreateParams *nsfPrm, Int32 nsf_id, Int32 numOutQue)
{
	switch (nsf_id) {
	case SYSTEM_LINK_ID_NSF_0:
		gVcapModuleContext.nsfId[0] 	= SYSTEM_LINK_ID_NSF_0;
	break;
	case SYSTEM_LINK_ID_NSF_1:
		gVcapModuleContext.nsfId[1] 	= SYSTEM_LINK_ID_NSF_1;
	break;
	default:
		printf("nsf number error\n");
	}
	nsfPrm->bypassNsf				 = TRUE;
	nsfPrm->tilerEnable				 = FALSE;
	nsfPrm->numOutQue                 = numOutQue;
	Bell_link_create(nsf_id, nsfPrm, sizeof(*nsfPrm));
}

static void bell_config_sclr(SclrLink_CreateParams *sclrPrm, Int32 sclr_id, Int32 numOutQue)
{
	gVcapModuleContext.sclrId[0]       = SYSTEM_LINK_ID_SCLR_INST_0;
	sclrPrm->tilerEnable                        = FALSE;
	sclrPrm->enableLineSkipSc                   = TRUE;//FALSE;
	sclrPrm->scaleMode                          = DEI_SCALE_MODE_ABSOLUTE;
	sclrPrm->outScaleFactor.absoluteResolution.outWidth= 1920;
	sclrPrm->outScaleFactor.absoluteResolution.outHeight= 1080;
	Bell_link_create(sclr_id, sclrPrm, sizeof(*sclrPrm));
}

static void bell_config_dsp_osd(IpcFramesOutLinkRTOS_CreateParams *ipcFramesOutVpssPrm, IpcFramesInLinkRTOS_CreateParams *ipcFramesInDspPrm)
{
    int chId;
    AlgLink_CreateParams dspAlgPrm;
    MULTICH_INIT_STRUCT(AlgLink_CreateParams, dspAlgPrm);

    gVcapModuleContext.dspAlgId[0] = SYSTEM_LINK_ID_ALG_0  ;
    gVcapModuleContext.ipcFramesOutVpssId[0] = SYSTEM_VPSS_LINK_ID_IPC_FRAMES_OUT_0;
    gVcapModuleContext.ipcFramesInDspId[0] = SYSTEM_DSP_LINK_ID_IPC_FRAMES_IN_0;

    ipcFramesOutVpssPrm->baseCreateParams.notifyPrevLink = TRUE;
    ipcFramesOutVpssPrm->baseCreateParams.numOutQue = 1;
    ipcFramesOutVpssPrm->baseCreateParams.notifyNextLink = TRUE;
    ipcFramesOutVpssPrm->baseCreateParams.noNotifyMode = FALSE;
    ipcFramesOutVpssPrm->baseCreateParams.processLink = gVcapModuleContext.ipcFramesInDspId[0];
    ipcFramesOutVpssPrm->baseCreateParams.notifyProcessLink = TRUE;
    
    ipcFramesInDspPrm->baseCreateParams.inQueParams.prevLinkId = gVcapModuleContext.ipcFramesOutVpssId[0];
    ipcFramesInDspPrm->baseCreateParams.inQueParams.prevLinkQueId = 0;
    ipcFramesInDspPrm->baseCreateParams.numOutQue   = 1;
    ipcFramesInDspPrm->baseCreateParams.outQueParams[0].nextLink = gVcapModuleContext.dspAlgId[0];
    ipcFramesInDspPrm->baseCreateParams.notifyPrevLink = TRUE;
    ipcFramesInDspPrm->baseCreateParams.notifyNextLink = TRUE;
    ipcFramesInDspPrm->baseCreateParams.noNotifyMode   = FALSE;
    
    dspAlgPrm.inQueParams.prevLinkId = gVcapModuleContext.ipcFramesInDspId[0];
    dspAlgPrm.inQueParams.prevLinkQueId = 0;
    dspAlgPrm.enableOSDAlg = TRUE;
    
    for(chId = 0; chId < ALG_LINK_OSD_MAX_CH; chId++)
    {
        AlgLink_OsdChWinParams * chWinPrm = &dspAlgPrm.osdChCreateParams[chId].chDefaultParams;

        /* set osd window max width and height */
        dspAlgPrm.osdChCreateParams[chId].maxWidth  = 320;
        dspAlgPrm.osdChCreateParams[chId].maxHeight = 64;
    
        chWinPrm->chId = chId;
        chWinPrm->numWindows = 0;
    }
    
    dspAlgPrm.enableSCDAlg = FALSE;
    dspAlgPrm.outQueParams[ALG_LINK_SCD_OUT_QUE].nextLink     = SYSTEM_LINK_ID_INVALID;

    Bell_link_create(gVcapModuleContext.ipcFramesOutVpssId[0], ipcFramesOutVpssPrm, sizeof(*ipcFramesOutVpssPrm));
    Bell_link_create(gVcapModuleContext.ipcFramesInDspId[0], ipcFramesInDspPrm, sizeof(*ipcFramesInDspPrm));
    Bell_link_create(gVcapModuleContext.dspAlgId[0] , &dspAlgPrm, sizeof(dspAlgPrm));
}

static void bell_config_vpss_osd(AlgLink_CreateParams *vpssAlgPrm)
{
    int chId;

    gVcapModuleContext.dspAlgId[0] = SYSTEM_LINK_ID_VPSS_ALG_0  ;

    vpssAlgPrm->enableOSDAlg = TRUE;
    for(chId = 0; chId < ALG_LINK_OSD_MAX_CH; chId++)
    {
        AlgLink_OsdChWinParams * chWinPrm = &vpssAlgPrm->osdChCreateParams[chId].chDefaultParams;

        /* set osd window max width and height */
        vpssAlgPrm->osdChCreateParams[chId].maxWidth  = 320;
        vpssAlgPrm->osdChCreateParams[chId].maxHeight = 64;
    
        chWinPrm->chId = chId;
        chWinPrm->numWindows = 0;
    }
    
    vpssAlgPrm->enableSCDAlg = FALSE;
    vpssAlgPrm->outQueParams[ALG_LINK_SCD_OUT_QUE].nextLink     = SYSTEM_LINK_ID_INVALID;

    Bell_link_create(gVcapModuleContext.dspAlgId[0] , vpssAlgPrm, sizeof(*vpssAlgPrm));
}


static void bell_config_encode(IpcLink_CreateParams *ipcOutVpssPrm)
{
    IpcLink_CreateParams        ipcInVideoPrm;
    EncLink_CreateParams        encPrm;
    IpcBitsOutLinkRTOS_CreateParams   ipcBitsOutVideoPrm;
    IpcBitsInLinkHLOS_CreateParams    ipcBitsInHostPrm;
    int i;
    UInt32 ipcOutVpssId;
    UInt32 ipcInVideoId;

    MULTICH_INIT_STRUCT(IpcLink_CreateParams,ipcInVideoPrm);
    MULTICH_INIT_STRUCT(IpcBitsOutLinkRTOS_CreateParams,ipcBitsOutVideoPrm);

    MULTICH_INIT_STRUCT(IpcBitsInLinkHLOS_CreateParams,ipcBitsInHostPrm);
    MULTICH_INIT_STRUCT(EncLink_CreateParams, encPrm);
    
    ipcOutVpssId = SYSTEM_VPSS_LINK_ID_IPC_OUT_M3_0;
    ipcInVideoId = SYSTEM_VIDEO_LINK_ID_IPC_IN_M3_0;
    gVencModuleContext.ipcBitsOutRTOSId  = SYSTEM_VIDEO_LINK_ID_IPC_BITS_OUT_0;
    gVencModuleContext.ipcBitsInHLOSId   = SYSTEM_HOST_LINK_ID_IPC_BITS_IN_0;
    gVencModuleContext.encId 			 = SYSTEM_LINK_ID_VENC_0;
    ipcOutVpssPrm->numOutQue = 1;
    ipcOutVpssPrm->outQueParams[0].nextLink     = ipcInVideoId;
    ipcOutVpssPrm->notifyNextLink               = TRUE;
    ipcOutVpssPrm->notifyPrevLink               = TRUE;
    ipcOutVpssPrm->noNotifyMode                 = FALSE;
    Bell_link_create(ipcOutVpssId , ipcOutVpssPrm , sizeof(*ipcOutVpssPrm) );

    ipcInVideoPrm.inQueParams.prevLinkId       = ipcOutVpssId;
    ipcInVideoPrm.inQueParams.prevLinkQueId    = 0;
    ipcInVideoPrm.numOutQue                    = 1;
    ipcInVideoPrm.outQueParams[0].nextLink     = gVencModuleContext.encId;
    ipcInVideoPrm.notifyNextLink               = TRUE;
    ipcInVideoPrm.notifyPrevLink               = TRUE;
    ipcInVideoPrm.noNotifyMode                 = FALSE;
    Bell_link_create(ipcInVideoId , &ipcInVideoPrm , sizeof(ipcInVideoPrm) );

    {
        EncLink_ChCreateParams *pLinkChPrm;
        EncLink_ChDynamicParams *pLinkDynPrm;
        VENC_CHN_DYNAMIC_PARAM_S *pDynPrm;
        VENC_CHN_PARAMS_S *pChPrm;

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
    }    
    
    encPrm.inQueParams.prevLinkId    = ipcInVideoId;
    encPrm.inQueParams.prevLinkQueId = 0;
    encPrm.outQueParams.nextLink     = gVencModuleContext.ipcBitsOutRTOSId;
    Bell_link_create(gVencModuleContext.encId, &encPrm, sizeof(encPrm));

    ipcBitsOutVideoPrm.baseCreateParams.inQueParams.prevLinkId = gVencModuleContext.encId;
    ipcBitsOutVideoPrm.baseCreateParams.inQueParams.prevLinkQueId = 0;
    ipcBitsOutVideoPrm.baseCreateParams.numOutQue                 = 1;
    ipcBitsOutVideoPrm.baseCreateParams.outQueParams[0].nextLink = gVencModuleContext.ipcBitsInHLOSId;
    Bell_ipcBitsInitCreateParams_BitsOutRTOS(&ipcBitsOutVideoPrm, TRUE);
    Bell_link_create(gVencModuleContext.ipcBitsOutRTOSId, &ipcBitsOutVideoPrm, sizeof(ipcBitsOutVideoPrm));

    ipcBitsInHostPrm.baseCreateParams.inQueParams.prevLinkId = gVencModuleContext.ipcBitsOutRTOSId;
    ipcBitsInHostPrm.baseCreateParams.inQueParams.prevLinkQueId = 0;
    Bell_ipcBitsInitCreateParams_BitsInHLOS(&ipcBitsInHostPrm);
    Bell_link_create(gVencModuleContext.ipcBitsInHLOSId, &ipcBitsInHostPrm, sizeof(ipcBitsInHostPrm));   
}

static void bell_config_decode(IpcLink_CreateParams *ipcInVpssPrm)
{
    IpcLink_CreateParams        ipcOutVideoPrm;
    DecLink_CreateParams        decPrm;
    IpcBitsInLinkRTOS_CreateParams    ipcBitsInVideoPrm;
    IpcBitsOutLinkHLOS_CreateParams   ipcBitsOutHostPrm;
    System_LinkInfo bitsProducerLinkInfo;
    int i;
    UInt32 ipcInVpssId;
    UInt32 ipcOutVideoId;

    MULTICH_INIT_STRUCT(IpcLink_CreateParams,ipcOutVideoPrm);
    MULTICH_INIT_STRUCT(IpcBitsOutLinkHLOS_CreateParams,ipcBitsOutHostPrm);

    MULTICH_INIT_STRUCT(IpcBitsInLinkRTOS_CreateParams,ipcBitsInVideoPrm);
    MULTICH_INIT_STRUCT(DecLink_CreateParams, decPrm);
    
    ipcOutVideoId= SYSTEM_VIDEO_LINK_ID_IPC_OUT_M3_0;
    ipcInVpssId  = SYSTEM_VPSS_LINK_ID_IPC_IN_M3_0;
    gVdecModuleContext.ipcBitsOutHLOSId  = SYSTEM_HOST_LINK_ID_IPC_BITS_OUT_0;
    gVdecModuleContext.ipcBitsInRTOSId   = SYSTEM_VIDEO_LINK_ID_IPC_BITS_IN_0;
	gVdecModuleContext.decId        	 = SYSTEM_LINK_ID_VDEC_0;

    ipcBitsOutHostPrm.baseCreateParams.outQueParams[0].nextLink = gVdecModuleContext.ipcBitsInRTOSId;
    Bell_ipcBitsInitCreateParams_BitsOutHLOS(&ipcBitsOutHostPrm, &bitsProducerLinkInfo.queInfo[0]);
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
            pChInfo->bufType        = SYSTEM_BUF_TYPE_VIDBITSTREAM;
            pChInfo->codingformat   = IVIDEO_H264HP;
            pChInfo->dataFormat     = 0;
            pChInfo->memType        = 0;
            pChInfo->startX         = 0;
            pChInfo->startY         = 0;
            pChInfo->pitch[0]       = 0;
            pChInfo->pitch[1]       = 0;
            pChInfo->pitch[2]       = 0;
            /* Not Used - End */
    
            pChInfo->width          = gVdecModuleContext.vdecConfig.decChannelParams[chId].maxVideoWidth;
            pChInfo->height         = gVdecModuleContext.vdecConfig.decChannelParams[chId].maxVideoHeight;
            pChInfo->scanFormat     = SYSTEM_SF_PROGRESSIVE;
        }
    }
    
    Bell_link_create(gVdecModuleContext.ipcBitsOutHLOSId,&ipcBitsOutHostPrm,sizeof(ipcBitsOutHostPrm));

    ipcBitsInVideoPrm.baseCreateParams.inQueParams.prevLinkId       = gVdecModuleContext.ipcBitsOutHLOSId;
    ipcBitsInVideoPrm.baseCreateParams.inQueParams.prevLinkQueId    = 0;
    ipcBitsInVideoPrm.baseCreateParams.numOutQue                    = 1;
    ipcBitsInVideoPrm.baseCreateParams.outQueParams[0].nextLink     = gVdecModuleContext.decId;
    Bell_ipcBitsInitCreateParams_BitsInRTOS(&ipcBitsInVideoPrm, TRUE);
    Bell_link_create(gVdecModuleContext.ipcBitsInRTOSId,&ipcBitsInVideoPrm,sizeof(ipcBitsInVideoPrm));

    {
        for (i=0; i<gVdecModuleContext.vdecConfig.numChn; i++) {
            decPrm.chCreateParams[i].format                 = IVIDEO_H264HP;
            decPrm.chCreateParams[i].profile                = IH264VDEC_PROFILE_ANY;
            decPrm.chCreateParams[i].fieldMergeDecodeEnable = FALSE;
            decPrm.chCreateParams[i].targetMaxWidth         = ipcBitsOutHostPrm.inQueInfo.chInfo[i].width;
            decPrm.chCreateParams[i].targetMaxHeight        = ipcBitsOutHostPrm.inQueInfo.chInfo[i].height;
            decPrm.chCreateParams[i].defaultDynamicParams.targetFrameRate = gVdecModuleContext.vdecConfig.decChannelParams[i].dynamicParam.frameRate;
            decPrm.chCreateParams[i].defaultDynamicParams.targetBitRate   = gVdecModuleContext.vdecConfig.decChannelParams[i].dynamicParam.targetBitRate;
            /* Max ref frames is only 2 as this is closed loop decoder */
            decPrm.chCreateParams[i].dpbBufSizeInFrames = IH264VDEC_DPB_NUMFRAMES_AUTO;
            decPrm.chCreateParams[i].numBufPerCh = 4;
        }
        decPrm.tilerEnable = FALSE;        
    }
    
    decPrm.inQueParams.prevLinkId    = gVdecModuleContext.ipcBitsInRTOSId;
    decPrm.inQueParams.prevLinkQueId = 0;
    decPrm.outQueParams.nextLink  = ipcOutVideoId;
    Bell_link_create(gVdecModuleContext.decId, &decPrm, sizeof(decPrm));
    Vdec_decErrReport(0, 1);
    
    ipcOutVideoPrm.inQueParams.prevLinkId    = gVdecModuleContext.decId;
    ipcOutVideoPrm.inQueParams.prevLinkQueId = 0;
    ipcOutVideoPrm.numOutQue                 = 1;
    ipcOutVideoPrm.outQueParams[0].nextLink  = ipcInVpssId;
    ipcOutVideoPrm.notifyNextLink            = TRUE;
    ipcOutVideoPrm.notifyPrevLink            = TRUE;
    ipcOutVideoPrm.noNotifyMode              = FALSE;
    Bell_link_create(ipcOutVideoId, &ipcOutVideoPrm, sizeof(ipcOutVideoPrm));
    
    ipcInVpssPrm->inQueParams.prevLinkId    = ipcOutVideoId;
    ipcInVpssPrm->inQueParams.prevLinkQueId = 0;
    ipcInVpssPrm->numOutQue                 = 1;
    ipcInVpssPrm->notifyNextLink            = TRUE;
    ipcInVpssPrm->notifyPrevLink            = TRUE;  
    ipcInVpssPrm->noNotifyMode              = FALSE;
    Bell_link_create(ipcInVpssId, ipcInVpssPrm, sizeof(*ipcInVpssPrm));
}


static void bell_config_decode_indep(IpcLink_CreateParams *ipcInVpssPrm)
{
    IpcLink_CreateParams        ipcOutVideoPrm;
    DecLink_CreateParams        decPrm;
    IpcBitsInLinkRTOS_CreateParams    ipcBitsInVideoPrm;
    IpcBitsOutLinkHLOS_CreateParams   ipcBitsOutHostPrm;    
    int i;
    UInt32 ipcInVpssId;
    UInt32 ipcOutVideoId;

    MULTICH_INIT_STRUCT(IpcLink_CreateParams,ipcOutVideoPrm);
    MULTICH_INIT_STRUCT(IpcBitsOutLinkHLOS_CreateParams,ipcBitsOutHostPrm);

    MULTICH_INIT_STRUCT(IpcBitsInLinkRTOS_CreateParams,ipcBitsInVideoPrm);
    MULTICH_INIT_STRUCT(DecLink_CreateParams, decPrm);
    
    ipcOutVideoId= SYSTEM_VIDEO_LINK_ID_IPC_OUT_M3_0;
    ipcInVpssId  = SYSTEM_VPSS_LINK_ID_IPC_IN_M3_0;
    gVdecModuleContext.ipcBitsOutHLOSId  = SYSTEM_HOST_LINK_ID_IPC_BITS_OUT_0;
    gVdecModuleContext.ipcBitsInRTOSId   = SYSTEM_VIDEO_LINK_ID_IPC_BITS_IN_0;
	gVdecModuleContext.decId        	 = SYSTEM_LINK_ID_VDEC_0;


	ipcBitsOutHostPrm.baseCreateParams.outQueParams[0].nextLink= gVdecModuleContext.ipcBitsInRTOSId;
	ipcBitsOutHostPrm.baseCreateParams.notifyNextLink		= FALSE;
	ipcBitsOutHostPrm.baseCreateParams.notifyPrevLink		= FALSE;
	ipcBitsOutHostPrm.baseCreateParams.noNotifyMode 		= TRUE;
	ipcBitsOutHostPrm.baseCreateParams.numOutQue			= 1;
	ipcBitsOutHostPrm.inQueInfo.numCh						= gVdecModuleContext.vdecConfig.numChn;
	ipcBitsOutHostPrm.bufPoolPerCh							= FALSE;

	for (i=0; i<ipcBitsOutHostPrm.inQueInfo.numCh; i++)
	{
		ipcBitsOutHostPrm.inQueInfo.chInfo[i].width =
			gVdecModuleContext.vdecConfig.decChannelParams[i].maxVideoWidth;

		ipcBitsOutHostPrm.inQueInfo.chInfo[i].height =
			gVdecModuleContext.vdecConfig.decChannelParams[i].maxVideoHeight;

		ipcBitsOutHostPrm.inQueInfo.chInfo[i].scanFormat =
			SYSTEM_SF_PROGRESSIVE;

		ipcBitsOutHostPrm.inQueInfo.chInfo[i].bufType		 = 1;
		ipcBitsOutHostPrm.inQueInfo.chInfo[i].codingformat	 = IVIDEO_H264HP;
		ipcBitsOutHostPrm.inQueInfo.chInfo[i].dataFormat	 = 0;
		ipcBitsOutHostPrm.inQueInfo.chInfo[i].memType		 = 0;
		ipcBitsOutHostPrm.inQueInfo.chInfo[i].startX		 = 0;
		ipcBitsOutHostPrm.inQueInfo.chInfo[i].startY		 = 0;
		ipcBitsOutHostPrm.inQueInfo.chInfo[i].pitch[0]		 = 0;
		ipcBitsOutHostPrm.inQueInfo.chInfo[i].pitch[1]		 = 0;
		ipcBitsOutHostPrm.inQueInfo.chInfo[i].pitch[2]		 = 0;

		ipcBitsOutHostPrm.inQueInfo.chInfo[i].width 		= gVdecModuleContext.vdecConfig.decChannelParams[i].maxVideoWidth;
		ipcBitsOutHostPrm.inQueInfo.chInfo[i].height		= gVdecModuleContext.vdecConfig.decChannelParams[i].maxVideoHeight;
		ipcBitsOutHostPrm.inQueInfo.chInfo[i].scanFormat	= SYSTEM_SF_PROGRESSIVE;
//		  ipcBitsOutHostPrm.numBufPerCh[i] = 6;
	}

    
    Bell_link_create(gVdecModuleContext.ipcBitsOutHLOSId,&ipcBitsOutHostPrm,sizeof(ipcBitsOutHostPrm));

    ipcBitsInVideoPrm.baseCreateParams.inQueParams.prevLinkId       = gVdecModuleContext.ipcBitsOutHLOSId;
    ipcBitsInVideoPrm.baseCreateParams.inQueParams.prevLinkQueId    = 0;
    ipcBitsInVideoPrm.baseCreateParams.numOutQue                    = 1;
    ipcBitsInVideoPrm.baseCreateParams.outQueParams[0].nextLink     = gVdecModuleContext.decId;
    Bell_ipcBitsInitCreateParams_BitsInRTOS(&ipcBitsInVideoPrm, TRUE);
    Bell_link_create(gVdecModuleContext.ipcBitsInRTOSId,&ipcBitsInVideoPrm,sizeof(ipcBitsInVideoPrm));

    {
        for (i=0; i<gVdecModuleContext.vdecConfig.numChn; i++) {
            decPrm.chCreateParams[i].format                 = IVIDEO_H264HP;
            decPrm.chCreateParams[i].profile                = IH264VDEC_PROFILE_ANY;
            decPrm.chCreateParams[i].fieldMergeDecodeEnable = FALSE;
            decPrm.chCreateParams[i].targetMaxWidth         = ipcBitsOutHostPrm.inQueInfo.chInfo[i].width;
            decPrm.chCreateParams[i].targetMaxHeight        = ipcBitsOutHostPrm.inQueInfo.chInfo[i].height;
            decPrm.chCreateParams[i].defaultDynamicParams.targetFrameRate = gVdecModuleContext.vdecConfig.decChannelParams[i].dynamicParam.frameRate;
            decPrm.chCreateParams[i].defaultDynamicParams.targetBitRate   = gVdecModuleContext.vdecConfig.decChannelParams[i].dynamicParam.targetBitRate;
            /* Max ref frames is only 2 as this is closed loop decoder */
            decPrm.chCreateParams[i].dpbBufSizeInFrames = IH264VDEC_DPB_NUMFRAMES_AUTO;
            decPrm.chCreateParams[i].numBufPerCh = 4;
        }
        decPrm.tilerEnable = FALSE;        
    }
    
    decPrm.inQueParams.prevLinkId    = gVdecModuleContext.ipcBitsInRTOSId;
    decPrm.inQueParams.prevLinkQueId = 0;
    decPrm.outQueParams.nextLink  = ipcOutVideoId;
    Bell_link_create(gVdecModuleContext.decId, &decPrm, sizeof(decPrm));
    
    ipcOutVideoPrm.inQueParams.prevLinkId    = gVdecModuleContext.decId;
    ipcOutVideoPrm.inQueParams.prevLinkQueId = 0;
    ipcOutVideoPrm.numOutQue                 = 1;
    ipcOutVideoPrm.outQueParams[0].nextLink  = ipcInVpssId;
    ipcOutVideoPrm.notifyNextLink            = TRUE;
    ipcOutVideoPrm.notifyPrevLink            = TRUE;
    ipcOutVideoPrm.noNotifyMode              = FALSE;
    Bell_link_create(ipcOutVideoId, &ipcOutVideoPrm, sizeof(ipcOutVideoPrm));

    ipcInVpssPrm->inQueParams.prevLinkId    = ipcOutVideoId;
    ipcInVpssPrm->inQueParams.prevLinkQueId = 0;
    ipcInVpssPrm->numOutQue                 = 1;
    ipcInVpssPrm->notifyNextLink            = TRUE;
    ipcInVpssPrm->notifyPrevLink            = TRUE;  
    ipcInVpssPrm->noNotifyMode              = FALSE;
    Bell_link_create(ipcInVpssId, ipcInVpssPrm, sizeof(*ipcInVpssPrm));
}

static
Void MultiCh_setCustomVcapSwMs(SwMsLink_CreateParams *swMsCreateArgs, Bool forceLowCostScaling, UInt32 mode)
{
    SwMsLink_LayoutPrm *layoutInfo;
    SwMsLink_LayoutWinInfo *winInfo;
    UInt32 outWidth, outHeight, /*row, col, */winId, widthAlign, heightAlign;
    UInt32 rowMax, colMax;

	outWidth = gVencModuleContext.vencConfig.encChannelParams[0].videoWidth;
	outHeight = gVencModuleContext.vencConfig.encChannelParams[0].videoHeight;

	widthAlign = 8;
	heightAlign = 1;

	layoutInfo = &swMsCreateArgs->layoutPrm;

	/* init to known default */
	memset(layoutInfo, 0, sizeof(*layoutInfo));

	rowMax = 2;
	colMax = 2;

	{
		layoutInfo->onlyCh2WinMapChanged = FALSE;
		layoutInfo->outputFPS = 32;
#if 0
      for(row=0; row<rowMax; row++)
      {
          for(col=0; col<colMax; col++)
          {
              winId = row*colMax+col;

              winInfo = &layoutInfo->winInfo[winId];

              winInfo->width  = SystemUtils_align(outWidth/colMax, widthAlign);
              winInfo->height = SystemUtils_align(outHeight/rowMax, heightAlign);
              winInfo->startX = winInfo->width*col;
              winInfo->startY = winInfo->height*row;
              if (forceLowCostScaling == TRUE)
                winInfo->bypass = TRUE;
              else
                winInfo->bypass = FALSE;
              winInfo->channelNum = winId;
          }
      }
#else
		winId = 0;
		winInfo = &layoutInfo->winInfo[winId];
		winInfo->width	= SystemUtils_align(outWidth, widthAlign);
		winInfo->height = SystemUtils_align(outHeight, heightAlign);
		winInfo->startX = 0;
		winInfo->startY = 0;
		if (forceLowCostScaling == TRUE)
			winInfo->bypass = TRUE;
		else
			winInfo->bypass = FALSE;
		if ((mode & MODE_CAP_MSC_HDMI_MAIN) && (mode & MODE_CAP_MSC_HDMI_IN))
			winInfo->channelNum = 2;
		else
			winInfo->channelNum = winId;

		if (mode & MODE_CAP_MSC_REMOTE) {
			winId++;
			winInfo = &layoutInfo->winInfo[winId];
			winInfo->width	= SystemUtils_align(outWidth/4, widthAlign);
			winInfo->height = SystemUtils_align(outHeight/4, heightAlign);
			winInfo->startX = SystemUtils_align(outWidth / 20, widthAlign);
			winInfo->startY = SystemUtils_align(outHeight - winInfo->height - outHeight/20, heightAlign);
			if (forceLowCostScaling == TRUE)
				winInfo->bypass = TRUE;
			else
				winInfo->bypass = FALSE;
			winInfo->channelNum = winId;
		}
		if (mode & MODE_CAP_MSC_HDMI_IN) {
			winId++;
			winInfo = &layoutInfo->winInfo[winId];
			winInfo->width	= SystemUtils_align(outWidth/4, widthAlign);
			winInfo->height = SystemUtils_align(outHeight/4, heightAlign);
			winInfo->startX = SystemUtils_align(outWidth * 3 / 8, widthAlign);
			winInfo->startY = SystemUtils_align(outHeight - winInfo->height - outHeight/20, heightAlign);
			if (forceLowCostScaling == TRUE)
				winInfo->bypass = TRUE;
			else
				winInfo->bypass = FALSE;
			if (mode & MODE_CAP_MSC_HDMI_MAIN)
				winInfo->channelNum = 0;
			else
				winInfo->channelNum = winId;
		}

		layoutInfo->numWin = winId + 1;

#endif
    }
}

static
Void MultiCh_setCustomAvsyncVidQuePrm(Avsync_SynchConfigParams *queCfg,
                                            Int chnum,
                                            UInt32 avsStartChNum,
                                            UInt32 avsEndChNum)
{
    queCfg->chNum = chnum;
    queCfg->audioPresent = FALSE;
    queCfg->avsyncEnable = FALSE;
    queCfg->clkAdjustPolicy.refClkType = AVSYNC_REFCLKADJUST_BYVIDEO;
    queCfg->playTimerStartTimeout = 0;
    queCfg->playStartMode = AVSYNC_PLAYBACK_START_MODE_WAITSYNCH;
    queCfg->clkAdjustPolicy.clkAdjustLead = AVSYNC_VIDEO_REFCLKADJUST_MAX_LEAD_MS;
    queCfg->clkAdjustPolicy.clkAdjustLag = AVSYNC_VIDEO_REFCLKADJUST_MAX_LAG_MS;
}

static
Void MultiCh_setCustomAvsyncPrm(AvsyncLink_LinkSynchConfigParams *avsyncPrm,
                                  UInt32 prevLinkID,
                                  UInt32 prevLinkQueId,
                                    UInt32 swMSId)
{
    System_LinkInfo                   swmsInLinkInfo;
    Int i;
    Int32 status;

    Vdis_getAvsyncConfig(VDIS_DEV_HDMI,avsyncPrm);
    avsyncPrm->displayID        = Vdis_getDisplayContextIndex(VDIS_DEV_HDMI);
    avsyncPrm->videoSynchLinkID = swMSId;
    System_linkGetInfo(prevLinkID,&swmsInLinkInfo);
    OSA_assert(swmsInLinkInfo.numQue > prevLinkQueId);

    avsyncPrm->numCh            = swmsInLinkInfo.queInfo[prevLinkQueId].numCh;
    avsyncPrm->syncMasterChnum = 0;
    for (i = 0; i < avsyncPrm->numCh;i++)
    {
        MultiCh_setCustomAvsyncVidQuePrm(&avsyncPrm->queCfg[i],
                                             i,
                                             0,
                                            (0 + (avsyncPrm->numCh - 1)));
    }
    Vdis_setAvsyncConfig(VDIS_DEV_HDMI,avsyncPrm);

    status = Avsync_configSyncConfigInfo(avsyncPrm);
    OSA_assert(status == 0);

}

UInt32 Bell_getResBySize(UInt32 width, UInt32 height)
{
    if(width >= 1920 && height >= 1080){
        return VSYS_STD_1080P_60;
    }else if(width >= 1280 && height >= 720){
       return VSYS_STD_720P_60;
    }else if(width >= 1024 && height >= 768){
        return VSYS_STD_XGA_60;
    }else if(width >= 1280 && height >= 1024){
        return VSYS_STD_SXGA_60;
    }else if(width >= 720 && height >= 576){
        return VSYS_STD_PAL;
    }else if(width >= 720 && height >= 480){
        return VSYS_STD_NTSC;    
    }else if(width >= 640 && height >= 480){
        return VSYS_STD_VGA_60;
    }else if(width >= 352 && height >= 288){
        return VSYS_STD_CIF;
    }else{
        return VSYS_STD_PAL;
    }
}

static Void bell_config_cap_swms(SwMsLink_CreateParams *capswMsPrm, Int32 capSwMsId, UInt32 mode)
{
	AvsyncLink_LinkSynchConfigParams avsyncPrms;
	MULTICH_INIT_STRUCT(AvsyncLink_LinkSynchConfigParams,avsyncPrms);
	Int32 outWidth = gVencModuleContext.vencConfig.encChannelParams[0].videoWidth;
	Int32 outHeight = gVencModuleContext.vencConfig.encChannelParams[0].videoHeight;

	gVcapModuleContext.capSwMsId = capSwMsId;
	capswMsPrm->numSwMsInst = 1;
	capswMsPrm->swMsInstId[0] = SYSTEM_SW_MS_SC_INST_SC5;
	capswMsPrm->swMsInstStartWin[0]	= 0;

	capswMsPrm->maxInputQueLen	= 0;
  	capswMsPrm->maxOutRes       = Bell_getResBySize(outWidth, outHeight);
	capswMsPrm->lineSkipMode = FALSE; // Double pitch not possible in tiler mode; so Line skip not possible
	capswMsPrm->enableLayoutGridDraw = gVdisModuleContext.vdisConfig.enableLayoutGridDraw;

	MultiCh_setCustomVcapSwMs(capswMsPrm, TRUE, mode);
	/* Set dummy avsync params & disable AVSync as this swMS Out is not going to display */
	MultiCh_setCustomAvsyncPrm(&avsyncPrms, 
					capswMsPrm->inQueParams.prevLinkId,
					capswMsPrm->inQueParams.prevLinkQueId,
					capSwMsId);

	Bell_link_create(capSwMsId, capswMsPrm, sizeof(*capswMsPrm));
}

static Void bell_config_swms(SwMsLink_CreateParams *swMsPrm, Int32 swms_id)
{
    gVdisModuleContext.swMsId[0]      = swms_id;

    swMsPrm->numSwMsInst = 1;
    swMsPrm->swMsInstId[0] = SYSTEM_SW_MS_SC_INST_DEI_SC_NO_DEI;
    swMsPrm->maxInputQueLen            = 4; // KC: changed to 4 to make the display smooth
    swMsPrm->maxOutRes                 = gVdisModuleContext.vdisConfig.deviceParams[0].resolution;
    swMsPrm->numOutBuf                 = 8;
#ifdef  SYSTEM_USE_TILER
    swMsPrm->lineSkipMode = FALSE; // Double pitch not possible in tiler mode; so Line skip not possible
#else
    swMsPrm->lineSkipMode = TRUE; // Set to TRUE for Enable low cost scaling
#endif
    swMsPrm->enableLayoutGridDraw = gVdisModuleContext.vdisConfig.enableLayoutGridDraw;

    Bell_swMsGetDefaultLayoutPrm(VDIS_DEV_HDMI, swMsPrm, TRUE);

    Bell_link_create(gVdisModuleContext.swMsId[0], swMsPrm, sizeof(*swMsPrm));
}

static Void bell_config_disp(DisplayLink_CreateParams *displayPrm, Int32 disp_id)
{
    gVdisModuleContext.displayId[0] = SYSTEM_LINK_ID_DISPLAY_0; // ON CHIP HDMI

    displayPrm->displayRes                    = gVdisModuleContext.vdisConfig.deviceParams[0].resolution;    
    Bell_link_create(gVdisModuleContext.displayId[0], displayPrm, sizeof(*displayPrm));
}
/* =============================================================================
 * Use case code
 * =============================================================================
 */
#if 0
static
void bell_create_path_no_mosaic()
{
	CameraLink_CreateParams 	cameraPrm;	
	DisplayLink_CreateParams	 displayPrm[VDIS_DEV_MAX];
	IpcLink_CreateParams		ipcOutVpssPrm;
	IpcLink_CreateParams		ipcInVpssPrm;
	static SwMsLink_CreateParams	  swMsPrm[VDIS_DEV_MAX];
	MergeLink_CreateParams			  mergePrm[2];
	DupLink_CreateParams			  dupPrm[2];
	
	Bool   enableEncDec = gVsysModuleContext.vsysConfig.enableEncode && gVsysModuleContext.vsysConfig.enableDecode;
	
	Bell_detectBoard();
	
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
	
	if(enableEncDec)
	{	
		MULTICH_INIT_STRUCT(IpcLink_CreateParams,ipcOutVpssPrm);
		MULTICH_INIT_STRUCT(IpcLink_CreateParams,ipcInVpssPrm);
	}
	
	MULTICH_INIT_STRUCT(SwMsLink_CreateParams ,swMsPrm[0]);
	MULTICH_INIT_STRUCT(DisplayLink_CreateParams,displayPrm[0]);
	
	//CameraLink_CreateParams_Init(&cameraPrm);
	bell_link_join(SYSTEM_LINK_ID_CAMERA, 0, &cameraPrm.outQueParams[0], SYSTEM_VPSS_LINK_ID_DUP_0, &dupPrm[0].inQueParams);
	bell_camera_config(&cameraPrm, SYSTEM_LINK_ID_CAMERA);
	
	bell_link_join(SYSTEM_VPSS_LINK_ID_DUP_0,0,&dupPrm[0].outQueParams[0],SYSTEM_VPSS_LINK_ID_IPC_OUT_M3_0,&ipcOutVpssPrm.inQueParams);
	bell_link_join(SYSTEM_VPSS_LINK_ID_DUP_0,1,&dupPrm[0].outQueParams[1],SYSTEM_VPSS_LINK_ID_MERGE_0,&mergePrm[0].inQueParams[1]);
	bell_config_dup(&dupPrm[0], SYSTEM_VPSS_LINK_ID_DUP_0, 2);
	
	System_LinkInfo bitsProducerLinkInfo;
	bell_link_join(SYSTEM_VPSS_LINK_ID_IPC_IN_M3_0,0,&ipcInVpssPrm.outQueParams[0],SYSTEM_VPSS_LINK_ID_MERGE_0,&mergePrm[0].inQueParams[0]);
	bell_config_encode(&ipcOutVpssPrm, &bitsProducerLinkInfo);
	bell_config_decode(&ipcInVpssPrm, &bitsProducerLinkInfo);
	
		//-----
	bell_link_join(SYSTEM_VPSS_LINK_ID_MERGE_0, 0, &mergePrm[0].outQueParams, SYSTEM_LINK_ID_SW_MS_MULTI_INST_0, &swMsPrm[0].inQueParams);
	bell_config_merge(&mergePrm[0],SYSTEM_VPSS_LINK_ID_MERGE_0,2);
	
		
	bell_link_join(SYSTEM_LINK_ID_SW_MS_MULTI_INST_0, 0, &swMsPrm[0].outQueParams, SYSTEM_LINK_ID_DISPLAY_0, &displayPrm[0].inQueParams[0]);
	bell_config_swms(&swMsPrm[0], SYSTEM_LINK_ID_SW_MS_MULTI_INST_0);
	
	bell_config_disp(&displayPrm[0],SYSTEM_LINK_ID_DISPLAY_0);
	
	Bell_memPrintHeapStatus();
}
#endif

static
void bell_create_path_belllite(CodecPrm *prm)
{
	CameraLink_CreateParams 	cameraPrm;
	DisplayLink_CreateParams    displayPrm;
	SclrLink_CreateParams       sclrPrm;
	DupLink_CreateParams		dupPrm;
	IpcLink_CreateParams		ipcOutVpssPrm;
	NsfLink_CreateParams		nsfPrm;
	SwMsLink_CreateParams		capswMsPrm;


	Bool   enableEncDec = gVsysModuleContext.vsysConfig.enableEncode && gVsysModuleContext.vsysConfig.enableDecode;
	

	Bell_detectBoard();

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
	MULTICH_INIT_STRUCT(DisplayLink_CreateParams,displayPrm);
	MULTICH_INIT_STRUCT(SclrLink_CreateParams,sclrPrm);
	MULTICH_INIT_STRUCT(SwMsLink_CreateParams ,capswMsPrm);
	MULTICH_INIT_STRUCT(NsfLink_CreateParams, nsfPrm);
	
	if(enableEncDec)
	{	
		MULTICH_INIT_STRUCT(IpcLink_CreateParams,ipcOutVpssPrm);
	}

	bell_link_join(SYSTEM_LINK_ID_CAMERA, 0, &cameraPrm.outQueParams[0], SYSTEM_VPSS_LINK_ID_DUP_0, &dupPrm.inQueParams);
	bell_camera_config(&cameraPrm, SYSTEM_LINK_ID_CAMERA);
		
	bell_link_join(SYSTEM_VPSS_LINK_ID_DUP_0,0,&dupPrm.outQueParams[0],SYSTEM_LINK_ID_SCLR_INST_0,&sclrPrm.inQueParams);
	bell_link_join(SYSTEM_VPSS_LINK_ID_DUP_0,1,&dupPrm.outQueParams[1],SYSTEM_LINK_ID_SW_MS_MULTI_INST_1,&capswMsPrm.inQueParams);
	bell_config_dup(&dupPrm, SYSTEM_VPSS_LINK_ID_DUP_0,2);

	bell_link_join(SYSTEM_LINK_ID_SW_MS_MULTI_INST_1, 0, &capswMsPrm.outQueParams, SYSTEM_LINK_ID_NSF_0, &nsfPrm.inQueParams);
	bell_config_cap_swms(&capswMsPrm, SYSTEM_LINK_ID_SW_MS_MULTI_INST_1, 0);

	bell_link_join(SYSTEM_LINK_ID_NSF_0, 0, &nsfPrm.outQueParams[0], SYSTEM_VPSS_LINK_ID_IPC_OUT_M3_0, &ipcOutVpssPrm.inQueParams);
	bell_config_nsf(&nsfPrm, SYSTEM_LINK_ID_NSF_0, 1);

	bell_config_encode(&ipcOutVpssPrm);

	bell_link_join(SYSTEM_LINK_ID_SCLR_INST_0, 0, &sclrPrm.outQueParams, SYSTEM_LINK_ID_DISPLAY_0, &displayPrm.inQueParams[0]);
	bell_config_sclr(&sclrPrm, SYSTEM_LINK_ID_SCLR_INST_0, 1);

	bell_config_disp(&displayPrm,SYSTEM_LINK_ID_DISPLAY_0);

	Bell_memPrintHeapStatus();
}

Void bell_create_path_ipncdemo(CodecPrm *prm)
{
    CameraLink_CreateParams     cameraPrm;
    DisplayLink_CreateParams    displayPrm[VDIS_DEV_MAX];
    IpcLink_CreateParams        ipcOutVpssPrm;
    IpcLink_CreateParams        ipcInVpssPrm;

    static SwMsLink_CreateParams      swMsPrm[VDIS_DEV_MAX];
	MergeLink_CreateParams            mergePrm[2];
    DupLink_CreateParams              dupPrm[2];

    Bool enableOsdAlgLink = gVsysModuleContext.vsysConfig.enableOsd;
    Bool enableEncDec = gVsysModuleContext.vsysConfig.enableEncode && gVsysModuleContext.vsysConfig.enableDecode;

    Int32 osdAlgIP = SYSTEM_LINK_ID_VPSS_ALG_0;

    if(enableOsdAlgLink == FALSE)
    {
        osdAlgIP = SYSTEM_LINK_ID_INVALID;
    }
    
    Bell_detectBoard();

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

    if(enableEncDec)
    {   
        MULTICH_INIT_STRUCT(IpcLink_CreateParams,ipcOutVpssPrm);
        MULTICH_INIT_STRUCT(IpcLink_CreateParams,ipcInVpssPrm);
    }

    MULTICH_INIT_STRUCT(SwMsLink_CreateParams ,swMsPrm[0]);
    MULTICH_INIT_STRUCT(DisplayLink_CreateParams,displayPrm[0]);

    //CameraLink_CreateParams_Init(&cameraPrm);
    bell_link_join(SYSTEM_LINK_ID_CAMERA, 0, &cameraPrm.outQueParams[0], SYSTEM_VPSS_LINK_ID_DUP_0,&dupPrm[0].inQueParams);
    bell_camera_config(&cameraPrm, SYSTEM_LINK_ID_CAMERA);

    bell_link_join(SYSTEM_VPSS_LINK_ID_DUP_0,0,&dupPrm[0].outQueParams[0],SYSTEM_VPSS_LINK_ID_MERGE_1,&mergePrm[1].inQueParams[0]);
    bell_link_join(SYSTEM_VPSS_LINK_ID_DUP_0,1,&dupPrm[0].outQueParams[1],SYSTEM_VPSS_LINK_ID_MERGE_0,&mergePrm[0].inQueParams[1]);
    bell_config_dup(&dupPrm[0], SYSTEM_VPSS_LINK_ID_DUP_0, 2);
	       
    if(osdAlgIP == SYSTEM_LINK_ID_ALG_0)
    {   
        IpcFramesInLinkRTOS_CreateParams  ipcFramesInDspPrm;
        IpcFramesOutLinkRTOS_CreateParams ipcFramesOutVpssPrm;
        MULTICH_INIT_STRUCT(IpcFramesInLinkRTOS_CreateParams,ipcFramesInDspPrm);
        MULTICH_INIT_STRUCT(IpcFramesOutLinkRTOS_CreateParams,ipcFramesOutVpssPrm);
        
        bell_link_join(SYSTEM_VPSS_LINK_ID_MERGE_1,0,&mergePrm[1].outQueParams,SYSTEM_VPSS_LINK_ID_IPC_FRAMES_OUT_0,&ipcFramesOutVpssPrm.baseCreateParams.inQueParams);
        bell_config_merge(&mergePrm[1],SYSTEM_VPSS_LINK_ID_MERGE_1,1);

        if(enableEncDec)
            bell_link_join(SYSTEM_VPSS_LINK_ID_IPC_FRAMES_OUT_0,ALG_LINK_FRAMES_OUT_QUE, &ipcFramesOutVpssPrm.baseCreateParams.outQueParams[ALG_LINK_FRAMES_OUT_QUE],SYSTEM_VPSS_LINK_ID_IPC_OUT_M3_0,&ipcOutVpssPrm.inQueParams);
        else
            bell_link_join(SYSTEM_VPSS_LINK_ID_IPC_FRAMES_OUT_0,ALG_LINK_FRAMES_OUT_QUE, &ipcFramesOutVpssPrm.baseCreateParams.outQueParams[ALG_LINK_FRAMES_OUT_QUE],SYSTEM_VPSS_LINK_ID_MERGE_0,&mergePrm[0].inQueParams[0]);

        bell_config_dsp_osd(&ipcFramesOutVpssPrm, &ipcFramesInDspPrm);
    }
    else if(osdAlgIP == SYSTEM_LINK_ID_VPSS_ALG_0)
    {
        AlgLink_CreateParams vpssAlgPrm;
        bell_link_join(SYSTEM_VPSS_LINK_ID_MERGE_1,0,&mergePrm[1].outQueParams,SYSTEM_LINK_ID_VPSS_ALG_0,&vpssAlgPrm.inQueParams);
        bell_config_merge(&mergePrm[1],SYSTEM_VPSS_LINK_ID_MERGE_1,1);
        if(enableEncDec)
            bell_link_join(SYSTEM_LINK_ID_VPSS_ALG_0,ALG_LINK_FRAMES_OUT_QUE,&vpssAlgPrm.outQueParams[ALG_LINK_FRAMES_OUT_QUE], SYSTEM_VPSS_LINK_ID_IPC_OUT_M3_0,&ipcOutVpssPrm.inQueParams);
        else
            bell_link_join(SYSTEM_LINK_ID_VPSS_ALG_0,ALG_LINK_FRAMES_OUT_QUE,&vpssAlgPrm.outQueParams[ALG_LINK_FRAMES_OUT_QUE],SYSTEM_VPSS_LINK_ID_MERGE_0,&mergePrm[0].inQueParams[0]);
        bell_config_vpss_osd(&vpssAlgPrm);
    }
    else
    {
        OSA_assert(osdAlgIP == SYSTEM_LINK_ID_INVALID);
        
        if(enableEncDec)
            bell_link_join(SYSTEM_VPSS_LINK_ID_MERGE_1,0,&mergePrm[1].outQueParams, SYSTEM_VPSS_LINK_ID_IPC_OUT_M3_0,&ipcOutVpssPrm.inQueParams);
        else
            bell_link_join(SYSTEM_VPSS_LINK_ID_MERGE_1,0,&mergePrm[1].outQueParams,SYSTEM_VPSS_LINK_ID_MERGE_0,&mergePrm[0].inQueParams[0]);
        bell_config_merge(&mergePrm[1],SYSTEM_VPSS_LINK_ID_MERGE_1,1);        
    }

	if(enableEncDec)
	{
        bell_link_join(SYSTEM_VPSS_LINK_ID_IPC_IN_M3_0,0,&ipcInVpssPrm.outQueParams[0],SYSTEM_VPSS_LINK_ID_MERGE_0,&mergePrm[0].inQueParams[0]);
        bell_config_encode(&ipcOutVpssPrm);
        bell_config_decode(&ipcInVpssPrm);
	}

    bell_link_join(SYSTEM_VPSS_LINK_ID_MERGE_0, 0, &mergePrm[0].outQueParams, SYSTEM_LINK_ID_SW_MS_MULTI_INST_0, &swMsPrm[0].inQueParams);
    bell_config_merge(&mergePrm[0],SYSTEM_VPSS_LINK_ID_MERGE_0,2);

    bell_link_join(SYSTEM_LINK_ID_SW_MS_MULTI_INST_0, 0, &swMsPrm[0].outQueParams, SYSTEM_LINK_ID_DISPLAY_0, &displayPrm[0].inQueParams[0]);
    bell_config_swms(&swMsPrm[0], SYSTEM_LINK_ID_SW_MS_MULTI_INST_0);

    bell_config_disp(&displayPrm[0],SYSTEM_LINK_ID_DISPLAY_0);

    Bell_memPrintHeapStatus();

}

Void bell_create_path_preview(CodecPrm *prm)
{
	CameraLink_CreateParams 	cameraPrm;
	DisplayLink_CreateParams    displayPrm[VDIS_DEV_MAX];
	SclrLink_CreateParams       sclrPrm;

	Bell_detectBoard();

	System_linkControl(
		SYSTEM_LINK_ID_M3VPSS,
		SYSTEM_M3VPSS_CMD_RESET_VIDEO_DEVICES,
		NULL,
		0,
		TRUE
		);
/*
	System_linkControl(
		SYSTEM_LINK_ID_M3VIDEO,
		SYSTEM_COMMON_CMD_SET_CH2IVAHD_MAP_TBL,
		&systemVid_encDecIvaChMapTbl,
		sizeof(SystemVideo_Ivahd2ChMap_Tbl),
		TRUE
		);
	MULTICH_INIT_STRUCT(DisplayLink_CreateParams,displayPrm[0]);
	*/
	MULTICH_INIT_STRUCT(DisplayLink_CreateParams,displayPrm[0]);
	MULTICH_INIT_STRUCT(SclrLink_CreateParams,sclrPrm);

	bell_link_join(SYSTEM_LINK_ID_CAMERA, 0, &cameraPrm.outQueParams[0], SYSTEM_LINK_ID_SCLR_INST_0,&sclrPrm.inQueParams);
	bell_camera_config(&cameraPrm, SYSTEM_LINK_ID_CAMERA);


	bell_link_join(SYSTEM_LINK_ID_SCLR_INST_0, 0, &sclrPrm.outQueParams, SYSTEM_LINK_ID_DISPLAY_0, &displayPrm[0].inQueParams[0]);
	bell_config_sclr(&sclrPrm, SYSTEM_LINK_ID_SCLR_INST_0, 1);

	bell_config_disp(&displayPrm[0],SYSTEM_LINK_ID_DISPLAY_0);

	Bell_memPrintHeapStatus();
}

static Void bell_create_path_bell_base(CodecPrm *prm,int mode)
{
	CameraLink_CreateParams     cameraPrm;
	CaptureLink_CreateParams    capturePrm;
	NsfLink_CreateParams		capnsfPrm;
	DisplayLink_CreateParams	displayPrm[VDIS_DEV_MAX];
	IpcLink_CreateParams		ipcOutVpssPrm;
	IpcLink_CreateParams		ipcInVpssPrm;
	NsfLink_CreateParams		nsfPrm;

	IpcBitsInLinkRTOS_CreateParams	  ipcBitsInVideoPrm;
	static SwMsLink_CreateParams	   dispswMsPrm;
	SwMsLink_CreateParams		capswMsPrm;

	MergeLink_CreateParams		mergePrm[2];
	DupLink_CreateParams		dupPrm[3];

	UInt32 i;

    int has_hdmi = mode & MODE_CAP_MSC_HDMI_IN;
    int has_remote = mode & MODE_CAP_MSC_REMOTE;
    
	MULTICH_INIT_STRUCT(IpcLink_CreateParams,ipcOutVpssPrm);
	MULTICH_INIT_STRUCT(IpcLink_CreateParams,ipcInVpssPrm);
	MULTICH_INIT_STRUCT(IpcBitsInLinkRTOS_CreateParams,ipcBitsInVideoPrm);
	MULTICH_INIT_STRUCT(SwMsLink_CreateParams ,capswMsPrm);
	MULTICH_INIT_STRUCT(SwMsLink_CreateParams ,dispswMsPrm);
	MULTICH_INIT_STRUCT(NsfLink_CreateParams, nsfPrm);
    if(has_hdmi)
    {
        MULTICH_INIT_STRUCT(NsfLink_CreateParams, capnsfPrm);
    	CaptureLink_CreateParams_Init(&capturePrm);
    }
	for (i = 0; i < VDIS_DEV_MAX; i++)
	{
		MULTICH_INIT_STRUCT(DisplayLink_CreateParams,displayPrm[i]);
	}

	Bell_detectBoard();

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

	bell_link_join(SYSTEM_LINK_ID_CAMERA, 0, &cameraPrm.outQueParams[0], SYSTEM_VPSS_LINK_ID_DUP_0,&dupPrm[0].inQueParams);
	bell_camera_config(&cameraPrm, SYSTEM_LINK_ID_CAMERA);

	bell_link_join(SYSTEM_VPSS_LINK_ID_IPC_IN_M3_0,0,&ipcInVpssPrm.outQueParams[0],SYSTEM_VPSS_LINK_ID_DUP_1,&dupPrm[1].inQueParams);
	bell_config_decode_indep(&ipcInVpssPrm);

    if(has_hdmi)
    {
    	bell_link_join(SYSTEM_LINK_ID_CAPTURE, 0, &capturePrm.outQueParams[0], SYSTEM_LINK_ID_NSF_1,&capnsfPrm.inQueParams);
    	bell_captrue_config(&capturePrm, SYSTEM_LINK_ID_CAPTURE);

    	bell_link_join(SYSTEM_LINK_ID_NSF_1, 0, &capnsfPrm.outQueParams[0], SYSTEM_VPSS_LINK_ID_DUP_2,&dupPrm[2].inQueParams);
    	bell_config_nsf(&capnsfPrm, SYSTEM_LINK_ID_NSF_1, 1);
    }

	bell_link_join(SYSTEM_VPSS_LINK_ID_DUP_0,0,&dupPrm[0].outQueParams[0],SYSTEM_VPSS_LINK_ID_MERGE_0,&mergePrm[0].inQueParams[1]);
	bell_link_join(SYSTEM_VPSS_LINK_ID_DUP_0,1,&dupPrm[0].outQueParams[1],SYSTEM_VPSS_LINK_ID_MERGE_1,&mergePrm[1].inQueParams[0]);
	bell_config_dup(&dupPrm[0], SYSTEM_VPSS_LINK_ID_DUP_0, 2);

    if(!has_remote)
    {
        bell_link_join(SYSTEM_VPSS_LINK_ID_DUP_1,0,&dupPrm[1].outQueParams[0],SYSTEM_VPSS_LINK_ID_MERGE_0,&mergePrm[0].inQueParams[0]);
    	bell_config_dup(&dupPrm[1], SYSTEM_VPSS_LINK_ID_DUP_1, 1);
    }
    else
    {
        bell_link_join(SYSTEM_VPSS_LINK_ID_DUP_1,0,&dupPrm[1].outQueParams[0],SYSTEM_VPSS_LINK_ID_MERGE_0,&mergePrm[0].inQueParams[0]);
    	bell_link_join(SYSTEM_VPSS_LINK_ID_DUP_1,1,&dupPrm[1].outQueParams[1],SYSTEM_VPSS_LINK_ID_MERGE_1,&mergePrm[1].inQueParams[1]);
    	bell_config_dup(&dupPrm[1], SYSTEM_VPSS_LINK_ID_DUP_1, 2);
    }

    if(has_hdmi)
    {
        bell_link_join(SYSTEM_VPSS_LINK_ID_DUP_2,0,&dupPrm[2].outQueParams[0],SYSTEM_VPSS_LINK_ID_MERGE_0,&mergePrm[0].inQueParams[2]);
        if(!has_remote)
        	bell_link_join(SYSTEM_VPSS_LINK_ID_DUP_2,1,&dupPrm[2].outQueParams[1],SYSTEM_VPSS_LINK_ID_MERGE_1,&mergePrm[1].inQueParams[1]);
        else
    	    bell_link_join(SYSTEM_VPSS_LINK_ID_DUP_2,1,&dupPrm[2].outQueParams[1],SYSTEM_VPSS_LINK_ID_MERGE_1,&mergePrm[1].inQueParams[2]);

    	bell_config_dup(&dupPrm[2], SYSTEM_VPSS_LINK_ID_DUP_2, 2);
    }

	bell_link_join(SYSTEM_VPSS_LINK_ID_MERGE_0, 0, &mergePrm[0].outQueParams, SYSTEM_LINK_ID_SW_MS_MULTI_INST_0, &dispswMsPrm.inQueParams);
    if(has_hdmi)
    	bell_config_merge(&mergePrm[0],SYSTEM_VPSS_LINK_ID_MERGE_0,3);
    else
	    bell_config_merge(&mergePrm[0],SYSTEM_VPSS_LINK_ID_MERGE_0,2);

	bell_link_join(SYSTEM_VPSS_LINK_ID_MERGE_1, 0, &mergePrm[1].outQueParams, SYSTEM_LINK_ID_SW_MS_MULTI_INST_1, &capswMsPrm.inQueParams);
    if(!has_remote)
    {
        if(has_hdmi)
        	bell_config_merge(&mergePrm[1],SYSTEM_VPSS_LINK_ID_MERGE_1,2);
        else
    	    bell_config_merge(&mergePrm[1],SYSTEM_VPSS_LINK_ID_MERGE_1,1);
    }
    else
    {
        if(has_hdmi)
        	bell_config_merge(&mergePrm[1],SYSTEM_VPSS_LINK_ID_MERGE_1,3);
        else
    	    bell_config_merge(&mergePrm[1],SYSTEM_VPSS_LINK_ID_MERGE_1,2);
    }

	bell_link_join(SYSTEM_LINK_ID_SW_MS_MULTI_INST_1, 0, &capswMsPrm.outQueParams, SYSTEM_LINK_ID_NSF_0, &nsfPrm.inQueParams);
	bell_config_cap_swms(&capswMsPrm, SYSTEM_LINK_ID_SW_MS_MULTI_INST_1, mode);

	bell_link_join(SYSTEM_LINK_ID_NSF_0, 0, &nsfPrm.outQueParams[0], SYSTEM_VPSS_LINK_ID_IPC_OUT_M3_0, &ipcOutVpssPrm.inQueParams);
	bell_config_nsf(&nsfPrm, SYSTEM_LINK_ID_NSF_0, 1);

	bell_config_encode(&ipcOutVpssPrm);

	bell_link_join(SYSTEM_LINK_ID_SW_MS_MULTI_INST_0, 0, &dispswMsPrm.outQueParams, SYSTEM_LINK_ID_DISPLAY_0, &displayPrm[0].inQueParams[0]);
	bell_config_swms(&dispswMsPrm, SYSTEM_LINK_ID_SW_MS_MULTI_INST_0);

    bell_config_disp(&displayPrm[0],SYSTEM_LINK_ID_DISPLAY_0);

	Bell_memPrintHeapStatus();

}

static Void bell_create_path_usecase_0(CodecPrm *prm)
{
	CameraLink_CreateParams 	cameraPrm;
	DisplayLink_CreateParams	displayPrm[VDIS_DEV_MAX];
	SclrLink_CreateParams		sclrPrm;
	IpcLink_CreateParams        ipcOutVpssPrm;
	IpcLink_CreateParams        ipcInVpssPrm;

	Bell_detectBoard();

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

	MULTICH_INIT_STRUCT(DisplayLink_CreateParams,displayPrm[0]);
	MULTICH_INIT_STRUCT(SclrLink_CreateParams,sclrPrm);
	MULTICH_INIT_STRUCT(IpcLink_CreateParams,ipcOutVpssPrm);
	MULTICH_INIT_STRUCT(IpcLink_CreateParams,ipcInVpssPrm);

    bell_link_join(SYSTEM_LINK_ID_CAMERA, 0, &cameraPrm.outQueParams[0], SYSTEM_VPSS_LINK_ID_IPC_OUT_M3_0, &ipcOutVpssPrm.inQueParams);
    bell_camera_config(&cameraPrm, SYSTEM_LINK_ID_CAMERA);

	bell_link_join(SYSTEM_VPSS_LINK_ID_IPC_IN_M3_0,0,&ipcInVpssPrm.outQueParams[0],SYSTEM_LINK_ID_SCLR_INST_0,&sclrPrm.inQueParams);
	bell_config_decode_indep(&ipcInVpssPrm);


	bell_config_encode(&ipcOutVpssPrm);

	bell_link_join(SYSTEM_LINK_ID_SCLR_INST_0, 0, &sclrPrm.outQueParams, SYSTEM_LINK_ID_DISPLAY_0, &displayPrm[0].inQueParams[0]);
	bell_config_sclr(&sclrPrm, SYSTEM_LINK_ID_SCLR_INST_0, 1);

	bell_config_disp(&displayPrm[0],SYSTEM_LINK_ID_DISPLAY_0);

	Bell_memPrintHeapStatus();
}

static Void bell_create_path_usecase_1(CodecPrm *prm)
{
	CameraLink_CreateParams 	cameraPrm;
	DisplayLink_CreateParams	displayPrm[VDIS_DEV_MAX];
	DupLink_CreateParams		dupPrm;
	IpcLink_CreateParams		ipcOutVpssPrm;
	IpcLink_CreateParams		ipcInVpssPrm;
	MergeLink_CreateParams		mergePrm;
	static SwMsLink_CreateParams	   dispswMsPrm;

	Bell_detectBoard();

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

	MULTICH_INIT_STRUCT(DisplayLink_CreateParams,displayPrm[0]);
	MULTICH_INIT_STRUCT(IpcLink_CreateParams,ipcOutVpssPrm);
	MULTICH_INIT_STRUCT(IpcLink_CreateParams,ipcInVpssPrm);
	MULTICH_INIT_STRUCT(SwMsLink_CreateParams ,dispswMsPrm);

	bell_link_join(SYSTEM_LINK_ID_CAMERA, 0, &cameraPrm.outQueParams[0], SYSTEM_VPSS_LINK_ID_DUP_0,&dupPrm.inQueParams);
	bell_camera_config(&cameraPrm, SYSTEM_LINK_ID_CAMERA);

	bell_link_join(SYSTEM_VPSS_LINK_ID_IPC_IN_M3_0,0,&ipcInVpssPrm.outQueParams[0],SYSTEM_VPSS_LINK_ID_MERGE_0,&mergePrm.inQueParams[0]);
	bell_config_decode_indep(&ipcInVpssPrm);

	bell_link_join(SYSTEM_VPSS_LINK_ID_DUP_0,1,&dupPrm.outQueParams[0],SYSTEM_VPSS_LINK_ID_MERGE_0,&mergePrm.inQueParams[1]);
	bell_link_join(SYSTEM_VPSS_LINK_ID_DUP_0,0,&dupPrm.outQueParams[1],SYSTEM_VPSS_LINK_ID_IPC_OUT_M3_0, &ipcOutVpssPrm.inQueParams);
	bell_config_dup(&dupPrm, SYSTEM_VPSS_LINK_ID_DUP_0, 2);

	bell_link_join(SYSTEM_VPSS_LINK_ID_MERGE_0, 0, &mergePrm.outQueParams, SYSTEM_LINK_ID_SW_MS_MULTI_INST_0, &dispswMsPrm.inQueParams);
    bell_config_merge(&mergePrm,SYSTEM_VPSS_LINK_ID_MERGE_0,2);

	bell_config_encode(&ipcOutVpssPrm);

	bell_link_join(SYSTEM_LINK_ID_SW_MS_MULTI_INST_0, 0, &dispswMsPrm.outQueParams, SYSTEM_LINK_ID_DISPLAY_0, &displayPrm[0].inQueParams[0]);
	bell_config_swms(&dispswMsPrm, SYSTEM_LINK_ID_SW_MS_MULTI_INST_0);

	bell_config_disp(&displayPrm[0],SYSTEM_LINK_ID_DISPLAY_0);

	Bell_memPrintHeapStatus();
}

static Void bell_create_path_usecase_2(CodecPrm *prm)
{
	bell_create_path_bell_base(prm,MODE_CAP_MSC_LOCAL | MODE_CAP_MSC_REMOTE);
}

static Void bell_create_path_usecase_3(CodecPrm *prm)
{
	bell_create_path_bell_base(prm,MODE_CAP_MSC_LOCAL | MODE_CAP_MSC_HDMI_IN);
}

static Void bell_create_path_usecase_4(CodecPrm *prm)
{
	bell_create_path_bell_base(prm,MODE_CAP_MSC_LOCAL | MODE_CAP_MSC_HDMI_IN | MODE_CAP_MSC_REMOTE);
}

static Void bell_create_path_usecase_5(CodecPrm *prm)
{
	bell_create_path_bell_base(prm,MODE_CAP_MSC_LOCAL | MODE_CAP_MSC_HDMI_IN | MODE_CAP_MSC_REMOTE | MODE_CAP_MSC_HDMI_MAIN);
}

static Void bell_create_path_usecase_6(void)
{
	CaptureLink_CreateParams    capturePrm;
	DisplayLink_CreateParams	displayPrm[VDIS_DEV_MAX];

	Bell_detectBoard();

	System_linkControl(
		SYSTEM_LINK_ID_M3VPSS,
		SYSTEM_M3VPSS_CMD_RESET_VIDEO_DEVICES,
		NULL,
		0,
		TRUE
		);

	CaptureLink_CreateParams_Init(&capturePrm);
	MULTICH_INIT_STRUCT(DisplayLink_CreateParams,displayPrm[0]);

	bell_link_join(SYSTEM_LINK_ID_CAPTURE, 0, &capturePrm.outQueParams[0], SYSTEM_LINK_ID_DISPLAY_0, &displayPrm[0].inQueParams[0]);
	bell_captrue_config(&capturePrm, SYSTEM_LINK_ID_CAPTURE);

	bell_config_disp(&displayPrm[0],SYSTEM_LINK_ID_DISPLAY_0);

	Bell_memPrintHeapStatus();
}

static Void bell_create_path_usecase_7(CodecPrm *prm)
{
	CameraLink_CreateParams 	cameraPrm;
	CaptureLink_CreateParams    capturePrm;
	MergeLink_CreateParams		mergePrm;
	SwMsLink_CreateParams		dispswMsPrm;
	DisplayLink_CreateParams	displayPrm[VDIS_DEV_MAX];

	Bell_detectBoard();

	System_linkControl(
		SYSTEM_LINK_ID_M3VPSS,
		SYSTEM_M3VPSS_CMD_RESET_VIDEO_DEVICES,
		NULL,
		0,
		TRUE
		);

	CaptureLink_CreateParams_Init(&capturePrm);
	MULTICH_INIT_STRUCT(SwMsLink_CreateParams ,dispswMsPrm);
	MULTICH_INIT_STRUCT(DisplayLink_CreateParams,displayPrm[0]);

	bell_link_join(SYSTEM_LINK_ID_CAMERA, 0, &cameraPrm.outQueParams[0], SYSTEM_VPSS_LINK_ID_MERGE_0,&mergePrm.inQueParams[0]);
	bell_camera_config(&cameraPrm, SYSTEM_LINK_ID_CAMERA);

	bell_link_join(SYSTEM_LINK_ID_CAPTURE, 0, &capturePrm.outQueParams[0], SYSTEM_VPSS_LINK_ID_MERGE_0,&mergePrm.inQueParams[1]);
	bell_captrue_config(&capturePrm, SYSTEM_LINK_ID_CAPTURE);

	bell_link_join(SYSTEM_VPSS_LINK_ID_MERGE_0, 0, &mergePrm.outQueParams, SYSTEM_LINK_ID_SW_MS_MULTI_INST_0, &dispswMsPrm.inQueParams);
    bell_config_merge(&mergePrm,SYSTEM_VPSS_LINK_ID_MERGE_0,2);

	bell_link_join(SYSTEM_LINK_ID_SW_MS_MULTI_INST_0, 0, &dispswMsPrm.outQueParams, SYSTEM_LINK_ID_DISPLAY_0, &displayPrm[0].inQueParams[0]);
	bell_config_swms(&dispswMsPrm, SYSTEM_LINK_ID_SW_MS_MULTI_INST_0);

	bell_config_disp(&displayPrm[0],SYSTEM_LINK_ID_DISPLAY_0);

	Bell_memPrintHeapStatus();
}

static Void bell_create_path_usecase_8(CodecPrm *prm)
{
	CameraLink_CreateParams 	cameraPrm;
	CaptureLink_CreateParams	capturePrm;
	MergeLink_CreateParams		mergePrm;
	SwMsLink_CreateParams		dispswMsPrm;
	DisplayLink_CreateParams	displayPrm[VDIS_DEV_MAX];

	Bell_detectBoard();

	System_linkControl(
		SYSTEM_LINK_ID_M3VPSS,
		SYSTEM_M3VPSS_CMD_RESET_VIDEO_DEVICES,
		NULL,
		0,
		TRUE
		);

	CaptureLink_CreateParams_Init(&capturePrm);
	MULTICH_INIT_STRUCT(SwMsLink_CreateParams ,dispswMsPrm);
	MULTICH_INIT_STRUCT(DisplayLink_CreateParams,displayPrm[0]);

	bell_link_join(SYSTEM_LINK_ID_CAMERA, 0, &cameraPrm.outQueParams[0], SYSTEM_VPSS_LINK_ID_MERGE_0,&mergePrm.inQueParams[1]);
	bell_camera_config(&cameraPrm, SYSTEM_LINK_ID_CAMERA);

	bell_link_join(SYSTEM_LINK_ID_CAPTURE, 0, &capturePrm.outQueParams[0], SYSTEM_VPSS_LINK_ID_MERGE_0,&mergePrm.inQueParams[0]);
	bell_captrue_config(&capturePrm, SYSTEM_LINK_ID_CAPTURE);

	bell_link_join(SYSTEM_VPSS_LINK_ID_MERGE_0, 0, &mergePrm.outQueParams, SYSTEM_LINK_ID_SW_MS_MULTI_INST_0, &dispswMsPrm.inQueParams);
	bell_config_merge(&mergePrm,SYSTEM_VPSS_LINK_ID_MERGE_0,2);

	bell_link_join(SYSTEM_LINK_ID_SW_MS_MULTI_INST_0, 0, &dispswMsPrm.outQueParams, SYSTEM_LINK_ID_DISPLAY_0, &displayPrm[0].inQueParams[0]);
	bell_config_swms(&dispswMsPrm, SYSTEM_LINK_ID_SW_MS_MULTI_INST_0);

	bell_config_disp(&displayPrm[0],SYSTEM_LINK_ID_DISPLAY_0);

	Bell_memPrintHeapStatus();
}


Void bell_create_path_bell_basic(CodecPrm *prm)
{
    bell_create_path_bell_base(prm,MODE_CAP_MSC_LOCAL);
}

Void bell_create_path_bell_basic_hdmi_in(CodecPrm *prm)
{
    bell_create_path_bell_base(prm,MODE_CAP_MSC_LOCAL | MODE_CAP_MSC_HDMI_IN |MODE_CAP_MSC_REMOTE);
}

Void bell_create_path_bell_hdmi_in(CodecPrm *prm)
{
    CameraLink_CreateParams     cameraPrm;
    CaptureLink_CreateParams    capturePrm;
    NsfLink_CreateParams        capnsfPrm;
    DisplayLink_CreateParams    displayPrm[VDIS_DEV_MAX];
    IpcLink_CreateParams        ipcOutVpssPrm;
    IpcLink_CreateParams        ipcInVpssPrm;
    NsfLink_CreateParams        nsfPrm;

    IpcBitsInLinkRTOS_CreateParams    ipcBitsInVideoPrm;
    static SwMsLink_CreateParams       dispswMsPrm;
    SwMsLink_CreateParams       capswMsPrm;

    MergeLink_CreateParams      mergePrm[2];
    DupLink_CreateParams        dupPrm[3];

    UInt32 i;

    int merge0_input_cnt = 0;
    int merge1_input_cnt = 0;
    
    MULTICH_INIT_STRUCT(IpcLink_CreateParams,ipcOutVpssPrm);
    MULTICH_INIT_STRUCT(IpcLink_CreateParams,ipcInVpssPrm);
    MULTICH_INIT_STRUCT(IpcBitsInLinkRTOS_CreateParams,ipcBitsInVideoPrm);
    MULTICH_INIT_STRUCT(SwMsLink_CreateParams ,capswMsPrm);
    MULTICH_INIT_STRUCT(SwMsLink_CreateParams ,dispswMsPrm);
    MULTICH_INIT_STRUCT(NsfLink_CreateParams, nsfPrm);
    MULTICH_INIT_STRUCT(NsfLink_CreateParams, capnsfPrm);
    CaptureLink_CreateParams_Init(&capturePrm);
    for (i = 0; i < VDIS_DEV_MAX; i++)
    {
        MULTICH_INIT_STRUCT(DisplayLink_CreateParams,displayPrm[i]);
    }

    Bell_detectBoard();

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

    bell_link_join(SYSTEM_LINK_ID_CAPTURE, 0, &capturePrm.outQueParams[0], SYSTEM_LINK_ID_NSF_1,&capnsfPrm.inQueParams);
    bell_captrue_config(&capturePrm, SYSTEM_LINK_ID_CAPTURE);

    bell_link_join(SYSTEM_LINK_ID_NSF_1, 0, &capnsfPrm.outQueParams[0], SYSTEM_VPSS_LINK_ID_DUP_0,&dupPrm[0].inQueParams);
    bell_config_nsf(&capnsfPrm, SYSTEM_LINK_ID_NSF_1, 1);
    
    bell_link_join(SYSTEM_VPSS_LINK_ID_IPC_IN_M3_0,0,&ipcInVpssPrm.outQueParams[0],SYSTEM_VPSS_LINK_ID_DUP_1,&dupPrm[1].inQueParams);
    bell_config_decode_indep(&ipcInVpssPrm);

    bell_link_join(SYSTEM_VPSS_LINK_ID_DUP_0,0,&dupPrm[0].outQueParams[0],SYSTEM_VPSS_LINK_ID_MERGE_0,&mergePrm[0].inQueParams[1]);
    bell_link_join(SYSTEM_VPSS_LINK_ID_DUP_0,1,&dupPrm[0].outQueParams[1],SYSTEM_VPSS_LINK_ID_MERGE_1,&mergePrm[1].inQueParams[0]);
    bell_config_dup(&dupPrm[0], SYSTEM_VPSS_LINK_ID_DUP_0, 2);
    merge0_input_cnt++;
    merge1_input_cnt++;

    bell_link_join(SYSTEM_VPSS_LINK_ID_DUP_1,0,&dupPrm[1].outQueParams[0],SYSTEM_VPSS_LINK_ID_MERGE_0,&mergePrm[0].inQueParams[0]);
    bell_config_dup(&dupPrm[1], SYSTEM_VPSS_LINK_ID_DUP_1, 1);        
    merge0_input_cnt++;

    bell_link_join(SYSTEM_VPSS_LINK_ID_MERGE_0, 0, &mergePrm[0].outQueParams, SYSTEM_LINK_ID_SW_MS_MULTI_INST_0, &dispswMsPrm.inQueParams);
    bell_config_merge(&mergePrm[0],SYSTEM_VPSS_LINK_ID_MERGE_0,merge0_input_cnt);
    bell_link_join(SYSTEM_VPSS_LINK_ID_MERGE_1, 0, &mergePrm[1].outQueParams, SYSTEM_LINK_ID_SW_MS_MULTI_INST_1, &capswMsPrm.inQueParams);
    bell_config_merge(&mergePrm[1],SYSTEM_VPSS_LINK_ID_MERGE_1,merge1_input_cnt);

    bell_link_join(SYSTEM_LINK_ID_SW_MS_MULTI_INST_1, 0, &capswMsPrm.outQueParams, SYSTEM_LINK_ID_NSF_0, &nsfPrm.inQueParams);
    bell_config_cap_swms(&capswMsPrm, SYSTEM_LINK_ID_SW_MS_MULTI_INST_1, 0);

    bell_link_join(SYSTEM_LINK_ID_NSF_0, 0, &nsfPrm.outQueParams[0], SYSTEM_VPSS_LINK_ID_IPC_OUT_M3_0, &ipcOutVpssPrm.inQueParams);
    bell_config_nsf(&nsfPrm, SYSTEM_LINK_ID_NSF_0, 1);

    bell_config_encode(&ipcOutVpssPrm);

    bell_link_join(SYSTEM_LINK_ID_SW_MS_MULTI_INST_0, 0, &dispswMsPrm.outQueParams, SYSTEM_LINK_ID_DISPLAY_0, &displayPrm[0].inQueParams[0]);
    bell_config_swms(&dispswMsPrm, SYSTEM_LINK_ID_SW_MS_MULTI_INST_0);

    bell_config_disp(&displayPrm[0],SYSTEM_LINK_ID_DISPLAY_0);

    Bell_memPrintHeapStatus();
}

Void bell_create_path(CodecPrm *prm)
{
    Int32 i = 0;
    for(i = 0; i < LINK_IN_USE_NUM; i++)
    {
        Links_in_use[i] = SYSTEM_LINK_ID_INVALID;
    }
    
    switch (prm->usecase) {
		case BELL_USECASE_PREVIEW:
			printf("%s():%s\n", __FUNCTION__,"BELL_USECASE_PREVIEW");
			bell_create_path_preview(prm);
			break;
        case BELL_USECASE_IPNCDEMO:
            printf("%s():%s\n", __FUNCTION__,"BELL_USECASE_IPNCDEMO");
            bell_create_path_ipncdemo(prm);
            break;
	    case BELL_USECASE_BELLLITE:
            printf("%s():%s\n", __FUNCTION__,"BELL_USECASE_BELLLITE");      //bell preview mode usecase
            bell_create_path_belllite(prm);
            break;
        case BELL_USECASE_BELL_BASIC:
            printf("%s():%s\n", __FUNCTION__,"BELL_USECASE_BELL_BASIC");    //no hdmi in
            bell_create_path_bell_basic(prm);
            break;
		case BELL_USECASE_BELL_BASIC_HDMI_IN:
			printf("%s():%s\n", __FUNCTION__,"BELL_USECASE_BELL_HDMI_IN");	//hdmi in
			bell_create_path_bell_basic_hdmi_in(prm);
			break;
		case BELL_USECASE_MODE0:
			bell_create_path_usecase_0(prm);
			break;
		case BELL_USECASE_MODE1:
			bell_create_path_usecase_1(prm);
			break;
		case BELL_USECASE_MODE2:
			bell_create_path_usecase_2(prm);
			break;
		case BELL_USECASE_MODE3:
			bell_create_path_usecase_3(prm);
			break;
		case BELL_USECASE_MODE4:
			bell_create_path_usecase_4(prm);
			break;
		case BELL_USECASE_MODE5_0:
			bell_create_path_bell_hdmi_in(prm);
			break;
		case BELL_USECASE_MODE5_1:
			bell_create_path_bell_hdmi_in(prm);
			break;
		case BELL_USECASE_MODE6:
			bell_create_path_usecase_6();
			break;
		case BELL_USECASE_MODE7:
			bell_create_path_usecase_7(prm);
			break;
		case BELL_USECASE_MODE8:
			bell_create_path_usecase_8(prm);
			break;
        default:
            break;
    }
    
    
}

Void bell_delete_path()
{
    Int32 i = 0;
    for(i = 0; i < LINK_IN_USE_NUM; i++)
    {
        if(Links_in_use[i] == SYSTEM_LINK_ID_INVALID)
        {
            i--;
            break;
        }
    }
    OSA_assert(i < LINK_IN_USE_NUM);

    while(i >= 0)
    {
        printf("Deleteing %x\n",Links_in_use[i]);
        System_linkDelete(Links_in_use[i]);
        Links_in_use[i] = SYSTEM_LINK_ID_INVALID;
        i--;
    }

    /* Print the HWI, SWI and all tasks load */
    /* Reset the accumulated timer ticks */
    Bell_prfLoadCalcEnable(FALSE, TRUE, FALSE);

}
