/*******************************************************************************
 *                                                                             *
 * Copyright (c) 2011 Texas Instruments Incorporated - http://www.ti.com/      *
 *                        ALL RIGHTS RESERVED                                  *
 *                                                                             *
 ******************************************************************************/

/**
    \file avsync_rtos.h
    \brief 
*/


#ifndef AVSYNC_RTOS_H_
#define AVSYNC_RTOS_H_

#include <mcfw/src_bios6/links_m3vpss/system/system_priv_m3vpss.h>      // for  FVID2_Frame
#include <mcfw/interfaces/link_api/avsync.h>
#include <mcfw/interfaces/link_api/avsync_internal.h>
#include <mcfw/src_bios6/utils/utils.h>


typedef struct AvsyncLink_DisplayObj
{
    UInt64           vSyncSTC;
    UInt32           videoBackendDelay;
} AvsyncLink_DisplayObj;

typedef struct AvsyncLink_VidQueCreateParams
{
    UInt32                syncLinkID;
    UInt32                displayID;
    UInt32                chNum;
    UInt32                maxElements;
    FVID2_Frame        ** queueMem;
} AvsyncLink_VidQueCreateParams;


typedef struct AvsyncLink_VidQueObj
{
    AvsyncLink_VidQueCreateParams cp;
    Avsync_SynchConfigParams    *cfg;
    Utils_QueHandle        vidFrmQ;
    Avsync_PlayerTimeObj   *playerTime;
    AvsyncLink_DisplayObj  *displayObj;
    Avsync_RefClkObj       *refClk;
    Avsync_VidStats        *stats;
    UInt32                 state;
    UInt32                 playTimerStartTimeout;
    UInt32                 displayID;
    UInt32                 syncMasterChnum;
#if (AVSYNC_APPLY_PTS_SMOOTHING_FILTER)
    Avsync_SimpleMovingAvgObj ptsDiffAvg;
#endif

} AvsyncLink_VidQueObj;


#define AVSYNC_VIDQUE_IS_SYNCH_ENABLED(queObj)           ((((queObj)->cfg) && ((queObj)->cfg->avsyncEnable)) \
                                                          ? TRUE                                             \
                                                          : FALSE)


Int32 AvsyncLink_init();
Int32 AvsyncLink_deInit();
Void Avsync_vidQueCreateParamsInit(AvsyncLink_VidQueCreateParams *cp);
Int Avsync_vidQueCreate(AvsyncLink_VidQueCreateParams *cp,
                        AvsyncLink_VidQueObj *queObj);
Int Avsync_vidQueDelete(AvsyncLink_VidQueObj *queObj);
Int Avsync_vidQuePut(AvsyncLink_VidQueObj *queObj,
                     FVID2_Frame *frame);
Int Avsync_vidQueGet(AvsyncLink_VidQueObj *queObj,
                     Bool  forceGet,
                     FVID2_Frame **framePtr,
                     FVID2_FrameList *freeFrameList);
UInt32 Avsync_vidQueGetQueLength(AvsyncLink_VidQueObj *queObj);
UInt32 Avsync_vidQueIsEmpty(AvsyncLink_VidQueObj *queObj);
UInt32 Avsync_vidQueGetMasterSynchChannel(UInt32  syncLinkID);
Int Avsync_vidQueFlush(AvsyncLink_VidQueObj *queObj,
                       FVID2_Frame **framePtr,
                       FVID2_FrameList *freeFrameList);
Void Avsync_vidSynchCallbackFxn(UInt32 displayID);
UInt32 Avsync_mapDisplayLinkID2Index(UInt32 linkID);
Void AvsyncLink_logCaptureTS(UInt32 chNum, UInt64 ts);

#endif /* AVSYNC_RTOS_H_ */
