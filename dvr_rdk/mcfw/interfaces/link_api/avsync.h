/*******************************************************************************
 *                                                                             *
 * Copyright (c) 2009 Texas Instruments Incorporated - http://www.ti.com/      *
 *                        ALL RIGHTS RESERVED                                  *
 *                                                                             *
 ******************************************************************************/

/**
    \ingroup AvSync library
*/

/**
    \file avsync.h
    \brief main data structures
*/

#ifndef _AVSYNC_H_
#define _AVSYNC_H_

#include <mcfw/interfaces/link_api/system_linkId.h>
#include <mcfw/interfaces/link_api/swMsLink.h>


#define AVSYNC_S_OK                                                         (0)
#define AVSYNC_E_FAIL                                                       (-1)
#define AVSYNC_E_INSUFFICIENTRESOURCES                                      (-2)
#define AVSYNC_E_INVALIDPARAMS                                              (-3)

#define AVSYNC_MAX_NUM_AUDIO_PLAYOUT_DEVICES                                      (2)
#define AVSYNC_MAX_NUM_DISPLAYS                                                   (SYSTEM_LINK_ID_DISPLAY_COUNT)
#define AVSYNC_MAX_CHANNELS_PER_DISPLAY                                           (SYSTEM_SW_MS_MAX_CH_ID)
#define AVSYNC_ALL_CHANNEL_ID                                                     (AVSYNC_MAX_CHANNELS_PER_DISPLAY + 1)

#define AVSYNC_VIDEO_PLAY_MAX_LAG_MS                                              (45)
#define AVSYNC_VIDEO_PLAY_MAX_LEAD_MS                                             (0)
#define AVSYNC_VIDEO_FUTURE_FRAME_DROP_THRESHOLD_MS                               (750)

#define AVSYNC_AUDIO_REFCLKADJUST_MAX_LEAD_MS                                     (45)
#define AVSYNC_AUDIO_REFCLKADJUST_MAX_LAG_MS                                      (45)

#define AVSYNC_VIDEO_REFCLKADJUST_MAX_LEAD_MS                                     (120)
#define AVSYNC_VIDEO_REFCLKADJUST_MAX_LAG_MS                                      (120)

#define AVSYNC_VIDEO_TIMEBASESHIFT_MAX_LEAD_MS                                    (3000)
#define AVSYNC_VIDEO_TIMEBASESHIFT_MAX_LAG_MS                                     (3000)

#define AVSYNC_VIDEO_BACKEND_DELAY_MS                                             (50)

#define AVSYNC_TIMESCALE_NORMAL_PLAY                                              (1000)

#define AVSYNC_LINK_CMD_INIT                                                      (0xA000)
#define AVSYNC_LINK_CMD_DEINIT                                                    (0xA001)
#define AVSYNC_LINK_CMD_PAUSE                                                     (0xA002)
#define AVSYNC_LINK_CMD_UNPAUSE                                                   (0xA003)
#define AVSYNC_LINK_CMD_TIMESCALE                                                 (0xA004)
#define AVSYNC_LINK_CMD_GETSROBJ                                                  (0xA005)
#define AVSYNC_LINK_CMD_AVSYNCCFG                                                 (0xA006)
#define AVSYNC_LINK_CMD_GETVIDSYNCHCHINFO                                         (0xA007)
#define AVSYNC_LINK_CMD_STEP_FWD                                                  (0xA008)
#define AVSYNC_LINK_CMD_SET_FIRST_VIDPTS                                          (0xA009)
#define AVSYNC_LINK_CMD_SET_FIRST_AUDPTS                                          (0xA00A)
#define AVSYNC_LINK_CMD_RESET_PLAYERTIME                                          (0xA00B)
#define AVSYNC_LINK_CMD_SET_VIDEO_BACKEND_DELAY                                   (0xA00C)
#define AVSYNC_LINK_CMD_PLAY                                                      (0xA00D)
#define AVSYNC_LINK_CMD_SEEK                                                      (0xA00E)
#define AVSYNC_LINK_CMD_SCAN                                                      (0xA00F)

#define AVSYNC_MAX_STC_VALUE                                                ((UInt64)(0xFFFFFFFFULL))
#define AVSYNC_INVALID_CHNUM                                                (~(0u))
#define AVSYNC_INVALID_PTS                                                  ((UInt64)(~(0ULL)))
#define AVSYNC_INVALID_AUDDEVID                                             (~(0u))
#define AVSYNC_INVALID_DISPLAY_ID                                              (~(0u))


