/** ==================================================================
 *  @file   cameraLink_priv.h                                                  
 *                                                                    
 *  @path   /ti/psp/examples/common/iss/chains/links/camera/                                                  
 *                                                                    
 *  @desc   This  File contains.                                      
 * ===================================================================
 *  Copyright (c) Texas Instruments Inc 2011, 2012                    
 *                                                                    
 *  Use of this software is controlled by the terms and conditions found
 *  in the license agreement under which this software has been supplied
 * ===================================================================*/

/* 
 * Channel Numbers in camera link ===============================
 * 
 * Camera driver needs that 'channelNum' be unique across all camera driver
 * instances.
 * 
 * Camera link requires that 'channelNum' of frames put in the output queue
 * start from 0 and go upto ((number of channels in que) - 1).
 * 
 * This is done so that the subsequent link need not have to do any channel
 * number remapping and complexity of channel number remapping is retricted
 * within the camera link.
 * 
 * Due to this it could happen that 'channelNum' of frame returned by camera
 * driver could be, say, 0xNN, and 'channelNum' of the same frame when put in 
 * output queue needs to be, say, 0xMM.
 * 
 * This 'channelNum' remapping between camera driver and camera link output
 * queue is done as below.
 * 
 * For a given camera driver channelNum, - first 4bits
 * (CAMERA_LINK_MAX_CH_PER_OUT_QUE) is CH ID of channel in QUE and - rest of
 * the bits are output queue number 0..CAMERA_LINK_MAX_OUT_QUE-1
 * 
 * When a frame is put in the output queue only the CH ID of channel in QUE
 * is retained in the FVID2_Frame
 * 
 * When a frame is returned back to the camera driver this CH ID is combined
 * with QUE ID to form 'channelNum' for camera driver.
 * 
 * Examples,
 * 
 * For 4CH per port input and single output queue channelNum will be as
 * follows
 * 
 * Camera driver Output Output Que channelNum QUE ID channelNum 0 CH0 VIP0
 * PortA 0 0 1 CH1 VIP0 PortA 0 1 2 CH2 VIP0 PortA 0 2 3 CH3 VIP0 PortA 0 3 4 
 * CH0 VIP0 PortB 0 4 5 CH1 VIP0 PortB 0 5 6 CH2 VIP0 PortB 0 6 7 CH3 VIP0
 * PortB 0 7 8 CH0 VIP1 PortA 0 8 9 CH1 VIP1 PortA 0 9 10 CH2 VIP1 PortA 0 10
 * 11 CH3 VIP1 PortA 0 11 12 CH0 VIP1 PortB 0 12 13 CH1 VIP1 PortB 0 13 14 CH2 
 * VIP1 PortB 0 14 15 CH3 VIP1 PortB 0 15
 * 
 * For 1CH input and 2 outputs per port and 4 output queue channelNum will be
 * as follows
 * 
 * Camera driver Output Output Que channelNum QUE ID channelNum 0 CH0 VIP0
 * PortA - output 0 0 0 16 CH0 VIP0 PortA - output 1 1 0 32 CH0 VIP1 PortA -
 * output 0 2 0 64 CH0 VIP1 PortA - output 1 3 0
 * 
 * For 1CH input and 2 outputs per port and 1 output queue channelNum will be
 * as follows
 * 
 * Camera driver Output Output Que channelNum QUE ID channelNum 0 CH0 VIP0
 * PortA - output 0 0 0 1 CH0 VIP0 PortA - output 1 0 1 2 CH0 VIP1 PortA -
 * output 0 0 2 3 CH0 VIP1 PortA - output 1 0 3
 * 
 * For 1CH input and 2 outputs per port and 2 output queue channelNum will be
 * as follows
 * 
 * Camera driver Output Output Que channelNum QUE ID channelNum 0 CH0 VIP0
 * PortA - output 0 0 0 1 CH0 VIP0 PortA - output 1 0 1 16 CH0 VIP1 PortA -
 * output 0 1 0 17 CH0 VIP1 PortA - output 1 1 1
 * 
 */
#ifndef _CAMERA_LINK_PRIV_H_
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#define _CAMERA_LINK_PRIV_H_
//#include <ti/psp/iss/iss.h>
//#include <ti/psp/vps/common/vps_utils.h>
#include <mcfw/src_bios6/links_m3vpss/system/system_priv_m3vpss.h>
#include <mcfw/interfaces/link_api/cameraLink.h>
//#include <ti/sysbios/utils/Load.h>
// #include <ti/psp/platforms/utility/issutils_tsk.h>
//#include <ti/psp/examples/utility/vpsutils_tsk.h>
#ifdef PLATFORM_EVM_SI
#define CAMERA_LINK_TSK_TRIGGER_COUNT   (1)
#else
#define CAMERA_LINK_TSK_TRIGGER_COUNT   (4)
#endif

