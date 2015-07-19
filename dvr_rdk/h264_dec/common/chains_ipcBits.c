/*******************************************************************************
 *                                                                             *
 * Copyright (c) 2011 Texas Instruments Incorporated - http://www.ti.com/      *
 *                        ALL RIGHTS RESERVED                                  *
 *                                                                             *
 ******************************************************************************/

/**
    \file chains_ipcBits.c
    \brief
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <sys/wait.h>
#include <sys/types.h>  // For stat().
#include <sys/stat.h>   // For stat().
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <osa_que.h>
#include <osa_mutex.h>
#include <osa_thr.h>
#include <osa_sem.h>
#include <avcap/common/chains_ipcBits.h>

#ifdef KB
#undef KB
#endif

#ifdef MB
#undef MB
#endif

#define KB                                                               (1024)
#define MB                                                               (KB*KB)

/* Set to TRUE to prevent doing fgets */
#define CHAINS_IPC_BITS_DISABLE_USER_INPUT                               (FALSE)


#define CHAINS_IPC_BITS_MAX_FILENAME_LENGTH                                 (64)
#define CHAINS_IPC_BITS_MAX_PATH_LENGTH                                     (256)
#define CHAINS_IPC_BITS_MAX_FULL_PATH_FILENAME_LENGTH                       (CHAINS_IPC_BITS_MAX_PATH_LENGTH+CHAINS_IPC_BITS_MAX_FILENAME_LENGTH)
#define CHAINS_IPC_BITS_FILE_STORE_DIR                                      "/dev/shm"
#define CHAINS_IPC_BITS_HDR_FILE_NAME                                       "VBITS_HDR"
#define CHAINS_IPC_BITS_DATA_FILE_NAME                                      "VBITS_DATA"
#define CHAINS_IPC_BITS_FILE_EXTENSION                                      "bin"

#define CHAINS_IPC_BITS_MAX_NUM_CHANNELS                                    (16)
#define CHAINS_IPC_BITS_NONOTIFYMODE_BITSIN                                 (TRUE)
#define CHAINS_IPC_BITS_NONOTIFYMODE_BITSOUT                                (TRUE)

#define CHAINS_IPC_BITS_MAX_DEFAULT_SIZE                                    (1*MB)
#define CHAINS_IPC_BITS_FREE_SPACE_RETRY_MS                                 (16)
#define CHAINS_IPC_BITS_FREE_SPACE_MAX_RETRY_CNT                            (500)

#define CHAINS_IPC_BITS_MAX_BUFCONSUMEWAIT_MS                               (1000)

#define CHAINS_IPC_BITS_MAX_FILE_SIZE           (CHAINS_IPC_BITS_MAX_DEFAULT_SIZE)

#define CHAINS_IPC_BITS_INIT_FILEHANDLE(fp)                                    \
                                                   do {                        \
                                                       if (fp != NULL)         \
                                                       {                       \
                                                           fclose(fp);         \
                                                           fp = NULL;          \
                                                       }                       \
                                                   } while (0)

#define CHAINS_IPC_BITS_ENCODER_FPS                      (30)
#define CHAINS_IPC_BITS_ENCODER_BITRATE                  (2 * 1024 * 1024)
#define CHAINS_IPC_BITS_FILEBUF_SIZE_HDR                 (sizeof(Bitstream_Buf) * CHAINS_IPC_BITS_ENCODER_FPS)
#define CHAINS_IPC_BITS_FILEBUF_SIZE_DATA                (CHAINS_IPC_BITS_ENCODER_BITRATE)

#define CHAINS_IPCBITS_SENDFXN_TSK_PRI                   (2)
#define CHAINS_IPCBITS_RECVFXN_TSK_PRI                   (2)

#define CHAINS_IPCBITS_SENDFXN_TSK_STACK_SIZE            (0) /* 0 means system default will be used */
#define CHAINS_IPCBITS_RECVFXN_TSK_STACK_SIZE            (0) /* 0 means system default will be used */

#define CHAINS_IPCBITS_SENDFXN_PERIOD_MS                 (16)
#define CHAINS_IPCBITS_RECVFXN_PERIOD_MS                 (16)