typedef enum
{
    AVSYNC_REFCLKADJUST_BYAUDIO = 0,
    AVSYNC_REFCLKADJUST_BYVIDEO = 1,
    AVSYNC_REFCLKADJUST_NONE    = 2,
    AVSYNC_REFCLKADJUST_LAST = 0x7FFFFFFF
}Avsync_RefClkMasterType;

typedef enum Avsync_PlaybackStartMode
{
    AVSYNC_PLAYBACK_START_MODE_WAITSYNCH,
    AVSYNC_PLAYBACK_START_MODE_DROPUNSYNCH,
    AVSYNC_PLAYBACK_START_MODE_PLAYUNSYNCH,
    AVSYNC_PLAYBACK_START_MODE_LAST = 0x80000000
} Avsync_PlaybackStartMode;

typedef enum Avsync_PTSInitMode
{
    AVSYNC_PTS_INIT_MODE_AUTO,
    AVSYNC_PTS_INIT_MODE_APP,
    AVSYNC_PTS_INIT_MODE_LAST = 0x80000000
} Avsync_PTSInitMode;

/**
    \brief Avsync AVSYNC_LINK_CMD_PAUSE params
*/

typedef struct Avsync_PauseParams
{
    UInt32 displayID;
    /*< displayID : 0 - (AVSYNC_MAX_NUM_DISPLAYS - 1) */

    UInt32 chNum;
    /*< chNum: 0 - (AVSYNC_MAX_CHANNELS_PER_DISPLAY - 1) */

}Avsync_PauseParams;

typedef struct Avsync_PauseParams Avsync_UnPauseParams;

typedef struct Avsync_TimeScaleParams
{
    UInt32 displayID;
    /*< displayID : 0 - (AVSYNC_MAX_NUM_DISPLAYS - 1) */

    UInt32 chNum;
    /*< chNum: 0 - (AVSYNC_MAX_CHANNELS_PER_DISPLAY - 1) */

    UInt32 timeScaleX1000;
    /*< Timescale for playback.
     *  1000 indicates 1X playback. - Normal playback
     *  2000 indicates 2X playback  - Fast play 2X
     *  0500 indicates 0.5x playback - Slow play 0.5x
     */
    UInt32 displaySeqId;
    /*< Sequence Id which will be associated with all frames
     *  received in play state.
     *  Application should set it to SYSTEM_DISPLAY_SEQID_DEFAULT
     *  if it doesnt want to change displaySeqId
     */
} Avsync_TimeScaleParams;

typedef struct Avsync_StepFwdParams
{
    UInt32 displayID;
    /*< displayID : 0 - (AVSYNC_MAX_NUM_DISPLAYS - 1) */

    UInt32 chNum;
    /*< chNum: 0 - (AVSYNC_MAX_CHANNELS_PER_DISPLAY - 1) */

} Avsync_StepFwdParams;

typedef struct Avsync_FirstVidPTSParams
{
    UInt32 displayID;
    /*< displayID : 0 - (AVSYNC_MAX_NUM_DISPLAYS - 1) */

    UInt32 chNum;
    /*< chNum: 0 - (AVSYNC_MAX_CHANNELS_PER_DISPLAY - 1) */

    UInt64 firstVidPTS;
    /*< Presentation timestamp of first video frame in sequence */
} Avsync_FirstVidPTSParams;

typedef struct Avsync_FirstAudPTSParams
{
    UInt32 displayID;
    /*< displayID : 0 - (AVSYNC_MAX_NUM_DISPLAYS - 1) */

    UInt32 chNum;
    /*< chNum: 0 - (AVSYNC_MAX_CHANNELS_PER_DISPLAY - 1) */

    UInt64 firstAudPTS;
    /*< Presentation timestamp of first video frame in sequence */
} Avsync_FirstAudPTSParams;

typedef struct Avsync_ResetPlayerTimerParams
{
    UInt32 displayID;
    /*< displayID : 0 - (AVSYNC_MAX_NUM_DISPLAYS - 1) */

    UInt32 chNum;
    /*< chNum: 0 - (AVSYNC_MAX_CHANNELS_PER_DISPLAY - 1) */
} Avsync_ResetPlayerTimerParams;

