/*******************************************************************************
 *                                                                             *
 * Copyright (c) 2009 Texas Instruments Incorporated - http://www.ti.com/      *
 *                        ALL RIGHTS RESERVED                                  *
 *                                                                             *
 ******************************************************************************/

#ifndef _CHAINS_SW_MS_H_
#define _CHAINS_SW_MS_H_


#include <avcap/common/chains.h>

#define CHAINS_SW_MS_MAX_DISPLAYS    (3)
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
typedef enum {
    BELL_TYPE_PROGRESSIVE,
    BELL_TYPE_INTERLACED
}bellType;


Void Bell_swMsGenerateLayout(UInt32 devId, UInt32 startChId, UInt32 maxChns, UInt32 layoutId,
     SwMsLink_LayoutPrm * vdMosaicParam, Bool forceLowCostScaling, UInt32 demoType, UInt32 resolution);
Void Bell_swMs_PrintLayoutParams(SwMsLink_LayoutPrm * vdMosaicParam);

//Void Bell_swMsGetDefaultLayoutPrm(UInt32 devId, SwMsLink_CreateParams *swMsCreateArgs, Bool forceLowCostScaling);
Int32 Bell_GetSize(UInt32 outRes, UInt32 * width, UInt32 * height);

Int32 Chains_swMsSwitchLayout(
            UInt32 swMsLinkId[CHAINS_SW_MS_MAX_DISPLAYS],
            SwMsLink_CreateParams swMsPrm[CHAINS_SW_MS_MAX_DISPLAYS],
            Bool switchLayout,
            Bool switchCh,
            UInt32 numDisplay);

Void Chains_swMsGenerateLayoutParams(UInt32 devId, UInt32 layoutId, 
            SwMsLink_CreateParams * swMsLayoutParams);

#endif