#define CHAINS_IPCBITS_INFO_PRINT_INTERVAL               (1000)

/** @enum CHAINS_IPCBITS_GET_BITBUF_SIZE
 *  @brief Macro that returns max size of encoded bitbuffer for a given resolution
 */
#define CHAINS_IPCBITS_DEFAULT_WIDTH                   (1920)
#define CHAINS_IPCBITS_DEFAULT_HEIGHT                  (1080)

#define CHAINS_IPCBITS_GET_BITBUF_SIZE(width,height)   ((width) * (height)/2)

#define CHAINS_IPCBITS_MAX_PENDING_RECV_SEM_COUNT      (10)

#define CHAINS_IPCBITS_MAX_NUM_FREE_BUFS_PER_CHANNEL    (6)
#define CHAINS_IPCBITS_FREE_QUE_MAX_LEN                 (CHAINS_IPC_BITS_MAX_NUM_CHANNELS * \
                                                         CHAINS_IPCBITS_MAX_NUM_FREE_BUFS_PER_CHANNEL)
#define CHAINS_IPCBITS_FULL_QUE_MAX_LEN                 (CHAINS_IPCBITS_FREE_QUE_MAX_LEN)

#define CHAINS_IPC_BITS_ENABLE_FILE_WRITE               (TRUE)

#define CHAINS_IPC_BITS_FWRITE_ENABLE_BITMASK_CHANNEL_0     (1 << 0)
#define CHAINS_IPC_BITS_FWRITE_ENABLE_BITMASK_ALLCHANNELS   ((1 << CHAINS_IPC_BITS_MAX_NUM_CHANNELS) - 1)

#define CHAINS_IPC_BITS_FWRITE_ENABLE_BITMASK_DEFAULT   (CHAINS_IPC_BITS_FWRITE_ENABLE_BITMASK_CHANNEL_0)

#define CHAINS_IPC_BITS_TRACE_ENABLE_FXN_ENTRY_EXIT           (1)
#define CHAINS_IPC_BITS_TRACE_INFO_PRINT_INTERVAL             (8192)


#if CHAINS_IPC_BITS_TRACE_ENABLE_FXN_ENTRY_EXIT
#define CHAINS_IPC_BITS_TRACE_FXN(str,...)         do {                           \
                                                     static Int printInterval = 0;\
                                                     if ((printInterval % CHAINS_IPC_BITS_TRACE_INFO_PRINT_INTERVAL) == 0) \
                                                     {                                                          \
                                                         OSA_printf("CHAINS_IPCBITS:%s function:%s",str,__func__);     \
                                                         OSA_printf(__VA_ARGS__);                               \
                                                     }                                                          \
                                                     printInterval++;                                           \
                                                   } while (0)
#define CHAINS_IPC_BITS_TRACE_FXN_ENTRY(...)                  CHAINS_IPC_BITS_TRACE_FXN("Entered",__VA_ARGS__)
#define CHAINS_IPC_BITS_TRACE_FXN_EXIT(...)                   CHAINS_IPC_BITS_TRACE_FXN("Leaving",__VA_ARGS__)
#else
#define CHAINS_IPC_BITS_TRACE_FXN_ENTRY(...)
#define CHAINS_IPC_BITS_TRACE_FXN_EXIT(...)
#endif


enum Chains_IpcBitsFileType {
    CHAINS_IPC_BITS_FILETYPE_HDR,
    CHAINS_IPC_BITS_FILETYPE_BUF
} ;


typedef struct Chains_IpcBitsCtrlFileObj {
    FILE *fpWrHdr[CHAINS_IPC_BITS_MAX_NUM_CHANNELS];
    FILE *fpWrData[CHAINS_IPC_BITS_MAX_NUM_CHANNELS];
    char    fileDirPath[CHAINS_IPC_BITS_MAX_PATH_LENGTH];
    UInt32  maxFileSize;
    Bool    enableFWrite;
    UInt32  fwriteEnableBitMask;
} Chains_IpcBitsCtrlFileObj;

