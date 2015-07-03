/*******************************************************************************
 *                                                                             *
 * Copyright (c) 2009 Texas Instruments Incorporated - http://www.ti.com/      *
 *                        ALL RIGHTS RESERVED                                  *
 *                                                                             *
 ******************************************************************************/



#include "system_priv_ipc.h"
#include <mcfw/interfaces/link_api/ipcLink.h>
#include <ti/syslink/SysLink.h>


System_IpcObj gSystem_ipcObj;

UInt32 gSystem_ipcEnableProcId[] =
        {
        SYSTEM_PROC_HOSTA8,
#if defined(TI_8107_BUILD)
        SYSTEM_PROC_INVALID,
#else
        SYSTEM_PROC_DSP,
#endif
        SYSTEM_PROC_M3VIDEO,
        SYSTEM_PROC_M3VPSS,
        SYSTEM_PROC_MAX /* Last entry */
};

Int32 System_ipcInit()
{
    printf(" %u: SYSTEM: IPC init in progress !!!\n", OSA_getCurTimeInMsec());

    SysLink_setup ();

    System_ipcMsgQInit();

    System_ipcNotifyInit();


    printf(" %u: SYSTEM: IPC init DONE !!!\n", OSA_getCurTimeInMsec());

    return OSA_SOK;
}

Int32 System_ipcDeInit()
{
    printf(" %u: SYSTEM: IPC de-init in progress !!!\n", OSA_getCurTimeInMsec());

    System_ipcNotifyDeInit();

    System_ipcMsgQDeInit();

    SysLink_destroy ();

    printf(" %u: SYSTEM: IPC de-init DONE !!!\n", OSA_getCurTimeInMsec());

    return OSA_SOK;
}

UInt32 System_getSelfProcId()
{
    return MultiProc_self();
}
