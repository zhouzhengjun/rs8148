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

#ifndef __TI_VDEC_COMMON_DEF_H__
#define __TI_VDEC_COMMON_DEF_H__


/**
    \brief Dec link decoder error message

*/
typedef struct VDEC_CH_ERROR_MSG
{
    Bool reportA8;
    /**< Flag to notify A8 about decoder error */

    Int32 chId;
    /**< Decoder channel number */

    Int32 frameNum;
    /**< Decoder channel number. Reserved for future use */

    Int32 errorMsg;
    /**< Decoder error message */

    UInt32 upperTimeStamp;
    /**< Time stamp:Upper 32 bit value. Reserved for future use*/

    UInt32 lowerTimeStamp;
    /**< Time stamp: Lower 32 bit value. Reserved for future use*/
} VDEC_CH_ERROR_MSG;


#endif


/* @} */