typedef struct Chains_IpcBitsCtrlThrObj {
    OSA_ThrHndl thrHandleBitsIn;
    OSA_ThrHndl thrHandleBitsOut;
    OSA_QueHndl bufQFullBufs;
    OSA_QueHndl bufQFreeBufs;
    OSA_SemHndl bitsInNotifySem;
    volatile Bool exitBitsInThread;
    volatile Bool exitBitsOutThread;
} Chains_IpcBitsCtrlThrObj;

typedef struct Chains_IpcBitsCtrl {
    Bool  noNotifyBitsInHLOS;
    Bool  noNotifyBitsOutHLOS;;
    Chains_IpcBitsCtrlFileObj fObj;
    Chains_IpcBitsCtrlThrObj  thrObj;

} Chains_IpcBitsCtrl;

Chains_IpcBitsCtrl gChains_ipcBitsCtrl =
{
    .fObj.fpWrHdr  = {NULL},
    .fObj.fpWrData = {NULL},
    .fObj.maxFileSize    = CHAINS_IPC_BITS_MAX_FILE_SIZE,
    .fObj.enableFWrite   = CHAINS_IPC_BITS_ENABLE_FILE_WRITE,
    .fObj.fwriteEnableBitMask = CHAINS_IPC_BITS_FWRITE_ENABLE_BITMASK_DEFAULT,
    .noNotifyBitsInHLOS  = CHAINS_IPC_BITS_NONOTIFYMODE_BITSIN,
    .noNotifyBitsOutHLOS = CHAINS_IPC_BITS_NONOTIFYMODE_BITSOUT,
};

static
Void Chains_ipcBitsGenerateFileName(char *dirPath,
                                    char *fname,
                                    UInt32 chNum,
                                    char *fsuffix,
                                    char *dstBuf,
                                    UInt32 maxLen);

static Void Chains_ipcBitsWriteWrap(FILE  * fp,
                                    UInt32 bytesToWrite,
                                    UInt32 maxFileSize)
{
    static Int printStatsIterval = 0;

    if (maxFileSize != CHAINS_IPC_BITS_MAX_FILE_SIZE_INFINITY)
    {
        if (((ftell(fp)) + bytesToWrite) > maxFileSize)
        {
            if ((printStatsIterval % CHAINS_IPCBITS_INFO_PRINT_INTERVAL) == 0)
                OSA_printf("CHAINS_IPCBITS:File wrap @ [%ld],MaxFileSize [%d]",
                           ftell(fp),maxFileSize);
            rewind(fp);
            printStatsIterval++;
        }
    }
}

static Void Chains_ipcBitsWriteBitsToFile (FILE  * fpHdr[CHAINS_IPC_BITS_MAX_NUM_CHANNELS],
                                           FILE  * fpBuf[CHAINS_IPC_BITS_MAX_NUM_CHANNELS],
                                           Bitstream_BufList *bufList,
                                           UInt32 maxFileSize,
                                           UInt32 fwriteEnableChannelBitmask)
{
    Int i;
    Bitstream_Buf *pBuf;
    size_t write_cnt;

    for (i = 0; i < bufList->numBufs;i++)
    {
        UInt32 fileIdx;

        pBuf = bufList->bufs[i];
        OSA_assert(pBuf->channelNum < CHAINS_IPC_BITS_MAX_NUM_CHANNELS);
        fileIdx = pBuf->channelNum;
        if (fwriteEnableChannelBitmask & (1 << fileIdx))
        {
            Chains_ipcBitsWriteWrap(fpBuf[fileIdx],pBuf->fillLength,maxFileSize);
            write_cnt = fwrite(pBuf->addr,sizeof(char),pBuf->fillLength,fpBuf[fileIdx]);
            OSA_assert(write_cnt == pBuf->fillLength);
            Chains_ipcBitsWriteWrap(fpHdr[fileIdx],sizeof(*pBuf),maxFileSize);
            write_cnt = fwrite(pBuf,sizeof(*pBuf),1,fpHdr[fileIdx]);
            OSA_assert(write_cnt == 1);
        }
    }
}

