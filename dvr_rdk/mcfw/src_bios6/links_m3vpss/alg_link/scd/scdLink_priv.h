/*******************************************************************************
 *                                                                             *
 * Copyright (c) 2009 Texas Instruments Incorporated - http://www.ti.com/      *
 *                        ALL RIGHTS RESERVED                                  *
 *                                                                             *
 ******************************************************************************/

#ifndef _ALG_LINK_SCD_PRIV_H_
#define _ALG_LINK_SCD_PRIV_H_

#include <mcfw/src_bios6/utils/utils.h>
#include <mcfw/src_bios6/links_m3vpss/system/system_priv_m3vpss.h>
#include <mcfw/interfaces/link_api/algLink.h>

#include <mcfw/src_bios6/alg/simcop/inc/simcop_scd.h>

#define ALG_LINK_SIMCOP_SCD_MAX_CH      (48)

typedef struct {

    UInt32 chId;
    /**< Channel number, 0..ALG_LINK_SIMCOP_SCD_MAX_CH-1 */

    UInt32 inFrameRecvCount;
    UInt32 inFrameProcessCount;
    UInt32 inFrameSkipCount;

    SCD_ProcessPrm          algProcessPrm;
    SCD_ProcessStatus       algProcessStatus;
    SCD_InitMeanVarMHIPrm   algInitMeanVarMHIPrm;
    SCD_AlgImagebufs        algTmpImageBufs;

    AlgLink_ScdOutput       scdStatus;
    AlgLink_ScdOutput       prevScdStatus;

    Utils_frameSkipContext  frameSkipContext;

    Bool                    enableScd;
    Bool                    isTiledMode;

    Ptr                     memBlockAddr[SCD_MAX_MEM_BLOCKS];

    UInt32                  startTime;
    Bool                    skipInitialFrames;

} AlgLink_ScdChObj;

typedef struct {

    System_LinkQueInfo * inQueInfo;

    AlgLink_ScdChObj chObj[ALG_LINK_SIMCOP_SCD_MAX_CH];

    AlgLink_ScdCreateParams  scdCreateParams;

    SCD_Obj             algObj;
    SCD_MemAllocPrm     algMemAllocPrm;
    SCD_MemAllocPrm     algPerChMemAllocPrm;
    SCD_CreatePrm       algCreatePrm;

    UInt32 statsStartTime;

    UInt32 processFrameCount;
    UInt32 totalTime;

} AlgLink_ScdObj;

Int32 AlgLink_scdAlgCreate(AlgLink_ScdObj * pObj);

Int32 AlgLink_scdAlgDelete(AlgLink_ScdObj * pObj);

Int32 AlgLink_scdAlgProcessFrames(Utils_TskHndl *pTsk, AlgLink_ScdObj * pObj,
                                FVID2_FrameList *pFrameList
                                );

Int32 AlgLink_scdAlgPrintStatistics(AlgLink_ScdObj * pObj, Bool resetAfterPrint);

Int32 AlgLink_scdAlgGetAllChFrameStatus(AlgLink_ScdObj * pObj, AlgLink_ScdAllChFrameStatus *pPrm);

#endif
