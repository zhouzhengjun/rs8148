/*******************************************************************************
 *                                                                             *
 * Copyright (c) 2009 Texas Instruments Incorporated - http://www.ti.com/      *
 *                        ALL RIGHTS RESERVED                                  *
 *                                                                             *
 ******************************************************************************/

/**
    \ingroup LINK_API
    \defgroup Audio API

    @{
*/

/**
    \file audio.h
    \brief Audio Encode, Decode APIs using RPE
*/

#ifndef __AUDIO_H__
#define __AUDIO_H__

#ifdef  __cplusplus
extern "C" {
#endif

#include <mcfw/interfaces/link_api/system_debug.h>

#define     AUDIO_ENABLE_INFO_PRINT
#define     AUDIO_ENABLE_ERROR_PRINT

#ifdef      AUDIO_ENABLE_INFO_PRINT
#define     AUDIO_INFO_PRINT(X)    printf X
#else
#define     AUDIO_INFO_PRINT(X)    
#endif

#ifdef      AUDIO_ENABLE_ERROR_PRINT
#define     AUDIO_ERROR_PRINT(X)    printf X
#else
#define     AUDIO_ERROR_PRINT(X)    
#endif


/**
 *  \brief    System initialization function.
 *              Does IPC/SysLink initialization between cores.
 *
 *  return     void     None
 */
Void Audio_systemProcInit (Void);

/**
 *  \brief      System de-initialization function.
 *              Does IPC/SysLink finalization.
 *
 *  \return     void     None
 */
void Audio_systemProcDeInit (Void);


/**
    \brief     Return size of encode context structure. 

    \param  bitrate                         [IN]   Type of encoder
    
    \return   Size of encode context struct
*/
Int32   Audio_getEncoderContextSize (Int32 codecType);


/**
    \brief     Create AAC Encoder Algorithm
    
    \param  ctxMem                       [IN]   Memory to store internal data structure
    \param  pPrm                          [IN,OUT]   bitrate, samplerate, no. of channels of stream to be encoded.
                                                                    returns min input / output buffer requirement

    \return   Encoder handle on success
*/
Void* Audio_createAacEncAlgorithm(Void *ctxMem, AENC_CREATE_PARAMS_S *pPrm);


/**
    \brief     Create G711 Encoder Algorithm
    
    \param  ctxMem                       [IN]   Memory to store internal data structure

    \return   Encoder handle on success
*/
Void *Audio_createG711EncAlgorithm(Void *ctxMem);


/**
    \brief     Return size of decode context structure. 

    \param  bitrate                         [IN]   Type of decoder
    
    \return   Size of decode context struct
*/
Int32  Audio_getDecoderContextSize (Int32 codecType);

/**
    \brief     Create AAC Decoder Algorithm
    
    \param  ctxMem                         [IN]   Memory to store internal data structure
    \param  pPrm                             [IN,OUT]   Channel Mode, returns min input / output buffer requirement

    \return   Decoder handle on success
*/
Void *Audio_createAacDecAlgorithm(Void *ctxMem, ADEC_CREATE_PARAMS_S *pPrm);


/**
    \brief     Create G711 Decoder Algorithm
    
    \param  ctxMem                       [IN]   Memory to store internal data structure

    \return   Decoder handle on success
*/
Void *Audio_createG711DecAlgorithm(Void *ctxMem);

/**
    \brief     Delete Encoder Algorithm
    
    \param  ctxMem                       [IN]   Memory with internal data structure

    \return   0 on success
*/
Int32 Audio_deleteEncAlgorithm(Void *ctxMem);



/**
    \brief     Delete  Decoder Algorithm
    
    \param  ctxMem                       [IN]   Memory with internal data structure

    \return   0 on success
*/
Int32 Audio_deleteDecAlgorithm(Void *ctxMem);

/**
    \brief     Encode process
    
    \param  ctxMem                       [IN]   Memory with internal data structure
    \param  pPrm                           [IN,OUT]   Encode parameters

    \return   0 on success
*/
Int32 Audio_encode(Void* ctxMem, AENC_PROCESS_PARAMS_S *pPrm);

/**
    \brief     Decode process
    
    \param  ctxMem                       [IN]   Memory with internal data structure
    \param  pPrm                           [IN,OUT]   Decode parameters

    \return   0 on success
*/
Int32 Audio_decode(Void* ctxMem, ADEC_PROCESS_PARAMS_S *pPrm);

/**
    \brief     Helper functions 
 */
Void* Audio_allocMem (Int32 _size);

Void Audio_freeMem (Void *buf);

/**
    \brief     Start capture / demux of audio samples
 */

Int32 Audio_startCapture (ACAP_PARAMS_S *pPrm);

/**
    \brief     Min Buffer Size <Samples> for each channel. App should provide this buffer size for each ch
 */
Int32   Audio_getMinCaptureChBufSize (Void);

/**
    \brief    Length of sample in bytes
 */
Int32   Audio_getSampleLenInBytes(Void);

/**
    \brief     Enable capture 
 */
Void Audio_enableCapChannel(Int32 chNum);


/**
    \brief     Disable capture
 */
Void Audio_disableCapChannel(Int32 chNum);

/**
    \brief     Stop capture task
 */
Int32 Audio_stopCapture (Void);


/**
    \brief     Get audio channel data
 */
Int32 Audio_getCapChData (UInt8 chNum, ACAP_GET_DATA_PARAMS_S *pPrm);


/**
    \brief     Set data consumed info 
 */
Int32 Audio_setCapConsumedChData(UInt8 chNum,
                           UInt32 captureDataSizeConsumed,
                           UInt32 encodeDataSizeConsumed
                                 );

#ifdef  __cplusplus
}
#endif
#endif /* __AUDIO_H__ */

