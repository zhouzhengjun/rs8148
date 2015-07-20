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
    UInt32  swMsId;
    UInt32  displayId;
	UInt32  dupId;
	UInt32	ipcBitsOutHostId;
	UInt32	ipcBitsInVideoId;
	UInt32	decId;
	UInt32	ipcOutVideoId;
	UInt32 	ipcInVpssId;	
	}df_ctx;

	void bitsincallback(void *ctx)
	{
		df_ctx *h = (df_ctx*)ctx;
		OSA_printf("*********bits in callback called************\n");
		OSA_semSignal(&h->bitsInSem);
	}
	
#define DFRAME_FRAMEPOOL_TBL_SIZE                            (128)
#define DFRAME_FRAMEPOOL_INVALIDID                           (~0u)

#define BUFFER_READ_LEN         (512)
#define FRAME_BUFFER_LEN         (1920*1280/2)

enum H264NALTYPE{
    H264NT_NAL = 0,
    H264NT_SLICE,       /*1, p֡*/
    H264NT_SLICE_DPA,   /*2*/
    H264NT_SLICE_DPB,   /*3*/
    H264NT_SLICE_DPC,   /*4*/
    H264NT_SLICE_IDR,   /*5, I֡*/
    H264NT_SEI,         /*6, SEI*/
    H264NT_SPS,         /*7, SPS, sequence parameter set*/
    H264NT_PPS,         /*8, PPS, Piture parameter set*/
    H264NT_SINGLE_PKT = 100,
};

static int dframe_read_frame_h264(FILE* fp,char * dest)
{

    int rlen = 0, offset = 0, frm_step = 0;
    int i;
    char rcvbuf[BUFFER_READ_LEN];
    //rlen = read(s->fd, rcvbuf/*frame_unit.frame_buf+offset*/, BUFFER_READ_LEN);
    rlen = fread (rcvbuf,1,BUFFER_READ_LEN,fp);
    if(rlen < 0)return -1;

    while(rlen > 0)
    {
        for(i=0; i<(rlen-5); i++)
        {
            if((rcvbuf[i]==0) && (rcvbuf[i+1]==0) && (rcvbuf[i+2]==0) && (rcvbuf[i+3]==1)){
                if((rcvbuf[i+4]&0x1f) == H264NT_SLICE){
                    frm_step++;
                    if(frm_step == 2)break;
                }
                else if((rcvbuf[i+4]&0x1f) == H264NT_SLICE_IDR){
                    ;
                }
                else if((rcvbuf[i+4]&0x1f) == H264NT_SPS){
                    frm_step++;
                    if(frm_step == 2)break;
                }
                else if((rcvbuf[i+4]&0x1f) == H264NT_PPS){
                    ;
                }
            }
        }//the end of the file
        if(i == 0)
        {
            memcpy(dest+offset, rcvbuf, rlen);
            offset += rlen;
        }
        else
        {
            
            memcpy(dest+offset, rcvbuf, i);
            offset += i;
            //lseek(s->fd, i-rlen, SEEK_CUR);
            fseek(fp, i-rlen, SEEK_CUR);
        }
        if(frm_step == 2)
        {
            break;
        }
        /*for next loop*/
        //rlen = read(s->fd, rcvbuf/*frame_unit.frame_buf+offset*/, BUFFER_READ_LEN);
        rlen = fread (rcvbuf,1,BUFFER_READ_LEN,fp);
        if(rlen < 0)return -1;
    }
    if((frm_step == 1 && rlen == 0) || (frm_step == 2))
    {
        return offset;
    }
    return -1;        
}

