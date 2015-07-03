/*******************************************************************************
 *                                                                            
 * Copyright (c) 2012 Texas Instruments Incorporated - http://www.ti.com/      
 *                        ALL RIGHTS RESERVED                                  
 *                                                                            
 ******************************************************************************/

#include <ti_media_common_def.h>
#include <ti_audio.h>
#include <osa_thr.h>
#include <osa_sem.h>
#include <link_api/audioLink.h>
#include <alsa/asoundlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <ti/syslink/utils/Cache.h>

/** < All these macros are specific to tvp5158 */

#define ACAP_BUFFERING_REQUIREMENT        64      /* For some reason, we have to read / feed this much data for playback */


#define ACAP_DRIVER_BUFFER_SIZE           ((8192*2)>>2)
#define ACAP_MIN_FRAMES_2_READ_PER_CH    (256)       //< Ensure to align with hw param BUFFER_SIZE; only < 256 works right now
#define ACAP_SAMPLE_LEN                     2
#define ACAP_BUFFER_SIZE_PER_CH           (ACAP_MIN_FRAMES_2_READ_PER_CH   \
                                                     * ACAP_BUFFERING_REQUIREMENT \
                                              )  
#define ACAP_CAPTURE_BUFFER_LENGTH        (ACAP_BUFFER_SIZE_PER_CH \
                                                     * ACAP_CHANNELS_MAX   \
                                              )

#define ACAP_CAPTURE_TSK_PRI               18
#define ACAP_CAPTURE_TSK_STACK_SIZE       (10*1024)

#define     AUD_DEVICE_PRINT_ERROR_AND_RETURN(str, err, hdl)        \
        fprintf (stderr, "AUDIO >> " str, snd_strerror (err));  \
        snd_pcm_close (hdl);    \
        return  -1;

#define AUDIO_DATA_MAX_PENDING_RECV_SEM_COUNT   1

#define AUDIO_ENCODE_FRAME_SIZE_IN_SAMPLES      1024 

typedef struct  
{
    Int32   errorCnt;
    Int32   lastError;
    UInt32  totalCaptureLen;
} ACAP_STATS_S;

typedef struct
{
    snd_pcm_t       *alsa_handle;
    ACAP_STATS_S    acapStats;
    Int8            audioPhyToDataIndexMap[ACAP_CHANNELS_MAX];
    UInt16          captureBuf[ACAP_CAPTURE_BUFFER_LENGTH  *4];
    Int32           captureLen;
    OSA_ThrHndl     captureThrHandle;
    Bool            taskExitFlag;
    UInt32          captureChannelMask;
    Bool            captureActive;

    UInt16          *chData[ACAP_CHANNELS_MAX];
    Int32           chDataWrIdx[ACAP_CHANNELS_MAX];
    Int32           chDataRdIdx[ACAP_CHANNELS_MAX];
    Int32           chDataMaxSize[ACAP_CHANNELS_MAX];
    OSA_SemHndl     dataNotifySem[ACAP_CHANNELS_MAX];
    OSA_SemHndl     dataConsumedNotifySem[ACAP_CHANNELS_MAX];
    ACAP_PARAMS_S   prm;

} ACAP_CONTEXT_INT_S;

/** Globals */
static ACAP_CONTEXT_INT_S  gAcapContextInt;
static UInt16              *writeBuf[ACAP_CHANNELS_MAX];
static Int32               wrIdx[ACAP_CHANNELS_MAX];
static Int32               bufMaxSize[ACAP_CHANNELS_MAX];
static UInt16              *readBuf[ACAP_CHANNELS_MAX];
static Int32               rdIdx[ACAP_CHANNELS_MAX];
static Void                *encHandle[ACAP_CHANNELS_MAX];
static UInt8               *encBuf[ACAP_CHANNELS_MAX];
static Int32               encBufMaxSize[ACAP_CHANNELS_MAX];
static Int32               totalFramesEncoded[ACAP_CHANNELS_MAX];
static Int32               totalFramesFailed[ACAP_CHANNELS_MAX];

