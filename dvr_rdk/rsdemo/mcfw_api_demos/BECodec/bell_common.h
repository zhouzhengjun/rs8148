/*******************************************************************************
 *                                                                             *
 *      Copyright (c) 2013 BroadEng Beijing - http://www.broadeng.net/         *
 *                        ALL RIGHTS RESERVED                                  *
 *                                                                             * 
 *******************************************************************************
 *                                                                             *
 * Author:      Zhang Tao <zhangtao@broadeng.net>                              *
 * Version:     v1.0.0                                                         *
 *                                                                             *
 ******************************************************************************/

#ifndef __BELLCOMMON_H__
#define __BELLCOMMON_H__

#include <osa.h>

#include "mcfw/src_linux/mcfw_api/ti_vsys_priv.h"
#include "mcfw/interfaces/common_def/ti_vsys_common_def.h"
#include "bell_api.h"

Int32 Bell_detectBoard();

Int32 Bell_memPrintHeapStatus();

Void Bell_swMsGetDefaultLayoutPrm(UInt32 devId, SwMsLink_CreateParams *swMsCreateArgs, Bool forceLowCostScaling);

Int32 Bell_prfLoadCalcEnable(Bool enable, Bool printStatus, Bool printTskLoad);

Void bell_create_path(CodecPrm *prm);

Void bell_delete_path();

Int32 Bell_GetSize(UInt32 outRes, UInt32 * width, UInt32 * height);

#endif