static Void Chains_ipcBitsCopyBitBufInfo (Bitstream_Buf *dst,
                                          const Bitstream_Buf *src)
{
    dst->channelNum = src->channelNum;
    dst->codingType = src->codingType;
    dst->fillLength = src->fillLength;
    dst->isKeyFrame = src->isKeyFrame;
    dst->timeStamp  = src->timeStamp;
    dst->mvDataFilledSize = src->mvDataFilledSize;
    dst->bottomFieldBitBufSize = src->bottomFieldBitBufSize;
    dst->inputFileChanged = src->inputFileChanged;
    CHAINS_IPC_BITS_TRACE_FXN_EXIT("BitBufInfo:"
                         "virt:%p,"
                         "bufSize:%d,"
                         "chnId:%d,"
                         "codecType:%d,"
                         "filledBufSize:%d,"
                         "mvDataFilledSize:%d,"
                         "timeStamp:%d,"
                         "isKeyFrame:%d,"
                         "phy:%x,"
                         "width:%d"
                         "height:%d",
                         src->addr,
                         src->bufSize,
                         src->channelNum,
                         src->codingType,
                         src->fillLength,
                         src->mvDataFilledSize,
                         src->timeStamp,
                         src->isKeyFrame,
                         src->phyAddr,
                         src->frameWidth,
                         src->frameHeight);
}


static Void Chains_ipcBitsCopyBitBufDataMem2Mem(Bitstream_Buf *dstBuf,
                                                Bitstream_Buf *srcBuf)
{

    OSA_assert(srcBuf->fillLength < dstBuf->bufSize);
    memcpy(dstBuf->addr,srcBuf->addr,srcBuf->fillLength);
}

static Void Chains_ipcBitsQueEmptyBitBufs(UInt32            ipcBitsInLinkId,
                                          OSA_QueHndl       *emptyQue)
{
    Bitstream_BufList emptyBufList;
    Bitstream_Buf *pBuf;
    IpcBitsOutLinkHLOS_BitstreamBufReqInfo reqInfo;
    Int i;
    Int status;
    UInt32 bitBufSize;


    bitBufSize = CHAINS_IPCBITS_GET_BITBUF_SIZE(CHAINS_IPCBITS_DEFAULT_WIDTH,
                                                CHAINS_IPCBITS_DEFAULT_HEIGHT);
    emptyBufList.numBufs = 0;
    reqInfo.numBufs = VIDBITSTREAM_MAX_BITSTREAM_BUFS;
    reqInfo.reqType = IPC_BITSOUTHLOS_BITBUFREQTYPE_BUFSIZE;
    for (i = 0; i < VIDBITSTREAM_MAX_BITSTREAM_BUFS; i++)
    {
        reqInfo.u[i].minBufSize = bitBufSize;
    }
    IpcBitsOutLink_getEmptyVideoBitStreamBufs(ipcBitsInLinkId,
                                              &emptyBufList,
                                              &reqInfo);
    for (i = 0; i < emptyBufList.numBufs; i++)
    {
        pBuf = emptyBufList.bufs[i];
        OSA_assert(pBuf->bufSize >= bitBufSize );

        status = OSA_quePut(emptyQue,(Int32)pBuf,OSA_TIMEOUT_NONE);
        OSA_assert(status == 0);
    }
}

static Void Chains_ipcBitsSendFullBitBufs(UInt32            ipcBitsInLinkId,
                                          OSA_QueHndl       *fullQue)
{
    Bitstream_BufList fullBufList;
    Bitstream_Buf *pBuf;
    Int status;

    fullBufList.numBufs = 0;
    while((status = OSA_queGet(fullQue,(Int32 *)(&pBuf),OSA_TIMEOUT_NONE)) == 0)
    {
        OSA_assert(fullBufList.numBufs < VIDBITSTREAM_MAX_BITSTREAM_BUFS);
        fullBufList.bufs[fullBufList.numBufs] = pBuf;
        fullBufList.numBufs++;
        if (fullBufList.numBufs == VIDBITSTREAM_MAX_BITSTREAM_BUFS)
        {
            break;
        }
    }
    if (fullBufList.numBufs)
    {
        IpcBitsOutLink_putFullVideoBitStreamBufs(ipcBitsInLinkId,
                                                 &fullBufList);
    }
}

