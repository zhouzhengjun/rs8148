/*******************************************************************************
 *                                                                             *
 * Copyright (c) 2009 Texas Instruments Incorporated - http://www.ti.com/      *
 *                        ALL RIGHTS RESERVED                                  *
 *                                                                             *
 ******************************************************************************/

/**
    \ingroup LINK_API
    \defgroup NULL_SOURCE_LINK_API Null Source Link API

    Video Src Link can be used to provide input to the next frames
    This is used to integrate other links when capture link is not available.

    This is useful when capture link is not used but some input is needed for other links


    @{
*/

/**
    \file NullSrclink.h
    \brief Video Source Link API
*/

#ifndef _NULL_SRC_LINK_H_
#define _NULL_SRC_LINK_H_

#include <mcfw/interfaces/link_api/system.h>


/** \brief Max Channels per output queue */
#define NULL_SRC_LINK_MAX_CH_PER_OUT_QUE   (16)

/**
*    brief Null Video source link create parameters
*/
typedef struct
{
    System_LinkOutQueParams   outQueParams;
    /**< output queue information */

    System_LinkQueInfo        inputInfo;

    UInt32                    timerPeriod;

    /**< Decoder link channel create params */
    UInt32                    tilerEnable;

} NullSrcLink_CreateParams;

/**
    \brief Video source link register and init

    - Creates link task
    - Registers as a link with the system API

    \return FVID2_SOK on success
*/
Int32 NullSrcLink_init();

/**
    \brief Null Video source link de-register and de-init

    - Deletes link task
    - De-registers as a link with the system API

    \return FVID2_SOK on success
*/
Int32 NullSrcLink_deInit();


#endif

/*@}*/

