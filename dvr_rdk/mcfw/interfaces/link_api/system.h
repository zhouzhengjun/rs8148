/*******************************************************************************
 *                                                                             *
 * Copyright (c) 2009 Texas Instruments Incorporated - http://www.ti.com/      *
 *                        ALL RIGHTS RESERVED                                  *
 *                                                                             *
 ******************************************************************************/

#ifndef _SYSTEM_H_
#define _SYSTEM_H_

/**
    \ingroup LINK_API
    \defgroup SYSTEM_LINK_API System API

    The API defined in this module are used to create links and connect
    then to each other to form a chain

    @{
*/

#include <mcfw/interfaces/link_api/system_debug.h>
#include <mcfw/interfaces/link_api/system_linkId.h>
#include <mcfw/interfaces/link_api/system_const.h>

/**
    \file system.h
    \brief System API
*/


/** \brief Floor a integer value. */
#define SystemUtils_floor(val, align)  (((val) / (align)) * (align))

/** \brief Align a integer value. */
#define SystemUtils_align(val, align)  SystemUtils_floor(((val) + (align)-1), (align))

/**
    \brief In queue params
*/
typedef struct {

    UInt32 prevLinkId;
    /**< Previous link ID to which current link will be connected */

    UInt32 prevLinkQueId;
    /**< Previous link Que ID, with which current link
        will exchange frames
    */

} System_LinkInQueParams;

/**
    \brief Out queue params
*/
typedef struct {
    UInt32 nextLink;
    /**< Next link ID to which current link will be connected */
} System_LinkOutQueParams;

/**
 * \brief Channel information
 *  Place holder to store the channel information.
 *  A destination LINK could use this information to configure itself to accept
 *  frame specified the source
 *  A Source LINK should specify the configurations of the all the channels that
 *  it will output
 *  A m2m LINK should specify the configurations of the all the channels that
 *  it will output
 */
typedef struct
{
    UInt32              bufType; /* see System_BufType */
    UInt32              codingformat;
    UInt32              dataFormat; /* see System_VideoDataFormat */
    UInt32              memType; /* see System_MemoryType - Tiled / non-tiled */
    UInt32              startX;
    UInt32              startY;
    UInt32              width;
    UInt32              height;
    UInt32              pitch[SYSTEM_MAX_PLANES];
    UInt32              scanFormat; /* see System_VideoScanFormat */
} System_LinkChInfo;


/**
 * \brief LINKs output queue information
 *  Specifies a place holder that describe the output information of the LINK
 */
typedef struct
{
    UInt32              numCh;
    /**< No of channel that would be sent out */
    System_LinkChInfo   chInfo[SYSTEM_MAX_CH_PER_OUT_QUE];
    /**< Each channels configurations */

} System_LinkQueInfo;

/**
 * \brief LINKs information
 *  Specifies a place holder that describe the LINK information
 */
typedef struct
{
    UInt32              numQue;
    /**< Number of output queue that a LINK supports */
    System_LinkQueInfo  queInfo[SYSTEM_MAX_OUT_QUE];
    /**< Each queue configurations */
} System_LinkInfo;

Int32 System_init();
Int32 System_deInit();

/**
    \brief Create a link

    Note, links of a chain should be create in
    start (source) to end (sink) order.

    Example, in a capture + display chain.

    The capture link is the source and should be created
    before the display link which is the sink for the source data.

    Create only create the link, driver, buffer memory
    and other related resources.

    Actual data transfer is not started when create is done.

    \param linkId       [IN] link ID
    \param createArgs   [IN] Create time link specific arguments

    \return FVID2_SOK on success
*/
Int32 System_linkCreate(UInt32 linkId, Ptr createArgs, UInt32 argsSize);

/**
    \brief Start the link

    The makes the link start generating or consuming data.

    Note, the order of starting links of a chain depend on
    specific link implementaion.

    \param linkId       [IN] link ID

    \return FVID2_SOK on success
*/
Int32 System_linkStart(UInt32 linkId);

/**
    \brief Stop the link

    The makes the link stop generating or consuming data.

    Note, the order of starting links of a chain depend on
    specific link implementaion.

    \param linkId       [IN] link ID

    \return FVID2_SOK on success
*/
Int32 System_linkStop(UInt32 linkId);

/**
    \brief Delete the link

    A link must be in stop state before it is deleted.

    The links of a chain can be deleted in any order.

    \param linkId       [IN] link ID

    \return FVID2_SOK on success
*/
Int32 System_linkDelete(UInt32 linkId);

/**
    \brief Send a control command to a link

    The link must be created before a control command could be sent.
    It need not be in start state for it to be able to received
    a control command

    \param linkId       [IN] link ID
    \param cmd          [IN] Link specific command ID
    \param pPrm         [IN] Link specific command parameters
    \param waitAck      [IN] TRUE: wait until link ACKs the sent command,
                             FALSE: return after sending command

    \return FVID2_SOK on success
*/
Int32 System_linkControl(UInt32 linkId, UInt32 cmd, Void *pPrm, UInt32 prmSize, Bool waitAck);