static Void *Chains_ipcBitsSendFxn(Void * prm)
{
    Chains_IpcBitsCtrlThrObj *thrObj = (Chains_IpcBitsCtrlThrObj *) prm;
    static Int printStatsInterval = 0;

    OSA_printf("CHAINS_IPCBITS:%s:Entered...",__func__);
    while (FALSE == thrObj->exitBitsOutThread)
    {
        OSA_waitMsecs(CHAINS_IPCBITS_SENDFXN_PERIOD_MS);
        Chains_ipcBitsQueEmptyBitBufs(SYSTEM_HOST_LINK_ID_IPC_BITS_OUT_0,
                                      &thrObj->bufQFreeBufs);
        Chains_ipcBitsSendFullBitBufs(SYSTEM_HOST_LINK_ID_IPC_BITS_OUT_0,
                                      &thrObj->bufQFullBufs);
        if ((printStatsInterval % CHAINS_IPCBITS_INFO_PRINT_INTERVAL) == 0)
        {
            OSA_printf("CHAINS_IPCBITS:%s:INFO: periodic print..",__func__);
        }
        printStatsInterval++;
    }
    OSA_printf("CHAINS_IPCBITS:%s:Leaving...",__func__);
    return NULL;
}

static Void Chains_ipcBitsProcessFullBufs(UInt32            ipcBitsInLinkId,
                                          Chains_IpcBitsCtrlThrObj *thrObj,
                                          Chains_IpcBitsCtrlFileObj *fObj)
{
    Bitstream_BufList fullBufList;
    Bitstream_Buf *pFullBuf;
    Bitstream_Buf *pEmptyBuf;
    Int i,status;

    IpcBitsInLink_getFullVideoBitStreamBufs(ipcBitsInLinkId,
                                            &fullBufList);
    for (i = 0; i < fullBufList.numBufs; i++)
    {
        status = OSA_queGet(&thrObj->bufQFreeBufs,(Int32 *)(&pEmptyBuf),
                            OSA_TIMEOUT_FOREVER);
        OSA_assert(status == 0);
        pFullBuf = fullBufList.bufs[i];
        Chains_ipcBitsCopyBitBufInfo(pEmptyBuf,pFullBuf);
        Chains_ipcBitsCopyBitBufDataMem2Mem(pEmptyBuf,pFullBuf);
        status = OSA_quePut(&thrObj->bufQFullBufs,
                            (Int32)pEmptyBuf,OSA_TIMEOUT_NONE);
        OSA_assert(status == 0);
    }
    if (fObj->enableFWrite)
    {
        Chains_ipcBitsWriteBitsToFile(fObj->fpWrHdr,
                                      fObj->fpWrData,
                                      &fullBufList,
                                      fObj->maxFileSize,
                                      fObj->fwriteEnableBitMask);

    }
    IpcBitsInLink_putEmptyVideoBitStreamBufs(ipcBitsInLinkId,
                                             &fullBufList);
}

static Void *Chains_ipcBitsRecvFxn(Void * prm)
{
    Chains_IpcBitsCtrl *ipcBitsCtrl = (Chains_IpcBitsCtrl *) prm;
    Chains_IpcBitsCtrlThrObj *thrObj = &ipcBitsCtrl->thrObj;
    Chains_IpcBitsCtrlFileObj *fObj =  &ipcBitsCtrl->fObj;
    static Int printStats;

    OSA_printf("CHAINS_IPCBITS:%s:Entered...",__func__);
    while (FALSE == thrObj->exitBitsInThread)
    {
        OSA_semWait(&thrObj->bitsInNotifySem,OSA_TIMEOUT_FOREVER);
        Chains_ipcBitsProcessFullBufs(SYSTEM_HOST_LINK_ID_IPC_BITS_IN_0,
                                      thrObj,
                                      fObj);
        if ((printStats % CHAINS_IPCBITS_INFO_PRINT_INTERVAL) == 0)
        {
            OSA_printf("CHAINS_IPCBITS:%s:INFO: periodic print..",__func__);
        }
        printStats++;
    }
    OSA_printf("CHAINS_IPCBITS:%s:Leaving...",__func__);
    return NULL;
}


