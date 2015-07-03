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

typedef struct _df_ctx{

	OSA_ThrHndl threadFrame;    
	volatile Bool exitThread;	
	OSA_SemHndl framesInSem;
    OSA_QueHndl bufQFrame;	
    OSA_DmaChHndl dmaChHdnl;
#ifdef A8_CONTROL_I2C	
	Device_Sii9135Handle sii9135Handle;
#endif	
	int     getStart;
	void    *chains;    
	UInt32  captureId ;
    UInt32  swMsId;
    UInt32  displayId;
	UInt32  dupId;
	UInt32  m3out;
	UInt32  a8in;	
	UInt32 sclrId;
	UInt32 nsfId;	
	UInt32 deiId;		
	}df_ctx;
		
		static Void _copyFrameInfo2User(df_fb *dstBuf,
												   VIDFrame_Buf    *srcBuf)
		{
			int i,j;
	
			for (i = 0; i < DF_MAX_FIELDS; i++)
			{
				for (j = 0; j < DF_MAX_PLANES; j++)
				{
					dstBuf->addr[i][j] = srcBuf->addr[i][j];
					dstBuf->phyAddr[i][j] = srcBuf->phyAddr[i][j];
				}
			}
			dstBuf->channelNum	= srcBuf->channelNum;
			dstBuf->fid 		= srcBuf->fid;
			dstBuf->frameWidth	= srcBuf->frameWidth;
			dstBuf->frameHeight = srcBuf->frameHeight;
			dstBuf->linkPrivate = srcBuf->linkPrivate;
			dstBuf->timeStamp	= srcBuf->timeStamp;
			dstBuf->framePitch[0] = srcBuf->framePitch[0];
			dstBuf->framePitch[1] = srcBuf->framePitch[1];
	
		}
		
		static Void _copyFrameInfo2Link(VIDFrame_Buf *dstBuf,
												   df_fb	*srcBuf)
		{
			int i,j;
	
			for (i = 0; i < DF_MAX_FIELDS; i++)
			{
				for (j = 0; j < DF_MAX_PLANES; j++)
				{
					dstBuf->addr[i][j] = srcBuf->addr[i][j];
					dstBuf->phyAddr[i][j] = srcBuf->phyAddr[i][j];
				}
			}
			dstBuf->channelNum	= srcBuf->channelNum;
			dstBuf->fid 		= srcBuf->fid;
			dstBuf->frameWidth	= srcBuf->frameWidth;
			dstBuf->frameHeight = srcBuf->frameHeight;
			dstBuf->linkPrivate = srcBuf->linkPrivate;
			dstBuf->timeStamp	= srcBuf->timeStamp;
			dstBuf->framePitch[0] = srcBuf->framePitch[0];
			dstBuf->framePitch[1] = srcBuf->framePitch[1];
	
		}
		
	static Void _ipcFramesPrintFrameInfo(VIDFrame_Buf *buf)
	{
		OSA_printf("CHAINS_IPCFRAMES:VIDFRAME_INFO:"
				   "chNum:%d\t"
				   "fid:%d\t"
				   "frameWidth:%d\t"
				   "frameHeight:%d\t"
				   "framePitch0:%d\t"
				   "framePitch1:%d\t"
				   "framePitch2:%d\t"				 
				   "timeStamp:%d\t"
				   "virtAddr[0][0]:%p\t"
				   "phyAddr[0][1]:%p\t"
				   "phyAddr[0][2]:%p\t"  
				   "phyAddr[1][0]:%p\t" 				
				   "phyAddr[0][0]:%p",
					buf->channelNum,
					buf->fid,
					buf->frameWidth,
					buf->frameHeight,
					buf->framePitch[0],
					buf->framePitch[1],
					buf->framePitch[2],
		
					buf->timeStamp,
					buf->addr[0][0],
					buf->phyAddr[0][1],
					buf->phyAddr[0][2],
					buf->phyAddr[1][0], 			   
					buf->phyAddr[0][0]);
		
		OSA_printf("\n");
	 
	
	}
	
	
	static Void _ipcFramesPrintFullFrameListInfo(VIDFrame_BufList *bufList,
													   char *listName)
	{
			Int i;
	
			OSA_printf("CHAINS_IPCFRAMES:VIDFRAMELIST_INFO:%s\t"
					   "numFullFrames:%d\n",
					   listName,
					   bufList->numFrames);
			for (i = 0; i < bufList->numFrames; i++)
			{
				_ipcFramesPrintFrameInfo(&bufList->frames[i]);
			}
	}	

	void frameincallback(void *ctx)
	{
		df_ctx *h = (df_ctx*)ctx;		
		//printf("frame in callback called\n");
		OSA_semSignal(&h->framesInSem);
	}	
	