static UInt8               *intEncodeBuffer[ACAP_CHANNELS_MAX];
static Int32               intEncBufSize[ACAP_CHANNELS_MAX];

static  Void *Audio_captureTaskFxn(Void * prm);
static  Int32 Audio_InitCaptureDevice (Int8 *device, Int32 numChannels, UInt32 sampleRate);
static  Int32 Audio_deInitCaptureDevice(Void);
static  Int32 Audio_captureData(UInt16 *buffer, Int32 *numSamples);
static  Void Audio_demux_tvp5158(UInt16 *captureBuf, Int32 captureSize);
static  Void Audio_demux_other(UInt16 *captureBuf, Int32 captureSize);


/************************** TVP5158 CAPTURED AUDIO DATA MAPPING*******************************/
/* Audio data captured from the TVP5158 is interleaved*/
/* TVP5158 Daughter card has following configuration for Audio Input (Hardware pins)
    --------------------------------------------------------------------------------------
    | AIN15 | AIN13 | AIN11 | AIN9  | AIN7 | AIN5 | AIN3 | AIN1 |
    --------------------------------------------------------------------------------------
    | AIN16 | AIN14 | AIN12 | AIN10 | AIN8 | AIN6 | AIN4 | AIN2 |
    --------------------------------------------------------------------------------------
    */

/*
    Channel Mapping for 4-channels audio capture
    <-----------------64bits----------------->
    <-16bits->
    --------------------------------------------
    | S16-0  | S16-1  | S16-2  | S16-3 |
    --------------------------------------------
    | AIN 3   | AIN 0   | AIN 2   | AIN 1  |
    --------------------------------------------
    */

/*
    Channel Mapping for 16-channels audio capture
    <---------------------------------------------------------------------------------256bits------------------------------------------------------------------------------------>
    <-16bits->
    -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    | S16-0  | S16-1  | S16-2  | S16-3 | S16-4  | S16-5  | S16-6  | S16-7 | S16-8  | S16-9  | S16-10 | S16-11 | S16-12 | S16-13 | S16-14  | S16-15 |
    -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    AIN 16  | AIN 1 | AIN 3 | AIN 5 | AIN 7 | AIN 9 | AIN 11 | AIN 13 | AIN 15 | AIN 2 | AIN 4 | AIN 6 | AIN 8 | AIN 10 | AIN 12 | AIN 14 |
    -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    */

static Void    Audio_initTvp5158ChMapping(Int32 numChannels)
{
    if (numChannels == 4)
    {
        gAcapContextInt.audioPhyToDataIndexMap[0]   = 1;
        gAcapContextInt.audioPhyToDataIndexMap[1]   = 3;
        gAcapContextInt.audioPhyToDataIndexMap[2]   = 2;
        gAcapContextInt.audioPhyToDataIndexMap[3]   = 0;
    }
    else
    {    
        gAcapContextInt.audioPhyToDataIndexMap[0]   = 1;
        gAcapContextInt.audioPhyToDataIndexMap[1]   = 9;
        gAcapContextInt.audioPhyToDataIndexMap[2]   = 2;
        gAcapContextInt.audioPhyToDataIndexMap[3]   = 10;

#if (ACAP_CHANNELS_MAX==16)
        gAcapContextInt.audioPhyToDataIndexMap[4]   = 3;
        gAcapContextInt.audioPhyToDataIndexMap[5]   = 11;
        gAcapContextInt.audioPhyToDataIndexMap[6]   = 4;
        gAcapContextInt.audioPhyToDataIndexMap[7]   = 12;

        gAcapContextInt.audioPhyToDataIndexMap[8]   = 5;
        gAcapContextInt.audioPhyToDataIndexMap[9]   = 13;
        gAcapContextInt.audioPhyToDataIndexMap[10]  = 6;
        gAcapContextInt.audioPhyToDataIndexMap[11]  = 14;

        gAcapContextInt.audioPhyToDataIndexMap[12]  = 7;
        gAcapContextInt.audioPhyToDataIndexMap[13]  = 15;
        gAcapContextInt.audioPhyToDataIndexMap[14]  = 8;
        gAcapContextInt.audioPhyToDataIndexMap[15]  = 0;
#endif
    }
}