typedef struct Avsync_PlayParams
{
    UInt32 displayID;
    /*< displayID : 0 - (AVSYNC_MAX_NUM_DISPLAYS - 1) */

    UInt32 chNum;
    /*< chNum: 0 - (AVSYNC_MAX_CHANNELS_PER_DISPLAY - 1) */

    UInt32 displaySeqId;
    /*< Sequence Id which will be associated with all frames
     *  received in play state.
     *  Application should set it to SYSTEM_DISPLAY_SEQID_DEFAULT
     *  if it doesnt want to change displaySeqId
     */
} Avsync_PlayParams;


typedef struct Avsync_ScanParams
{
    UInt32 displayID;
    /*< displayID : 0 - (AVSYNC_MAX_NUM_DISPLAYS - 1) */

    UInt32 chNum;
    /*< chNum: 0 - (AVSYNC_MAX_CHANNELS_PER_DISPLAY - 1) */

    UInt32 frameDisplayDurationMS;
    /*< Time in ms to display each frame. */

    UInt32 displaySeqId;
    /*< Sequence Id which will be associated with all frames
     *  received in play state.
     *  Application should set it to SYSTEM_DISPLAY_SEQID_DEFAULT
     *  if it doesnt want to change displaySeqId
     */
} Avsync_ScanParams;

typedef struct Avsync_SeekParams
{
    UInt32 displayID;
    /*< displayID : 0 - (AVSYNC_MAX_NUM_DISPLAYS - 1) */

    UInt32 chNum;
    /*< chNum: 0 - (AVSYNC_MAX_CHANNELS_PER_DISPLAY - 1) */

    UInt64 seekAudPTS;
    /*< Audio PTS value from whichh playback should begin */

    UInt64 seekVidPTS;
    /*< Video PTS value from whichh playback should begin */

    UInt32 displaySeqId;
    /*< Sequence Id which will be associated with all frames
     *  received in play state.
     *  Application should set it to SYSTEM_DISPLAY_SEQID_DEFAULT
     *  if it doesnt want to change displaySeqId
     */
} Avsync_SeekParams;


typedef struct Avsync_VidStats
{
    UInt32 numFramesRendered;
    UInt32 numFramesReplayed;
    UInt32 numFramesSkippedLate;
    UInt32 numFramesSkippedEarly;
    UInt32 numUnderrun;
    UInt32 numOverflow;
} Avsync_VidStats;

typedef struct Avsync_AudStats
{
    UInt32 numFramesPlayed;
    UInt32 numFramesReplayed;
    UInt32 numFramesSkippedEarly;
    UInt32 numFramesSkippedLate;
} Avsync_AudStats;

typedef struct Avsync_SystemClockStats
{
    UInt32 numClockAdjustPositive;
    UInt32 numClockAdjustNegative;
} Avsync_SystemClockStats;

typedef struct Avsync_InitParams
{
    UInt64 wallTimeInitTS;
} Avsync_InitParams;


typedef struct Avsync_GetSrObjParams
{
    UInt32 srObjSrPtr;
} Avsync_GetSrObjParams;

typedef struct Avsync_vidSchedulePolicy
{
    Int32 playMaxLag;
    Int32 playMaxLead;
    Int32 maxReplayLead;
    UInt32 doMarginScaling;
} Avsync_vidSchedulePolicy;

typedef struct Avsync_refClkAdjustPolicy
{
    Avsync_RefClkMasterType  refClkType;
    Int32                   clkAdjustLead;
    Int32                   clkAdjustLag;
} Avsync_refClkAdjustPolicy;


typedef struct Avsync_SynchConfigParams
{
    Avsync_PlaybackStartMode playStartMode;
    UInt32                   chNum;
    UInt32                   avsyncEnable;
    Avsync_vidSchedulePolicy  vidSynchPolicy;
    Avsync_refClkAdjustPolicy clkAdjustPolicy;
    Avsync_PTSInitMode       ptsInitMode;
    UInt32                   audioPresent;
    UInt32                   videoPresent;
    UInt32                   playTimerStartTimeout;
    UInt32                   blockOnAudioGet;
} Avsync_SynchConfigParams;