#define DFRAME_FRAMEPOOL_TBL_SIZE                            (128)
#define DFRAME_FRAMEPOOL_INVALIDID                           (~0u)
	
	typedef struct _dframe_framepool
	{
		OSA_MutexHndl mutex;
		UInt32		  freeIndex;
		struct dframe_framepool_entry
		{
			VIDFrame_Buf fBuf;
			UInt32			 nextFreeIndex;
		} tbl[DFRAME_FRAMEPOOL_TBL_SIZE];
	} dframe_framepool;

	dframe_framepool g_framepool;
	
	static
	Void dframe_framepool_init()
	{
		Int status,i;
	
		status = OSA_mutexCreate(&g_framepool.mutex);
		OSA_assert(status == OSA_SOK);
		OSA_mutexLock(&g_framepool.mutex);
		for (i = 0; i < (OSA_ARRAYSIZE(g_framepool.tbl) - 1);i++)
		{
			g_framepool.tbl[i].nextFreeIndex = (i + 1);
		}
		g_framepool.tbl[i].nextFreeIndex =
			DFRAME_FRAMEPOOL_INVALIDID;
		g_framepool.freeIndex = 0;
		OSA_mutexUnlock(&g_framepool.mutex);
	}
	
	static
	Void dframe_framepool_deinit()
	{
		Int status,i;
	
		status = OSA_mutexDelete(&g_framepool.mutex);
		OSA_assert(status == OSA_SOK);
	
		for (i = 0; i < (OSA_ARRAYSIZE(g_framepool.tbl) - 1);i++)
		{
			g_framepool.tbl[i].nextFreeIndex = (i + 1);
		}
		g_framepool.tbl[i].nextFreeIndex =
			DFRAME_FRAMEPOOL_INVALIDID;
		g_framepool.freeIndex = 0;
	}
	
	static
	VIDFrame_Buf * dframe_framepool_alloc()
	{
		VIDFrame_Buf *frameBuf = NULL;
		struct dframe_framepool_entry * entry = NULL;
	
		OSA_mutexLock(&g_framepool.mutex);
		OSA_assert((g_framepool.freeIndex !=
					DFRAME_FRAMEPOOL_INVALIDID)
				   &&
				   (g_framepool.freeIndex <
					OSA_ARRAYSIZE(g_framepool.tbl)));
		entry = &g_framepool.tbl[g_framepool.freeIndex];
		g_framepool.freeIndex = entry->nextFreeIndex;
		entry->nextFreeIndex = DFRAME_FRAMEPOOL_INVALIDID;
		frameBuf = &entry->fBuf;
		OSA_mutexUnlock(&g_framepool.mutex);
	
		return frameBuf;
	}
	
	static
	Int dframe_framepool_getindex(VIDFrame_Buf * frameBuf)
	{
	   Int index;
	   struct dframe_framepool_entry *entry = (struct dframe_framepool_entry *)frameBuf;
	
	   OSA_COMPILETIME_ASSERT(offsetof(struct dframe_framepool_entry,fBuf) == 0);
	
	   index = entry - &(g_framepool.tbl[0]);
	   return index;
	}
	
	static
	Void dframe_framepool_free(VIDFrame_Buf * frameBuf)
	{
		Int entryIndex;
		struct dframe_framepool_entry * entry = NULL;
	
		OSA_mutexLock(&g_framepool.mutex);
		entryIndex = dframe_framepool_getindex(frameBuf);
		OSA_assert((entryIndex >= 0) &&
				   (entryIndex < OSA_ARRAYSIZE(g_framepool.tbl)));
		entry = &g_framepool.tbl[entryIndex];
		entry->nextFreeIndex = g_framepool.freeIndex;
		g_framepool.freeIndex = entryIndex;
		OSA_mutexUnlock(&g_framepool.mutex);
	}

	
	typedef struct
	{
	  unsigned int memAddr;
	  unsigned int memSize;
	  unsigned int mmapMemAddr;
	  unsigned int mmapMemSize;  
	  unsigned int memOffset;
	
	  int	 memDevFd;
	  volatile unsigned int *pMemVirtAddr;
	  
	} frameMapCtrl;