Int32   Audio_getMinCaptureChBufSize (Void)
{
    return (ACAP_BUFFER_SIZE_PER_CH * 2);   // This is per sample & not in bytes 
}

Int32   Audio_getSampleLenInBytes(Void)
{
    return ACAP_SAMPLE_LEN;
}

Int32   Audio_deInitCapture (Int32 successCh, OSA_SemHndl     sem[])
{
    Int32 i;
    for (i=0; i<successCh; i++)
    {
        OSA_semDelete(&sem[i]);
    }
    return 0;
}

Int32   Audio_deInitEncode (ACAP_PARAMS_S *pPrm)
{
    Int32 ch;
    AENC_CREATE_PARAMS_S *encParam;

    for (ch=0; ch<pPrm->numChannels; ch++)
    {
        encParam = &pPrm->chPrm[ch].encodeParam;

        if (encHandle[ch] != NULL)
        {
            Audio_deleteEncAlgorithm(encHandle[ch]);
            Audio_freeMem(encHandle[ch]);
            encHandle[ch] = NULL;
        }
        if (intEncodeBuffer[ch] != NULL)
        {
            Audio_freeSharedRegionBuf(intEncodeBuffer[ch], intEncBufSize[ch]);
            intEncodeBuffer[ch] = NULL;
        }
    }
    return 0;
}

Int32   Audio_initEncode (ACAP_PARAMS_S *pPrm)
{
    Int32 ch;
    AENC_CREATE_PARAMS_S *encParam;
    Void *handle = NULL;

    for (ch=0; ch<pPrm->numChannels; ch++)
    {
        encParam = &pPrm->chPrm[ch].encodeParam;

        encHandle[ch] = intEncodeBuffer[ch] = NULL;
        encBuf[ch] = (UInt8*)pPrm->chPrm[ch].encodeBuf.dataBuf;
        encBufMaxSize[ch] = pPrm->chPrm[ch].encodeBuf.dataBufSize;
        totalFramesEncoded[ch] = totalFramesFailed[ch] = 0;

        if (pPrm->chPrm[ch].enableEncode)
        {
            encParam->sampleRate = gAcapContextInt.prm.sampleRate;

            switch (encParam->encoderType )
            {
                case AUDIO_CODEC_TYPE_AAC_LC:
                {
                    handle = Audio_allocMem(Audio_getEncoderContextSize(encParam->encoderType));
                    if (handle)
                    {
                        if (Audio_createAacEncAlgorithm(handle, encParam) != NULL)
                        {
                            encHandle[ch] = handle;
                            if (encBufMaxSize[ch] < encParam->minOutBufSize ||
                                    gAcapContextInt.chDataMaxSize[ch] < encParam->minInBufSize)
                            {
                                AUDIO_ERROR_PRINT(("AUDIO: CAPTURE -> enc or capture buffer less for ch %d\n", ch));
                                return -1;
                            }
                        }
                        else
                        {
                            Audio_freeMem(handle);
                            return -1;
                        }
                    }
                }
                break;
            
                case AUDIO_CODEC_TYPE_G711:
                {
                    handle = Audio_allocMem(Audio_getEncoderContextSize(encParam->encoderType));
                    if (handle)
                    {
                        if (Audio_createG711EncAlgorithm(handle) != NULL)
                        {
                            encHandle[ch] = handle;
                        }
                        else
                        {
                            Audio_freeMem(handle);
                            return -1;
                        }
                    }
                }
            }
            intEncBufSize[ch] = encParam->minOutBufSize;
            intEncodeBuffer[ch] = Audio_allocateSharedRegionBuf(intEncBufSize[ch]);
            if ((encHandle[ch] == NULL) || (intEncodeBuffer[ch] == NULL))
            {
                if (intEncodeBuffer[ch])
                    Audio_freeSharedRegionBuf(intEncodeBuffer[ch], intEncBufSize[ch]);
                AUDIO_ERROR_PRINT(("AUDIO: CAPTURE -> Encoder creation failed for CH - %d\n", ch));
                return -1;
            }
            else
            {
                AUDIO_ERROR_PRINT(("AUDIO: CAPTURE -> Encoder creation SUCCESS for CH - %d\n", ch));
            }
        }
    }
    return 0;
}

