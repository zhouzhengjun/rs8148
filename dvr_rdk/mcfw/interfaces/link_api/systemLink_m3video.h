/*******************************************************************************
 *                                                                             *
 * Copyright (c) 2009 Texas Instruments Incorporated - http://www.ti.com/      *
 *                        ALL RIGHTS RESERVED                                  *
 *                                                                             *
 ******************************************************************************/

/**
    \ingroup LINK_API
    \defgroup VIDEOM3_LINK_API Video M3 Link API

    @{
*/

/**
    \file systemLink_m3video.h
    \brief Video M3 Link API
*/

#ifndef _SYSTEM_M3VIDEO_H_
#define _SYSTEM_M3VIDEO_H_

#include <mcfw/interfaces/link_api/system.h>
#include <mcfw/interfaces/link_api/systemLink_common.h>

/**
   \brief Max number of encoder or decoder video channles 
          that can be assign to any single IVA-HD
*/
#define SYSTEMVIDEO_MAX_IVACH                      (48)

#define SYSTEMVIDEO_MAX_NUMHDVICP                  (3)

/**
   \brief Data structure to assign the video channles to any single IVA-HD
*/
typedef struct SystemVideo_Ivahd2ChMap {
    UInt32 EncNumCh;  
    /**< Number of Encoder channels */
    UInt32 EncChList[SYSTEMVIDEO_MAX_IVACH];
    /**< Encoder channel list */
    UInt32 DecNumCh;
    /**< Number of Decoder channels */
    UInt32 DecChList[SYSTEMVIDEO_MAX_IVACH];
    /**< Decoder channel list */
} SystemVideo_Ivahd2ChMap;

/**
   \brief Data structure to assign the video channles to all 3 IVA-HDs
*/
typedef struct SystemVideo_Ivahd2ChMap_Tbl {
    UInt32 isPopulated;
    /**< Flag to verify if the table is populated */
    SystemVideo_Ivahd2ChMap ivaMap[SYSTEMVIDEO_MAX_NUMHDVICP];
    /**< Structure to assign the video channles to all 3 IVA-HDs */
} SystemVideo_Ivahd2ChMap_Tbl;

#endif

/* @} */
