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

#include "bell_bits_rdwr.h"

#define ENABLE_DMA_COPY

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>
#include <ctype.h>
#include <sys/wait.h>
#include <sys/types.h>  // For stat().
#include <sys/stat.h>   // For stat().
#include <sys/statvfs.h>// For statvfs()
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <osa_que.h>
#include <osa_mutex.h>
#include <osa_thr.h>
#include <osa_sem.h>
#include <osa_dma.h>

#define BELL_IPC_BITS_MAX_NUM_CHANNELS                 (48)
#define BELL_IPC_BITS_NONOTIFYMODE_BITSIN              (TRUE)
#define BELL_IPC_BITS_NONOTIFYMODE_BITSOUT             (TRUE)

#define BELL_IPCBITS_SENDFXN_TSK_PRI                   (2)
#define BELL_IPCBITS_RECVFXN_TSK_PRI                   (2)

#define BELL_IPCBITS_SENDFXN_TSK_STACK_SIZE            (0) /* 0 means system default will be used */
#define BELL_IPCBITS_RECVFXN_TSK_STACK_SIZE            (0) /* 0 means system default will be used */

#define BELL_IPCBITS_SENDFXN_PERIOD_MS                 (8)
#define BELL_IPCBITS_RECVFXN_PERIOD_MS                 (8)

#define BELL_IPCBITS_INFO_PRINT_INTERVAL               (1000)

/** @enum BELL_IPCBITS_GET_BITBUF_SIZE
 *  @brief Macro that returns max size of encoded bitbuffer for a given resolution
 */
#define BELL_IPCBITS_GET_BITBUF_SIZE(width,height)  ((width) * (height)/2)

#define BELL_IPCBITS_MAX_PENDING_RECV_SEM_COUNT     (10)

#define BELL_IPCBITS_MAX_NUM_FREE_BUFS_PER_CHANNEL  (6)
#define BELL_IPCBITS_FREE_QUE_MAX_LEN               (BELL_IPC_BITS_MAX_NUM_CHANNELS * \
                                                         BELL_IPCBITS_MAX_NUM_FREE_BUFS_PER_CHANNEL)
#define BELL_IPCBITS_FULL_QUE_MAX_LEN               (BELL_IPCBITS_FREE_QUE_MAX_LEN)

#define BELL_IPC_BITS_USE_DMA_FOR_BITBUF_COPY       (TRUE)
#define BELL_IPC_BITS_DMA_MAX_TRANSFERS             (4)

typedef struct {

    UInt32 totalDataSize;
    UInt32 numKeyFrames;
    UInt32 numFrames;
    UInt32 maxWidth;
    UInt32 minWidth;
    UInt32 maxHeight;
    UInt32 minHeight;
    UInt32 maxLatency;
    UInt32 minLatency;

} Bell_ChInfo;

typedef struct Bell_IpcBitsCtrlThrObj {
    OSA_ThrHndl thrHandleBitsOut;
    OSA_QueHndl bufQFullBufs;
    OSA_QueHndl bufQFreeBufs;
    OSA_SemHndl bitsInNotifySem;
    volatile Bool exitBitsOutThread;
    Bell_res  resolution[IPCBITS_RESOLUTION_TYPES];

} Bell_IpcBitsCtrlThrObj;

typedef struct Bell_IpcBitsDmaObj {
    OSA_DmaChHndl dmaChHdnl;
    Bool          useDma;
} Bell_IpcBitsDmaObj;

typedef struct Bell_IpcBitsCtrl {
    Bool  noNotifyBitsInHLOS;
    Bool  noNotifyBitsOutHLOS;
    Bell_IpcBitsCtrlThrObj  thrObj;
    Bell_IpcBitsDmaObj      dmaObj;

    Bell_ChInfo chInfo[VENC_CHN_MAX];

    UInt32 statsStartTime;
    UInt32 totalEncFrames;
    UInt32 totalDecFrames;

} Bell_IpcBitsCtrl;

