/*******************************************************************************
 *                                                                             *
 * Copyright (c) 2009 Texas Instruments Incorporated - http://www.ti.com/      *
 *                        ALL RIGHTS RESERVED                                  *
 *                                                                             *
 ******************************************************************************/

/**
    \ingroup LINK_API
    \defgroup MERGE_LINK_API Frame Merge (MERGE) Link API

    @{
*/

/**
    \file mergeLink.h
    \brief Frame Merge (MERGE) Link API
*/

#ifndef _MERGE_LINK_H_
#define _MERGE_LINK_H_

#include <mcfw/interfaces/link_api/system.h>

/** \brief Max input queues to which a given MERGE link can connect to */
#define MERGE_LINK_MAX_IN_QUE	(6)

/**
    \brief MERGE link create parameters
*/
typedef struct
{
    UInt32 numInQue;
    /**< Number of inputs queue's */

    System_LinkInQueParams   inQueParams[MERGE_LINK_MAX_IN_QUE];
    /**< Input queue information */

    System_LinkOutQueParams   outQueParams;
    /**< Output queue information */

	UInt32 notifyNextLink;
	/**< TRUE: send command to next link notifying that new data is ready in que */

} MergeLink_CreateParams;

/**
    \brief MERGE link register and init

    - Creates link task
    - Registers as a link with the system API

    \return FVID2_SOK on success
*/
Int32 MergeLink_init();

/**
    \brief MERGE link de-register and de-init

    - Deletes link task
    - De-registers as a link with the system API

    \return FVID2_SOK on success
*/
Int32 MergeLink_deInit();

#endif

/*@}*/