#define CAMERA_LINK_TMP_BUF_SIZE        (1920*1080*2)

#define CAMERA_LINK_HEIGHT_PAD_LINES    (48)

#define CAMERA_LINK_RAW_VBI_LINES       (40)

/* frame allocated per channel - new camera approach needs more frames */
#define CAMERA_LINK_FRAMES_PER_CH    (8)

/* Worst case frames per handle */
#define CAMERA_LINK_MAX_FRAMES_PER_HANDLE \
    (ISS_CAPT_STREAM_ID_MAX* \
     ISS_CAPT_CH_PER_PORT_MAX* \
      CAMERA_LINK_FRAMES_PER_CH \
    )

/* make camera driver channelNum */
/* ===================================================================
 *  @func     CameraLink_makeChannelNum                                               
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
static inline UInt32 CameraLink_makeChannelNum(UInt32 queId, UInt32 queChId)
{
    return CAMERA_LINK_MAX_CH_PER_OUT_QUE * queId + queChId;
}

/* extract que ID from camera driver channelNum */
/* ===================================================================
 *  @func     CameraLink_getQueId                                               
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
static inline UInt32 CameraLink_getQueId(UInt32 channelNum)
{
    return channelNum / CAMERA_LINK_MAX_CH_PER_OUT_QUE;
}

/* extract que CH ID from camera driver channelNum */
/* ===================================================================
 *  @func     CameraLink_getQueChId                                               
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
static inline UInt32 CameraLink_getQueChId(UInt32 channelNum)
{
    return channelNum % CAMERA_LINK_MAX_CH_PER_OUT_QUE;
}

/* 
 * Driver instance information */
typedef struct {

    /* 
     * VIP camera driver Instance ID
     */
    UInt32 instId;

    /* 
     * Camera driver create time parameters
     */
    Iss_CaptCreateParams createArgs;

    /* 
     * Create status returned by camera driver during FVID2_create()
     */
    Iss_CaptCreateStatus createStatus;

    /* 
     * Camera driver Callback params
     */
    FVID2_CbParams cbPrm;

    /* 
     * Camera driver FVID2 handle
     */
    FVID2_Handle cameraVipHandle;

    /* 
     * FVID2 Frames that will be used for camera
     */
    FVID2_Frame frames[CAMERA_LINK_MAX_FRAMES_PER_HANDLE];

    /* 
     * Run-time parameter structure for every frame
     * One run-time parameter is associated to one FVID2_Frame during link create
     */
    System_FrameInfo frameInfo[CAMERA_LINK_MAX_FRAMES_PER_HANDLE];

    /* 
     * Video decoder device FVID2 handle
     */
    FVID2_Handle SensorHandle;

    /* Video decoder device create args */
    Iss_SensorCreateParams sensorCreateArgs;

    /* Video decoder device create status */
    Iss_SensorCreateStatus sensorCreateStatus;

    /* Video decoder device video mode setup args */
    Iss_SensorVideoModeParams sensorVideoModeArgs;

} CameraLink_InstObj;

/* Camera link information */
typedef struct {
    /* Camera link task */
    Utils_TskHndl tsk;

    /* Camera link create arguments */
    CameraLink_CreateParams createArgs;

    /* Camera link output queues */
    Utils_BufHndl bufQue[CAMERA_LINK_MAX_OUT_QUE];

    /* Global camera driver handle */
    FVID2_Handle fvidHandleVipAll;

    /* Max possible width x height across all channels Camera buffers are
     * allocated for this width x height */
    UInt32 maxWidth;
    UInt32 maxHeight;

    /* Camera driver instance specific information */
    CameraLink_InstObj instObj[ISS_CAPT_INST_MAX];

    /* Camera link info that is returned when queried by next link */
    System_LinkInfo info;

    /* camera link run-time statistics for debug */
    UInt32 cameraDequeuedFrameCount;
    UInt32 cameraQueuedFrameCount;
    UInt32 cbCount;
    UInt32 cbCountServicedCount;
    UInt32
        cameraFrameCount[CAMERA_LINK_MAX_OUT_QUE]
        [CAMERA_LINK_MAX_CH_PER_OUT_QUE];

    UInt32 startTime;
    UInt32 prevTime;
    UInt32 prevFrameCount;
    UInt32 exeTime;

    UInt32 totalCpuLoad;
    Uint32 cpuLoadCount;

    UInt8 *tmpBufAddr;

    Bool enableCheckOverflowDetect;

    UInt32 resetCount;
    UInt32 resetTotalTime;
    UInt32 prevResetTime;

    Bool isPalMode;

    Int32 brightness;
    Int32 contrast;
    Int32 saturation;

} CameraLink_Obj;