typedef struct AvsyncLink_LinkSynchConfigParams
{
    UInt32  numCh;
    UInt32  videoSynchLinkID;
    UInt32  displayID;
    UInt32  audioDevId;
    UInt32  syncMasterChnum;
    Avsync_SynchConfigParams queCfg[AVSYNC_MAX_CHANNELS_PER_DISPLAY];
} AvsyncLink_LinkSynchConfigParams;

/**
    \brief Avsync AVSYNC_LINK_CMD_SET_VIDEO_BACKEND_DELAY params
*/

typedef struct Avsync_VideoBackendDelayParams
{
    UInt32 displayID;
    UInt32 backendDelayMS;
} Avsync_VideoBackendDelayParams;


UInt64 Avsync_getWallTime();
Int32 Avsync_configSyncConfigInfo(AvsyncLink_LinkSynchConfigParams *cfg);
Int32 Avsync_setPlaybackSpeed(Avsync_TimeScaleParams *timeScaleParams);
Int32 Avsync_doPause(Avsync_PauseParams *pauseParams);
Int32 Avsync_doUnPause(Avsync_UnPauseParams *unPauseParams);
Int32 Avsync_stepFwd(Avsync_StepFwdParams *stepFwdParams);
Int32 Avsync_setFirstAudPTS(Avsync_FirstAudPTSParams *audPTSParams);
Int32 Avsync_setFirstVidPTS(Avsync_FirstVidPTSParams *vidPTSParams);
Int32 Avsync_resetPlayerTimer(Avsync_ResetPlayerTimerParams *resetParams);
Int32 Avsync_setVideoBackEndDelay(Avsync_VideoBackendDelayParams *delayParams);
Int32 Avsync_doPlay(Avsync_PlayParams *playParams);
Int32 Avsync_doScan(Avsync_ScanParams *scanParams);
Int32 Avsync_seekPlayback(Avsync_SeekParams *seekParams);
Int32 Avsync_printStats();
Int32 Avsync_setWallTimeBase(UInt64 wallTimeBase);


static inline void Avsync_SynchConfigParams_Init(Avsync_SynchConfigParams *prms)
{
    prms->audioPresent    = FALSE;
    prms->blockOnAudioGet = FALSE;
    prms->chNum           = AVSYNC_INVALID_CHNUM;
    prms->playStartMode   = AVSYNC_PLAYBACK_START_MODE_WAITSYNCH;
    prms->playTimerStartTimeout = 0;
    prms->ptsInitMode = AVSYNC_PTS_INIT_MODE_AUTO;
    prms->avsyncEnable   = FALSE;
    prms->videoPresent = TRUE;
    prms->vidSynchPolicy.maxReplayLead = AVSYNC_VIDEO_FUTURE_FRAME_DROP_THRESHOLD_MS;
    prms->vidSynchPolicy.playMaxLag    = AVSYNC_VIDEO_PLAY_MAX_LAG_MS;
    prms->vidSynchPolicy.playMaxLead   = AVSYNC_VIDEO_PLAY_MAX_LEAD_MS;
    prms->vidSynchPolicy.doMarginScaling = FALSE;
    prms->clkAdjustPolicy.refClkType    = AVSYNC_REFCLKADJUST_NONE;
    prms->clkAdjustPolicy.clkAdjustLead = AVSYNC_VIDEO_TIMEBASESHIFT_MAX_LEAD_MS;
    prms->clkAdjustPolicy.clkAdjustLag  = AVSYNC_VIDEO_TIMEBASESHIFT_MAX_LAG_MS;

}

static inline void AvsyncLink_LinkSynchConfigParams_Init(AvsyncLink_LinkSynchConfigParams *prms)
{
    Int i;

    memset(prms,0,sizeof(*prms));
    prms->audioDevId = AVSYNC_INVALID_AUDDEVID;
    prms->displayID  = AVSYNC_INVALID_DISPLAY_ID;
    prms->numCh      = 0;
    prms->syncMasterChnum = AVSYNC_INVALID_CHNUM;
    prms->videoSynchLinkID = SYSTEM_LINK_ID_INVALID;
    for (i = 0 ; i < AVSYNC_MAX_CHANNELS_PER_DISPLAY; i++)
    {
        Avsync_SynchConfigParams_Init(&prms->queCfg[i]);
    }
}


#define AVSYNC_INIT_SEQID(seqId)   ((seqId) = SYSTEM_DISPLAY_SEQID_DEFAULT)

#endif

/* @} */
