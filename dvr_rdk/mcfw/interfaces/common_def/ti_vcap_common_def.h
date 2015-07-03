/*******************************************************************************
 *                                                                             *
 * Copyright (c) 2009 Texas Instruments Incorporated - http://www.ti.com/      *
 *                        ALL RIGHTS RESERVED                                  *
 *                                                                             *
 ******************************************************************************/

/**
    \ingroup MCFW_VCAP_API

    @{
*/

/**
    \file ti_vcap_common_def.h
    \brief McFW Video Capture (VCAP) API- Common definitions and data structures
*/

#ifndef __TI_VCAP_COMMON_DEF_H__
#define __TI_VCAP_COMMON_DEF_H__


/** Maximum Video Capture channels */
#define VCAP_CHN_MAX        (16)

typedef struct
{
  UInt32 chId;
  /**< channel ID */

  UInt32 vipInstId;
  /**< VIP instance ID */

  UInt32 isVideoDetect;
  /**<  TRUE: Video signal is present, FALSE: no video signal */

  UInt32 frameWidth;
  /**< Frame or field width in pixels
   *
   * This is detected video signal frame or field width.
   *
   * Further change in width or height due to additional
   * cropping, scaling like CIF, HALF-D1 is not accoutned for in this field
  */

  UInt32 frameHeight;
  /**< Frame or field height in lines
   *
   * This is detected video signal frame or field height.
   *
   * Further change in width or height due to additional
   * cropping, scaling like CIF, HALF-D1 is not accoutned for in this field
  */

  UInt32 frameInterval;
  /**< Interval between two fields or frames in micro-seconds */

  UInt32 isInterlaced;
  /**< TRUE: Source is Interlaced, FALSE: Source is Progressive */

} VCAP_VIDEO_SOURCE_CH_STATUS_S;


typedef struct {

    UInt32 channelNum;

} VCAP_VIDEO_SOURCE_STATUS_PARAMS_S;


typedef struct {

    UInt32 numChannels;

    VCAP_VIDEO_SOURCE_CH_STATUS_S chStatus[VCAP_CHN_MAX];

} VCAP_VIDEO_SOURCE_STATUS_S;



#endif


/* @} */