#define APP_IPCBITSCTRL_FREE_BITBUFINFO_TBL_SIZE                            (128)
#define APP_IPCBITSCTRL_FREE_BITBUFINFO_INVALIDID                           (~0u)

typedef struct Bell_FreeBitBufInfoTbl
{
    OSA_MutexHndl mutex;
    UInt32        freeIndex;
    struct Bell_FreeBitBufInfoEntry
    {
        VCODEC_BITSBUF_S bitBuf;
        UInt32           nextFreeIndex;
    } tbl[APP_IPCBITSCTRL_FREE_BITBUFINFO_TBL_SIZE];
} Bell_FreeBitBufInfoTbl;

Bell_IpcBitsCtrl gBell_ipcBitsCtrl =
{
    .noNotifyBitsInHLOS  = BELL_IPC_BITS_NONOTIFYMODE_BITSIN,
    .noNotifyBitsOutHLOS = BELL_IPC_BITS_NONOTIFYMODE_BITSOUT,
    #ifdef ENABLE_DMA_COPY
    .dmaObj.useDma       = BELL_IPC_BITS_USE_DMA_FOR_BITBUF_COPY,
    #else
    .dmaObj.useDma       = FALSE,
    #endif
};

Bell_FreeBitBufInfoTbl g_FreeBitBufInfoTbl;

static
Void Bell_FreeBitBufInit()
{
    Int status,i;

    status = OSA_mutexCreate(&g_FreeBitBufInfoTbl.mutex);
    OSA_assert(status == OSA_SOK);
    OSA_mutexLock(&g_FreeBitBufInfoTbl.mutex);
    for (i = 0; i < (OSA_ARRAYSIZE(g_FreeBitBufInfoTbl.tbl) - 1);i++)
    {
        g_FreeBitBufInfoTbl.tbl[i].nextFreeIndex = (i + 1);
    }
    g_FreeBitBufInfoTbl.tbl[i].nextFreeIndex =
        APP_IPCBITSCTRL_FREE_BITBUFINFO_INVALIDID;
    g_FreeBitBufInfoTbl.freeIndex = 0;
    OSA_mutexUnlock(&g_FreeBitBufInfoTbl.mutex);
}

static
Void Bell_FreeBitBufDeInit()
{
    Int status,i;

    status = OSA_mutexDelete(&g_FreeBitBufInfoTbl.mutex);
    OSA_assert(status == OSA_SOK);

    for (i = 0; i < (OSA_ARRAYSIZE(g_FreeBitBufInfoTbl.tbl) - 1);i++)
    {
        g_FreeBitBufInfoTbl.tbl[i].nextFreeIndex = (i + 1);
    }
    g_FreeBitBufInfoTbl.tbl[i].nextFreeIndex =
        APP_IPCBITSCTRL_FREE_BITBUFINFO_INVALIDID;
    g_FreeBitBufInfoTbl.freeIndex = 0;
}

static
VCODEC_BITSBUF_S * Bell_FreeBitBufAlloc()
{
    VCODEC_BITSBUF_S *freeBitBufInfo = NULL;
    struct Bell_FreeBitBufInfoEntry * entry = NULL;

    OSA_mutexLock(&g_FreeBitBufInfoTbl.mutex);
    OSA_assert((g_FreeBitBufInfoTbl.freeIndex !=
                APP_IPCBITSCTRL_FREE_BITBUFINFO_INVALIDID)
               &&
               (g_FreeBitBufInfoTbl.freeIndex <
                OSA_ARRAYSIZE(g_FreeBitBufInfoTbl.tbl)));
    entry = &g_FreeBitBufInfoTbl.tbl[g_FreeBitBufInfoTbl.freeIndex];
    g_FreeBitBufInfoTbl.freeIndex = entry->nextFreeIndex;
    entry->nextFreeIndex = APP_IPCBITSCTRL_FREE_BITBUFINFO_INVALIDID;
    freeBitBufInfo = &entry->bitBuf;
    OSA_mutexUnlock(&g_FreeBitBufInfoTbl.mutex);

    return freeBitBufInfo;
}

