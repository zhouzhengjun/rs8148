/*******************************************************************************
 *                                                                             *
 * Copyright (c) 2009 Texas Instruments Incorporated - http://www.ti.com/      *
 *                        ALL RIGHTS RESERVED                                  *
 *                                                                             *
 ******************************************************************************/

#ifndef __TI_VGRPX_COMMON_DEF_H__
#define __TI_VGRPX_COMMON_DEF_H__

#include "ti_vsys_common_def.h"

#define VGRPX_ID_0   (0)
#define VGRPX_ID_1   (1)
#define VGRPX_ID_2   (2)
#define VGRPX_ID_MAX (3)

#define VGRPX_DATA_FORMAT_RGB565        (0)
#define VGRPX_DATA_FORMAT_ARGB888       (1)

#define VGRPX_SF_INTERLACED             (0)
#define VGRPX_SF_PROGRESSIVE            (1)

typedef struct {

    UInt32 scaleEnable;
    UInt32 transperencyEnable;
    UInt32 transperencyColor;

    UInt32 inWidth;
    UInt32 inHeight;
    UInt32 displayWidth;
    UInt32 displayHeight;
    UInt32 displayStartX;
    UInt32 displayStartY;

} VGRPX_DYNAMIC_PARAM_S;

typedef struct {

    UInt32 bufferPhysAddr;
    /* if 0, then buffer is allocated by the link */

    UInt32 bufferPitch;
    /* if 0, then pitch is auto-calculated by the link */

    UInt32 bufferWidth;
    UInt32 bufferHeight;
    /* this will be used for allocation, actual input size can be lesser than this */

    UInt32 dataFormat;
    /* VGRPX_DATA_FORMAT_RGB565 or VGRPX_DATA_FORMAT_ARGB888 */

    UInt32 scanFormat;
    /* VGRPX_SF_INTERLACED or VGRPX_SF_PROGRESSIVE */

} VGRPX_BUFFER_INFO_S;

typedef struct {

    UInt32 grpxId;

    VGRPX_BUFFER_INFO_S   bufferInfo;
    VGRPX_DYNAMIC_PARAM_S dynPrm;

} VGRPX_CREATE_PARAM_S;

#endif  /* __TI_VGRPX_COMMON_DEF_H__ */

/* @} */