/**
    \brief Send a control command to a link

    The link must be created before a control command could be sent.
    It need not be in start state for it to be able to received
    a control command

    \param linkId       [IN] link ID
    \param cmd          [IN] Link specific command ID
    \param pPrm         [IN] Link specific command parameters
    \param waitAck      [IN] TRUE: wait until link ACKs the sent command,
                             FALSE: return after sending command
    \param timeout      [IN] Waiting time, how long funtion should wait to get 
                             ACK from recipient link.
    \return FVID2_SOK on success
*/
Int32 System_linkControlWithTimeout(UInt32 linkId, UInt32 cmd, Void *pPrm, UInt32 prmSize, Bool waitAck, UInt32 timeout);

Int32 System_linkGetInfo(UInt32 linkId, System_LinkInfo *info);

static inline void System_LinkInQueParams_Init(System_LinkInQueParams *pPrm)
{
    pPrm->prevLinkId = SYSTEM_LINK_ID_INVALID;
    pPrm->prevLinkQueId = 0;
}

#endif

/*@}*/

/**
    \defgroup MCFW_API Multi-Channel Framework (McFW) API
*/

/**
    \defgroup LINK_API TI Link API

    Links and chains APIs allow user to connect different drivers in a
    logical consistant way inorder to make a chain of data flow.

    Example,
    Capture + NSF + DEI + SC + Display
    OR
    Capture + Display

    A link is basically a task which exchange frames with other links and
    makes FVID2 driver  calls to process the frames.

    A chain is a connection of links.

    Links exchange frames with each other via buffer queue's.

    Links exchange information with each other and the top level system task
    via mail box.

    When a link is connected to another link, it basically means output queue of
    one link is connected to input que of another link.

    All links have a common minimum interface which makes it possible for a link
    to exchange frames with another link without knowing the other links specific
    details. This allow the same link to connect to different other links in
    different data flow scenario's

    Example,
    Capture can be connected to either display in the Capture + Display chain
    OR
    Capture can be connected to NSF in the Capture + NSF + DEI + SC + Display
    chain
*/


/**
    \mainpage DVR RDK

    \par IMPORTANT NOTE

     <b>
     The interfaces defined in this package are bound to change.
     Kindly treat the interfaces as work in progress.
     Release notes/user guide list the additional limitation/restriction
     of this module/interfaces.
     </b> \n
     See also \ref TI_DISCLAIMER.

    \par Introduction

    The DVR RDK is a multi-processor SDK for TI81xx platform and is optimized
    for multi-channel applications like Surveillance DVR, NVR, Hybrid-DVR, HD-DVR.
    The SDK allows a user to create different multi-channel data flows involving
    video capture, video processing (DEI, Noise Filter, Encode, Decode, SW Mosaic)
    and video display.
    The DVR-RDK is implemented using TI's Multi-Channel "Link" FrameWork (McFW).
    This document has the detailed API Reference in order to the use the McFW.

    See the following documents for more details or release specific details.
    - Release Notes - DM8168_DVR_RDK_ReleaseNotes.pdf
    - Quick Start Guide - DM8168_DVR_RDK_Quick_Start_Guide.pdf
    - Install Guide - DM8168_DVR_RDK_Install_Guide.pdf
    - Framework User Guide - DVR_RDK_McFW_UserGuide.pdf

    \par Multi-Channel FrameWork (McFW) API

     <b> See \ref MCFW_API for detailed API </b>

     <b> The user API for using DVR-RDK is via the Multi-Channel FrameWork (McFW) API. </b>

     Multi-Channel FrameWork (McFW) API allows user to setup
     and control pre-defined application specific chains for DVR, NVR, using a
     single simplified API interface. This allows users to directly use the underlying "links" framework
     without having to understand the detailed link API.
     The McFW API is platform independent and same API will work on DM816x, DM814x, DM810x.

     The McFW API is divided in four subsystems as listed below
     - \ref MCFW_SYS_API for System Level Configuration and Control
     - \ref MCFW_VCAP_API for Video Capture Configuration and Control
     - \ref MCFW_VDIS_API for Video Display Configuration and Control
     - \ref MCFW_VENC_API for Video Encode Configuration and Control
     - \ref MCFW_VDEC_API for Video Encode Configuration and Control
     - \ref MCFW_COMMON_API

    \par Link API

     <b> See \ref LINK_API for detailed API </b>

     The McFW API is built on top of the links framework. The Link API is meant to be used by advanced users for
     low level control of the hardware platform. The link API allows users to create , connect, and control links
     on HOSTA8, VPSS M3, Video M3 and DSP. Link API is used to create a chain of links
     which forms a user defined use-case. The connection of links to each other is platform dependant.

*/

/**
 \page  TI_DISCLAIMER  TI Disclaimer

 \htmlinclude ti_disclaim.htm
*/