static
Int Bell_FreeBitBufGetIndex(VCODEC_BITSBUF_S * bitBufInfo)
{
   Int index;
   struct Bell_FreeBitBufInfoEntry *entry = (struct Bell_FreeBitBufInfoEntry *)bitBufInfo;

   OSA_COMPILETIME_ASSERT(offsetof(struct Bell_FreeBitBufInfoEntry,bitBuf) == 0);

   index = entry - &(g_FreeBitBufInfoTbl.tbl[0]);
   return index;
}

static
Void Bell_FreeBitBufFree(VCODEC_BITSBUF_S * bitBufInfo)
{
    Int entryIndex;
    struct Bell_FreeBitBufInfoEntry * entry = NULL;

    OSA_mutexLock(&g_FreeBitBufInfoTbl.mutex);
    entryIndex = Bell_FreeBitBufGetIndex(bitBufInfo);
    OSA_assert((entryIndex >= 0) &&
               (entryIndex < OSA_ARRAYSIZE(g_FreeBitBufInfoTbl.tbl)));
    entry = &g_FreeBitBufInfoTbl.tbl[entryIndex];
    entry->nextFreeIndex = g_FreeBitBufInfoTbl.freeIndex;
    g_FreeBitBufInfoTbl.freeIndex = entryIndex;
    OSA_mutexUnlock(&g_FreeBitBufInfoTbl.mutex);
}

static Void Bell_ipcBitsInCbFxn (Ptr cbCtx)
{
    Bell_IpcBitsCtrl *app_ipcBitsCtrl;
    static Int printInterval;

    OSA_assert(cbCtx = &gBell_ipcBitsCtrl);
    app_ipcBitsCtrl = cbCtx;
    OSA_semSignal(&app_ipcBitsCtrl->thrObj.bitsInNotifySem);
    #ifdef IPC_BITS_DEBUG
    if ((printInterval % BELL_IPCBITS_INFO_PRINT_INTERVAL) == 0)
    {
        OSA_printf("MCFW_IPCBITS: Callback function:%s",__func__);
    }
    #endif
    printInterval++;
}

static Void Bell_ipcBitsCopyBitBufInfo (VCODEC_BITSBUF_S *dst,
                                          const VCODEC_BITSBUF_S *src)
{
    dst->chnId = src->chnId;
    dst->codecType = src->codecType;
    dst->filledBufSize = src->filledBufSize;
    dst->bottomFieldBitBufSize = src->bottomFieldBitBufSize;
    dst->inputFileChanged = src->inputFileChanged;
    dst->frameType = src->frameType;
    dst->timestamp  = src->timestamp;
    dst->upperTimeStamp= src->upperTimeStamp;
    dst->lowerTimeStamp= src->lowerTimeStamp;
    dst->seqId         = VDIS_DEV_HDMI;
}

static Void Bell_ipcBitsCopyBitBufDataMem2Mem(VCODEC_BITSBUF_S *dstBuf,
                                                VCODEC_BITSBUF_S *srcBuf)
{
    OSA_assert(srcBuf->filledBufSize < dstBuf->bufSize);

    if (FALSE == gBell_ipcBitsCtrl.dmaObj.useDma)
    {
        memcpy(dstBuf->bufVirtAddr,srcBuf->bufVirtAddr,srcBuf->filledBufSize);
    }
    else
    {
        OSA_DmaCopy1D prm;

        prm.srcPhysAddr = (UInt32)srcBuf->bufPhysAddr;
        prm.dstPhysAddr = (UInt32)dstBuf->bufPhysAddr;
        prm.size        = srcBuf->filledBufSize;
        OSA_dmaCopy1D(&gBell_ipcBitsCtrl.dmaObj.dmaChHdnl,
                      &prm,
                      1);

    }
}

