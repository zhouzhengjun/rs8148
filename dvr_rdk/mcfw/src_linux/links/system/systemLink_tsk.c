/*******************************************************************************
 *                                                                             *
 * Copyright (c) 2009 Texas Instruments Incorporated - http://www.ti.com/      *
 *                        ALL RIGHTS RESERVED                                  *
 *                                                                             *
 ******************************************************************************/


#include "systemLink_priv.h"


SystemLink_Obj gSystemLink_obj;

Int32 SystemLink_tskMain(struct OSA_TskHndl *pTsk, OSA_MsgHndl *pMsg, Uint32 curState)
{
    Int32 status = OSA_EFAIL;
    SystemLink_Obj *pObj = (SystemLink_Obj*)pTsk->appData;

    if(pObj->eventHandler!=NULL)
    {
        status = pObj->eventHandler(
                OSA_msgGetCmd(pMsg),
                OSA_msgGetPrm(pMsg),
                pObj->eventHandlerAppData
                );
    }

    OSA_tskAckOrFreeMsg(pMsg, status);

    return 0;
}

Int32 SystemLink_init()
{
    Int32 status;
    System_LinkObj linkObj;
    SystemLink_Obj  *pObj;
    char tskName[32];

    pObj = &gSystemLink_obj;

    memset(pObj, 0, sizeof(*pObj));

    pObj->tskId = SYSTEM_LINK_ID_HOST;

    linkObj.pTsk = &pObj->tsk;
    linkObj.getLinkInfo = NULL;

    pObj->tsk.appData = pObj;

    System_registerLink(pObj->tskId, &linkObj);

    sprintf(tskName, "SYSTEM_HOST%d", pObj->tskId);

    status = OSA_tskCreate(
                &pObj->tsk,
                SystemLink_tskMain,
                SYSTEM_TSK_PRI,
                SYSTEM_TSK_STACK_SIZE,
                0,
                pObj);
    UTILS_assert(  status==OSA_SOK);

    return status;
}


Int32 SystemLink_deInit()
{
    OSA_tskDelete(&gSystemLink_obj.tsk);

    return OSA_SOK;
}

Int32 Vsys_registerEventHandler(VSYS_EVENT_HANDLER_CALLBACK callback, Ptr appData)
{
    gSystemLink_obj.eventHandlerAppData = appData;
    gSystemLink_obj.eventHandler = callback;

    return 0;
}