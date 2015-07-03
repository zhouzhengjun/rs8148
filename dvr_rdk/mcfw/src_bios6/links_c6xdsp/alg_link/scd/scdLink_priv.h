/*******************************************************************************
 *                                                                             *
 * Copyright (c) 2009 Texas Instruments Incorporated - http://www.ti.com/      *
 *                        ALL RIGHTS RESERVED                                  *
 *                                                                             *
 ******************************************************************************/

#ifndef _ALG_LINK_SCD_PRIV_H_
#define _ALG_LINK_SCD_PRIV_H_

#include <mcfw/src_bios6/utils/utils.h>
#include <mcfw/src_bios6/links_c6xdsp/system/system_priv_c6xdsp.h>
#include <mcfw/interfaces/link_api/algLink.h>

#include "scd.h"


#define ALG_LINK_SCD_OBJ_MAX                        (SYSTEM_LINK_ID_SCD_COUNT)

#define ALG_LINK_SCD_MAX_OUT_FRAMES_PER_CH          (SYSTEM_LINK_FRAMES_PER_CH)

#define ALG_LINK_SCD_MAX_OUT_FRAMES                 (ALG_LINK_SCD_MAX_CH*ALG_LINK_SCD_MAX_OUT_FRAMES_PER_CH)

#define MOTION_DETECTION_SENSITIVITY(x,y)           ((x * y)/5) // 20%
#define MOTION_DETECTION_SENSITIVITY_STEP           (10)        // 10%
#define NUM_BLOCK_MOTION_DETECTION_THRESOLD(x)      (x >> 1)    // 50%

typedef struct AlgLink_ScdOutObj {
    Utils_BitBufHndl bufOutQue;
    UInt32 numAllocPools;
    Bitstream_Buf outBufs[ALG_LINK_SCD_MAX_OUT_FRAMES];
    UInt32 outNumBufs[UTILS_BITBUF_MAX_ALLOC_POOLS];
    UInt32 buf_size[UTILS_BITBUF_MAX_ALLOC_POOLS];
    UInt32 ch2poolMap[ALG_LINK_SCD_MAX_CH];
} AlgLink_ScdOutObj;

/**
    \brief SCD Channel parameters used to SET BEFORE CALLING PROCESS
*/
typedef struct
{
    U32 chId;
    /**< Unique video channel identifier, e.g. channel no. */

 	SCD_Mode mode;
    /**< Enable/Disable scd running on algo*/

    U32 width;
    /**< Set the width (in pixels) of video frame that scd will process */

    U32 height;
    /**< Set the height (in pixels) of video frame that scd will process */

    U32 stride;
    /**< Set the video frame pitch/stride of the images in the video buffer*/

    PTR curFrame;
    /** Luma pointer to current frame */

    UInt32 chBlkConfigUpdate;
    /**<Flag to check if block config update is required. */


    SCD_Sensitivity frmSensitivity;
    /**< internal threshold for scd change tolerance; globally applied to entire frame */

    U32 frmIgnoreLightsON;
    /**< Set to 1 to ignore sudden, significantly brighter avg. scene luminance */

    U32 frmIgnoreLightsOFF;
    /**< Set to 1 to ignore sudden, significantly darker avg. scene luminance */

    U32 frmEdgeThreshold;
    /**< Set minimum number of edge pixels required to indicate non-tamper event */
    
    SCD_blkChngConfig blkConfig[ALG_LINK_SCD_MAX_BLOCKS_IN_FRAME];
    /**< Linear array of configuration of frame blocks that 
         scd will monitor for motion detection (configuration) */
} AlgLink_ScdchPrm;

typedef struct {
    U32 chId;
    /**< Unique video channel identifier, e.g. channel no. */

    UInt32 inFrameRecvCount;
    /**< input frame recevied from previous link */

    UInt32 inFrameUserSkipCount;
    /**< input frame rejected due mismatch in FID */

    UInt32 inFrameProcessCount;
    /**< input frames actually processed */
    
    UInt32 inFrameProcessTime;

    AlgLink_ScdOutput scdChStat;

    Utils_frameSkipContext frameSkipCtx;
    /**< Data structure for frame skip to achieve expected output frame rate */    
} AlgLink_ScdChObj;

typedef struct AlgLink_ScdObj {
    System_LinkQueInfo * inQueInfo;

    AlgLink_ScdchPrm chParams[ALG_LINK_SCD_MAX_CH];

    AlgLink_ScdCreateParams createArgs;

    AlgLink_ScdChObj   chObj[ALG_LINK_SCD_MAX_CH];

    void *algHndl;
    /**< handle to SCD algorithm */

    UInt32 statsStartTime;

    UInt32 totalFrameCount;

} AlgLink_ScdObj;


Int32 AlgLink_ScdalgCreate(AlgLink_ScdObj * pObj);
Int32 AlgLink_ScdalgProcessData(AlgLink_ScdObj * pObj, FVID2_FrameList *frameList, Utils_BitBufHndl *bufOutQue);
Int32 AlgLink_ScdalgDelete(AlgLink_ScdObj * pObj);

Int32 AlgLink_ScdalgSetChblkUpdate(AlgLink_ScdObj * pObj,
                            AlgLink_ScdChblkUpdate * params);
                                                          
Int32 AlgLink_ScdalgSetChScdSensitivity(AlgLink_ScdObj * pObj,
                            AlgLink_ScdChParams * params);
Int32 AlgLink_ScdalgSetChScdMode(AlgLink_ScdObj * pObj,
                            AlgLink_ScdChParams * params);
Int32 AlgLink_ScdalgSetChScdIgnoreLightsOn(AlgLink_ScdObj * pObj,
                            AlgLink_ScdChParams * params);
Int32 AlgLink_ScdalgSetChScdIgnoreLightsOff(AlgLink_ScdObj * pObj,
                            AlgLink_ScdChParams * params);
#if 0
Int32 AlgLink_ScdalgGetChTamperState(AlgLink_ScdObj * pObj,
                            AlgLink_ScdChTamperState * params);
#endif
Int32 AlgLink_ScdalgGetChResetChannel(AlgLink_ScdObj * pObj,
                            AlgLink_ScdChCtrl * params);

Int32 AlgLink_ScdresetStatistics(AlgLink_ScdObj *pObj);
Int32 AlgLink_ScdprintStatistics (AlgLink_ScdObj *pObj, Bool resetAfterPrint);
Int32 AlgLink_scdAlgGetAllChFrameStatus(AlgLink_ScdObj * pObj, AlgLink_ScdAllChFrameStatus *pPrm);
/*
void AlgLink_ScdprocessSCDmetadata(AlgLink_ScdResult	* chlMetadata,
                            AlgLink_ScdchPrm	* chlParams, 
                            AlgLink_ScdChStatus * chlCtrl);
*/
#endif
