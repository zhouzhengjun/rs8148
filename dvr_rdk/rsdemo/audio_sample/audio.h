/*******************************************************************************
 *                                                                             *
 * Copyright (c) 2009 Texas Instruments Incorporated - http://www.ti.com/      *
 *                        ALL RIGHTS RESERVED                                  *
 *                                                                             *
 ******************************************************************************/

#ifndef	_AUDIO_H_
#define	_AUDIO_H_

#include <osa.h>


#ifdef TI_814X_BUILD
#define	AUDIO_MAX_CHANNELS			4	///< Max number of audio channel supported for recording / playback
#else
#define	AUDIO_MAX_CHANNELS			16	///< Max number of audio channel supported for recording / playback
#endif

#define AUDIO_STATUS_OK      0  ///< Status : OK
#define AUDIO_STATUS_EFAIL   -1  ///< Status : Generic error

#define AUDIO_SAMPLE_RATE_DEFAULT                         16000
#define AUDIO_VOLUME_DEFAULT							5

// #define	USE_DEFAULT_STORAGE_PATH
#define AUDFRM_MAX_FRAME_PTR                             (8)

typedef struct AudFrm_Buf {
    Int8 *audBuf;
    Int32 len;
    UInt32  muteFlag;
    UInt64 timestamp;
} AudFrm_Buf;

typedef struct AudFrm_BufList {
    UInt32 numFrames;
    AudFrm_Buf * frames[AUDFRM_MAX_FRAME_PTR];
} AudFrm_BufList;

Int32 Audio_captureIsStart();


/**
    \brief Create Audio capture module

    - Creates Audio capture task
    - Creates storage directories if not existing

    \return AUDIO_STATUS_OK on success
*/
Int32 Audio_captureCreate (Void);

/**
    \brief Start Audio capture

    - Start audio capture / file write

    \return AUDIO_STATUS_OK on success
*/
Int32 Audio_captureStart (Int8 chNum);

/**
    \brief Stop Audio capture

    - Stop audio capture / file write

    \return AUDIO_STATUS_OK on success
*/
Int32 Audio_captureStop (Void);

/**
    \brief Delete Audio capture

    - Deinit audio capture, delete task

    \return AUDIO_STATUS_OK on success
*/
Int32 Audio_captureDelete (Void);

/**
    \brief Get Audio Sampling Rate to Default sampling Rate

    \return AUDIO_STATUS_OK on success
*/
Int32 Audio_playGetSamplingFreq (Uint32 *freq, Int32 *vol);
/**
    \brief Set Audio Sampling Rate to Default sampling Rate

    \return AUDIO_STATUS_OK on success
*/
Int32 Audio_playSetSamplingFreq (Uint32 freq, Uint32 vol);

/**
    \brief Create Audio playback module

    - Creates Audio playback task

    \return AUDIO_STATUS_OK on success
*/
Int32 Audio_playCreate (Void);

/**
    \brief Start Audio playback

    - Start audio file read / playback
    - playbackDevId selects the playback device
    - 0 for AIC3x playback, 1 for HDMI audio playback

    \return AUDIO_STATUS_OK on success
*/
Int32 Audio_playStart (Int8 chNum, UInt32 playbackDevId);

/**
    \brief Stop Audio playback

    - Stop audio playback

    \return AUDIO_STATUS_OK on success
*/
Int32 Audio_playStop (Void);

/**
    \brief Delete Audio playback

    - Deinit audio playback, delete task

    \return AUDIO_STATUS_OK on success
*/
Int32 Audio_playDelete (Void);

/**
    \brief Set storage for audio files
    - Tries to create directory if not existing, if there is any issue with new path defaults to old
    - Tries to a default path if not set
    \return AUDIO_STATUS_OK on success
*/
Int32 Audio_setStoragePath (Int8 *path);

/**
    \brief Prints capture statistics
    \return AUDIO_STATUS_OK on success
*/
Int32 Audio_capturePrintStats (Void);

/**
    \brief Prints Audio capture Parameters
    \return AUDIO_STATUS_OK on success
*/
Int32 Audio_pramsPrint();

/**
    \brief Prints playback statistics
    \return AUDIO_STATUS_OK on success
*/
Int32 Audio_playPrintStats (Void);

#endif	/* _AUDIO_H_ */
