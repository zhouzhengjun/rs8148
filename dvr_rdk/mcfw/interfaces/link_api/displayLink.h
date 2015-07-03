/*******************************************************************************
 *                                                                             *
 * Copyright (c) 2009 Texas Instruments Incorporated - http://www.ti.com/      *
 *                        ALL RIGHTS RESERVED                                  *
 *                                                                             *
 ******************************************************************************/

/**
    \ingroup LINK_API
    \defgroup DISPLAY_LINK_API Display Link API

    Display Link can be used to instantiate non-mosiac display
    over HDMI, HDDAC or SDTV.

    For each of the display a different display link instance needs to be
    created using the system API.

    The display link can take input for a single input queue.
    The single input can contain multiple channels but since this is
    a non-mosiac display only one of the channel can be shown at a time.

    By default CH0 is shown on the display.

    User can use the command DISPLAY_LINK_CMD_SWITCH_CH to switch the
    channel that is displayed on the diplay

    @{
*/

/**
    \file displayLink.h
    \brief Display Link API

*/

#ifndef _DISPLAY_LINK_H_
#define _DISPLAY_LINK_H_

#include <mcfw/interfaces/link_api/system.h>

#define DISPLAY_LINK_MAX_NUM_INPUT_QUEUES        (2)

/**
    \brief Link CMD: Switch channel that is being displayed

    \param UInt32 chId  [IN] channel ID to display
*/
#define DISPLAY_LINK_CMD_SWITCH_CH            (0x4000)

#define DISPLAY_LINK_CMD_PRINT_STATISTICS     (0x4001)

#define DISPLAY_LINK_CMD_SWITCH_ACTIVE_QUEUE  (0x4002)

#define DISPLAY_LINK_CMD_CHANGE_RESOLUTION    (0x4003)

#define DISPLAY_LINK_CMD_STOP_DRV           (0x4004)

#define DISPLAY_LINK_CMD_START_DRV          (0x4005)

/**
    \brief Link CMD: Switch input mode

    \param DisplayLink_SwitchInputMode * [IN]
*/
#define DISPLAY_LINK_CMD_SWITCH_INPUT_MODE  (0x4006)

#define DISPLAY_LINK_CMD_PRINT_BUFFER_STATISTICS  (0x4007)

/**
    \brief Display link create parameters
*/
typedef struct
{
    UInt32                   numInputQueues;
    /**< Display link number of input queues */

    UInt32                   activeQueue;
    /**< Active queueID from which frames should be displayed */

    System_LinkInQueParams   inQueParams[DISPLAY_LINK_MAX_NUM_INPUT_QUEUES];
    /**< Display link input information */

    UInt32                   displayRes;
    /** Display resolution ID, SYSTEM_RES_xxx */

    UInt32                   forceFieldSeparatedInputMode;
    /** force field separate interlaced mode */

} DisplayLink_CreateParams;

typedef struct {
    UInt32 resolution;
    /**< Display link resolution information */
} DisplayLink_RtParams;


/**
    \brief Display link switch queue params
*/
typedef struct DisplayLink_SwitchActiveQueueParams
{
    UInt32                   activeQueueId;
    /**< Active queueID from which frames should be displayed */
} DisplayLink_SwitchActiveQueueParams;

/**
    \brief Display SDTV channel switch params
*/
typedef struct DisplayLink_SwitchChannelParams
{
    UInt32                   activeChId;
    /**< Active chID from which frames should be displayed */
} DisplayLink_SwitchChannelParams;


/**
    \brief Switch input mode between progressive input,
    interlaced display OR interlaced input, interlaced display.

    Only valid for interlaced displays.
    Only valid if DisplayLink_CreateParams.forceFieldSeparatedInputMode = TRUE

    No effect other wise.
*/
typedef struct {

    Bool enableFieldSeparatedInputMode;
    /**<
        TRUE: interlaced input, interlaced display
        FALSE: progressive input, interlaced display
    */
} DisplayLink_SwitchInputMode;

/**
    \brief Display link register and init

    For each display instance (HDMI, HDDAC, SDTV)
    - Creates link task
    - Registers as a link with the system API

    \return FVID2_SOK on success
*/
Int32 DisplayLink_init();

/**
    \brief Display link de-register and de-init

    For each display instance (HDMI, HDDAC, SDTV)
    - Deletes link task
    - De-registers as a link with the system API

    \return FVID2_SOK on success
*/
Int32 DisplayLink_deInit();

/**
 * \fn DisplayLink_CreateParams_Init
 * \Sets default values for Display link
 *
 */
static inline Void DisplayLink_CreateParams_Init(DisplayLink_CreateParams *prm)
{
    memset(prm, 0, sizeof(*prm));

    prm->numInputQueues = 1;
    prm->activeQueue    = 0;
    prm->forceFieldSeparatedInputMode = FALSE;

    memset(prm->inQueParams,0,sizeof(prm->inQueParams));
}

#endif

/*@}*/