Int32   Audio_startCapture(ACAP_PARAMS_S *pPrm)
{
    Int32 status = ERROR_FAIL, ch;

    memset(&gAcapContextInt, 0, sizeof(ACAP_CONTEXT_INT_S));
    gAcapContextInt.alsa_handle   = NULL;
    gAcapContextInt.taskExitFlag  = FALSE;
    gAcapContextInt.prm           = *pPrm;

    if (pPrm->enableTVP5158 == TRUE)
    {
        Audio_initTvp5158ChMapping(pPrm->numChannels);
    }
    
    for (ch=0; ch<pPrm->numChannels; ch++)
    {
        gAcapContextInt.chData[ch] = (UInt16*) pPrm->chPrm[ch].captureBuf.dataBuf;
        gAcapContextInt.chDataRdIdx[ch] = 0;
        gAcapContextInt.chDataWrIdx[ch] = 0;
        gAcapContextInt.chDataMaxSize[ch] = pPrm->chPrm[ch].captureBuf.dataBufSize;

        /** Capture task uses double buffering. 1 buffer is available always for capture to store ch data 
                      Once this buffer is full, its made available to application while capture switches to the other buffer for storage
                 */

        writeBuf[ch] = gAcapContextInt.chData[ch];
        wrIdx[ch] = 0;
        bufMaxSize[ch] = gAcapContextInt.chDataMaxSize[ch] >> 1;

        readBuf[ch] = writeBuf[ch] + bufMaxSize[ch];
        rdIdx[ch] = 0;
        AUDIO_INFO_PRINT (("chData Ptr %X, Size %d, wrBuf %X, rdBuf %X, encBuf %X, encBufSize %d\n", 
                (UInt32)gAcapContextInt.chData[ch], gAcapContextInt.chDataMaxSize[ch],
                (UInt32)writeBuf[ch], (UInt32)readBuf[ch],
                (UInt32)pPrm->chPrm[ch].encodeBuf.dataBuf, pPrm->chPrm[ch].encodeBuf.dataBufSize));

        status = OSA_semCreate(&gAcapContextInt.dataNotifySem[ch],
                        AUDIO_DATA_MAX_PENDING_RECV_SEM_COUNT, 
                        0);
        if (status != OSA_SOK)
        {
            Audio_deInitCapture(ch, gAcapContextInt.dataNotifySem);
            return status;
        }
        status = OSA_semCreate(&gAcapContextInt.dataConsumedNotifySem[ch],
                        AUDIO_DATA_MAX_PENDING_RECV_SEM_COUNT, 
                        1);
        if (status != OSA_SOK)
        {
            Audio_deInitCapture(ch, gAcapContextInt.dataNotifySem);
            Audio_deInitCapture(ch, gAcapContextInt.dataConsumedNotifySem);
            return status;
        }
    }
    
    if (Audio_initEncode(pPrm) < 0)
    {
        Audio_deInitCapture(ch, gAcapContextInt.dataNotifySem);
        Audio_deInitCapture(ch, gAcapContextInt.dataConsumedNotifySem);
        Audio_deInitEncode(pPrm);
        return ERROR_FAIL;
    }

    if (status == OSA_SOK)
    {
        status = OSA_thrCreate(&gAcapContextInt.captureThrHandle,
                      Audio_captureTaskFxn,
                      ACAP_CAPTURE_TSK_PRI, 
                      ACAP_CAPTURE_TSK_STACK_SIZE, 
                      &gAcapContextInt);
    }
    AUDIO_INFO_PRINT(("AUDIO: Audio_startCapture() success....\n\n"));
    return  status;
}