#define MMAP_MEM_PAGEALIGN         (4*1024-1)

	frameMapCtrl * frame_mMap(UInt32 physAddr, UInt32 memSize , UInt32 *pMemVirtAddr)
	{
	frameMapCtrl *pMapCtrl;
		pMapCtrl = malloc(sizeof(frameMapCtrl));
		pMapCtrl->memDevFd = open("/dev/mem",O_RDWR|O_SYNC);
	
		if(pMapCtrl->memDevFd < 0)
		{
		  printf(" ERROR: /dev/mem open failed !!!\n");
		  return NULL;
		}
	
	
		pMapCtrl->memOffset   = physAddr & MMAP_MEM_PAGEALIGN;
	
		pMapCtrl->mmapMemAddr = physAddr - pMapCtrl->memOffset;
	
		pMapCtrl->mmapMemSize = memSize + pMapCtrl->memOffset;
	
		pMapCtrl->pMemVirtAddr = mmap(	
			   (void	*)pMapCtrl->mmapMemAddr,
			   pMapCtrl->mmapMemSize,
			   PROT_READ|PROT_WRITE|PROT_EXEC,MAP_SHARED,
			   pMapCtrl->memDevFd,
			   pMapCtrl->mmapMemAddr
			   );
	
	   if (pMapCtrl->pMemVirtAddr==NULL)
	   {
		 printf(" ERROR: mmap() failed !!!\n");
		 return NULL;
	   }
		*pMemVirtAddr = (UInt32)((UInt32)pMapCtrl->pMemVirtAddr + pMapCtrl->memOffset);
	
		return pMapCtrl;
	}
	
	int32_t frame_unmapMem(frameMapCtrl *pMapCtrl)
	{
		if(pMapCtrl->pMemVirtAddr)
		  munmap((void*)pMapCtrl->pMemVirtAddr, pMapCtrl->mmapMemSize);
		  
		if(pMapCtrl->memDevFd >= 0)
		  close(pMapCtrl->memDevFd);
		  
		return 0;
	}
static Void * dframe_ipcFramesSendRecvFxn(Void * prm)
{
     df_ctx *ctx = ( df_ctx *) prm;
	int i;
    Int status;
	VIDFrame_BufList  vidBufList;
	
    OSA_printf("DFRAME:%s:Entered...",__func__);
    OSA_semWait(&ctx->framesInSem,OSA_TIMEOUT_FOREVER);
    OSA_printf("DFRAME:Received first frame notify...!!!\n");
    while (FALSE == ctx->exitThread)
    {
		if(ctx->getStart)
		{
		    vidBufList.numFrames = 0;
		    IpcFramesInLink_getFullVideoFrames(ctx->a8in,&vidBufList);
		    if (vidBufList.numFrames)
		    {
		        // _ipcFramesPrintFullFrameListInfo(&vidBufList,"full frames");
				//put frames into frame que
				for(i=0;i<vidBufList.numFrames;i++)
				{
					VIDFrame_Buf *pBuf;			
					pBuf = dframe_framepool_alloc();
					OSA_assert(pBuf != NULL);
					*pBuf = vidBufList.frames[i];
					//when que is full, block thread
					status = OSA_quePut(&ctx->bufQFrame,(Int32)pBuf,OSA_TIMEOUT_FOREVER);
					OSA_assert(status == 0);
				}
		    }
		}
		else
		{
		    vidBufList.numFrames = 0;
		    IpcFramesInLink_getFullVideoFrames(ctx->a8in,&vidBufList);	
			if (vidBufList.numFrames)
		    {
		       // _ipcFramesPrintFullFrameListInfo(&vidBufList,"full frames");			
				status =IpcFramesInLink_putEmptyVideoFrames(ctx->a8in,&vidBufList);
		    }
		}
        OSA_waitMsecs(DFRAME_SENDRECVFXN_PERIOD_MS);
    }

    OSA_printf("DFRAME:%s:Leaving...",__func__);
    return NULL;
}