static Void Bell_ipcBitsQueEmptyBitBufs(Bell_res resolution, OSA_QueHndl       *emptyQue)
{
    VCODEC_BITSBUF_LIST_S emptyBufList;
    VCODEC_BITSBUF_S *pBuf;
    VDEC_BUF_REQUEST_S reqInfo;
    Int i;
    Int status;
    UInt32 bitBufSize;

    bitBufSize = BELL_IPCBITS_GET_BITBUF_SIZE(resolution.width,
                                                resolution.height);
    if (bitBufSize <=0 )
        return;

    emptyBufList.numBufs = 0;
    reqInfo.numBufs = VCODEC_BITSBUF_MAX;
    reqInfo.reqType = VDEC_BUFREQTYPE_BUFSIZE;
    for (i = 0; i < VCODEC_BITSBUF_MAX; i++)
    {
        reqInfo.u[i].minBufSize = bitBufSize;
    }
    Vdec_requestBitstreamBuffer(&reqInfo, &emptyBufList, 0);
    for (i = 0; i < emptyBufList.numBufs; i++)
    {
        pBuf = Bell_FreeBitBufAlloc();
        OSA_assert(pBuf != NULL);
        *pBuf = emptyBufList.bitsBuf[i];
        OSA_assert(pBuf->bufSize >= bitBufSize );

        status = OSA_quePut(emptyQue,(Int32)pBuf,OSA_TIMEOUT_NONE);
        OSA_assert(status == 0);
    }
}

static Void Bell_ipcBitsSendFullBitBufs(OSA_QueHndl       *fullQue)
{
    VCODEC_BITSBUF_LIST_S fullBufList;
    VCODEC_BITSBUF_S *pBuf;
    Int status;

    fullBufList.numBufs = 0;
    while((status = OSA_queGet(fullQue,(Int32 *)(&pBuf),OSA_TIMEOUT_NONE)) == 0)
    {
        OSA_assert(fullBufList.numBufs < VCODEC_BITSBUF_MAX);
        fullBufList.bitsBuf[fullBufList.numBufs] = *pBuf;
        fullBufList.numBufs++;
        Bell_FreeBitBufFree(pBuf);
        if (fullBufList.numBufs == VCODEC_BITSBUF_MAX)
        {
            break;
        }
    }
    if (fullBufList.numBufs)
    {
        gBell_ipcBitsCtrl.totalDecFrames+=fullBufList.numBufs;
        Vdec_putBitstreamBuffer(&fullBufList);
    }
}

static Void *Bell_ipcBitsSendFxn(Void * prm)
{
    Bell_IpcBitsCtrlThrObj *thrObj = (Bell_IpcBitsCtrlThrObj *) prm;
    static Int printStatsInterval = 0, i;

    OSA_printf("MCFW_IPCBITS:%s:Entered...",__func__);
    while (FALSE == thrObj->exitBitsOutThread)
    {
        OSA_waitMsecs(BELL_IPCBITS_SENDFXN_PERIOD_MS);
        for(i=0; i<IPCBITS_RESOLUTION_TYPES; i++)
            Bell_ipcBitsQueEmptyBitBufs(thrObj->resolution[i], &thrObj->bufQFreeBufs);
        Bell_ipcBitsSendFullBitBufs(&thrObj->bufQFullBufs);
        
        #ifdef IPC_BITS_DEBUG
        if ((printStatsInterval % BELL_IPCBITS_INFO_PRINT_INTERVAL) == 0)
        {
            OSA_printf("MCFW_IPCBITS:%s:INFO: periodic print..",__func__);
        }
        #endif
        printStatsInterval++;
    }
    OSA_printf("MCFW_IPCBITS:%s:Leaving...",__func__);
    return NULL;
}

