#ifndef __TI_VDEC_PRIV_H__
#define __TI_VDEC_PRIV_H__


#include "ti_vdec.h"
#include "ti_vsys_priv.h"

/* =============================================================================
 * Structure
 * =============================================================================
 */
typedef struct
{
    UInt32 decId;
    UInt32 ipcBitsInRTOSId;
    UInt32 ipcBitsOutHLOSId;
    UInt32 ipcM3OutId;
    UInt32 ipcM3InId;

    VDEC_PARAMS_S vdecConfig;
}VDEC_MODULE_CONTEXT_S;

extern VDEC_MODULE_CONTEXT_S gVdecModuleContext;

Int32 Vdec_create(System_LinkInQueParams *vdecOutQue, UInt32 vdecNextLinkId, Bool tilerEnable, UInt32 numFramesPerCh);
Int32 Vdec_delete();

#endif


