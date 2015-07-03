#ifndef __TI_VSYS_PRIV_H__
#define __TI_VSYS_PRIV_H__

#include <mcfw/interfaces/link_api/system_const.h>

#include "ti_vsys.h"

#include <osa.h>
#include <mcfw/interfaces/link_api/system.h>
#include <mcfw/interfaces/link_api/captureLink.h>
#include <mcfw/interfaces/link_api/cameraLink.h>
#include <mcfw/interfaces/link_api/deiLink.h>
#include <mcfw/interfaces/link_api/nsfLink.h>
#include <mcfw/interfaces/link_api/displayLink.h>
#include <mcfw/interfaces/link_api/nullLink.h>
#include <mcfw/interfaces/link_api/grpxLink.h>
#include <mcfw/interfaces/link_api/dupLink.h>
#include <mcfw/interfaces/link_api/selectLink.h>
#include <mcfw/interfaces/link_api/swMsLink.h>
#include <mcfw/interfaces/link_api/mergeLink.h>
#include <mcfw/interfaces/link_api/nullSrcLink.h>
#include <mcfw/interfaces/link_api/ipcLink.h>
#include <mcfw/interfaces/link_api/systemLink_m3vpss.h>
#include <mcfw/interfaces/link_api/systemLink_m3video.h>
#include <mcfw/interfaces/link_api/encLink.h>
#include <mcfw/interfaces/link_api/decLink.h>

#include <mcfw/interfaces/link_api/sclrLink.h>
#include <mcfw/interfaces/link_api/avsync.h>

#include <ti/syslink/utils/IHeap.h>
#include <ti/syslink/utils/Memory.h>
#include <ti/ipc/SharedRegion.h>
#include <string.h>

#include <ti/xdais/xdas.h>
#include <ti/xdais/dm/xdm.h>
#include <ti/xdais/dm/ivideo.h>
#include <ih264enc.h>
#include <ih264vdec.h>

/* =============================================================================
 * Structure
 * =============================================================================
 */
typedef struct
{
    VSYS_PARAMS_S vsysConfig;

    Bool enableFastUsecaseSwitch;

} VSYS_MODULE_CONTEXT_S;

#define MULTICH_INIT_STRUCT(structName,structObj)  structName##_Init(&structObj)

#include "ti_vdis_priv.h"
#include "ti_venc_priv.h"
#include "ti_vdec_priv.h"
#include "ti_vcap_priv.h"


extern VSYS_MODULE_CONTEXT_S gVsysModuleContext;

Int32 MultiCh_prfLoadCalcEnable(Bool enable, Bool printStatus, Bool printTskLoad);
Int32 MultiCh_prfLoadPrint(Bool printTskLoad,Bool resetTaskLoad);

Void MultiCh_createVcapVdis();
Void MultiCh_deleteVcapVdis();

Void MultiCh_createVcap7002Vdis();
Void MultiCh_deleteVcap7002Vdis();

void MultiCh_createProgressiveVcapVencVdecVdis();
void MultiCh_deleteProgressiveVcapVencVdecVdis();

void MultiCh_createProgressive4D1VcapVencVdecVdis();
void MultiCh_deleteProgressive4D1VcapVencVdecVdis();

void MultiCh_createProgressive8D1VcapVencVdecVdis();
void MultiCh_deleteProgressive8D1VcapVencVdecVdis();

void MultiCh_createProgressive16ChVcapVencVdecVdis();
void MultiCh_deleteProgressive16ChVcapVencVdecVdis();
Int32 MultiCh_progressive16ChVcapVencVdecVdisSwitchLayout(VDIS_DEV vdDevId, SwMsLink_LayoutPrm *pSwMsLayout);
void MultiCh_progressive16ChVcapVencVdecVdisSwmsChReMap(VDIS_MOSAIC_S *psVdMosaicParam);
Int32 MultiCh_progressive16ChVcapVencVdecVdisSetCapFrameRate(VCAP_CHN vcChnId, VCAP_STRM vStrmId, Int32 inputFrameRate, Int32 outputFrameRate);
Int32 MultiCh_progressive16ChVcapVencVdecVdis_enableDisableCapChn(VCAP_CHN vcChnId, VCAP_STRM vcStrmId, Bool enableChn);
Int32 MultiCh_progressive16ChVcapVencVdecVdis_setCapDynamicParamChn(VCAP_CHN vcChnId, VCAP_CHN_DYNAMIC_PARAM_S *psCapChnDynaParam, VCAP_PARAMS_E paramId);
Int32 MultiCh_progressive16ChVcapVencVdecVdis_getCapDynamicParamChn(VCAP_CHN vcChnId, VCAP_CHN_DYNAMIC_PARAM_S *psCapChnDynaParam, VCAP_PARAMS_E paramId);

void MultiCh_createProgressive16ChNrtVcapVencVdecVdis();
void MultiCh_deleteProgressive16ChNrtVcapVencVdecVdis();
Int32 MultiCh_progressive16ChNrtVcapVencVdecVdisSetCapFrameRate(VCAP_CHN vcChnId, VCAP_STRM vStrmId, Int32 inputFrameRate, Int32 outputFrameRate);
Int32 MultiCh_progressive16ChNrtVcapVencVdecVdisEnableDisableCapChn(VCAP_CHN vcChnId, VCAP_STRM vStrmId, Bool enableChn);
Int32 MultiCh_progressive16ChNrtVcapVencVdecVdisSetOutRes(VCAP_CHN vcChnId, VCAP_CHN_DYNAMIC_PARAM_S *psCapChnDynaParam);
Int32 MultiCh_progressive16ChNrtVcapVencVdecVdisGetOutRes(VCAP_CHN vcChnId, VCAP_CHN_DYNAMIC_PARAM_S *psCapChnDynaParam);

Void MultiCh_createInterlacedVcapVencVdecVdis();
Void MultiCh_deleteInterlacedVcapVencVdecVdis();

Void MultiCh_createVdecVdis();
Void MultiCh_deleteVdecVdis();

Void MultiCh_createVcapVencVdis();
Void MultiCh_deleteVcapVencVdis();

Void MultiChHd_createVcapVencVdis();
Void MultiChHd_deleteVcapVencVdis();

Void MultiCh_create_8CifProgressiveVcapVencVdecVdis();
Void MultiCh_delete_8CifProgressiveVcapVencVdecVdis();

Void MultiCh_createCustomVcapVencVdecVdis();
Void MultiCh_deleteCustomVcapVencVdecVdis();

Void MultiCh_createHybridDVR();
Void MultiCh_deleteHybridDVR();

Void MultiCh_createCarDVR();
Void MultiCh_deleteCarDVR();
Void MultiCh_carDVRSwitchDisplayCh(Int displayChId);

Void MultiCh_ipcBitsInitCreateParams_BitsInHLOS(IpcBitsInLinkHLOS_CreateParams *cp);
Void MultiCh_ipcBitsInitCreateParams_BitsInHLOSVcap(IpcBitsInLinkHLOS_CreateParams *cp);
Void MultiCh_ipcBitsInitCreateParams_BitsInRTOS(IpcBitsInLinkRTOS_CreateParams *cp,
                                                Bool notifyNextLink);
Void MultiCh_ipcBitsInitCreateParams_BitsOutHLOS(IpcBitsOutLinkHLOS_CreateParams *cp,
                                                 System_LinkQueInfo *inQueInfo);
Void MultiCh_ipcBitsInitCreateParams_BitsOutRTOS(IpcBitsOutLinkRTOS_CreateParams *cp,
                                                Bool notifyPrevLink);

#endif