static void dframe_createFrameThread(df_ctx *thrObj)
{
	dframe_framepool_init();	

    OSA_queCreate(&thrObj->bufQFrame,DFRAME_QUE_MAX_LEN);
    OSA_semCreate(&thrObj->framesInSem, DFRAME_MAX_PENDING_RECV_SEM_COUNT,0);
    thrObj->exitThread = FALSE;
    OSA_thrCreate(&thrObj->threadFrame,
                   dframe_ipcFramesSendRecvFxn,
                  DFRAME_SENDRECVFXN_TSK_PRI,
                  DFRAME_SENDRECVFXN_TSK_STACK_SIZE,
                  thrObj);

}

static void dframe_deleteFrameThread(df_ctx *thrObj)
{
    thrObj->exitThread = TRUE;
    OSA_thrDelete(&thrObj->threadFrame);
    OSA_queDelete(&thrObj->bufQFrame);	
    OSA_semDelete(&thrObj->framesInSem);
	dframe_framepool_deinit();	

}	
	void* dframe_create(int outwidth, int outheight, int videostd)
		{
    	df_ctx *ctx = (df_ctx*)malloc(sizeof(df_ctx));
		
		if(ctx == NULL) return NULL;
        Int32 chId;
        DeiLink_CreateParams deiPrm;
		CaptureLink_CreateParams	capturePrm;
		NsfLink_CreateParams		nsfPrm;
		SwMsLink_CreateParams		swMsPrm;
		DisplayLink_CreateParams	displayPrm;
		//	MergeLink_CreateParams		mergePrm;
		DupLink_CreateParams		dupPrm;
		SclrLink_CreateParams 		sclrPrm;
			
		IpcFramesOutLinkRTOS_CreateParams  ipcFramesOutVpssToHostPrm;
		IpcFramesInLinkHLOS_CreateParams   ipcFramesInHostPrm;
		//	IpcFramesOutLinkHLOS_CreateParams  ipcFramesOutHostPrm;
		//	IpcFramesInLinkRTOS_CreateParams   ipcFramesInVpssFromHostPrm;		

		CaptureLink_VipInstParams *pCaptureInstPrm;
		CaptureLink_OutParams	  *pCaptureOutPrm;

		UInt32 captureId, deiId, nsfId, swMsId, displayId, dupId, m3out, a8in, sclrId;
#ifdef A8_CONTROL_I2C			
		Device_Sii9135Handle sii9135Handle;
#endif		
		System_init();

		captureId	= SYSTEM_LINK_ID_CAPTURE;
		deiId = SYSTEM_LINK_ID_DEI_0;
		nsfId		= SYSTEM_LINK_ID_NSF_0;
		swMsId		= SYSTEM_LINK_ID_SW_MS_MULTI_INST_0;
		displayId	= SYSTEM_LINK_ID_DISPLAY_0;
		dupId = SYSTEM_VPSS_LINK_ID_DUP_0;
		m3out = SYSTEM_VPSS_LINK_ID_IPC_FRAMES_OUT_1;
		a8in = SYSTEM_HOST_LINK_ID_IPC_FRAMES_IN_0;
		sclrId = SYSTEM_LINK_ID_SCLR_INST_0;


		CaptureLink_CreateParams_Init(&capturePrm);
		DeiLink_CreateParams_Init(&deiPrm);		
		SwMsLink_CreateParams_Init(&swMsPrm);
		DisplayLink_CreateParams_Init(&displayPrm);
		NsfLink_CreateParams_Init(&nsfPrm);
		IpcFramesOutLinkRTOS_CreateParams_Init(&ipcFramesOutVpssToHostPrm);
		IpcFramesInLinkHLOS_CreateParams_Init(&ipcFramesInHostPrm);
		SclrLink_CreateParams_Init(&sclrPrm);
		
#ifdef A8_CONTROL_I2C
    {
        Int32 status = 0;
        Device_VideoDecoderChipIdParams      vidDecChipIdArgs;
        Device_VideoDecoderChipIdStatus      vidDecChipIdStatus;
        VCAP_VIDEO_SOURCE_STATUS_PARAMS_S    videoStatusArgs;
        VCAP_VIDEO_SOURCE_CH_STATUS_S        videoStatus;

        Device_VideoDecoderCreateParams      createArgs;
        Device_VideoDecoderCreateStatus      createStatusArgs;

        Device_VideoDecoderVideoModeParams                 vidDecVideoModeArgs;

        printf(" set sii9135\r\n");
        /* Initialize and create video decoders */
        Device_sii9135Init();

        memset(&createArgs, 0, sizeof(Device_VideoDecoderCreateParams));

        createArgs.deviceI2cInstId    = 0;
        createArgs.numDevicesAtPort   = 1;
        createArgs.deviceI2cAddr[0]   = Device_getVidDecI2cAddr(DEVICE_VID_DEC_SII9135_DRV,0);
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
        vidDecVideoModeArgs.standard           = videostd;
        vidDecVideoModeArgs.videoCaptureMode   = DEVICE_CAPT_VIDEO_CAPTURE_MODE_SINGLE_CH_NON_MUX_EMBEDDED_SYNC;
        vidDecVideoModeArgs.videoSystem        = DEVICE_VIDEO_DECODER_VIDEO_SYSTEM_NONE;
        vidDecVideoModeArgs.videoCropEnable    = FALSE;
        vidDecVideoModeArgs.videoAutoDetectTimeout = -1;

        status = Device_sii9135Control(sii9135Handle,IOCTL_DEVICE_VIDEO_DECODER_SET_VIDEO_MODE,&vidDecVideoModeArgs,NULL);

    }
		ctx->sii9135Handle=sii9135Handle;	
#endif
        deiPrm.inQueParams.prevLinkId                        = dupId;
        deiPrm.inQueParams.prevLinkQueId                     = 0;
        deiPrm.outQueParams[DEI_LINK_OUT_QUE_DEI_SC].nextLink               = displayId;
        deiPrm.enableOut[DEI_LINK_OUT_QUE_DEI_SC]               = TRUE;
        deiPrm.enableOut[DEI_LINK_OUT_QUE_VIP_SC_SECONDARY_OUT] = FALSE;
        deiPrm.enableOut[DEI_LINK_OUT_QUE_VIP_SC]               = FALSE;
        deiPrm.tilerEnable                                   = FALSE;
        deiPrm.comprEnable                                   = FALSE;
        deiPrm.setVipScYuv422Format                          = FALSE;
		if(videostd==DF_STD_1080I_60) deiPrm.enableDeiForceBypass                          = FALSE;
		else
			deiPrm.enableDeiForceBypass                          = TRUE;
        deiPrm.enableLineSkipSc                              = FALSE;

        deiPrm.outScaleFactor[DEI_LINK_OUT_QUE_DEI_SC][0].scaleMode = DEI_SCALE_MODE_RATIO;
        deiPrm.outScaleFactor[DEI_LINK_OUT_QUE_DEI_SC][0].ratio.heightRatio.numerator   = 1;
        deiPrm.outScaleFactor[DEI_LINK_OUT_QUE_DEI_SC][0].ratio.heightRatio.denominator = 1;
        deiPrm.outScaleFactor[DEI_LINK_OUT_QUE_DEI_SC][0].ratio.widthRatio.numerator = 1;
        deiPrm.outScaleFactor[DEI_LINK_OUT_QUE_DEI_SC][0].ratio.widthRatio.denominator = 1;
        for (chId=1; chId < DEI_LINK_MAX_CH; chId++)
            deiPrm.outScaleFactor[DEI_LINK_OUT_QUE_DEI_SC][chId] = deiPrm.outScaleFactor[DEI_LINK_OUT_QUE_DEI_SC][0];

        /* FPS rates of DEI queue connected to Display */ 
        deiPrm.inputFrameRate[DEI_LINK_OUT_QUE_DEI_SC]  = 60;
        deiPrm.outputFrameRate[DEI_LINK_OUT_QUE_DEI_SC] = 30;

	
		capturePrm.numVipInst = 1;
		capturePrm.outQueParams[0].nextLink = dupId;
		capturePrm.tilerEnable				= FALSE;
		capturePrm.enableSdCrop 			= FALSE;

		pCaptureInstPrm 					= &capturePrm.vipInst[0];
		pCaptureInstPrm->vipInstId			= SYSTEM_CAPTURE_INST_VIP1_PORTA;
#ifndef A8_CONTROL_I2C			
		pCaptureInstPrm->videoDecoderId 	= SYSTEM_DEVICE_VID_DEC_SII9135_DRV;
#endif	
		pCaptureInstPrm->inDataFormat		= SYSTEM_DF_YUV422P;
		pCaptureInstPrm->standard			= videostd;
		pCaptureInstPrm->numOutput			= 1;

		pCaptureOutPrm						= &pCaptureInstPrm->outParams[0];
		pCaptureOutPrm->dataFormat			= SYSTEM_DF_YUV422I_YUYV;
		pCaptureOutPrm->scEnable			= FALSE;
		pCaptureOutPrm->scOutWidth			= 0;
		pCaptureOutPrm->scOutHeight 		= 0;
		pCaptureOutPrm->outQueId			= 0;
	
		dupPrm.inQueParams.prevLinkId = captureId;
		dupPrm.inQueParams.prevLinkQueId= 0;//DEI_LINK_OUT_QUE_DEI_SC;
		dupPrm.outQueParams[0].nextLink = deiId;
		dupPrm.outQueParams[1].nextLink = m3out;
		dupPrm.numOutQue				   = 2;
		dupPrm.notifyNextLink			   = TRUE;
			
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

		ipcFramesInHostPrm.cbCtx = ctx;
		ipcFramesInHostPrm.cbFxn = frameincallback;

		
		swMsPrm.inQueParams.prevLinkId = dupId;
		swMsPrm.inQueParams.prevLinkQueId = 0;
		swMsPrm.outQueParams.nextLink	  = displayId;
		swMsPrm.numSwMsInst = 1;
		//	  swMsPrm.swMsInstId[0] = SYSTEM_SW_MS_SC_INST_DEI_SC;
		swMsPrm.swMsInstId[0] = SYSTEM_SW_MS_SC_INST_SC5;		
		swMsPrm.maxInputQueLen			  = 4;
		swMsPrm.maxOutRes				  = VSYS_STD_1080P_60;
		swMsPrm.numOutBuf				  = 8;
		swMsPrm.lineSkipMode			  = FALSE;
		swMsPrm.layoutPrm.outputFPS 	  = 60;

		Chains_swMsGenerateLayoutParams(0, 2, &swMsPrm);


		displayPrm.inQueParams[0].prevLinkId	= deiId;
		displayPrm.inQueParams[0].prevLinkQueId = DEI_LINK_OUT_QUE_DEI_SC;
		displayPrm.displayRes				 = swMsPrm.maxOutRes;

	//	System_linkControl(
	//		SYSTEM_LINK_ID_M3VPSS,
	//		SYSTEM_M3VPSS_CMD_RESET_VIDEO_DEVICES,
	//		NULL,
	//		0,
	//		TRUE
	//		);
    UInt32 displayRes[SYSTEM_DC_MAX_VENC] =
        {VSYS_STD_1080P_60,   //SYSTEM_DC_VENC_HDMI,
         VSYS_STD_1080P_60,    //SYSTEM_DC_VENC_HDCOMP,
         VSYS_STD_1080P_60,    //SYSTEM_DC_VENC_DVO2
         VSYS_STD_NTSC        //SYSTEM_DC_VENC_SD,
        };		

		Chains_displayCtrlInit(displayRes);

		System_linkCreate (captureId, &capturePrm, sizeof(capturePrm));
		//System_linkControl(captureId, CAPTURE_LINK_CMD_CONFIGURE_VIP_DECODERS, NULL, 0, TRUE);
		//System_linkCreate(sclrId, &sclrPrm, sizeof(sclrPrm));			
		//System_linkCreate(nsfId	  , &nsfPrm, sizeof(nsfPrm));
		System_linkCreate(deiId	  , &deiPrm, sizeof(deiPrm));
		System_linkCreate(dupId, &dupPrm, sizeof(dupPrm));	
		System_linkCreate(m3out, &ipcFramesOutVpssToHostPrm, sizeof(ipcFramesOutVpssToHostPrm));
		System_linkCreate(a8in, &ipcFramesInHostPrm, sizeof(ipcFramesInHostPrm));
	//	System_linkCreate(swMsId   , &swMsPrm, sizeof(swMsPrm));
		System_linkCreate(displayId, &displayPrm, sizeof(displayPrm));

		ctx->captureId	= captureId;
		ctx->swMsId		= swMsId;
		ctx->sclrId		= sclrId;	
		ctx->nsfId		= nsfId;	
		ctx->deiId		= deiId;			
		ctx->displayId	= displayId;
		ctx->dupId = dupId;
		ctx->m3out = m3out;
		ctx->a8in = a8in;

		dframe_createFrameThread(ctx);
		return ctx;
		}
	void dframe_delete(void *ctx)
		{
    	df_ctx *h = (df_ctx*)ctx;
    	if(h == NULL) return;
		dframe_deleteFrameThread(ctx);		
		System_linkDelete(h->captureId);
		//System_linkDelete(h->sclrId);				
		//System_linkDelete(h->nsfId    );
		System_linkDelete(h->deiId    );
		System_linkDelete(h->dupId	 );
		System_linkDelete(h->m3out	 );
		System_linkDelete(h->a8in );	 
	//	System_linkDelete(h->swMsId	 );
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
		System_linkStart(h->displayId);
		//System_linkStart(h->swMsId	 );
		System_linkStart(h->a8in	 ); 		
		System_linkStart(h->m3out );
		System_linkStart(h->dupId );   		
		System_linkStart(h->deiId);
		//System_linkStart(h->nsfId);		
		//System_linkStart(h->sclrId);			
		System_linkStart(h->captureId);

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
		//System_linkStop(h->sclrId);		
		//usleep(900000);		
		//System_linkStop(h->nsfId);		
		System_linkStop(h->deiId);		
		System_linkStop(h->dupId);  
		System_linkStop(h->m3out	 );   
		System_linkStop(h->a8in	 );
		//System_linkStop(h->swMsId );
		System_linkStop(h->displayId);

		status =
		OSA_dmaClose(&h->dmaChHdnl);
		OSA_assert(status == OSA_SOK);
	
		status = OSA_dmaExit();
		OSA_assert(status == OSA_SOK);


		return 0;		
		}
