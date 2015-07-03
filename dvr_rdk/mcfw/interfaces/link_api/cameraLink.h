/** ==================================================================
 *  @file   cameraLink.h                                                  
 *                                                                    
 *  @path   /ti/psp/examples/common/iss/chains/links/                                                  
 *                                                                    
 *  @desc   This  File contains.                                      
 * ===================================================================
 *  Copyright (c) Texas Instruments Inc 2011, 2012                    
 *                                                                    
 *  Use of this software is controlled by the terms and conditions found
 *  in the license agreement under which this software has been supplied
 * ===================================================================*/

/**
    \ingroup ISSEXAMPLE_LINKS_AND_CHAIN_API
    \defgroup ISSEXAMPLE_CAMERA_LINK_API Camera Link API

    Camera Link can be used to instantiate camera upto
    ISS_CAPT_INST_MAX VIP instances.

    Each instance can have upto two outputs.

    The frames from these camera outputs can be put in upto
    four output queue.

    Each output queue can inturn to be connected to a link like
    display or DEI or NSF.

    @{
*/

/**
    \file cameraLink.h
    \brief Camera Link API
*/

#ifndef _CAMERA_H_
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#define _CAMERA_H_
#include <mcfw/interfaces/link_api/system.h>

//#include <ti/psp/examples/common/iss/chains/links/system.h>
//#include <ti/psp/iss/iss_capture.h>
//#include <ti/psp/devices/iss_sensorDriver.h>

/** \brief Max outputs per VIP instance */
#define CAMERA_LINK_MAX_OUTPUT_PER_INST  (2)

/** \brief Max output queues in the camera link */
#define CAMERA_LINK_MAX_OUT_QUE          (2)

/** \brief Max Channels per output queue */
#define CAMERA_LINK_MAX_CH_PER_OUT_QUE   (1)

/**
    \brief Link CMD: Detect camera video source format

    This command can make the camera link wait until video
    source is detect on all the expected input video sources

    \param UInt32 timeout  [IN] BIOS_WAIT_FOREVER or BIOS_NO_WAIT
*/
#define CAMERA_LINK_CMD_DETECT_VIDEO    (0x1000)

/** \brief Command to detect overflow and force reset */
#define CAMERA_LINK_CMD_FORCE_RESET     (0x1001)

/** \brief Command to print advanced statistics */
#define CAMERA_LINK_CMD_PRINT_ADV_STATISTICS   (0x1002)

/** \brief Command to change brightness by provided value */
#define CAMERA_LINK_CMD_CHANGE_BRIGHTNESS      (0x1003)

/** \brief Command to change contrast by provided value */
#define CAMERA_LINK_CMD_CHANGE_CONTRAST        (0x1004)

/** \brief Command to change saturation by provided value */
#define CAMERA_LINK_CMD_CHANGE_SATURATION      (0x1005)

/** \brief Command to halt execution */
#define CAMERA_LINK_CMD_HALT_EXECUTION         (0x1006)

/** \brief Command to change resolution by provided value */
#define CAMERA_LINK_CMD_CHANGE_RESOLUTION      (0x1007)

/**
    \brief Camera output parameters
*/
typedef struct {
    UInt32 dataFormat;
    /**< output data format, YUV422, YUV420, RGB */

    Bool scEnable;
    /**< TRUE: enable scaling, FALSE: disable scaling */

    UInt32 scOutWidth;
    /**< Scaler output width */

    UInt32 scOutHeight;
    /**< Scaler output height */

    UInt32 outQueId;
    /**< Link output que ID to which this VIP instance output frames are put */

} CameraLink_OutParams;

/**
    \brief VIP instance information
*/
typedef struct {
    UInt32 vipInstId;
    /**< VIP camera driver instance ID */

    UInt32 SensorId;
    /**< Video decoder instance ID */

    UInt32 inDataFormat;
    /**< Input source data format, RGB or YUV422 */

    UInt32 standard;
    /**< Required video standard */

    UInt32 numOutput;
    /**< Number of outputs */

    CameraLink_OutParams outParams[CAMERA_LINK_MAX_OUTPUT_PER_INST];
    /**< Information about each output */

} CameraLink_VipInstParams;

/**
    \brief Camera Link create parameters
*/
typedef struct {
    UInt16 numVipInst;
    /**< Number of VIP instances in this link */

    CameraLink_VipInstParams vipInst[SYSTEM_CAMERA_INST_MAX];
    /**< VIP instance information */

    System_LinkOutQueParams outQueParams[CAMERA_LINK_MAX_OUT_QUE];
    /**< Output queue information */

    Bool tilerEnable;
    /**< Enable/disable tiler */

} CameraLink_CreateParams;

/**
    \brief Camera link register and init

    - Creates link task
    - Registers as a link with the system API

    \return FVID2_SOK on success
*/
/* ===================================================================
 *  @func     CameraLink_init                                               
 *                                                                    
 *  @desc     Function does the following                             
 *                                                                    
 *  @modif    This function modifies the following structures         
 *                                                                    
 *  @inputs   This function takes the following inputs                
 *            <argument name>                                         
 *            Description of usage                                    
 *            <argument name>                                         
 *            Description of usage                                    
 *                                                                    
 *  @outputs  <argument name>                                         
 *            Description of usage                                    
 *                                                                    
 *  @return   Return value of this function if any                    
 *  ==================================================================
 */                                                                   
Int32 CameraLink_init();

/**
    \brief Camera link de-register and de-init

    - Deletes link task
    - De-registers as a link with the system API

    \return FVID2_SOK on success
*/
/* ===================================================================
 *  @func     CameraLink_deInit                                               
 *                                                                    
 *  @desc     Function does the following                             
 *                                                                    
 *  @modif    This function modifies the following structures         
 *                                                                    
 *  @inputs   This function takes the following inputs                
 *            <argument name>                                         
 *            Description of usage                                    
 *            <argument name>                                         
 *            Description of usage                                    
 *                                                                    
 *  @outputs  <argument name>                                         
 *            Description of usage                                    
 *                                                                    
 *  @return   Return value of this function if any                    
 *  ==================================================================
 */                                                                   
Int32 CameraLink_deInit();

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif

/* @} */