Int32   Audio_stopCapture(Void)
{
    /* Only 5158 config supported now */
    if (gAcapContextInt.captureActive == TRUE)
    {
        Int32 ch;

        gAcapContextInt.taskExitFlag = TRUE;

        OSA_waitMsecs(100);
        OSA_thrDelete(&gAcapContextInt.captureThrHandle);
        Audio_deInitEncode(&gAcapContextInt.prm);

        /* signal semaphores to unlock waiting app threads */
        for (ch=0; ch<gAcapContextInt.prm.numChannels; ch++)
            OSA_semSignal(&gAcapContextInt.dataNotifySem[ch]);
        OSA_waitMsecs(100);

        Audio_deInitCapture(gAcapContextInt.prm.numChannels, gAcapContextInt.dataConsumedNotifySem);
        Audio_deInitCapture(gAcapContextInt.prm.numChannels, gAcapContextInt.dataNotifySem);
        Audio_deInitCaptureDevice();
        gAcapContextInt.captureActive = FALSE;
    }
    return 0;
}

Void Audio_enableCapChannel(Int32 chNum)
{
    gAcapContextInt.captureChannelMask |= (1 << chNum);
}

Void Audio_disableCapChannel(Int32 chNum)
{
    gAcapContextInt.captureChannelMask &= ~(1 << chNum);
}

static
Void *Audio_captureTaskFxn(Void * prm)
{
    ACAP_CONTEXT_INT_S  *ctx;
    UInt16               *captureBuf;
    Int32               len, err, buffering;

    ctx = prm;

    ctx->captureActive = TRUE;

    while (!ctx->captureChannelMask)
    {
        usleep(1000);
        if (ctx->taskExitFlag == TRUE)
            break;
    }

    AUDIO_ERROR_PRINT(("\n\nAUDIO: STARTING AUDIO CAPTURE!!!!!\n"));

    if (Audio_InitCaptureDevice(gAcapContextInt.prm.captureDevice, gAcapContextInt.prm.numChannels, gAcapContextInt.prm.sampleRate) != ERROR_NONE)
    {
        AUDIO_ERROR_PRINT(("\n\nAUDIO: Capture -> device init failed....- exiting!!!!!\n"));
        return NULL;
    }

    while (ctx->taskExitFlag == FALSE)
    {
        buffering = 0;
        ctx->captureLen = 0;
        captureBuf = ctx->captureBuf;
        while (buffering < ACAP_BUFFERING_REQUIREMENT)
        {
            if (ctx->taskExitFlag == TRUE)
                break;

            len = ACAP_MIN_FRAMES_2_READ_PER_CH;
            err = Audio_captureData(captureBuf, &len);
            if (len > 0)
            {
                ctx->acapStats.totalCaptureLen += len;
                ctx->captureLen += len;
                captureBuf += (len * ctx->prm.numChannels);

            }
            else
            {
                snd_pcm_prepare(gAcapContextInt.alsa_handle);
            }
            if (!(ctx->acapStats.totalCaptureLen % (16000)))
                AUDIO_INFO_PRINT(("%d: AUDIO: CAPTURE -> %d samples captured.. Err Ct %d, last Err %d <%s> / wr-%X: %d\n", 
                    OSA_getCurTimeInMsec(),
                    ctx->acapStats.totalCaptureLen, 
                    ctx->acapStats.errorCnt,
                    ctx->acapStats.lastError,
                    snd_strerror(ctx->acapStats.lastError),
                    (UInt32)&ctx->chDataWrIdx[0], ctx->chDataWrIdx[0]
                    ));
            buffering++;
        }
        if ((ctx->captureLen) && (ctx->taskExitFlag == FALSE))
        {
            if (gAcapContextInt.prm.enableTVP5158 == TRUE)
                Audio_demux_tvp5158(ctx->captureBuf, ctx->captureLen);
            else
                Audio_demux_other(ctx->captureBuf, ctx->captureLen);
        }
    }
    AUDIO_ERROR_PRINT(("\n\nAUDIO: ENDING AUDIO CAPTURE!!!!!\n"));
    return NULL;
}

