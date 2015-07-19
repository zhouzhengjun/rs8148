#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#include <sys/file.h> 
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <pthread.h>
#include <sys/mman.h>
#include <semaphore.h> 

#include <osa.h>
#include <osa_thr.h>
#include <osa_sem.h>
#include <osa_que.h>
#include <osa_mutex.h>
#include <osa_dma.h>

#include <mcfw/interfaces/link_api/system_const.h>
#include <mcfw/interfaces/link_api/system.h>
#include <mcfw/interfaces/link_api/captureLink.h>
#include <mcfw/interfaces/link_api/cameraLink.h>
#include <mcfw/interfaces/link_api/deiLink.h>
#include <mcfw/interfaces/link_api/nsfLink.h>
#include <mcfw/interfaces/link_api/displayLink.h>
#include <mcfw/interfaces/link_api/nullLink.h>
#include <mcfw/interfaces/link_api/grpxLink.h>
#include <mcfw/interfaces/link_api/dupLink.h>
#include <mcfw/interfaces/link_api/selectLink.h>
#include <mcfw/interfaces/link_api/swMsLink.h>
#include <mcfw/interfaces/link_api/mergeLink.h>
#include <mcfw/interfaces/link_api/nullSrcLink.h>
#include <mcfw/interfaces/link_api/ipcLink.h>
#include <mcfw/interfaces/link_api/systemLink_m3vpss.h>
#include <mcfw/interfaces/link_api/systemLink_m3video.h>
#include <mcfw/interfaces/link_api/encLink.h>
#include <mcfw/interfaces/link_api/decLink.h>
#include <mcfw/interfaces/link_api/sclrLink.h>
#include <mcfw/interfaces/link_api/avsync.h>

#include <ti/syslink/utils/IHeap.h>
#include <ti/syslink/utils/Memory.h>
#include <ti/ipc/SharedRegion.h>


#include <ti/xdais/xdas.h>
#include <ti/xdais/dm/xdm.h>
#include <ti/xdais/dm/ivideo.h>

#include "directframe.h"
#include "chains.h"

#define DFRAME_SENDRECVFXN_TSK_PRI                             (2)
#define DFRAME_SENDRECVFXN_TSK_STACK_SIZE                      (0) /* 0 means system default will be used */
#define DFRAME_SENDRECVFXN_PERIOD_MS                           (8)
#define DFRAME_MAX_PENDING_RECV_SEM_COUNT                      (5)
#define DFRAME_QUE_MAX_LEN				             		   (10)
#define A8_CONTROL_I2C

#define CHAINS_IPCBITS_MAX_PENDING_RECV_SEM_COUNT      (10)
#define CHAINS_IPCBITS_FREE_QUE_MAX_LEN      (10)
#define CHAINS_IPCBITS_FULL_QUE_MAX_LEN      (10)
#define CHAINS_IPCBITS_RECVFXN_TSK_PRI                   (2)
#define CHAINS_IPCBITS_RECVFXN_TSK_STACK_SIZE            (0) /* 0 means system default will be used */
#define CHAINS_IPCBITS_SENDFXN_PERIOD_MS                 (16)
#define CHAINS_IPCBITS_INFO_PRINT_INTERVAL               (1000)
#define CHAINS_IPCBITS_SENDFXN_TSK_PRI                   (2)
#define CHAINS_IPCBITS_SENDFXN_TSK_STACK_SIZE            (0) /* 0 means system default will be used */
#define CHAINS_IPCBITS_DEFAULT_WIDTH                   (1920)
#define CHAINS_IPCBITS_DEFAULT_HEIGHT                  (1080)
#define CHAINS_IPCBITS_GET_BITBUF_SIZE(width,height)   ((width) * (height)/2)


typedef struct _df_ctx{
	FILE * fp;
	//OSA_MutexHndl mutex;
	OSA_ThrHndl threadFrame;
	OSA_ThrHndl thrHandleBitsIn;
	OSA_ThrHndl thrHandleBitsOut;
	volatile Bool exitThread;
	volatile Bool exitBitsInThread;
	volatile Bool exitBitsOutThread;	
	OSA_SemHndl framesInSem;
	OSA_SemHndl bitsInSem;
    OSA_QueHndl bufQFrame;
	OSA_QueHndl bufQFreeBufs;
	OSA_QueHndl bufQFullBufs;
    OSA_DmaChHndl dmaChHdnl;
#ifdef A8_CONTROL_I2C	
	Device_Sii9135Handle sii9135Handle;
#endif	
	int     getStart;
	void    *chains;    
	UInt32  captureId ;
    UInt32  deiId;
    UInt32  displayId;
	UInt32  dupId;
	UInt32  ipcOutVpssId;
	UInt32  ipcInVideoId;
	UInt32  encId;
	UInt32  ipcBitsOutVideoId;
	UInt32  ipcBitsInHostId;
	UInt32 sclrId;
	UInt32 nsfId;	
	}df_ctx;

	void bitsincallback(void *ctx)
	{
		df_ctx *h = (df_ctx*)ctx;
		OSA_printf("*********bits in callback called************\n");
		OSA_semSignal(&h->bitsInSem);
	}