static Void Chains_ipcBitsInitThrObj(Chains_IpcBitsCtrlThrObj *thrObj)
{

    OSA_semCreate(&thrObj->bitsInNotifySem,
                  CHAINS_IPCBITS_MAX_PENDING_RECV_SEM_COUNT,0);
    thrObj->exitBitsInThread = FALSE;
    thrObj->exitBitsOutThread = FALSE;
    OSA_queCreate(&thrObj->bufQFreeBufs,CHAINS_IPCBITS_FREE_QUE_MAX_LEN);
    OSA_queCreate(&thrObj->bufQFullBufs,CHAINS_IPCBITS_FULL_QUE_MAX_LEN);
    OSA_thrCreate(&thrObj->thrHandleBitsOut,
                  Chains_ipcBitsSendFxn,
                  CHAINS_IPCBITS_SENDFXN_TSK_PRI,
                  CHAINS_IPCBITS_SENDFXN_TSK_STACK_SIZE,
                  thrObj);

    OSA_thrCreate(&thrObj->thrHandleBitsIn,
                  Chains_ipcBitsRecvFxn,
                  CHAINS_IPCBITS_RECVFXN_TSK_PRI,
                  CHAINS_IPCBITS_RECVFXN_TSK_STACK_SIZE,
                  &gChains_ipcBitsCtrl);

}

static Void Chains_ipcBitsDeInitThrObj(Chains_IpcBitsCtrlThrObj *thrObj)
{
    thrObj->exitBitsInThread = TRUE;
    thrObj->exitBitsOutThread = TRUE;
    OSA_thrDelete(&thrObj->thrHandleBitsOut);
    OSA_thrDelete(&thrObj->thrHandleBitsIn);
    OSA_semDelete(&thrObj->bitsInNotifySem);
    OSA_queDelete(&thrObj->bufQFreeBufs);
    OSA_queDelete(&thrObj->bufQFullBufs);

}


static
Void Chains_ipcBitsGenerateFileName(char *dirPath,
                                    char *fname,
                                    UInt32 chNum,
                                    char *fsuffix,
                                    char *dstBuf,
                                    UInt32 maxLen)
{
    snprintf(dstBuf,
             (maxLen - 2),
             "%s/%s_%d.%s",
             dirPath,
             fname,
             chNum,
             fsuffix);
    dstBuf[(maxLen - 1)] = 0;
}

static Void Chains_ipcBitsInCbFxn (Ptr cbCtx)
{
    Chains_IpcBitsCtrl *chains_ipcBitsCtrl;
    static Int printInterval;

    OSA_assert(cbCtx = &gChains_ipcBitsCtrl);
    chains_ipcBitsCtrl = cbCtx;
    OSA_semSignal(&chains_ipcBitsCtrl->thrObj.bitsInNotifySem);
    if ((printInterval % CHAINS_IPCBITS_INFO_PRINT_INTERVAL) == 0)
    {
        OSA_printf("CHAINS_IPCBITS: Callback function:%s",__func__);
    }
    printInterval++;
}

Void Chains_ipcBitsInitCreateParams_BitsInHLOS(IpcBitsInLinkHLOS_CreateParams *cp)
{
    cp->baseCreateParams.noNotifyMode = gChains_ipcBitsCtrl.noNotifyBitsInHLOS;
    cp->cbFxn = Chains_ipcBitsInCbFxn;
    cp->cbCtx = &gChains_ipcBitsCtrl;
    cp->baseCreateParams.notifyNextLink = FALSE;
    /* Previous link of bitsInHLOS is bitsOutRTOS. So, notifyPrevLink
     * should be set to false if bitsInHLOS is to operate in
     * NO_NOTIFY_MODE
     */
    cp->baseCreateParams.notifyPrevLink = !(gChains_ipcBitsCtrl.noNotifyBitsInHLOS);
}

Void Chains_ipcBitsInitCreateParams_BitsInRTOS(IpcBitsInLinkRTOS_CreateParams *cp,
                                                Bool notifyNextLink)
{
    /* Previous link of bitsInRTOS is bitsOutHLOSE. So, notifyPrevLink
     * should be set to false if bitsOutHLOS is to operate in
     * NO_NOTIFY_MODE
     */
    cp->baseCreateParams.noNotifyMode = gChains_ipcBitsCtrl.noNotifyBitsOutHLOS;
    cp->baseCreateParams.notifyNextLink = notifyNextLink;
    cp->baseCreateParams.notifyPrevLink = !(gChains_ipcBitsCtrl.noNotifyBitsOutHLOS);
}