//#define DFRAME_FRAME_LOG

	//when no frame in que, block 
	int dframe_cap_getframe(void *ctx,unsigned char *frame, int *size){
		df_ctx *h = (df_ctx*)ctx;		

    Int status;
	VIDFrame_BufList  vidBufList;
    VIDFrame_Buf *frameBuf;

	if(h->getStart==0) h->getStart=1;
	
    status = OSA_queGet(&h->bufQFrame,(Int32 *)(&frameBuf), OSA_TIMEOUT_FOREVER);
    
    OSA_assert(status == OSA_SOK);	
	//_ipcFramesPrintFrameInfo(frameBuf);
		int w1,h1,w0,h0;
		int p0,p1;
		OSA_DmaCopy2D copy2D;	
		OSA_DmaCopy1D copy1D;
		OSA_DmaYUYVtoYUV420p prm;
		
		UInt8 *inbuf;
		UInt8 *outbuf;

	//copy Y	
		inbuf=frameBuf->phyAddr[0][0];
		outbuf=frame;
		w0=frameBuf->frameWidth;
		h0=frameBuf->frameHeight;
		p0=frameBuf->framePitch[0];
		p1=frameBuf->framePitch[1];
		printf("got frame from w0=%d, h0=%d, p0=%d, p1=%d\n", w0,h0,p0,p1);

		copy2D.srcPhysAddr=	(unsigned long)inbuf;
		copy2D.dstPhysAddr= (unsigned long)outbuf;
		copy2D.copyWidth= w0;
		copy2D.copyHeight= h0;
		
		copy2D.srcOffsetH= p0;
		copy2D.dstOffsetH= w0;
		copy2D.skipH=0;
		OSA_dmaCopy2D(&h->dmaChHdnl, &copy2D, 1);

	//copy U
		w1=w0/2;
		h1=h0/2;
		inbuf=frameBuf->phyAddr[0][1];		
		outbuf=frame+w0*h0;

		prm.srcPhysAddr = (UInt32)inbuf;
		prm.dstPhysAddr = (UInt32)outbuf;
		prm.imageWidth	= w1;
		prm.imageHeight	= h1;	
		prm.srcOffsetH= p1;
		prm.dstOffsetH= w1;
		OSA_dmaCopyYUYVtoYUV420p(&h->dmaChHdnl,&prm,1);
	//copy V
		inbuf=inbuf+1;
		outbuf=outbuf+w1*h1;
		prm.srcPhysAddr = (UInt32)inbuf;
		prm.dstPhysAddr = (UInt32)outbuf;
		prm.imageWidth	= w1;
		prm.imageHeight	= h1;	
		prm.srcOffsetH= p1;
		prm.dstOffsetH= w1;		
		OSA_dmaCopyYUYVtoYUV420p(&h->dmaChHdnl,&prm,1);
		
#ifdef DFRAME_FRAME_LOG
  char filename_tmp[128];
  FILE *fd_tmp;

  sprintf(filename_tmp, "dframe.yuv");
  fd_tmp = fopen(filename_tmp,"wb");

	frameMapCtrl *pMapCtrl, *mapctrl2;
	unsigned int pMemVirtAddr, va;
	pMapCtrl=frame_mMap(frameBuf->phyAddr[0][0], w0*h0*2, &pMemVirtAddr);
	mapctrl2=frame_mMap((Uint32)frame, w0*h0*1.5, &va);	
//	memcpy(va,pMemVirtAddr,w0*h0*1.5);	

	fwrite(frame, 1, w0*h0*1.5, fd_tmp);
	frame_unmapMem(mapctrl2);	
	frame_unmapMem(pMapCtrl);
      fclose(fd_tmp);
#endif
	*size=w0*h0*1.5;
	vidBufList.numFrames = 1;
	memcpy(&vidBufList.frames[0],frameBuf,sizeof(VIDFrame_Buf));
	status =IpcFramesInLink_putEmptyVideoFrames(h->a8in,&vidBufList);
	dframe_framepool_free(frameBuf);
	return 1;
		}

#if 1
#define MAX_INPUT_STR_SIZE 256
int main ( int argc, char **argv )
{
void *dfctx;
    Bool done;
    char ch[MAX_INPUT_STR_SIZE];	
	printf("version 0713\n");
	
  dfctx=dframe_create(1920, 1080, DF_STD_1080P_60);
  dframe_start(dfctx);
    done = FALSE;
  
    while(!done)
    {
      //  Chains_menuMainShow();

        fgets(ch, MAX_INPUT_STR_SIZE, stdin);
        if(ch[1] != '\n' || ch[0] == '\n')
            continue;

        printf("This is a simple video capture/display test video source should be 1080P 60hz, please input x then ENTER for exit, otherwise vpss m4 will need reboot!!\r\n");

        switch(ch[0])
        {
            case 's':
				//dframe_cap_getframe(dfctx);
                break;
            case 'x':
                done = TRUE;
                break;
            case 'd':
              //  Chains_detectBoard();
                break;
			default:
			printf("This is a simple video capture/display test video source should be 1080P 60hz, please input x then ENTER for exit, otherwise vpss m4 will need reboot!!\r\n");
			    break;
        }

    }  
  dframe_stop(dfctx);
  dframe_delete(dfctx);
  
    return (0);
}
#endif