static Void *dframe_ipcBitsRecvFxn(Void * prm)
{
   	df_ctx *ctx = ( df_ctx *) prm;
    Int i,status;
	Bitstream_BufList fullBufList;
    Bitstream_Buf *pFullBuf;
    Bitstream_Buf *pEmptyBuf;
	//Bitstream_BufList* pfullBufList=(Bitstream_BufList* )malloc(sizeof(Bitstream_BufList));
	Bitstream_BufList* pfullBufList=&fullBufList;

    while (FALSE == ctx->exitBitsInThread)
    {
    OSA_semWait(&ctx->bitsInSem,OSA_TIMEOUT_FOREVER);
    //if(ctx->getStart)
    if(1)
	{
	pfullBufList->numBufs = 0;
    IpcBitsInLink_getFullVideoBitStreamBufs(ctx->ipcBitsInHostId,pfullBufList);
	if (pfullBufList->numBufs)
	{
    for (i = 0; i < pfullBufList->numBufs; i++)
    {			
        pFullBuf = (fullBufList.bufs[i]);
        if(pFullBuf->fillLength){
			fwrite(pFullBuf->addr,sizeof(char),pFullBuf->fillLength,ctx->fp);
			printf("BitBufInfo:"
                         "bufSize:%d,"
                         "chnId:%d,"
                         "codecType:%d,"
                         "filledBufSize:%d,"
                         "mvDataFilledSize:%d,"
                         "timeStamp:%d,"
                         "isKeyFrame:%d,"
                         "phy:%x,"
                         "width:%d"
                         "height:%d\n",
                         pFullBuf->bufSize,
                         pFullBuf->channelNum,
                         pFullBuf->codingType,
                         pFullBuf->fillLength,
                         pFullBuf->mvDataFilledSize,
                         pFullBuf->timeStamp,
                         pFullBuf->isKeyFrame,
                         pFullBuf->phyAddr,
                         pFullBuf->frameWidth,
                         pFullBuf->frameHeight);
      	}
    }
	OSA_waitMsecs(DFRAME_SENDRECVFXN_PERIOD_MS);
	IpcBitsInLink_putEmptyVideoBitStreamBufs(ctx->ipcBitsInHostId,pfullBufList);
	}
	
    }
    else
		{
		    pfullBufList->numBufs = 0;
		    IpcBitsInLink_getFullVideoBitStreamBufs(ctx->ipcBitsInHostId,pfullBufList);
			if (pfullBufList->numBufs)
		    {
				status =IpcBitsInLink_putEmptyVideoBitStreamBufs(ctx->ipcBitsInHostId,pfullBufList);
		    }
		}
	OSA_waitMsecs(DFRAME_SENDRECVFXN_PERIOD_MS);	
	OSA_printf("DFRAME:%s:Leaving...",__func__);
    }
    return NULL;
}

	static Void dframe_ipcBitsInitThrObj(df_ctx *thrObj)
	{
		OSA_semCreate(&thrObj->bitsInSem,CHAINS_IPCBITS_MAX_PENDING_RECV_SEM_COUNT,0);
		thrObj->exitBitsInThread = FALSE;
		thrObj->exitBitsOutThread = FALSE;
		OSA_queCreate(&thrObj->bufQFreeBufs,CHAINS_IPCBITS_FREE_QUE_MAX_LEN);
		OSA_queCreate(&thrObj->bufQFullBufs,CHAINS_IPCBITS_FULL_QUE_MAX_LEN);
		OSA_thrCreate(&thrObj->thrHandleBitsIn,
					  dframe_ipcBitsRecvFxn,
					  CHAINS_IPCBITS_RECVFXN_TSK_PRI,
					  CHAINS_IPCBITS_RECVFXN_TSK_STACK_SIZE,
					  thrObj);
	}
	
	
	static Void dframe_ipcBitsDeInitThrObj(df_ctx *thrObj)
	{
		thrObj->exitBitsInThread = TRUE;
		thrObj->exitBitsOutThread = TRUE;
		OSA_thrDelete(&thrObj->thrHandleBitsIn);
		OSA_thrDelete(&thrObj->thrHandleBitsOut);
		OSA_semDelete(&thrObj->bitsInSem);
		OSA_queDelete(&thrObj->bufQFreeBufs);
		OSA_queDelete(&thrObj->bufQFullBufs);
		fclose(thrObj->fp);
	}

	void* dframe_create(int outwidth, int outheight, int videostd)
		{
    	df_ctx *ctx = (df_ctx*)malloc(sizeof(df_ctx));		
		if(ctx == NULL) return NULL;
		char name[20];
		fflush(stdout);
		OSA_printf("\n\nCHAINS:Enter file store name:");
        fflush(stdin);
        fscanf(stdin,
               "%s",
               name);
		if((ctx->fp=fopen(name,"w"))==NULL)
			return NULL;

		DeiLink_CreateParams deiPrm;
		CaptureLink_CreateParams	capturePrm;
		CaptureLink_VipInstParams *pCaptureInstPrm;// only a vessel
		CaptureLink_OutParams	  *pCaptureOutPrm;
		SwMsLink_CreateParams		swMsPrm;
		DisplayLink_CreateParams	displayPrm;
		DupLink_CreateParams		dupPrm;
		EncLink_CreateParams	 encPrm;
		DecLink_CreateParams	 decPrm;
		IpcLink_CreateParams	 ipcOutVpssPrm;
		IpcLink_CreateParams	 ipcInVpssPrm;
		IpcLink_CreateParams	 ipcOutVideoPrm;
		IpcLink_CreateParams	 ipcInVideoPrm;
		IpcBitsOutLinkHLOS_CreateParams   ipcBitsOutHostPrm;
		IpcBitsOutLinkRTOS_CreateParams   ipcBitsOutVideoPrm;
		IpcBitsInLinkHLOS_CreateParams	  ipcBitsInHostPrm;
		IpcBitsInLinkRTOS_CreateParams	  ipcBitsInVideoPrm;
		Int i;
		Bool isProgressive;
		System_LinkInfo bitsProducerLinkInfo;
		
		UInt32 captureId, deiId, displayId;
		UInt32 encId, decId, snkId, dupId;
		UInt32 ipcOutVpssId, ipcInVpssId;
		UInt32 ipcOutVideoId, ipcInVideoId;
		UInt32 ipcBitsOutVideoId, ipcBitsInHostId;
		UInt32 ipcBitsInVideoId, ipcBitsOutHostId;
		char ch;
		UInt32 vipInstId;
		
		//Chains_ipcBitsInit();
		CaptureLink_CreateParams_Init(&capturePrm);
		DisplayLink_CreateParams_Init(&displayPrm);
		DeiLink_CreateParams_Init(&deiPrm);	
		CHAINS_INIT_STRUCT(IpcLink_CreateParams,ipcOutVpssPrm);
		CHAINS_INIT_STRUCT(IpcLink_CreateParams,ipcInVpssPrm);
		CHAINS_INIT_STRUCT(IpcLink_CreateParams,ipcOutVideoPrm);
		CHAINS_INIT_STRUCT(IpcLink_CreateParams,ipcInVideoPrm);
		CHAINS_INIT_STRUCT(IpcBitsOutLinkHLOS_CreateParams,ipcBitsOutHostPrm);
		CHAINS_INIT_STRUCT(IpcBitsOutLinkRTOS_CreateParams,ipcBitsOutVideoPrm);
		CHAINS_INIT_STRUCT(IpcBitsInLinkHLOS_CreateParams,ipcBitsInHostPrm);
		CHAINS_INIT_STRUCT(IpcBitsInLinkRTOS_CreateParams,ipcBitsInVideoPrm);
		CHAINS_INIT_STRUCT(DecLink_CreateParams, decPrm);
		CHAINS_INIT_STRUCT(EncLink_CreateParams, encPrm);
		
		captureId	= SYSTEM_LINK_ID_CAPTURE;
		deiId = SYSTEM_LINK_ID_DEI_0;
		displayId	= SYSTEM_LINK_ID_DISPLAY_0;
		encId  = SYSTEM_LINK_ID_VENC_0;
		dupId = SYSTEM_VPSS_LINK_ID_DUP_0;
		ipcOutVpssId = SYSTEM_VPSS_LINK_ID_IPC_OUT_M3_0;
		ipcInVideoId = SYSTEM_VIDEO_LINK_ID_IPC_IN_M3_0;
		ipcOutVideoId= SYSTEM_VIDEO_LINK_ID_IPC_OUT_M3_0;
		ipcInVpssId  = SYSTEM_VPSS_LINK_ID_IPC_IN_M3_0;		
		ipcBitsOutVideoId = SYSTEM_VIDEO_LINK_ID_IPC_BITS_OUT_0;
		ipcBitsInHostId   = SYSTEM_HOST_LINK_ID_IPC_BITS_IN_0;
		ipcBitsOutHostId  = SYSTEM_HOST_LINK_ID_IPC_BITS_OUT_0;
		ipcBitsInVideoId  = SYSTEM_VIDEO_LINK_ID_IPC_BITS_IN_0;

#ifdef A8_CONTROL_I2C			
		Device_Sii9135Handle sii9135Handle;
#endif	
#ifdef A8_CONTROL_I2C
			{
				Int32 status = 0;
				Device_VideoDecoderChipIdParams 	 vidDecChipIdArgs;
				Device_VideoDecoderChipIdStatus 	 vidDecChipIdStatus;
				VCAP_VIDEO_SOURCE_STATUS_PARAMS_S	 videoStatusArgs;
				VCAP_VIDEO_SOURCE_CH_STATUS_S		 videoStatus;
		
				Device_VideoDecoderCreateParams 	 createArgs;
				Device_VideoDecoderCreateStatus 	 createStatusArgs;
		
				Device_VideoDecoderVideoModeParams				   vidDecVideoModeArgs;
		
				printf(" set sii9135\r\n");
				/* Initialize and create video decoders */
				Device_sii9135Init();
		
				memset(&createArgs, 0, sizeof(Device_VideoDecoderCreateParams));
		
				createArgs.deviceI2cInstId	  = 0;
				createArgs.numDevicesAtPort   = 1;
				createArgs.deviceI2cAddr[0]   = Device_getVidDecI2cAddr(DEVICE_VID_DEC_SII9135_DRV,0);
				createArgs.deviceResetGpio[0] = DEVICE_VIDEO_DECODER_GPIO_NONE;
		
				sii9135Handle = Device_sii9135Create(		   DEVICE_VID_DEC_SII9135_DRV,
																 0, // instId - need to change
																 &(createArgs),
																 &(createStatusArgs));
		
			//usleep(100000);
				vidDecChipIdArgs.deviceNum = 0;
		
				status = Device_sii9135Control(sii9135Handle,
												   IOCTL_DEVICE_VIDEO_DECODER_GET_CHIP_ID,
												   &vidDecChipIdArgs,
												   &vidDecChipIdStatus);
			//usleep(100000);
				if (status >= 0)
				{
						videoStatusArgs.channelNum = 0;
		
						status = Device_sii9135Control(sii9135Handle,
													   IOCTL_DEVICE_VIDEO_DECODER_GET_VIDEO_STATUS,
													   &videoStatusArgs, &videoStatus);
		
						if (videoStatus.isVideoDetect)
						{
								if(videoStatus.frameInterval==0) videoStatus.frameInterval=1;
							printf(" VCAP: SII9135 (0x%02x): Detected video (%dx%d@%dHz, %d) !!!\n",
									   createArgs.deviceI2cAddr[0],
									   videoStatus.frameWidth,
									   videoStatus.frameHeight,
									   1000000 / videoStatus.frameInterval,
									   videoStatus.isInterlaced);
								if(videoStatus.isInterlaced) videostd = DF_STD_1080I_60;
								else videostd = DF_STD_1080P_60;
						}
						else
						{
							printf(" VCAP: SII9135 (0x%02x):  NO Video Detected !, try again\n", createArgs.deviceI2cAddr[0]);
#if 1
					usleep(100000);
							status = Device_sii9135Control(sii9135Handle,
													   IOCTL_DEVICE_VIDEO_DECODER_GET_VIDEO_STATUS,
													   &videoStatusArgs, &videoStatus);
							if (videoStatus.isVideoDetect)
							{
								if(videoStatus.frameInterval==0) videoStatus.frameInterval=1;
										printf(" VCAP: SII9135 (0x%02x): Detected video (%dx%d@%dHz, %d) !!!\n",
											createArgs.deviceI2cAddr[0],
											videoStatus.frameWidth,
											videoStatus.frameHeight,
											1000000 / videoStatus.frameInterval,
											videoStatus.isInterlaced);
							if(videoStatus.isInterlaced) videostd = DF_STD_1080I_60;
							else videostd = DF_STD_1080P_60;
							}
					else {
								printf(" VCAP: SII9135 (0x%02x):  NO Video Detected !!!\n", createArgs.deviceI2cAddr[0]);
		
					Device_sii9135Delete(sii9135Handle, NULL);
					Device_sii9135DeInit(); 		
					System_deInit();	
					return NULL;
							}
#endif
				}
				}
				else
				{
						printf(" VCAP: SII9135 (0x%02x): Device not found !!!\n", createArgs.deviceI2cAddr[0]);
			}
				/* Configure video decoder */
		
				memset(&vidDecVideoModeArgs,0, sizeof(Device_VideoDecoderVideoModeParams));
				
				vidDecVideoModeArgs.videoIfMode 	   = DEVICE_CAPT_VIDEO_IF_MODE_16BIT;
				vidDecVideoModeArgs.videoDataFormat    = SYSTEM_DF_YUV422P;
				vidDecVideoModeArgs.standard		   = videostd;
				vidDecVideoModeArgs.videoCaptureMode   = DEVICE_CAPT_VIDEO_CAPTURE_MODE_SINGLE_CH_NON_MUX_EMBEDDED_SYNC;
				vidDecVideoModeArgs.videoSystem 	   = DEVICE_VIDEO_DECODER_VIDEO_SYSTEM_NONE;
				vidDecVideoModeArgs.videoCropEnable    = FALSE;
				vidDecVideoModeArgs.videoAutoDetectTimeout = -1;
		
				status = Device_sii9135Control(sii9135Handle,IOCTL_DEVICE_VIDEO_DECODER_SET_VIDEO_MODE,&vidDecVideoModeArgs,NULL);
		
			}
				ctx->sii9135Handle=sii9135Handle;	
#endif

		System_init();
		capturePrm.numVipInst = 1;
		capturePrm.outQueParams[0].nextLink = dupId;
		capturePrm.tilerEnable				= FALSE;
		capturePrm.enableSdCrop 			= FALSE;
		pCaptureInstPrm 					= &capturePrm.vipInst[0];
		pCaptureInstPrm->vipInstId			= SYSTEM_CAPTURE_INST_VIP1_PORTA;
		//pCaptureInstPrm->vipInstId			= SYSTEM_CAPTURE_INST_VIP0_PORTA;
#ifndef A8_CONTROL_I2C			
		pCaptureInstPrm->videoDecoderId 	= SYSTEM_DEVICE_VID_DEC_SII9135_DRV;
#endif	
		pCaptureInstPrm->inDataFormat		= SYSTEM_DF_YUV422P;
		pCaptureInstPrm->standard			= videostd;
		pCaptureInstPrm->numOutput			= 1;
		pCaptureOutPrm						= &pCaptureInstPrm->outParams[0];
		pCaptureOutPrm->dataFormat			= SYSTEM_DF_YUV420SP_UV;
		pCaptureOutPrm->scEnable			= FALSE;
		pCaptureOutPrm->scOutWidth			= outwidth;
		pCaptureOutPrm->scOutHeight 		= outheight;
		pCaptureOutPrm->outQueId			= 0;

		dupPrm.inQueParams.prevLinkId = captureId;
		dupPrm.inQueParams.prevLinkQueId= 0;
		dupPrm.outQueParams[0].nextLink = deiId;
		dupPrm.outQueParams[1].nextLink = ipcOutVpssId;
		dupPrm.numOutQue				   = 2;
		dupPrm.notifyNextLink			   = TRUE;

		ipcOutVpssPrm.inQueParams.prevLinkId    = dupId;
		ipcOutVpssPrm.inQueParams.prevLinkQueId = 1;
		ipcOutVpssPrm.numOutQue                 = 1;
		ipcOutVpssPrm.outQueParams[0].nextLink  = ipcInVideoId;
		ipcOutVpssPrm.notifyNextLink            = TRUE;
		ipcOutVpssPrm.notifyPrevLink            = TRUE;
		ipcOutVpssPrm.noNotifyMode              = FALSE;

		ipcInVideoPrm.inQueParams.prevLinkId    = ipcOutVpssId;
		ipcInVideoPrm.inQueParams.prevLinkQueId = 0;
		ipcInVideoPrm.numOutQue                 = 1;
		ipcInVideoPrm.outQueParams[0].nextLink  = encId;
		ipcInVideoPrm.notifyNextLink            = TRUE;
		ipcInVideoPrm.notifyPrevLink            = TRUE;
		ipcInVideoPrm.noNotifyMode              = FALSE;

		for (i=0; i<1; i++) 
		{
			encPrm.chCreateParams[i].format 	= IVIDEO_H264HP;
			encPrm.chCreateParams[i].profile	= IH264_HIGH_PROFILE;
			encPrm.chCreateParams[i].dataLayout = IVIDEO_FIELD_SEPARATED;
			if (TRUE)
				encPrm.chCreateParams[i].fieldMergeEncodeEnable  = FALSE;
			else
				encPrm.chCreateParams[i].fieldMergeEncodeEnable  = TRUE;
			encPrm.chCreateParams[i].maxBitRate = -1;
			encPrm.chCreateParams[i].encodingPreset = 3;
			encPrm.chCreateParams[i].rateControlPreset = 0;
			encPrm.chCreateParams[i].enableHighSpeed = 0;
			encPrm.chCreateParams[i].defaultDynamicParams.intraFrameInterval = 30;
			encPrm.chCreateParams[i].encodingPreset = XDM_DEFAULT;
			encPrm.chCreateParams[i].enableAnalyticinfo = 0;
			encPrm.chCreateParams[i].enableWaterMarking = 0;
			encPrm.chCreateParams[i].rateControlPreset =
									 IVIDEO_STORAGE;
			encPrm.chCreateParams[i].defaultDynamicParams.inputFrameRate = 30;
			encPrm.chCreateParams[i].defaultDynamicParams.targetBitRate =
									 (2 * 1000 * 1000);
			encPrm.chCreateParams[i].defaultDynamicParams.interFrameInterval = 1;
			encPrm.chCreateParams[i].defaultDynamicParams.mvAccuracy =
									 IVIDENC2_MOTIONVECTOR_QUARTERPEL;
			encPrm.chCreateParams[i].defaultDynamicParams.rcAlg = 0 ;
			encPrm.chCreateParams[i].defaultDynamicParams.qpMin = 10;
			encPrm.chCreateParams[i].defaultDynamicParams.qpMax = 40;
			encPrm.chCreateParams[i].defaultDynamicParams.qpInit = -1;
			encPrm.chCreateParams[i].defaultDynamicParams.vbrDuration = 8;
			encPrm.chCreateParams[i].defaultDynamicParams.vbrSensitivity = 0;
		}

		encPrm.inQueParams.prevLinkId    = ipcInVideoId;
    	encPrm.inQueParams.prevLinkQueId = 0;
    	encPrm.outQueParams.nextLink     = ipcBitsOutVideoId;
		
		ipcBitsOutVideoPrm.baseCreateParams.inQueParams.prevLinkId    = encId;
		ipcBitsOutVideoPrm.baseCreateParams.inQueParams.prevLinkQueId = 0;
		ipcBitsOutVideoPrm.baseCreateParams.numOutQue                 = 1;
		ipcBitsOutVideoPrm.baseCreateParams.outQueParams[0].nextLink   = ipcBitsInHostId;
		ipcBitsOutVideoPrm.baseCreateParams.notifyPrevLink = TRUE;
		ipcBitsOutVideoPrm.baseCreateParams.notifyNextLink = FALSE;
		ipcBitsOutVideoPrm.baseCreateParams.noNotifyMode = TRUE;
		
		ipcBitsInHostPrm.baseCreateParams.inQueParams.prevLinkId = ipcBitsOutVideoId;
		ipcBitsInHostPrm.baseCreateParams.inQueParams.prevLinkQueId = 0;
		ipcBitsInHostPrm.cbCtx = ctx;
		ipcBitsInHostPrm.cbFxn = bitsincallback;
		ipcBitsInHostPrm.baseCreateParams.notifyNextLink = FALSE;
		ipcBitsInHostPrm.baseCreateParams.notifyPrevLink = FALSE;

        deiPrm.inQueParams.prevLinkId                        = dupId;
        deiPrm.inQueParams.prevLinkQueId                     = 0;
        deiPrm.outQueParams[DEI_LINK_OUT_QUE_DEI_SC].nextLink               = displayId;
        deiPrm.enableOut[DEI_LINK_OUT_QUE_DEI_SC]               = TRUE;
        deiPrm.enableOut[DEI_LINK_OUT_QUE_VIP_SC_SECONDARY_OUT] = FALSE;
        deiPrm.enableOut[DEI_LINK_OUT_QUE_VIP_SC]               = FALSE;
        deiPrm.tilerEnable                                   = FALSE;
        deiPrm.comprEnable                                   = FALSE;
        deiPrm.setVipScYuv422Format                          = FALSE;
		if(videostd==DF_STD_1080I_60) deiPrm.enableDeiForceBypass  = FALSE;
		else
			deiPrm.enableDeiForceBypass                      = TRUE;
        deiPrm.enableLineSkipSc                              = FALSE;

        deiPrm.outScaleFactor[DEI_LINK_OUT_QUE_DEI_SC][0].scaleMode = DEI_SCALE_MODE_RATIO;
        deiPrm.outScaleFactor[DEI_LINK_OUT_QUE_DEI_SC][0].ratio.heightRatio.numerator   = 1;
        deiPrm.outScaleFactor[DEI_LINK_OUT_QUE_DEI_SC][0].ratio.heightRatio.denominator = 1;
        deiPrm.outScaleFactor[DEI_LINK_OUT_QUE_DEI_SC][0].ratio.widthRatio.numerator = 1;
        deiPrm.outScaleFactor[DEI_LINK_OUT_QUE_DEI_SC][0].ratio.widthRatio.denominator = 1;
        deiPrm.outScaleFactor[DEI_LINK_OUT_QUE_DEI_SC][0] = deiPrm.outScaleFactor[DEI_LINK_OUT_QUE_DEI_SC][0];
        /* FPS rates of DEI queue connected to Display */ 
        deiPrm.inputFrameRate[DEI_LINK_OUT_QUE_DEI_SC]  = 60;
        deiPrm.outputFrameRate[DEI_LINK_OUT_QUE_DEI_SC] = 30;
		
		displayPrm.inQueParams[0].prevLinkId	= deiId;
		displayPrm.inQueParams[0].prevLinkQueId = 0;
		displayPrm.displayRes				 = VSYS_STD_1080P_60;

    	UInt32 displayRes[SYSTEM_DC_MAX_VENC] =
        {VSYS_STD_1080P_60,   //SYSTEM_DC_VENC_HDMI,
         VSYS_STD_1080P_60,    //SYSTEM_DC_VENC_HDCOMP,
         VSYS_STD_1080P_60,    //SYSTEM_DC_VENC_DVO2
         VSYS_STD_NTSC        //SYSTEM_DC_VENC_SD,
        };		
		Chains_displayCtrlInit(displayRes);

		printf("*************captureId**************\n");
		System_linkCreate (captureId, &capturePrm, sizeof(capturePrm));
		printf("*************dupId**************\n");
		System_linkCreate(dupId, &dupPrm, sizeof(dupPrm));
		printf("*************deiId**************\n");
		System_linkCreate(deiId	 , &deiPrm, sizeof(deiPrm));
		printf("*************ipcOutVpssId**************\n");
		System_linkCreate(ipcOutVpssId , &ipcOutVpssPrm , sizeof(ipcOutVpssPrm) );
		printf("*************ipcInVideoId**************\n");
		System_linkCreate(ipcInVideoId , &ipcInVideoPrm , sizeof(ipcInVideoPrm) );
		printf("*************encId**************\n");
		System_linkCreate(encId, &encPrm, sizeof(encPrm));
		printf("**************ipcBitsOutVideoId*************\n");
		System_linkCreate(ipcBitsOutVideoId, &ipcBitsOutVideoPrm, sizeof(ipcBitsOutVideoPrm));
		printf("************ipcBitsInHostId***************\n");
		System_linkCreate(ipcBitsInHostId, &ipcBitsInHostPrm, sizeof(ipcBitsInHostPrm));
		printf("**************displayId*************\n");
		System_linkCreate(displayId, &displayPrm, sizeof(displayPrm));

		ctx->captureId	= captureId;
		ctx->displayId	= displayId;
		ctx->dupId = dupId;
		ctx->deiId = deiId;
		ctx->ipcOutVpssId = ipcOutVpssId;
		ctx->ipcInVideoId = ipcInVideoId;
		ctx->encId = encId;
		ctx->ipcBitsOutVideoId = ipcBitsOutVideoId;
		ctx->ipcBitsInHostId = ipcBitsInHostId;
		
		dframe_ipcBitsInitThrObj(ctx);
		return ctx;
		}
	void dframe_delete(void *ctx)
		{
    	df_ctx *h = (df_ctx*)ctx;
    	if(h == NULL) return;

	dframe_ipcBitsDeInitThrObj(h);
	System_linkDelete(h->captureId);
	System_linkDelete(h->dupId );
	System_linkDelete(h->deiId );
    System_linkDelete(h->ipcOutVpssId );
    System_linkDelete(h->ipcInVideoId );
    System_linkDelete(h->encId);
    System_linkDelete(h->ipcBitsOutVideoId);
    System_linkDelete(h->ipcBitsInHostId); 
	System_linkDelete(h->displayId);
		
		Chains_displayCtrlDeInit();
#ifdef A8_CONTROL_I2C		
		Device_sii9135Delete(h->sii9135Handle, NULL);
		Device_sii9135DeInit();			
#endif		
    	System_deInit();		
	    free(h);		
		}
	
	int dframe_start(void *ctx){

		df_ctx *h = (df_ctx*)ctx;
	    Int32 status = OSA_SOK;	
		if(h == NULL) return -1;

        status = OSA_dmaInit();
        OSA_assert(status == OSA_SOK);
		h->getStart=0;
        status = OSA_dmaOpen(&h->dmaChHdnl,OSA_DMA_MODE_NORMAL,OSA_DMA_MAX_NORMAL_TRANSFERS);
#ifdef A8_CONTROL_I2C			
	    Device_sii9135Control(h->sii9135Handle,DEVICE_CMD_START,NULL,NULL);
#endif	
		System_linkStart(h->captureId);
		System_linkStart(h->dupId );
		System_linkStart(h->deiId );
		System_linkStart(h->ipcOutVpssId );
		System_linkStart(h->ipcInVideoId );
		System_linkStart(h->encId);
		System_linkStart(h->ipcBitsOutVideoId);
		System_linkStart(h->ipcBitsInHostId);
		System_linkStart(h->displayId);

		printf("Start Done \n");
		return 0;		
		}
	int dframe_stop(void *ctx){

		df_ctx *h = (df_ctx*)ctx;
		Int32 status = OSA_SOK;
				
		if(h == NULL)	return -1;
		if(h->getStart==1) h->getStart =0;
#ifdef A8_CONTROL_I2C			
	    Device_sii9135Control(h->sii9135Handle,DEVICE_CMD_STOP,NULL,NULL);	
#endif		
	System_linkStop(h->captureId);
	System_linkStop(h->dupId );
	System_linkStop(h->deiId );
    System_linkStop(h->ipcOutVpssId );
    System_linkStop(h->ipcInVideoId );
    System_linkStop(h->encId);
    System_linkStop(h->ipcBitsOutVideoId);
    System_linkStop(h->ipcBitsInHostId); 
	System_linkStop(h->displayId);		
	OSA_dmaClose(&h->dmaChHdnl);
	OSA_assert(status == OSA_SOK);
	status = OSA_dmaExit();
	OSA_assert(status == OSA_SOK);
	return 0;		
}


