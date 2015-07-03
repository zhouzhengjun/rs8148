/** ==================================================================
 *  @file   cameraLink_tsk.c                                                  
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
 * This file implements the state machine logic for this link. A message
 * command will cause the state machine to take some action and then move to
 * a different state.
 * 
 * The state machine table is as shown below
 * 
 * Cmds| CREATE | DETECT_VIDEO | START | NEW_DATA | STOP | DELETE | States
 * |========|==============|=======|===========|========|========| IDLE |
 * READY | - | - | - | - | - | READY| - | READY | RUN | - | READY | IDLE |
 * RUN | - | - | - | RUN | READY | IDLE |
 * 
 */

#include "cameraLink_priv.h"

/* link stack */
#pragma DATA_ALIGN(gCameraLink_tskStack, 32)
#pragma DATA_SECTION(gCameraLink_tskStack, ".bss:taskStackSection")
UInt8 gCameraLink_tskStack[CAMERA_LINK_TSK_STACK_SIZE];

/* link object, stores all link related information */
CameraLink_Obj gCameraLink_obj;

/* 
 * Run state implementation
 * 
 * In this state link cameras frames from VIP ports and sends it to the next
 * link. */
/* ===================================================================
 *  @func     CameraLink_tskRun                                               
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
Int32 CameraLink_tskRun(CameraLink_Obj * pObj, Utils_TskHndl * pTsk,
                        Utils_MsgHndl ** pMsg, Bool * done, Bool * ackMsg)
{
    Int32 status = FVID2_SOK;

    Bool runDone, runAckMsg;

    Utils_MsgHndl *pRunMsg;

    UInt32 cmd;

    Int32 value;

    /* READY loop done and ackMsg status */
    *done = FALSE;
    *ackMsg = FALSE;
    *pMsg = NULL;

    /* RUN loop done and ackMsg status */
    runDone = FALSE;
    runAckMsg = FALSE;

    /* RUN state loop */
    while (!runDone)
    {
        /* wait for message */
        status = Utils_tskRecvMsg(pTsk, &pRunMsg, BIOS_WAIT_FOREVER);
        if (status != FVID2_SOK)
            break;

        /* extract message command from message */
        cmd = Utils_msgGetCmd(pRunMsg);

        switch (cmd)
        {
            case SYSTEM_CMD_NEW_DATA:
                /* new data frames have been camerad, process them */

                /* ACK or free message before proceding */
                Utils_tskAckOrFreeMsg(pRunMsg, status);

                status = CameraLink_drvProcessData(pObj);
                if (status != FVID2_SOK)
                {
                    /* in case of error exit RUN loop */
                    runDone = TRUE;

                    /* since message is already ACK'ed or free'ed do not ACK
                     * or free it again */
                    runAckMsg = FALSE;
                }
                break;

            case CAMERA_LINK_CMD_FORCE_RESET:
                /* new data frames have been camerad, process them */

                /* ACK or free message before proceding */
                Utils_tskAckOrFreeMsg(pRunMsg, status);

                CameraLink_drvOverflowDetectAndReset(pObj, TRUE);
                break;

            case CAMERA_LINK_CMD_PRINT_ADV_STATISTICS:
                /* new data frames have been camerad, process them */

                /* ACK or free message before proceding */
                Utils_tskAckOrFreeMsg(pRunMsg, status);

                CameraLink_drvPrintStatus(pObj);

                break;
            case CAMERA_LINK_CMD_HALT_EXECUTION:

                /* ACK or free message before proceding */
                Utils_tskAckOrFreeMsg(pRunMsg, status);

                System_haltExecution();
                break;

            case CAMERA_LINK_CMD_CHANGE_BRIGHTNESS:

                value = (Int32) Utils_msgGetPrm(pRunMsg);

                /* ACK or free message before proceding */
                Utils_tskAckOrFreeMsg(pRunMsg, status);

                CameraLink_drvSetColor(pObj, value, 0, 0, 0);
                break;

            case CAMERA_LINK_CMD_CHANGE_CONTRAST:

                value = (Int32) Utils_msgGetPrm(pRunMsg);

                /* ACK or free message before proceding */
                Utils_tskAckOrFreeMsg(pRunMsg, status);

                CameraLink_drvSetColor(pObj, 0, value, 0, 0);
                break;

            case CAMERA_LINK_CMD_CHANGE_SATURATION:

                value = (Int32) Utils_msgGetPrm(pRunMsg);

                /* ACK or free message before proceding */
                Utils_tskAckOrFreeMsg(pRunMsg, status);

                CameraLink_drvSetColor(pObj, 0, 0, value, 0);
                break;

            case SYSTEM_CMD_STOP:
                /* stop RUN loop and goto READY state */
                runDone = TRUE;

                /* ACK message after actually stopping the driver outside the 
                 * RUN loop */
                runAckMsg = TRUE;
                break;
            case SYSTEM_CMD_DELETE:

                /* stop RUN loop and goto IDLE state */

                /* exit RUN loop */
                runDone = TRUE;

                /* exit READY loop */
                *done = TRUE;

                /* ACK message after exiting READY loop */
                *ackMsg = TRUE;

                /* Pass the received message to the READY loop */
                *pMsg = pRunMsg;

                break;
            default:

                /* invalid command for this state ACK it and continue RUN
                 * loop */
                Utils_tskAckOrFreeMsg(pRunMsg, status);
                break;
        }

    }

    /* RUN loop exited, stop driver */
    CameraLink_drvStop(pObj);

    /* ACK message if not ACKed earlier */
    if (runAckMsg)
        Utils_tskAckOrFreeMsg(pRunMsg, status);

    return status;
}

