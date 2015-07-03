/*******************************************************************************
 *                                                                             *
 * Copyright (c) 2009 Texas Instruments Incorporated - http://www.ti.com/      *
 *                        ALL RIGHTS RESERVED                                  *
 *                                                                             *
 ******************************************************************************/

/**
    \ingroup MCFW_VENC_API

    @{
*/

/**
    \file ti_venc_common_def.h
    \brief McFW Encoder (VENC) API- Common definitions and data structures
*/

#ifndef __TI_VENC_COMMON_DEF_H__
#define __TI_VENC_COMMON_DEF_H__

/* =============================================================================
 * Enums
 * =============================================================================
 */
 /**
    \brief Encoder/Decoder compression type
*/
typedef enum VCODEC_TYPE_E
{
    VCODEC_TYPE_H264,
    /**< H264 */

    VCODEC_TYPE_MPEG4,
    /**< MPEG4 */

    VCODEC_TYPE_MJPEG,
    /**< MJPEG */

    VCODEC_TYPE_MPEG2,
    /**< MPEG2 */

    VCODEC_TYPE_MAX,
    /**< Max Codec Type */

} VCODEC_TYPE_E;
 
 /* =============================================================================
 * Defines
 * =============================================================================
 */

/* =============================================================================
 *  Structures
 * =============================================================================
 */

/**
    \brief Enc link MV data export structure

    Defines the MV Element info structure, for easy export interface

    This structure usage has been described in the H264 Codec User
    Guide, Section "Motion Vector and SAD Access API" in much detail.    
*/
typedef struct EncLink_h264_ElementInfo
{

    UInt32 startPos;
    /**< starting position of data from base address */

    UInt16 jump;
    /**< number of bytes to jump from current postion 
        *     to get next data of this element group */

    UInt32 count;
    /**< number of data elements in this group */

}EncLink_h264_ElementInfo;


/**
    \brief Enc link MV Header data export structure

    Defines the MV Header info structure, for easy export interface.

    This structure usage has been described in the H264 Codec User
    Guide, Section "Motion Vector and SAD Access API" in much detail.
*/
typedef struct EncLink_h264_AnalyticHeaderInfo { 

    UInt32 NumElements; 
    /**< Total number of elements in the buffer */

    EncLink_h264_ElementInfo elementInfoField0SAD; 

    EncLink_h264_ElementInfo elementInfoField1SAD; 

    EncLink_h264_ElementInfo elementInfoField0MVL0; 

    EncLink_h264_ElementInfo elementInfoField0MVL1; 

    EncLink_h264_ElementInfo elementInfoField1MVL0; 

    EncLink_h264_ElementInfo elementInfoField1MVL1; 

} EncLink_h264_AnalyticHeaderInfo; 

#endif


/* @} */