#if 1
#define MAX_INPUT_STR_SIZE 256
int main ( int argc, char **argv )
{
void *dfctx;
df_ctx * ctx;

    Bool done;
    char ch[MAX_INPUT_STR_SIZE];
	printf("**************dframe_create*************\n");
  dfctx=dframe_create(1920, 1080, VSYS_STD_1080P_60);
  //ctx=(df_ctx*)dfctx;
 // ctx->getStart=1;
 printf("**************dframe_start*************\n");
  dframe_start(dfctx);
    done = FALSE;
  
    while(!done)
    {
      //  Chains_menuMainShow();

        fgets(ch, MAX_INPUT_STR_SIZE, stdin);
        if(ch[1] != '\n' || ch[0] == '\n')
            continue;

        //printf("This is a simple video capture/display test video source should be 1080P 60hz, please input x then ENTER for exit, otherwise vpss m4 will need reboot!!\r\n");

        switch(ch[0])
        {
            case 's':
				//ctx->getStart=1;
                break;
            case 'x':
                done = TRUE;
                break;
            case 'e':
				//ctx->getStart=0;
                break;
                break;
			default:
			//printf("This is a simple video capture/display test video source should be 1080P 60hz, please input x then ENTER for exit, otherwise vpss m4 will need reboot!!\r\n");
			break;
        }

    }  
  dframe_stop(dfctx);
  dframe_delete(dfctx);
  
    return (0);
}
#endif
