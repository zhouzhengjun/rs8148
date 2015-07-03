/*******************************************************************************
 *                                                                             *
 * Copyright (c) 2009 Texas Instruments Incorporated - http://www.ti.com/      *
 *                        ALL RIGHTS RESERVED                                  *
 *                                                                             *
 ******************************************************************************/

#ifndef _SYSTEM_LINK_PRIV_H_
#define _SYSTEM_LINK_PRIV_H_

#include "system_priv_common.h"

#include <mcfw/interfaces/ti_vsys.h>

typedef struct
{
    UInt32 tskId;

    OSA_TskHndl tsk;

    VSYS_EVENT_HANDLER_CALLBACK eventHandler;

    Ptr eventHandlerAppData;

} SystemLink_Obj;




#endif