static Void *dframe_ipcBitsSendFxn(Void * prm)
{
   	df_ctx *thrObj = ( df_ctx *) prm;
    Int i,status;
	Bitstream_BufList emptyBufList;
	Bitstream_Buf *pBuf;
	UInt32 bitBufSize,framesize;
	//char framebuf[FRAME_BUFFER_LEN];
    IpcBitsOutLinkHLOS_BitstreamBufReqInfo reqInfo;

	char *framebuf=(char *)malloc(FRAME_BUFFER_LEN*sizeof(char));
	if(framebuf==NULL)return NULL;
	memset(framebuf,0,FRAME_BUFFER_LEN*sizeof(char));
    while (FALSE == thrObj->exitBitsOutThread)
    {
    OSA_waitMsecs(DFRAME_SENDRECVFXN_PERIOD_MS);
    bitBufSize = CHAINS_IPCBITS_GET_BITBUF_SIZE(CHAINS_IPCBITS_DEFAULT_WIDTH,
                                                CHAINS_IPCBITS_DEFAULT_HEIGHT);
    emptyBufList.numBufs = 0;
    reqInfo.numBufs = VIDBITSTREAM_MAX_BITSTREAM_BUFS;
    reqInfo.reqType = IPC_BITSOUTHLOS_BITBUFREQTYPE_BUFSIZE;
    for (i = 0; i < VIDBITSTREAM_MAX_BITSTREAM_BUFS; i++)
    {
        reqInfo.u[i].minBufSize = bitBufSize;
    }
    IpcBitsOutLink_getEmptyVideoBitStreamBufs(thrObj->ipcBitsOutHostId,&emptyBufList,&reqInfo);
	if (emptyBufList.numBufs)
	{
	OSA_printf("DFRAME:%s:***********getbufs******************",__func__);
	for (i = 0; i < emptyBufList.numBufs;)
    {
        pBuf = emptyBufList.bufs[i];
		pBuf->channelNum = 0;
    	pBuf->codingType = 0;
    	pBuf->fillLength = 0;
    	pBuf->isKeyFrame = 0;
    	pBuf->timeStamp  = 0;
    	pBuf->mvDataFilledSize = 0;
    	pBuf->bottomFieldBitBufSize = 0;
    	pBuf->inputFileChanged = 0;
    	
        //OSA_quePut(emptyQue,(Int32)pBuf,OSA_TIMEOUT_NONE);
        //here to copy
		framesize=0;
		framesize =dframe_read_frame_h264(thrObj->fp,framebuf);
		if(framesize==-1)break;
		if(framesize!=-1 && framesize){
			memcpy(pBuf->addr, framebuf, framesize);
			pBuf->fillLength=framesize;
			printf("framesize:***********%d******************",framesize);
			i++;
		}
    }
    IpcBitsOutLink_putFullVideoBitStreamBufs(thrObj->ipcBitsOutHostId,&emptyBufList);
	}
	}	
	free(framebuf);
	OSA_printf("DFRAME:%s:Leaving...",__func__);
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
					  dframe_ipcBitsSendFxn,
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

	void* dframe_create(int outwidth, int outheight, int videostd,int argc, char **argv)
		{
    	df_ctx *ctx = (df_ctx*)malloc(sizeof(df_ctx));
		
		if(ctx == NULL) return NULL;
		if(argc>=2){
			if((ctx->fp=fopen(argv[argc-1],"r"))==NULL)
				return NULL;
		}else{
		char name[20];
		fflush(stdout);
		OSA_printf("\n\nCHAINS:Enter file store name:");
        fflush(stdin);
        fscanf(stdin,
               "%s",
               name);
		if((ctx->fp=fopen(name,"r"))==NULL)
			return NULL;
		}
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
		MergeLink_CreateParams			   mergePrm;
		
		Int i;
		Bool isProgressive;
		//System_LinkInfo bitsProducerLinkInfo;
		System_LinkQueInfo bitsProducerLinkInfo;
		
		UInt32 captureId, swMsId, displayId;
		UInt32 encId, decId, mergeId, snkId, dupId;
		UInt32 ipcOutVpssId, ipcInVpssId;
		UInt32 ipcOutVideoId, ipcInVideoId;
		UInt32 ipcBitsOutVideoId, ipcBitsInHostId;
		UInt32 ipcBitsInVideoId, ipcBitsOutHostId;
		char ch;
		UInt32 vipInstId;
		
		SwMsLink_CreateParams_Init(&swMsPrm);
		DisplayLink_CreateParams_Init(&displayPrm);
		CHAINS_INIT_STRUCT(IpcLink_CreateParams,ipcInVpssPrm);
		CHAINS_INIT_STRUCT(IpcLink_CreateParams,ipcOutVideoPrm);
		CHAINS_INIT_STRUCT(IpcBitsOutLinkHLOS_CreateParams,ipcBitsOutHostPrm);
		CHAINS_INIT_STRUCT(IpcBitsInLinkRTOS_CreateParams,ipcBitsInVideoPrm);
		CHAINS_INIT_STRUCT(DecLink_CreateParams, decPrm);
		
		swMsId		= SYSTEM_LINK_ID_SW_MS_MULTI_INST_0;
		displayId	= SYSTEM_LINK_ID_DISPLAY_0;
		decId  = SYSTEM_LINK_ID_VDEC_0;
		ipcOutVideoId= SYSTEM_VIDEO_LINK_ID_IPC_OUT_M3_0;
		ipcInVpssId  = SYSTEM_VPSS_LINK_ID_IPC_IN_M3_0;		
		ipcBitsOutHostId  = SYSTEM_HOST_LINK_ID_IPC_BITS_OUT_0;
		ipcBitsInVideoId  = SYSTEM_VIDEO_LINK_ID_IPC_BITS_IN_0;

#ifdef A8_CONTROL_I2C			
		Device_Sii9135Handle sii9135Handle;
#endif		
		System_init();

			//System_linkGetInfo(ipcBitsInHostId,&bitsProducerLinkInfo);
			//OSA_assert(bitsProducerLinkInfo.numQue = 1);
			ipcBitsOutHostPrm.inQueInfo.numCh = 1;
			ipcBitsOutHostPrm.inQueInfo.chInfo[0].bufType = SYSTEM_BUF_TYPE_VIDBITSTREAM;
			ipcBitsOutHostPrm.inQueInfo.chInfo[0].height = 1080;
			ipcBitsOutHostPrm.inQueInfo.chInfo[0].width = 1920;
			ipcBitsOutHostPrm.inQueInfo.chInfo[0].codingformat = IVIDEO_H264HP;
			ipcBitsOutHostPrm.inQueInfo.chInfo[0].memType = SYSTEM_MT_TILEDMEM;
			ipcBitsOutHostPrm.inQueInfo.chInfo[0].dataFormat = IVIDEO_H264HP;
			ipcBitsOutHostPrm.inQueInfo.chInfo[0].scanFormat = SYSTEM_SF_PROGRESSIVE;
			ipcBitsOutHostPrm.inQueInfo.chInfo[0].startX         = 0; // NOT USED
        	ipcBitsOutHostPrm.inQueInfo.chInfo[0].startY         = 0; // NOT USED
        	ipcBitsOutHostPrm.inQueInfo.chInfo[0].pitch[0]       = 0; // NOT USED
        	ipcBitsOutHostPrm.inQueInfo.chInfo[0].pitch[1]       = 0; // NOT USED
        	ipcBitsOutHostPrm.inQueInfo.chInfo[0].pitch[2]       = 0; // NOT USED
			//ipcBitsOutHostPrm.bufPoolPerCh = TRUE;
			
			//ipcBitsOutHostPrm.baseCreateParams.inQueParams.prevLinkId = SYSTEM_LINK_ID_INVALID;
			//ipcBitsOutHostPrm.baseCreateParams.inQueParams.prevLinkQueId = 0;
			ipcBitsOutHostPrm.baseCreateParams.numOutQue = 1;
			ipcBitsOutHostPrm.baseCreateParams.numChPerOutQue[0] = 1;
			ipcBitsOutHostPrm.baseCreateParams.outQueParams[0].nextLink = ipcBitsInVideoId;
			ipcBitsOutHostPrm.baseCreateParams.notifyNextLink = FALSE;
			ipcBitsOutHostPrm.baseCreateParams.notifyPrevLink = FALSE;
			ipcBitsOutHostPrm.baseCreateParams.noNotifyMode = TRUE;
    		//ipcBitsOutHostPrm.inQueInfo = bitsProducerLinkInfo;
			//Chains_ipcBitsInitCreateParams_BitsOutHLOS(&ipcBitsOutHostPrm,&bitsProducerLinkInfo.queInfo[0]);
			//Chains_ipcBitsInitCreateParams_BitsOutHLOS(&ipcBitsOutHostPrm,&bitsProducerLinkInfo);

			ipcBitsInVideoPrm.baseCreateParams.inQueParams.prevLinkId = ipcBitsOutHostId;
			ipcBitsInVideoPrm.baseCreateParams.inQueParams.prevLinkQueId = 0;
			ipcBitsInVideoPrm.baseCreateParams.numOutQue				 = 1;
			ipcBitsInVideoPrm.baseCreateParams.outQueParams[0].nextLink = decId;
			Chains_ipcBitsInitCreateParams_BitsInRTOS(&ipcBitsInVideoPrm,
													  TRUE);
			for (i=0; i<1; i++) {
				decPrm.chCreateParams[i].format 		 = IVIDEO_H264HP;
				decPrm.chCreateParams[i].profile		 = IH264VDEC_PROFILE_ANY;
				decPrm.chCreateParams[i].targetMaxWidth  = 1920;
				decPrm.chCreateParams[i].targetMaxHeight = 1080;
				if (isProgressive)
					decPrm.chCreateParams[i].fieldMergeDecodeEnable  = FALSE;
				else
					decPrm.chCreateParams[i].fieldMergeDecodeEnable  = TRUE;
				decPrm.chCreateParams[i].numBufPerCh = 6;
				decPrm.chCreateParams[i].defaultDynamicParams.targetFrameRate = 30;
				decPrm.chCreateParams[i].defaultDynamicParams.targetBitRate =
										 (2 * 1000 * 1000);
			}
			decPrm.inQueParams.prevLinkId = ipcBitsInVideoId;
			decPrm.inQueParams.prevLinkQueId = 0;
			decPrm.outQueParams.nextLink	 = ipcOutVideoId;
		
			ipcOutVideoPrm.inQueParams.prevLinkId	 = decId;
			ipcOutVideoPrm.inQueParams.prevLinkQueId = 0;
			ipcOutVideoPrm.numOutQue				   = 1;
			ipcOutVideoPrm.outQueParams[0].nextLink 	= ipcInVpssId;
			ipcOutVideoPrm.notifyNextLink			 = TRUE;
			ipcOutVideoPrm.notifyPrevLink			 = TRUE;
			//ipcOutVideoPrm.noNotifyMode 			 = FALSE;
		
			ipcInVpssPrm.inQueParams.prevLinkId    = ipcOutVideoId;
			ipcInVpssPrm.inQueParams.prevLinkQueId = 0;
			ipcInVpssPrm.numOutQue					  = 1;
			ipcInVpssPrm.outQueParams[0].nextLink	= swMsId;
			ipcInVpssPrm.notifyNextLink 		   = TRUE;
			ipcInVpssPrm.notifyPrevLink 		   = TRUE;
			//ipcInVpssPrm.noNotifyMode			   = FALSE;

		swMsPrm.inQueParams.prevLinkId = ipcInVpssId;
		swMsPrm.inQueParams.prevLinkQueId = 0;
		swMsPrm.outQueParams.nextLink	  = displayId;
		swMsPrm.numSwMsInst = 1;
		//swMsPrm.swMsInstId[0] = SYSTEM_SW_MS_SC_INST_VIP1_SC;
		swMsPrm.swMsInstId[0] = SYSTEM_SW_MS_SC_INST_SC5;		
		swMsPrm.maxInputQueLen			  = 4;
		swMsPrm.maxOutRes				  = VSYS_STD_1080P_60;
		swMsPrm.numOutBuf				  = 8;
		swMsPrm.lineSkipMode			  = TRUE;
		swMsPrm.layoutPrm.outputFPS 	  = 60;
		Chains_swMsGenerateLayoutParams(0, 2, &swMsPrm);
		//Chains_swMsGenerateLayoutParams(0, 0, &swMsPrm);
		displayPrm.inQueParams[0].prevLinkId	= swMsId;
		displayPrm.inQueParams[0].prevLinkQueId = 0;
		displayPrm.displayRes	= swMsPrm.maxOutRes;

    	UInt32 displayRes[SYSTEM_DC_MAX_VENC] =
        {VSYS_STD_1080P_60,   //SYSTEM_DC_VENC_HDMI,
         VSYS_STD_1080P_60,    //SYSTEM_DC_VENC_HDCOMP,
         VSYS_STD_1080P_60,    //SYSTEM_DC_VENC_DVO2
         VSYS_STD_NTSC        //SYSTEM_DC_VENC_SD,
        };		
		Chains_displayCtrlInit(displayRes);

		//printf("*************dupId**************\n");
		//System_linkCreate(dupId, &dupPrm, sizeof(dupPrm));		
		printf("**************ipcBitsOutHostId*************\n");
		System_linkCreate(ipcBitsOutHostId,&ipcBitsOutHostPrm,sizeof(ipcBitsOutHostPrm));
		printf("**************ipcBitsInVideoId*************\n");
		System_linkCreate(ipcBitsInVideoId,&ipcBitsInVideoPrm,sizeof(ipcBitsInVideoPrm));
		printf("**************decId*************\n");
		System_linkCreate(decId, &decPrm, sizeof(decPrm));
		printf("**************ipcOutVideoId*************\n");
		System_linkCreate(ipcOutVideoId, &ipcOutVideoPrm, sizeof(ipcOutVideoPrm));
		printf("**************ipcInVpssId*************\n");
		System_linkCreate(ipcInVpssId  , &ipcInVpssPrm	, sizeof(ipcInVpssPrm)	);
		printf("*************swMsId**************\n");
		System_linkCreate(swMsId  , &swMsPrm, sizeof(swMsPrm));
		printf("**************displayId*************\n");
		System_linkCreate(displayId, &displayPrm, sizeof(displayPrm));

		ctx->swMsId		= swMsId;	
		ctx->displayId	= displayId;
		ctx->ipcBitsOutHostId	= ipcBitsOutHostId;
		ctx->ipcBitsInVideoId	= ipcBitsInVideoId;
		ctx->decId	= decId;
		ctx->ipcOutVideoId	= ipcOutVideoId;
		ctx->ipcInVpssId	= ipcInVpssId;
		dframe_ipcBitsInitThrObj(ctx);
		return ctx;
		}
	void dframe_delete(void *ctx)
	{
    	df_ctx *h = (df_ctx*)ctx;
    	if(h == NULL) return;
	//dframe_deleteFrameThread(ctx);
	dframe_ipcBitsDeInitThrObj(h);
		System_linkDelete(h->ipcBitsOutHostId);
    	System_linkDelete(h->ipcBitsInVideoId);
    	System_linkDelete(h->decId);
    	System_linkDelete(h->ipcOutVideoId);
    	System_linkDelete(h->ipcInVpssId  );	
	System_linkDelete(h->swMsId	 );
	System_linkDelete(h->displayId);
		
		Chains_displayCtrlDeInit();
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
		System_linkStart(h->ipcBitsOutHostId);
    	System_linkStart(h->ipcBitsInVideoId);
    	System_linkStart(h->decId);
    	System_linkStart(h->ipcOutVideoId);
    	System_linkStart(h->ipcInVpssId  );
		System_linkStart(h->displayId);
		System_linkStart(h->swMsId	 );
		printf("Start Done \n");
		return 0;		
		}
	int dframe_stop(void *ctx){

		df_ctx *h = (df_ctx*)ctx;
		Int32 status = OSA_SOK;
				
		if(h == NULL)	return -1;
		if(h->getStart==1) h->getStart =0;
    System_linkStop(h->ipcBitsOutHostId);
    System_linkStop(h->ipcBitsInVideoId);
    System_linkStop(h->decId);
    System_linkStop(h->ipcOutVideoId);
    System_linkStop(h->ipcInVpssId  ); 
	System_linkStop(h->swMsId );
	System_linkStop(h->displayId);		
	OSA_dmaClose(&h->dmaChHdnl);
	OSA_assert(status == OSA_SOK);
	status = OSA_dmaExit();
	OSA_assert(status == OSA_SOK);
	return 0;		
}
//#define DFRAME_FRAME_LOG

	//when no frame in que, block 
#if 1
#define MAX_INPUT_STR_SIZE 256
int main ( int argc, char **argv )
{
void *dfctx;
df_ctx * ctx;

    Bool done;
    char ch[MAX_INPUT_STR_SIZE];
	printf("**************dframe_create*************\n");
  dfctx=dframe_create(1920, 1080, VSYS_STD_1080P_60,argc,argv);
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
