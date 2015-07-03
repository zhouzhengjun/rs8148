/*******************************************************************************
 *                                                                             *
 * Copyright (c) 2011 Texas Instruments Incorporated - http://www.ti.com/      *
 *                        ALL RIGHTS RESERVED                                  *
 *                                                                             *
 ******************************************************************************/

/**
    \file  avsync_hlos.h
    \brief Interface header file to Avsync module to be included on HLOS side.
*/


#ifndef AVSYNC_HLOS_H_
#define AVSYNC_HLOS_H_

#include <osa.h>
#include <osa_que.h>
#include <mcfw/interfaces/ti_media_std.h>
#include <mcfw/interfaces/ti_media_error_def.h>
#include <mcfw/interfaces/link_api/avsync.h>
#include <mcfw/interfaces/link_api/avsync_internal.h>
#include <rsdemo/audio_sample/audio.h>

typedef struct AvsyncLink_AudioBackEndInfo
{
    UInt64           aSyncSTC;
    UInt32           audioBackendDelay;
} AvsyncLink_AudioBackEndInfo;


typedef struct AvsyncLink_AudQueCreateParams
{
    UInt32                audioDevID;
    UInt32                chNum;
    UInt32                maxElements;
} AvsyncLink_AudQueCreateParams;

typedef struct AvsyncLink_AudQueObj
{
    AvsyncLink_AudQueCreateParams cp;
    OSA_QueHndl            audFrmQ;
    Avsync_PlayerTimeObj   *playerTime;
    Avsync_SynchConfigParams *cfg;
    Avsync_AudStats        *stats;
    Avsync_RefClkObj       *refClk;
    UInt32                 state;
    UInt32                 playTimerStartTimeout;
    UInt32                 videoStreamDisplayID;
    UInt32                 syncMasterChnum;
} AvsyncLink_AudQueObj;


Int32 AvsyncLink_init();
Int32 AvsyncLink_deInit();
Int Avsync_audQueDelete(AvsyncLink_AudQueObj *queObj);
Int Avsync_audQueCreate(AvsyncLink_AudQueCreateParams *cp,
                        AvsyncLink_AudQueObj *queObj);
Int Avsync_audQuePut(AvsyncLink_AudQueObj *queObj,
                     AudFrm_Buf *frame);
Int Avsync_audQueGet(AvsyncLink_AudQueObj *queObj,
                     AudFrm_Buf **framePtr,
                     AudFrm_BufList *freeFrameList,
                     AvsyncLink_AudioBackEndInfo *abeInfo);
Bool Avsync_audQueWaitForPlayerStart(AvsyncLink_AudQueObj *queObj,
                                     Int32 timeOut);
Avsync_PlayerTimeState Avsync_audQueGetPlayerState(AvsyncLink_AudQueObj *queObj);
Void AvsyncLink_logIpcBitsOutTS(UInt32 chNum, UInt64 ts);

#endif /* AVSYNC_HLOS_H_ */
