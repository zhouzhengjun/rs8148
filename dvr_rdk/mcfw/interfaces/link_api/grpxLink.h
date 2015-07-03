/*******************************************************************************
 *                                                                             *
 * Copyright (c) 2009 Texas Instruments Incorporated - http://www.ti.com/      *
 *                        ALL RIGHTS RESERVED                                  *
 *                                                                             *
 ******************************************************************************/

/**
    \ingroup LINK_API
    \defgroup GRPX_LINK_API GRPX Link API

    This will start the GRPX plane displays and keeps displaying the
    TI logo on top of the video planes.

    @{
*/

/**
    \file grpxLink.h
    \brief GRPX Link API
*/

#ifndef _GRPX_LINK_H
#define _GRPX_LINK_H

#include <mcfw/interfaces/link_api/system.h>
#include <mcfw/interfaces/common_def/ti_vgrpx_common_def.h>

/*

    \param GrpxLink_DynamicParams * [IN]
*/
#define GRPX_LINK_CMD_SET_DYNAMIC_PARAMS     (0xB001)

/*

    \param GrpxLink_BufferInfo * [OUT]
*/
#define GRPX_LINK_CMD_GET_BUFFER_INFO        (0xB002)




Int32 GrpxLink_init();
Int32 GrpxLink_deInit();


#endif  /* _GRPX_LINK_H */

/*@}*/
