/*******************************************************************************
 *                                                                             *
 * Copyright (c) 2009 Texas Instruments Incorporated - http://www.ti.com/      *
 *                        ALL RIGHTS RESERVED                                  *
 *                                                                             *
 ******************************************************************************/

/**
    \ingroup ALG_LINK_API
    \defgroup ALG_OSD_LINK_API ALG Link: OSD API

    @{
*/

/**
    \file osdLink.h
    \brief ALG Link: OSD API
*/

#ifndef _OSD_LINK_H_
#define _OSD_LINK_H_

/* Defines */

/* @{ */

/**
    \brief Max supported OSD windows
*/
#define ALG_LINK_OSD_MAX_WINDOWS (10)

/**
    \brief Max supported OSD channels
*/
#define ALG_LINK_OSD_MAX_CH      (48)

/* @} */

/* Control Commands */

/**
    \ingroup ALG_LINK_API_CMD
    \addtogroup ALG_OSD_LINK_API_CMD  ALG Link: OSD API Control Commands

    @{
*/

/**
    \brief Link CMD: Configure Osd params

    SUPPORTED in ALL platforms

    \param AlgLink_OsdChWinParams * [IN] OSD Window parameters
*/
#define ALG_LINK_OSD_CMD_SET_CHANNEL_WIN_PRM            (0x6001)


/**
    \brief Link CMD: Set blind window params

    ONLY SUPPORTED in DM810x

    \param AlgLink_OsdChBlindWinParams * [IN] Blind window parameters
*/
#define ALG_LINK_OSD_CMD_SET_CHANNEL_BLIND_WIN_PRM      (0x6002)

/* @}  */

/* Data structure's */

/**
  \brief OSD window parameter's
*/
typedef struct
{
  UInt8  *addr[2][2];
  /**< OSD buffer pointer

        MUST be physical address

        When data format is YUV422
        addr[0][0] - YUV422 plane pointer

        When data format is YUV420
        addr[0][0] - Y-plane pointer
        addr[0][1] - C-plane pointer

        Other arrary element not used.
    */

  UInt32 format;
  /**< SYSTEM_DF_YUV420SP_UV or SYSTEM_DF_YUV422I_YUYV

    SYSTEM_DF_YUV422I_YUYV
    - ONLY SUPPORTED in DM816x, DM814x

    SYSTEM_DF_YUV420SP_UV
    - SUPPORTED in ALL platforms
  */

  UInt32 startX;
  /**< Start position of window in X direction,
        specified in pixels,
        relative to start of video window,
        must be multiple of 2
    */

  UInt32 startY;
  /**< Start position of window in Y direction,
        specified in lines,
        relative to start of video window,
        must be multiple of 2
    */

  UInt32 width;
  /**< Width of window,
        specified in pixels,
        must be multiple of 4
    */

  UInt32 height;
  /**< Height of window,
        specified in pixels,
        must be multiple of 4
    */


  UInt32 lineOffset;
  /**<
        Line offset in pixels,
        must be >= width, must be multiple of 4,
        recommended to be multiple of 32 for DDR efficiency
    */

  UInt32 globalAlpha;
  /**<
        8-bit global Alpha Value, used only if Alpha window is not enabled,
        Set to 0 to disable alpha blend.

        0   : Min Alpha, show only video
        0x80: Max Alpha, show only OSD

        Supported in DM816x, DM814x
        - on per CH, per Window basis.
        - can be changed dynamically

        Supported in DM810x with the following restrictions,
        - Value specified for CH0, WIN0 is applied for all CHs and all windows
        - Value specified during create time is used and cannot be updated at run-time
    */

  UInt32 transperencyEnable;
  /**<
        TRUE: enable transperency,

        when OSD pixel = colorKey
        then
            video is shown
        else
            OSD pixel is blend with Video taking into account globalAlpha

        FALSE: disable transperency

        OSD pixel is always blended with Video taking into account globalAlpha

        Supported in DM816x, DM814x
        - on per CH, per Window basis.
        - can be changed dynamically

        Supported in DM810x with the following restrictions,
        - Value specified for CH0, WIN0 is applied for all CHs and all windows
        - Value specified during create time is used and cannot be updated at run-time
    */

  UInt32 enableWin;
   /**< TRUE: Enable display of OSD window for the channel
        FALSE: Disable display of OSD window for the channel
    */

} AlgLink_OsdWindowPrm;


/**
    \brief OSD channel - window parameters

    Specifies OSD parameters that can be changed dynamically
    on a per channel basis

    See structure/field details for restrictions applicable for DM810x platform.
*/
typedef struct AlgLink_OsdChWinParams
{
    UInt32 chId;
    /**< OSD channel number.
         Valid values: 0 .. ALG_LINK_OSD_MAX_CH-1
    */

    UInt32 numWindows;
    /**< Number of OSD windows for this channel.
         Valid values: 0..ALG_LINK_OSD_MAX_WINDOWS
    */

    AlgLink_OsdWindowPrm winPrm[ALG_LINK_OSD_MAX_WINDOWS];
    /**< OSD window parameters */

    UInt32  colorKey[3];
    /**< Color key for Y, U, V.
        This is used when AlgLink_OsdWindowPrm.transperencyEnable = TRUE
    */

} AlgLink_OsdChWinParams;

/**
  \brief OSD blind window parameter's

    Blind window is rectangle which is drawn on top of the video to cover
    the specified video region.

    This is useful in application like privacy masking.
*/
typedef struct
{
  UInt32 startX;
  /**< Start position of window in X direction,
        specified in pixels,
        relative to start of video window,
        must be multiple of 2
    */

  UInt32 startY;
  /**< Start position of window in Y direction,
        specified in lines,
        relative to start of video window,
        must be multiple of 2
    */

  UInt32 width;
  /**< Width of window,
        specified in pixels,
        must be multiple of 4
    */

  UInt32 height;
  /**< Height of window,
        specified in pixels,
        must be multiple of 4
    */

  UInt32 fillColorYUYV;
  /**< Windows color in YUYV format */

  UInt32 enableWin;
  /**< TRUE: Draw this blind area,
        FALSE: Do not draw this area
    */
} AlgLink_OsdBlindWindowPrm;

/**
    \brief OSD channel - blind window parameters

    Specifies blind window parameters that can be changed dynamically
    on a per channel basis

    SUPPORTED only in DM810x
*/
typedef struct
{
    UInt32                  chId;
    /**< OSD channel number.
         Valid values: 0 .. ALG_LINK_OSD_MAX_CH-1
    */

    UInt32                  numWindows;
    /**< Number of blind windows for this channel.
         Valid values: 0..ALG_LINK_OSD_MAX_WINDOWS
    */

    AlgLink_OsdBlindWindowPrm    winPrm[ALG_LINK_OSD_MAX_WINDOWS];
    /**< Blind window parameters */

} AlgLink_OsdChBlindWinParams;

/**
    \brief OSD channel create time configuration parameters
*/
typedef struct
{
    UInt32 maxWidth;
    /**< Set the max width of OSD window */

    UInt32 maxHeight;
    /**< Set the max height of OSD window */

    AlgLink_OsdChWinParams chDefaultParams;
    /**< Initial window params for all channels */

} AlgLink_OsdChCreateParams;


#endif

/*@}*/