VCODEC_BITSBUF_S* Bell_ipcGetEmptyBuf(void *ipcHdl, int size)
{
    int status;
    Bell_IpcBitsCtrl *ipcBitsCtrl = (Bell_IpcBitsCtrl *) ipcHdl;
    Bell_IpcBitsCtrlThrObj *thrObj = &ipcBitsCtrl->thrObj;

    VCODEC_BITSBUF_S *pEmptyBuf;
    status = OSA_queGet(&thrObj->bufQFreeBufs,(Int32 *)(&pEmptyBuf),
                        OSA_TIMEOUT_FOREVER);
    
    OSA_assert(status == 0);
    
    if (size > pEmptyBuf->bufSize)
    {
        status = OSA_quePut(&thrObj->bufQFreeBufs,(Int32)pEmptyBuf,
                            OSA_TIMEOUT_FOREVER);
        OSA_assert(status == 0);
        return NULL;
    }
    
    return pEmptyBuf;
}

int Bell_ipcBitsFeed(void *ipcHdl, VCODEC_BITSBUF_S *pFullBuf)
{
    Bell_IpcBitsCtrl *ipcBitsCtrl = (Bell_IpcBitsCtrl *) ipcHdl;
    Bell_IpcBitsCtrlThrObj *thrObj = &ipcBitsCtrl->thrObj;

    Int status;
   
    status = OSA_quePut(&thrObj->bufQFullBufs,
                        (Int32)pFullBuf,OSA_TIMEOUT_NONE);
    OSA_assert(status == 0);
    return status;
}

int Bell_ipcBitsGet(void *ipcHdl, VCODEC_BITSBUF_LIST_S *buflist)
{
    Bell_IpcBitsCtrl *ipcBitsCtrl = (Bell_IpcBitsCtrl *) ipcHdl;
    Bell_IpcBitsCtrlThrObj *thrObj = NULL;
    VCODEC_BITSBUF_LIST_S fullBufList;

    Int status;

    if(ipcHdl == NULL)
        return -1;

    thrObj = &ipcBitsCtrl->thrObj;
    
    status = OSA_semWait(&thrObj->bitsInNotifySem,OSA_TIMEOUT_NONE);
    if(status == OSA_EFAIL)
        return -1;
   
    return Venc_getBitstreamBuffer(buflist, 0);
}

int Bell_ipcBitsPut(VCODEC_BITSBUF_LIST_S *buflist)
{
    return Venc_releaseBitstreamBuffer(buflist);
}

static Void Bell_ipcBitsInitThrObj(Bell_IpcBitsCtrlThrObj *thrObj)
{
    OSA_semCreate(&thrObj->bitsInNotifySem,
                  BELL_IPCBITS_MAX_PENDING_RECV_SEM_COUNT,0);
    thrObj->exitBitsOutThread = FALSE;
    OSA_queCreate(&thrObj->bufQFreeBufs,BELL_IPCBITS_FREE_QUE_MAX_LEN);
    OSA_queCreate(&thrObj->bufQFullBufs,BELL_IPCBITS_FULL_QUE_MAX_LEN);
    OSA_thrCreate(&thrObj->thrHandleBitsOut,
                  Bell_ipcBitsSendFxn,
                  BELL_IPCBITS_SENDFXN_TSK_PRI,
                  BELL_IPCBITS_SENDFXN_TSK_STACK_SIZE,
                  thrObj);
}

static Void Bell_ipcBitsDeInitThrObj(Bell_IpcBitsCtrlThrObj *thrObj)
{
    thrObj->exitBitsOutThread = TRUE;
    OSA_thrDelete(&thrObj->thrHandleBitsOut);
    OSA_semDelete(&thrObj->bitsInNotifySem);
    OSA_queDelete(&thrObj->bufQFreeBufs);
    OSA_queDelete(&thrObj->bufQFullBufs);
}

Void Bell_ipcBitsInitSetBitsInNoNotifyMode(Bool noNotifyMode)
{
    gBell_ipcBitsCtrl.noNotifyBitsInHLOS = noNotifyMode;
}