/* IDLE and READY state implementation */
/* ===================================================================
 *  @func     CameraLink_tskMain                                               
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
Void CameraLink_tskMain(struct Utils_TskHndl * pTsk, Utils_MsgHndl * pMsg)
{
    UInt32 cmd = Utils_msgGetCmd(pMsg);

    Bool ackMsg, done;

    Int32 status;

    CameraLink_Obj *pObj;

    /* IDLE state */

    pObj = (CameraLink_Obj *) pTsk->appData;

    if (cmd != SYSTEM_CMD_CREATE)
    {
        /* invalid command recived in IDLE status, be in IDLE state and ACK
         * with error status */
        Utils_tskAckOrFreeMsg(pMsg, FVID2_EFAIL);
        return;
    }

    /* Create command received, create the driver */

    status = CameraLink_drvCreate(pObj, Utils_msgGetPrm(pMsg));

    /* ACK based on create status */
    Utils_tskAckOrFreeMsg(pMsg, status);

    /* if create status is error then remain in IDLE state */
    if (status != FVID2_SOK)
        return;

    /* create success, entering READY state */

    done = FALSE;
    ackMsg = FALSE;

    /* READY state loop */
    while (!done)
    {
        /* wait for message */
        status = Utils_tskRecvMsg(pTsk, &pMsg, BIOS_WAIT_FOREVER);
        if (status != FVID2_SOK)
            break;

        /* extract message command from message */
        cmd = Utils_msgGetCmd(pMsg);

        switch (cmd)
        {
            case CAMERA_LINK_CMD_DETECT_VIDEO:
                /* detect video source, remain in READY state */
                status =
                    CameraLink_drvDetectVideo(pObj,
                                              (UInt32)
                                              Utils_msgGetPrm(pMsg));
                Utils_tskAckOrFreeMsg(pMsg, status);
                break;

            case SYSTEM_CMD_START:
                /* Start camera driver */
                status = CameraLink_drvStart(pObj);

                /* ACK based on create status */
                Utils_tskAckOrFreeMsg(pMsg, status);

                /* if start status is error then remain in READY state */
                if (status == FVID2_SOK)
                {
                    /* start success, entering RUN state */
                    status =
                        CameraLink_tskRun(pObj, pTsk, &pMsg, &done, &ackMsg);

                    /* done = FALSE, exit RUN state done = TRUE, exit RUN and 
                     * READY state */
                }

                break;
            case SYSTEM_CMD_DELETE:

                /* exit READY state */
                done = TRUE;
                ackMsg = TRUE;
                break;
            default:
                /* invalid command for this state ACK it and continue READY
                 * loop */
                Utils_tskAckOrFreeMsg(pMsg, status);
                break;
        }
    }

    /* exiting READY state, delete driver */
    CameraLink_drvDelete(pObj);

    /* ACK message if not previously ACK'ed */
    if (ackMsg && pMsg != NULL)
        Utils_tskAckOrFreeMsg(pMsg, status);

    /* entering IDLE state */
    return;
}

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
Int32 CameraLink_init()
{
    Int32 status;

    System_LinkObj linkObj;

    CameraLink_Obj *pObj;

    /* register link with system API */

    pObj = &gCameraLink_obj;

    memset(pObj, 0, sizeof(*pObj));

    linkObj.pTsk = &pObj->tsk;
    linkObj.linkGetFullFrames = CameraLink_getFullFrames;
    linkObj.linkPutEmptyFrames = CameraLink_putEmptyFrames;
    linkObj.getLinkInfo = CameraLink_getInfo;

    System_registerLink(SYSTEM_LINK_ID_CAMERA, &linkObj);

    /* Create link task, task remains in IDLE state CameraLink_tskMain is
     * called when a message command is received */

    status = Utils_tskCreate(&pObj->tsk,
                                CameraLink_tskMain,
                                CAMERA_LINK_TSK_PRI,
                                gCameraLink_tskStack,
                                CAMERA_LINK_TSK_STACK_SIZE, pObj, "CAMERA ");
    UTILS_assert( status == FVID2_SOK);

    return status;
}

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
Int32 CameraLink_deInit()
{
    CameraLink_Obj *pObj;

    pObj = &gCameraLink_obj;

    /* 
     * Delete link task */
    Utils_tskDelete(&pObj->tsk);

    return FVID2_SOK;
}

/* return camera link info to the next link calling this API via system API */
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
Int32 CameraLink_getInfo(Utils_TskHndl * pTsk, System_LinkInfo * info)
{
   CameraLink_Obj *pObj = (CameraLink_Obj *) pTsk->appData;

    memcpy(info, &pObj->info, sizeof(*info));

    return FVID2_SOK;
}

/* return camerad frames to the next link calling this API via system API */
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
                               FVID2_FrameList * pFrameList)
{
    CameraLink_Obj *pObj = (CameraLink_Obj *) pTsk->appData;

    UTILS_assert( queId < CAMERA_LINK_MAX_OUT_QUE);

    return Utils_bufGetFull(&pObj->bufQue[queId], pFrameList, BIOS_NO_WAIT);
}

/* release camerad frames to driver when the next link calls this API via
 * system API */
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
                                FVID2_FrameList * pFrameList)
{
    CameraLink_Obj *pObj = (CameraLink_Obj *) pTsk->appData;

    UTILS_assert( queId < CAMERA_LINK_MAX_OUT_QUE);

    return CameraLink_drvPutEmptyFrames(pObj, pFrameList);
}
