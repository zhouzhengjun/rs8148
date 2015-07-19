/*******************************************************************************
 *                                                                             *
 * Copyright (c) 2009 Texas Instruments Incorporated - http://www.ti.com/      *
 *                        ALL RIGHTS RESERVED                                  *
 *                                                                             *
 ******************************************************************************/

/**
    \ingroup SYSTEM_LINK_API

    @{
*/

/**
    \file system_debug.h
    \brief System debug API

    Modify this file to enable/disable prints from different links.

    When SYSTEM_DEBUG_xxx_RT  is enabled a print is done for every frame.
    In real-time system such prints may slow down the system and hence
    are intended to used only for debug purposes.
*/

#ifndef _SYSTEM_DEBUG_H_
#define _SYSTEM_DEBUG_H_


#if defined(TI814X_DVR)
#define HDVPSS_VER_01_00_01_36
#endif

#if defined(TI_814X_BUILD) || defined(TI_8107_BUILD)
//#undef SYSTEM_USE_TILER
#define SYSTEM_USE_TILER
//#define SYSTEM_USE_VIDEO_DECODER
//#define SYSTEM_DISABLE_AUDIO
#else
#define SYSTEM_USE_TILER
#endif
/* Uncomment to Disable tiler at build time */
//#undef SYSTEM_USE_TILER

/* define this to use I2C from M3 side  */
#if defined(TI8107_DVR)
#define SYSTEM_USE_VIDEO_DECODER
#endif

/* RPE Integration with DSP */
#if !defined(TI_8107_BUILD)
 #define DSP_RPE_AUDIO_ENABLE
#endif

/* Define SYSTEM_DISABLE_AUDIO set by build options */
#ifndef SYSTEM_DISABLE_AUDIO
#define SYSTEM_ENABLE_AUDIO
#endif

//#define __DEBUG_ENABLE
#ifdef __DEBUG_ENABLE //zhangtao
//#define SYSTEM_DEBUG_VIP_RES_ALLOC
//#define SYSTEM_DEBUG_TILER
//#define SYSTEM_DEBUG_CMD_ERROR

#define SYSTEM_DEBUG
#define SYSTEM_DEBUG_CAMERA
#define SYSTEM_DEBUG_CAPTURE
#define SYSTEM_DEBUG_NSF
#define SYSTEM_DEBUG_DEI
#define SYSTEM_DEBUG_DISPLAY
#define SYSTEM_DEBUG_DISPLAY_HWMS
#define SYSTEM_DEBUG_SCALAR
#define SYSTEM_DEBUG_NULL
#define SYSTEM_DEBUG_DUP
#define SYSTEM_DEBUG_SWMS
#define SYSTEM_DEBUG_SCLR
#define SYSTEM_DEBUG_IPC
#define SYSTEM_DEBUG_MERGE
#define SYSTEM_DEBUG_ENC
#define SYSTEM_DEBUG_DEC
#define SYSTEM_DEBUG_SCD
#define SYSTEM_DEBUG_SELECT
#define SYSTEM_DEBUG_GRPX
#define SYSTEM_DEBUG_AVSYNC_DETAILED_LOGS

//#define SYSTEM_DEBUG_MEMALLOC
//#define SYSTEM_DEBUG_DISABLE_PROCESS_MULTI
//#define SYSTEM_VERBOSE_PRINTS
//#define SYSTEM_DEBUG_RT
#endif

/* enabling avg log print for DM810x ONLY by default */
#if defined(TI_8107_BUILD)
//#define SYSTEM_PRINT_RT_AVG_STATS_LOG
#endif

#define SYSTEM_RT_STATS_LOG_INTERVAL    (10) /* in secs */

#ifdef SYSTEM_DEBUG_RT
    #define SYSTEM_DEBUG_CAPTURE_RT
    #define SYSTEM_DEBUG_NSF_RT
    #define SYSTEM_DEBUG_DEI_RT
    #define SYSTEM_DEBUG_DISPLAY_RT
    #define SYSTEM_DEBUG_SCALAR_RT
    #define SYSTEM_DEBUG_NULL_RT
    #define SYSTEM_DEBUG_SWMS_RT
    #define SYSTEM_DEBUG_SCLR_RT
    #define SYSTEM_DEBUG_IPC_RT
    #define SYSTEM_DEBUG_ENC_RT
    #define SYSTEM_DEBUG_DEC_RT
#endif

#endif

/* @} */