extern CameraLink_Obj gCameraLink_obj;

/* ===================================================================
 *  @func     CameraLink_getInfo                                               
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
Int32 CameraLink_getInfo(Utils_TskHndl * pTsk, System_LinkInfo * info);

/* ===================================================================
 *  @func     CameraLink_getFullFrames                                               
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
Int32 CameraLink_getFullFrames(Utils_TskHndl * pTsk, UInt16 queId,
                               FVID2_FrameList * pFrameList);
/* ===================================================================
 *  @func     CameraLink_putEmptyFrames                                               
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
Int32 CameraLink_putEmptyFrames(Utils_TskHndl * pTsk, UInt16 queId,
                                FVID2_FrameList * pFrameList);

/* ===================================================================
 *  @func     CameraLink_drvCreate                                               
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
Int32 CameraLink_drvCreate(CameraLink_Obj * pObj,
                           CameraLink_CreateParams * pPrm);
/* ===================================================================
 *  @func     CameraLink_drvDetectVideo                                               
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
Int32 CameraLink_drvDetectVideo(CameraLink_Obj * pObj, UInt32 timeout);

/* ===================================================================
 *  @func     CameraLink_drvStart                                               
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
Int32 CameraLink_drvStart(CameraLink_Obj * pObj);

/* ===================================================================
 *  @func     CameraLink_drvProcessData                                               
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
Int32 CameraLink_drvProcessData(CameraLink_Obj * pObj);

/* ===================================================================
 *  @func     CameraLink_drvStop                                               
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
Int32 CameraLink_drvStop(CameraLink_Obj * pObj);

/* ===================================================================
 *  @func     CameraLink_drvDelete                                               
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
Int32 CameraLink_drvDelete(CameraLink_Obj * pObj);

/* ===================================================================
 *  @func     CameraLink_drvAllocAndQueueFrames                                               
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
Int32 CameraLink_drvAllocAndQueueFrames(CameraLink_Obj * pObj,
                                        CameraLink_InstObj * pDrvObj);
/* ===================================================================
 *  @func     CameraLink_drvFreeFrames                                               
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
Int32 CameraLink_drvFreeFrames(CameraLink_Obj * pObj,
                               CameraLink_InstObj * pDrvObj);

/* ===================================================================
 *  @func     CameraLink_drvIsDataFormatTiled                                               
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
UInt32 CameraLink_drvIsDataFormatTiled(Iss_CaptCreateParams * createArgs,
                                       UInt16 streamId);
/* ===================================================================
 *  @func     CameraLink_drvInitCreateArgs                                               
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
Int32 CameraLink_drvInitCreateArgs(Iss_CaptCreateParams * createArgs,
                                   CameraLink_VipInstParams * pInstPrm);

/* ===================================================================
 *  @func     CameraLink_drvPutEmptyFrames                                               
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
Int32 CameraLink_drvPutEmptyFrames(CameraLink_Obj * pObj,
                                   FVID2_FrameList * pFrameList);
/* ===================================================================
 *  @func     CameraLink_getCpuLoad                                               
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
Int32 CameraLink_getCpuLoad();

/* ===================================================================
 *  @func     CameraLink_drvPrintStatus                                               
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
Int32 CameraLink_drvPrintStatus(CameraLink_Obj * pObj);

/* ===================================================================
 *  @func     CameraLink_drvOverflowDetectAndReset                                               
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
Int32 CameraLink_drvOverflowDetectAndReset(CameraLink_Obj * pObj,
                                           Bool doForceReset);

/* ===================================================================
 *  @func     CameraLink_drvDetectVideoStandard                                               
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
Int32 CameraLink_drvDetectVideoStandard(CameraLink_Obj * pObj);

/* ===================================================================
 *  @func     CameraLink_drvSetColor                                               
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
Int32 CameraLink_drvSetColor(CameraLink_Obj * pObj, Int32 contrast,
                             Int32 brightness, Int32 saturation, Int32 hue);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