Void Bell_ipcBitsInitSetBitsOutNoNotifyMode(Bool noNotifyMode)
{
    gBell_ipcBitsCtrl.noNotifyBitsOutHLOS = noNotifyMode;
}

static Int32 Bell_ipcBitsInitDmaObj()
{
    Int32 status = OSA_SOK;

    if (gBell_ipcBitsCtrl.dmaObj.useDma)
    {
        status = OSA_dmaInit();
        OSA_assert(status == OSA_SOK);

        status =
        OSA_dmaOpen(&gBell_ipcBitsCtrl.dmaObj.dmaChHdnl,
                    OSA_DMA_MODE_NORMAL,
                    BELL_IPC_BITS_DMA_MAX_TRANSFERS);
        OSA_assert(status == OSA_SOK);

    }
    if (OSA_SOK != status)
    {
        OSA_printf("MCFW_IPC_BITS_CTRL:Disabling DMA as channel open failed[%d]",
                   status);
        gBell_ipcBitsCtrl.dmaObj.useDma = FALSE;
    }
    return status;
}

static Int32 Bell_ipcBitsDeInitDmaObj()
{
    Int32 status = OSA_SOK;

    if (gBell_ipcBitsCtrl.dmaObj.useDma)
    {

        status =
        OSA_dmaClose(&gBell_ipcBitsCtrl.dmaObj.dmaChHdnl);
        OSA_assert(status == OSA_SOK);

        status = OSA_dmaExit();
        OSA_assert(status == OSA_SOK);
    }
    return status;
}

void* Bell_ipcBitsInit(Bell_res resolution[])
{
    VENC_CALLBACK_S callback;
    Int i;

    Bell_resetAvgStatistics();
    Bell_resetStatistics();

    callback.newDataAvailableCb = Bell_ipcBitsInCbFxn;
    /* Register call back with encoder */
    Venc_registerCallback(&callback,
                         (Ptr)&gBell_ipcBitsCtrl);

    Bell_FreeBitBufInit();
    Bell_ipcBitsInitDmaObj();

    for (i=0; i<IPCBITS_RESOLUTION_TYPES; i++)
        gBell_ipcBitsCtrl.thrObj.resolution[i] = resolution[i];
    
    Bell_ipcBitsInitThrObj(&gBell_ipcBitsCtrl.thrObj);
    return (Ptr)&gBell_ipcBitsCtrl;
}

Int32 Bell_ipcBitsExit()
{
    OSA_printf("Entered:%s...",__func__);    
    Bell_FreeBitBufDeInit();
    Bell_ipcBitsDeInitDmaObj();
    Bell_ipcBitsDeInitThrObj(&gBell_ipcBitsCtrl.thrObj);
    OSA_printf("Leaving:%s...",__func__);
    return OSA_SOK;
}

Int32 Bell_resetStatistics()
{
    UInt32 chId;
    Bell_ChInfo *pChInfo;

    for(chId=0; chId<VENC_CHN_MAX; chId++)
    {
        pChInfo = &gBell_ipcBitsCtrl.chInfo[chId];

        pChInfo->totalDataSize = 0;
        pChInfo->numKeyFrames = 0;
        pChInfo->numFrames = 0;
        pChInfo->maxWidth = 0;
        pChInfo->minWidth = 0xFFF;
        pChInfo->maxHeight= 0;
        pChInfo->minHeight= 0xFFF;
        pChInfo->maxLatency= 0;
        pChInfo->minLatency= 0xFFF;

    }

    gBell_ipcBitsCtrl.statsStartTime = OSA_getCurTimeInMsec();

    return 0;
}

Int32 Bell_resetAvgStatistics()
{
    gBell_ipcBitsCtrl.totalEncFrames = 0;
    gBell_ipcBitsCtrl.totalDecFrames = 0;

    return 0;
}