Void Chains_ipcBitsInitSetBitsInNoNotifyMode(Bool noNotifyMode)
{
    gChains_ipcBitsCtrl.noNotifyBitsInHLOS = noNotifyMode;
}

Void Chains_ipcBitsInitSetBitsOutNoNotifyMode(Bool noNotifyMode)
{
    gChains_ipcBitsCtrl.noNotifyBitsOutHLOS = noNotifyMode;
}

Void Chains_ipcBitsInitCreateParams_BitsOutHLOS(IpcBitsOutLinkHLOS_CreateParams *cp,
                                                System_LinkQueInfo *inQueInfo)
{
    /* Next link of bitsOutRTOS is bitsInHLOS. So, notifyPrevLink
     * should be set to false if bitsInHLOS is to operate in
     * NO_NOTIFY_MODE
     */
    cp->baseCreateParams.notifyNextLink = !(gChains_ipcBitsCtrl.noNotifyBitsOutHLOS);
    cp->baseCreateParams.notifyPrevLink = FALSE;
    cp->inQueInfo = *inQueInfo;
}

Void Chains_ipcBitsInitCreateParams_BitsOutRTOS(IpcBitsOutLinkRTOS_CreateParams *cp,
                                                Bool notifyPrevLink)
{
    /* Next link of bitsOutRTOS is bitsInHLOS. So, notifyPrevLink
     * should be set to false if bitsInHLOS is to operate in
     * NO_NOTIFY_MODE
     */
    cp->baseCreateParams.noNotifyMode = gChains_ipcBitsCtrl.noNotifyBitsInHLOS;
    cp->baseCreateParams.notifyNextLink = !(gChains_ipcBitsCtrl.noNotifyBitsOutHLOS);
    cp->baseCreateParams.notifyPrevLink = notifyPrevLink;
}

static
Bool Chains_ipcBitsDirectoryExists( const char* absolutePath )
{

    if(access( absolutePath, F_OK ) == 0 ){

        struct stat status;
        stat( absolutePath, &status );

        return (status.st_mode & S_IFDIR) != 0;
    }
    return FALSE;
}

static
Int   Chains_ipcBitsOpenFileHandles()
{
    Int status = OSA_SOK;
    Int i;
    char fileNameHdr[128];
    char fileNameBuffer[128];


    for (i = 0; i < CHAINS_IPC_BITS_MAX_NUM_CHANNELS; i++)
    {
        Chains_ipcBitsGenerateFileName(gChains_ipcBitsCtrl.fObj.fileDirPath,
                                       CHAINS_IPC_BITS_HDR_FILE_NAME,
                                       i,
                                       CHAINS_IPC_BITS_FILE_EXTENSION,
                                       fileNameHdr,
                                       sizeof(fileNameHdr));
        gChains_ipcBitsCtrl.fObj.fpWrHdr[i] = fopen(fileNameHdr,"wb");
        OSA_assert(gChains_ipcBitsCtrl.fObj.fpWrHdr[i] != NULL);
        status =  setvbuf(gChains_ipcBitsCtrl.fObj.fpWrHdr[i],
                          NULL,
                          _IOFBF,
                          CHAINS_IPC_BITS_FILEBUF_SIZE_HDR);
        OSA_assert(status != -1);

        Chains_ipcBitsGenerateFileName(gChains_ipcBitsCtrl.fObj.fileDirPath,
                                       CHAINS_IPC_BITS_DATA_FILE_NAME,
                                       i,
                                       CHAINS_IPC_BITS_FILE_EXTENSION,
                                       fileNameBuffer,
                                       sizeof(fileNameBuffer));


        gChains_ipcBitsCtrl.fObj.fpWrData[i] = fopen(fileNameBuffer,"wb");
        OSA_assert(gChains_ipcBitsCtrl.fObj.fpWrData[i] != NULL);
        status =  setvbuf(gChains_ipcBitsCtrl.fObj.fpWrData[i],
                          NULL,
                          _IOFBF,
                          CHAINS_IPC_BITS_FILEBUF_SIZE_DATA);
        OSA_assert(status != -1);

    }
    return status;
}