static 
Int32 Audio_InitCaptureDevice (Int8 *device, Int32 numChannels, UInt32 sampleRate)
{
    snd_pcm_hw_params_t *hw_params;
    Int32 err;
    snd_pcm_t       *alsa_handle;
    snd_pcm_uframes_t bufferSizeMax;

    if ((err = snd_pcm_open (&alsa_handle, device, SND_PCM_STREAM_CAPTURE, 0)) < 0)
    {
        AUDIO_ERROR_PRINT(("\n\nAUDIO >> Cannot open audio device %s (%s)\n", device, snd_strerror (err)));
        return  ERROR_FAIL;
    }

    if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0)
    {
        AUD_DEVICE_PRINT_ERROR_AND_RETURN("\n\nAUDIO >>  cannot allocate hardware parameter structure (%s)\n", err, alsa_handle);
    }

    if ((err = snd_pcm_hw_params_any (alsa_handle, hw_params)) < 0)
    {
        AUD_DEVICE_PRINT_ERROR_AND_RETURN("\n\nAUDIO >>  cannot initialize hardware parameter structure (%s)\n", err, alsa_handle);
    }

    if ((err = snd_pcm_hw_params_set_access (alsa_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0)
    {
        AUD_DEVICE_PRINT_ERROR_AND_RETURN("\n\nAUDIO >>  cannot set access type (%s)\n", err, alsa_handle);
    }

    if ((err = snd_pcm_hw_params_set_format (alsa_handle, hw_params, SND_PCM_FORMAT_S16_LE)) < 0)
    {
        AUD_DEVICE_PRINT_ERROR_AND_RETURN("\n\nAUDIO >>  cannot set sample format (%s)\n", err, alsa_handle);
    }

    if ((err = snd_pcm_hw_params_set_rate_near (alsa_handle, hw_params, &sampleRate, 0)) < 0)
    {
        AUD_DEVICE_PRINT_ERROR_AND_RETURN("\n\nAUDIO >>  cannot set sample rate (%s)\n", err, alsa_handle);
    }

    if ((err = snd_pcm_hw_params_set_channels (alsa_handle, hw_params, numChannels)) < 0)
    {
        AUD_DEVICE_PRINT_ERROR_AND_RETURN("\n\nAUDIO >>  cannot set channel count (%s)\n", err, alsa_handle);
    }

    if ((err = snd_pcm_hw_params_set_buffer_size_last	(alsa_handle, hw_params, &bufferSizeMax)) >= 0)
    {
        AUDIO_INFO_PRINT(("******* ALSA : Max buffer size ==> %lu\n", bufferSizeMax));
    }
    else
    {
        bufferSizeMax = ACAP_DRIVER_BUFFER_SIZE;
    }

    if ((err = snd_pcm_hw_params_set_buffer_size (alsa_handle, hw_params, bufferSizeMax)) < 0)
    {
        AUD_DEVICE_PRINT_ERROR_AND_RETURN("\n\nAUDIO >>  cannot set buffer size (%s)\n", err, alsa_handle);
    }

    if ((err = snd_pcm_hw_params (alsa_handle, hw_params)) < 0)
    {
        AUD_DEVICE_PRINT_ERROR_AND_RETURN("\n\nAUDIO >>  cannot set parameters (%s)\n", err, alsa_handle);
    }

    snd_pcm_hw_params_free (hw_params);

    if ((err = snd_pcm_prepare (alsa_handle)) < 0)
    {
        AUD_DEVICE_PRINT_ERROR_AND_RETURN("\n\nAUDIO >>  cannot prepare audio interface for use (%s)\n", err, alsa_handle);
    }
    gAcapContextInt.alsa_handle = alsa_handle;
    AUDIO_ERROR_PRINT(("\n\nAUDIO: AUDIO CAPTURE DEVICE Init Done!!!!!\n"));
    sleep(1);
    return err;
}


static 
Int32    Audio_captureData(UInt16 *buffer, Int32 *numSamples)
{
    Int32 err = -1;

    if (gAcapContextInt.alsa_handle)
    {
        if ((err = snd_pcm_readi (gAcapContextInt.alsa_handle, buffer, *numSamples)) != *numSamples)
        {
//            AUDIO_INFO_PRINT ((" AUDIO >> read from audio interface failed (%s), buffer %X, capBuf %X, already Captured - %d\n", 
//                snd_strerror (err), (UInt32) buffer, (UInt32) gAcapContextInt.captureBuf, gAcapContextInt.acapStats.totalCaptureLen));
            *numSamples = 0;
            gAcapContextInt.acapStats.errorCnt++;
            gAcapContextInt.acapStats.lastError = err;

        }
    }
    else
    {
        *numSamples = 0;
    }
    return err;
}


static 
Int32 Audio_deInitCaptureDevice(Void)
{

    if (gAcapContextInt.alsa_handle)
    {
        snd_pcm_drain(gAcapContextInt.alsa_handle);
        snd_pcm_close(gAcapContextInt.alsa_handle);
        gAcapContextInt.alsa_handle = NULL;
        AUDIO_ERROR_PRINT(("AUDIO: Capture device deInit done....\n"));
    }
    AUDIO_ERROR_PRINT(("\n\nAUDIO: AUDIO CAPTURE DEVICE De_Init Done!!!!!\n"));
    return 0;
}

Int32 Audio_doEncode (UInt8 chNum,ACAP_GET_DATA_PARAMS_S * pPrm)
{
    Int32   bytesGenerated = 0, iter = 0;
    Int32 remainingSamples = 0;

    if (gAcapContextInt.prm.chPrm[chNum].encodeParam.encoderType == AUDIO_CODEC_TYPE_AAC_LC)
    {
        Int32 outBufAvailable;
        AENC_PROCESS_PARAMS_S     prm;
        UInt16                  *input;
        UInt8                   *output;

        iter = bufMaxSize[chNum] / AUDIO_ENCODE_FRAME_SIZE_IN_SAMPLES;
        // TODO: to be handled!!!!    
        remainingSamples = bufMaxSize[chNum] % AUDIO_ENCODE_FRAME_SIZE_IN_SAMPLES;
        
        input = readBuf[chNum];
        output = encBuf[chNum];
        outBufAvailable = encBufMaxSize[chNum];

        /* Assuming that encode buffer will be sufficient */
        while (iter)
        {
            prm.inBuf.dataBuf = input; 
            prm.inBuf.dataBufSize = (AUDIO_ENCODE_FRAME_SIZE_IN_SAMPLES*ACAP_SAMPLE_LEN);
            
            Cache_wbInv(prm.inBuf.dataBuf,
                        prm.inBuf.dataBufSize, Cache_Type_ALL, TRUE);

            prm.outBuf.dataBuf = intEncodeBuffer[chNum];
            prm.outBuf.dataBufSize = intEncBufSize[chNum];
            Audio_encode(encHandle[chNum], &prm);
            /** ?????? ****/
            if (prm.outBuf.dataBufSize == 0)
            {
                AUDIO_ERROR_PRINT (("AUDIO: ENC -> Encode failed [Enc:%d/ Failed: %d] - input %X <%X %X>, output %d, iter %d\n", 
                        totalFramesEncoded[chNum], totalFramesFailed[chNum],
                        (UInt32)input, (UInt32)writeBuf[chNum], (UInt32)readBuf[chNum], prm.outBuf.dataBufSize, iter));
                totalFramesFailed[chNum] ++;
            }
            else
            {
                totalFramesEncoded[chNum] ++;
                if (outBufAvailable >= prm.outBuf.dataBufSize)
                {
                    memcpy(output, prm.outBuf.dataBuf, prm.outBuf.dataBufSize); 
                    output += prm.outBuf.dataBufSize;
                    outBufAvailable -= prm.outBuf.dataBufSize;
                    bytesGenerated += prm.outBuf.dataBufSize;
                }
                else
                {
                    break;
                }
            }
            input += AUDIO_ENCODE_FRAME_SIZE_IN_SAMPLES;
            iter--;
        }
        if (0)
        {
            static FILE *ch1 = NULL;
        
            if (!ch1)
                ch1 = fopen("capture.pcm", "wb");
            if (ch1)
            {
                fwrite(readBuf[chNum], 1, (bufMaxSize[chNum] * ACAP_SAMPLE_LEN), ch1);
            }
        }

    }
    pPrm->encodeDataBuf = encBuf[chNum];
    pPrm->encodeDataSize = bytesGenerated;
    return 0;
}

Int32 Audio_getCapChData(UInt8 chNum,ACAP_GET_DATA_PARAMS_S * pPrm)
{
    if ((gAcapContextInt.taskExitFlag == FALSE) && (gAcapContextInt.captureChannelMask & (1 << chNum)))
    {
        OSA_semWait(&gAcapContextInt.dataNotifySem[chNum], OSA_TIMEOUT_FOREVER);

        if (gAcapContextInt.taskExitFlag == TRUE)
            goto AUDIO_FAILURE;

        if (gAcapContextInt.prm.chPrm[chNum].enableEncode)
        {
            Audio_doEncode(chNum, pPrm);
        }
        else
        {
            pPrm->captureDataBuf = readBuf[chNum];
            pPrm->captureDataSize = bufMaxSize[chNum];
        }
//        OSA_semSignal(&gAcapContextInt.dataConsumedNotifySem[chNum]);
        return 0;
    }
AUDIO_FAILURE:
    pPrm->captureDataBuf = NULL;
    pPrm->captureDataSize = 0;
    pPrm->encodeDataBuf = NULL;
    pPrm->encodeDataSize = 0;
    return -1;
}

Int32 Audio_setCapConsumedChData(UInt8 chNum,
                           UInt32 captureDataSizeConsumed,
                           UInt32 encodeDataSizeConsumed
                                 )
{
    return 0;
}

Void Audio_demux_tvp5158(UInt16 *captureBuf, Int32 captureSize)
{
    Int32 stored, ch, samplesCaptured = 0;
    stored = 0;
        
    while (stored < captureSize)
    {
        for (ch=0; ch<ACAP_CHANNELS_MAX; ch++)
        {
            if (gAcapContextInt.captureChannelMask & (1 << ch))
            {
//                OSA_semWait(&gAcapContextInt.dataConsumedNotifySem[ch], OSA_TIMEOUT_FOREVER);
                *(writeBuf[ch] + wrIdx[ch]) = 
                    *(captureBuf + gAcapContextInt.audioPhyToDataIndexMap[ch]);

                wrIdx[ch] ++;
                if (wrIdx[ch] >= bufMaxSize[ch])
                {
                    UInt16 *tmp;

                    /* Swap buffers */
                    tmp = writeBuf[ch];
                    writeBuf[ch] = readBuf[ch];                       
                    wrIdx[ch] = 0;
                    readBuf[ch] = tmp;
                }
                OSA_semSignal(&gAcapContextInt.dataNotifySem[ch]);
            }
            samplesCaptured++;
        }
        captureBuf += ACAP_CHANNELS_MAX;
        stored ++;
    }
}

Void Audio_demux_other(UInt16 *captureBuf, Int32 captureSize)
{
    Int32 stored, ch, samplesCaptured = 0;
    stored = 0;
        
    while (stored < captureSize)
    {
        for (ch=0; ch<ACAP_CHANNELS_MAX; ch++)
        {
            if (gAcapContextInt.captureChannelMask & (1 << ch))
            {
//                OSA_semWait(&gAcapContextInt.dataConsumedNotifySem[ch], OSA_TIMEOUT_FOREVER);
                *(writeBuf[ch] + wrIdx[ch]) = 
                    *(captureBuf + ch);

                wrIdx[ch] ++;
                if (wrIdx[ch] >= bufMaxSize[ch])
                {
                    UInt16 *tmp;

                    /* Swap buffers */
                    tmp = writeBuf[ch];
                    writeBuf[ch] = readBuf[ch];                       
                    wrIdx[ch] = 0;
                    readBuf[ch] = tmp;
                }
                OSA_semSignal(&gAcapContextInt.dataNotifySem[ch]);
            }
            samplesCaptured++;
        }
        captureBuf += ACAP_CHANNELS_MAX;
        stored ++;
    }
}