Int32 Bell_printAvgStatistics(UInt32 elaspedTime, Bool resetStats)
{
    UInt32 totalFrames;

    totalFrames = gBell_ipcBitsCtrl.totalEncFrames+gBell_ipcBitsCtrl.totalDecFrames;

    /* show total average encode FPS */
    printf( " ##>> AVERAGE: ENCODE [%6.1f] FPS, DECODE [%6.1f] FPS, ENC+DEC [%6.1f] FPS ... in %4.1f secs, \n",
            gBell_ipcBitsCtrl.totalEncFrames*1000.0/elaspedTime,
            gBell_ipcBitsCtrl.totalDecFrames*1000.0/elaspedTime,
            totalFrames*1000.0/elaspedTime,
            elaspedTime/1000.0
            );

    if(resetStats)
        Bell_resetAvgStatistics();

    return 0;
}

Int32 Bell_printStatistics(Bool resetStats, Bool allChs)
{
    UInt32 chId;
    Bell_ChInfo *pChInfo;
    float elaspedTime;

    elaspedTime = OSA_getCurTimeInMsec() - gBell_ipcBitsCtrl.statsStartTime;
    elaspedTime /= 1000.0; // in secs

    if(allChs)
    {
        printf( "\n"
            "\n *** Encode Bitstream Received Statistics *** "
            "\n"
            "\n Elased time = %6.1f secs"
            "\n"
            "\n CH | Bitrate (Kbps) | FPS | Key-frame FPS | Width (max/min) | Height (max/min) | Latency (max/min)"
            "\n --------------------------------------------------------------------------------------------------",
            elaspedTime
            );

        for(chId=0; chId<VENC_CHN_MAX;chId++)
        {
            pChInfo = &gBell_ipcBitsCtrl.chInfo[chId];

            if(pChInfo->numFrames)
            {
                printf("\n %2d | %14.2f | %3.1f | %13.1f | %5d / %6d | %6d / %6d  | %6d / %6d",
                    chId,
                    (pChInfo->totalDataSize*8.0/elaspedTime)/1024.0,
                    pChInfo->numFrames*1.0/elaspedTime,
                    pChInfo->numKeyFrames*1.0/elaspedTime,
                    pChInfo->maxWidth,
                    pChInfo->minWidth,
                    pChInfo->maxHeight,
                    pChInfo->minHeight,
                    pChInfo->maxLatency,
                    pChInfo->minLatency

                );
            }
        }

        printf("\n");

        if(resetStats)
            Bell_resetStatistics();
    }
    else
    {
    }

    return 0;
}

Int32 Bell_updateStatistics(VCODEC_BITSBUF_S *pBuf)
{
    Bell_ChInfo *pChInfo;
    UInt32 latency;

    if(pBuf->chnId<VENC_CHN_MAX)
    {
        pChInfo = &gBell_ipcBitsCtrl.chInfo[pBuf->chnId];

        pChInfo->totalDataSize += pBuf->filledBufSize;
        pChInfo->numFrames++;
        gBell_ipcBitsCtrl.totalEncFrames++;
        if(pBuf->frameType==VCODEC_FRAME_TYPE_I_FRAME)
        {
            pChInfo->numKeyFrames++;
        }

        latency = pBuf->encodeTimestamp - pBuf->timestamp;

        if(latency > pChInfo->maxLatency)
            pChInfo->maxLatency = latency;

        if(latency < pChInfo->minLatency)
            pChInfo->minLatency = latency;

        if(pBuf->frameWidth > pChInfo->maxWidth)
            pChInfo->maxWidth = pBuf->frameWidth;

        if(pBuf->frameWidth < pChInfo->minWidth)
            pChInfo->minWidth = pBuf->frameWidth;

        if(pBuf->frameHeight > pChInfo->maxHeight)
            pChInfo->maxHeight = pBuf->frameHeight;

        if(pBuf->frameHeight < pChInfo->minHeight)
            pChInfo->minHeight = pBuf->frameHeight;

    }
    return 0;
}