static
Int   Chains_ipcBitsInitFileHandles()
{
    Int i;

    for (i = 0; i < CHAINS_IPC_BITS_MAX_NUM_CHANNELS; i++)
    {
        CHAINS_IPC_BITS_INIT_FILEHANDLE (gChains_ipcBitsCtrl.fObj.fpWrHdr[i]);
        CHAINS_IPC_BITS_INIT_FILEHANDLE (gChains_ipcBitsCtrl.fObj.fpWrData[i]);
    }
    return OSA_SOK;
}

static Int32 Chains_ipcBitsInitFObj()
{
    static Bool fileDirInputDone = FALSE;

    Chains_ipcBitsInitFileHandles();
#if (CHAINS_IPC_BITS_DISABLE_USER_INPUT != TRUE)
    if (!fileDirInputDone)
    {
        OSA_printf("\n\nCHAINS:Enter file store path:");
        fflush(stdin);
        fscanf(stdin,
               "%s",
               gChains_ipcBitsCtrl.fObj.fileDirPath);

        if (Chains_ipcBitsDirectoryExists(gChains_ipcBitsCtrl.fObj.fileDirPath) == 0)
        {
            OSA_printf("\n\nCHAINS:INVALID DIR PATH!!!!\n"
                       "\nUsing default file store path:%s \n",
                       CHAINS_IPC_BITS_FILE_STORE_DIR);
            strncpy(gChains_ipcBitsCtrl.fObj.fileDirPath,CHAINS_IPC_BITS_FILE_STORE_DIR,
                    (sizeof(gChains_ipcBitsCtrl.fObj.fileDirPath) - 1));
            gChains_ipcBitsCtrl.fObj.fileDirPath[(sizeof(gChains_ipcBitsCtrl.fObj.fileDirPath) - 1)] = 0;
            fileDirInputDone = TRUE;
        }
    }
#else
    OSA_printf("\n\nCHAINS:Using default file store path:%s \n",
               CHAINS_IPC_BITS_FILE_STORE_DIR);
    strncpy(gChains_ipcBitsCtrl.fileDirPath,CHAINS_IPC_BITS_FILE_STORE_DIR,
            (sizeof(gChains_ipcBitsCtrl.fileDirPath) - 1));
    gChains_ipcBitsCtrl.fileDirPath[(sizeof(gChains_ipcBitsCtrl.fileDirPath) - 1)] = 0;
#endif
    OSA_printf("\n\nCHAINS:Selected File store path:%s",gChains_ipcBitsCtrl.fObj.fileDirPath);
    Chains_ipcBitsOpenFileHandles();
    return OSA_SOK;
}



Int32 Chains_ipcBitsInit()
{
    char opMode[128];

    OSA_printf("CHAINS:Enable file write :(y -- yes/n -- no):");
    fflush(stdin);
    fscanf(stdin,"%s",&(opMode[0]));
    if (strcmp("y",opMode) == 0)
    {
        gChains_ipcBitsCtrl.fObj.enableFWrite = TRUE;
    }
    else
    {
        gChains_ipcBitsCtrl.fObj.enableFWrite = FALSE;
    }
    if (gChains_ipcBitsCtrl.fObj.enableFWrite == TRUE){
        Chains_ipcBitsInitFObj();
    }
    Chains_ipcBitsInitThrObj(&gChains_ipcBitsCtrl.thrObj);
    return OSA_SOK;
}

Void Chains_ipcBitsStop(void)
{
    gChains_ipcBitsCtrl.thrObj.exitBitsInThread = TRUE;
    gChains_ipcBitsCtrl.thrObj.exitBitsOutThread = TRUE;
}

Int32 Chains_ipcBitsExit()
{
    OSA_printf("Entered:%s...",__func__);
    if (gChains_ipcBitsCtrl.fObj.enableFWrite == TRUE){
        Chains_ipcBitsInitFileHandles();
    }
    Chains_ipcBitsDeInitThrObj(&gChains_ipcBitsCtrl.thrObj);
    OSA_printf("Leaving:%s...",__func__);
    return OSA_SOK;
}


