/*******************************************************************************
 *                                                                             *
 * Copyright (c) 2009 Texas Instruments Incorporated - http://www.ti.com/      *
 *                        ALL RIGHTS RESERVED                                  *
 *                                                                             *
 ******************************************************************************/

/**
    \ingroup MCFW_API
    \defgroup MCFW_VDIS_API McFW Video Display (VDIS) API

    @{
*/

/**
    \file ti_vdis_common_def.h
    \brief McFW Video Display (VDIS) API
*/

#ifndef __TI_VDIS_COMMON_DEF_H__
#define __TI_VDIS_COMMON_DEF_H__

#include "ti_vsys_common_def.h"


/* =============================================================================
 * Defines
 * =============================================================================
 */
#define VDIS_DEV_MAX         (0x04)

/** Maximum Windows in a display mosaic */

#ifdef TI_816X_BUILD
#define VDIS_MOSAIC_WIN_MAX     (36)
#endif

#if defined(TI_814X_BUILD) || defined(TI_8107_BUILD)
#define VDIS_MOSAIC_WIN_MAX     (20)
#endif


/* =============================================================================
 *  Structures
 * =============================================================================
 */


/**
    \brief Display controller output info
*/
typedef struct  VDIS_DC_OUTPUT_INFO_S
{
    UInt32 vencNodeNum;
    /**< Node Number of the Venc */

    UInt32 dvoFmt;
    /**< digital output. See #Vps_DcDigitalFmt for the possible Values */

    UInt32 aFmt;
    /**< Analog output. See #Vps_DcAnalogFmt for the possible Values */

    UInt32 dataFormat;
    /**< Output Data format from Venc. Currently, valid values are
         FVID2_DF_RGB24_888, FVID2_DF_YUV444P, FVID2_DF_YUV422SP_UV */

    UInt32 dvoFidPolarity;
    /**< Polarity for the field id signal for the digital output only
         valid values see #vps_DcSignalPolarity */

    UInt32 dvoVsPolarity;
    /**< Polarity for the vertical sync signal for the digital output only
         valid values see #vps_DcSignalPolarity */

    UInt32 dvoHsPolarity;
    /**< Polarity for the horizontal sync signal for the digital output only
         valid values see #vps_DcSignalPolarity */

    UInt32 dvoActVidPolarity;
    /**< Polarity for the active video signal for the digital output only
         valid values see #vps_DcSignalPolarity */

}VDIS_DC_OUTPUT_INFO_S;

/**
    \brief Display Device Parameter
*/
typedef struct  VDIS_DEV_PARAM_S
{
    UInt32                  enable;
    /**< Enable/Disable Venc usage */

    UInt32                  enableGrpx;
    /**< Enable/Disable Grpx for the device */

    UInt32                  backGroundColor;
    /**< RGB 24-bit color to use for filling background color on a display device */

    UInt32                  colorSpaceMode;
    /**< Color space conversion mode */

    UInt32                  maxOutRes;
    /**< Display resolution */

    UInt32                  resolution;
    /**< Display resolution */

    VDIS_DC_OUTPUT_INFO_S   outputInfo;
    /**< Display output info */


} VDIS_DEV_PARAM_S;


/**
    \brief Mosaic Display Information
*/
typedef struct VDIS_MOSAIC_S
{
    WINDOW_S            displayWindow;
    /**< Target area we want to show mosaic at */

    UInt32              numberOfWindows;
    /**< Number of windows in the mosaic */

    WINDOW_S            winList[VDIS_MOSAIC_WIN_MAX];
    /**< Position and size of each window in the mosaic */

    Bool                useLowCostScaling[VDIS_MOSAIC_WIN_MAX];
    /**< Enable/Disable low cost scaling in mosaic scaling for performance */

    UInt32              chnMap[VDIS_MOSAIC_WIN_MAX];
    /**< Window to channel mapping for each window. Set to VDIS_CHN_INVALID is no channel is mapped to a window */

    Bool                onlyCh2WinMapChanged;
    /**< If only Channel to window mapping is changed then set this flag to TRUE for a smoother window mapping transition.
         If window information is changed keep this to FALSE for a smoother window mapping transition.

     */

    UInt32              outputFPS;
    /**< Rate at which output frames should be generated,
         should typically be equal to the display rate

         Example, for 60fps display, outputFPS should be 60
    */

    Bool                userSetDefaultSWMLayout; //To be Removed

} VDIS_MOSAIC_S;

/**
    \brief Display Sub-system initialization parameters
*/
typedef struct
{
    UInt32           numUserChannels;
    /**< Number of displays that are active */

    UInt32           tiedDevicesMask;
    /**< Vencs tied together. This can be a ORed value of
         VDIS_VENC_HDMI,
         VDIS_VENC_HDCOMP,
         VDIS_VENC_DVO2,
         VDIS_VENC_SD */

    UInt32           enableConfigExtThsFilter;
    /**< When FALSE do not configure THS from M3 / McFW side */

    UInt32           enableConfigExtVideoEncoder;
    /**< When FALSE do not configure Sii9022A from M3 / McFW side */

    UInt32           enableEdgeEnhancement;
    /**< Enable/Disable edge enhancement */

    UInt32 enableLayoutGridDraw;
    /**< Enable/Disable Drawing of Layout specific Grids: TRUE/FALSE */

    VDIS_DEV_PARAM_S deviceParams[VDIS_DEV_MAX];
    /**< Device parameters */

    VDIS_MOSAIC_S    mosaicParams[VDIS_DEV_MAX];
    /**< Initial mosaic for each display device */

    UInt32           numChannels;
    /**< Number of channel inputs to display device */

} VDIS_PARAMS_S;



#endif  /* __TI_VDIS_COMMON_DEF_H__ */

/* @} */
