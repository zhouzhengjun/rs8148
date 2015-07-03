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

#ifndef _BELL_SW_MS_H_
#define _BELL_SW_MS_H_

#include "bell_header.h"

#define LAYOUT_MODE_4CH_4CH    0
#define LAYOUT_MODE_4CH        1
#define LAYOUT_MODE_1CH        2
#define LAYOUT_MODE_9CH        3
#define LAYOUT_MODE_6CH        4
#define LAYOUT_MODE_16CH       5
#define LAYOUT_MODE_7CH_1CH    6
#define LAYOUT_MODE_PIP    7
#define LAYOUT_MODE_20CH_4X5   8
#define LAYOUT_MODE_25CH_5X5   9
#define LAYOUT_MODE_30CH_5X6   10
#define LAYOUT_MODE_36CH_6X6   11
#define LAYOUT_MODE_PIP2    12
#define LAYOUT_MODE_PIP2_MAIN2    13
#define LAYOUT_MODE_PIP_FAKE    14
#define LAYOUT_MAX             15

#define SW_MS_INVALID_ID                   (0xFF)


Void Bell_swMsGenerateLayout(VDIS_DEV devId, UInt32 startChId, UInt32 maxChns, UInt32 layoutId,
     VDIS_MOSAIC_S * vdMosaicParam, Bool forceLowCostScaling, UInt32 demoType, UInt32 resolution);
Void Bell_swMsGenerateLayout_for_switch(VDIS_DEV devId, UInt32 startChId, UInt32 maxChns, UInt32 layoutId,
     VDIS_MOSAIC_S * vdMosaicParam, Bool forceLowCostScaling, UInt32 demoType, UInt32 resolution);

Void Bell_swMs_PrintLayoutParams(VDIS_MOSAIC_S * vdMosaicParam);
Void Bell_swMsSetOutputFPS(VDIS_MOSAIC_S * vdMosaicParam, UInt32 outputFPS);
Int32 Bell_swMsGetOutputFPS(VDIS_MOSAIC_S * vdMosaicParam);


#endif

