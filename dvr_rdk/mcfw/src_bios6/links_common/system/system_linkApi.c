/*******************************************************************************
 *                                                                             *
 * Copyright (c) 2009 Texas Instruments Incorporated - http://www.ti.com/      *
 *                        ALL RIGHTS RESERVED                                  *
 *                                                                             *
 ******************************************************************************/

#include "system_priv_common.h"
#include "system_priv_ipc.h"

Int32 System_linkCreate(UInt32 linkId, Ptr createArgs, UInt32 argsSize)
{
    return System_linkControl(linkId, SYSTEM_CMD_CREATE, createArgs, argsSize,
                              TRUE);
}

Int32 System_linkStart(UInt32 linkId)
{
    return System_linkControl(linkId, SYSTEM_CMD_START, NULL, 0, TRUE);
}

Int32 System_linkStop(UInt32 linkId)
{
    return System_linkControl(linkId, SYSTEM_CMD_STOP, NULL, 0, TRUE);
}

Int32 System_linkDelete(UInt32 linkId)
{
    return System_linkControl(linkId, SYSTEM_CMD_DELETE, NULL, 0, TRUE);
}

Int32 System_linkControl(UInt32 linkId, UInt32 cmd, Void * pPrm, UInt32 prmSize,
                         Bool waitAck)
{
    Int32 status;
    UInt32 procId;

    procId = SYSTEM_GET_PROC_ID(linkId);

    UTILS_assert(procId < SYSTEM_PROC_MAX);

    if ((procId != System_getSelfProcId()) && (procId != SYSTEM_PROC_INVALID))
    {
        status = System_ipcMsgQSendMsg(linkId, cmd, pPrm, prmSize, waitAck);
    }
    else
    {
        status = System_linkControl_local(linkId, cmd, pPrm, prmSize, waitAck);
    }

    return status;
}

Int32 System_sendLinkCmd(UInt32 linkId, UInt32 cmd)
{
    Int32 status;
    UInt32 procId;

    procId = SYSTEM_GET_PROC_ID(linkId);

    UTILS_assert(procId < SYSTEM_PROC_MAX);

    if ((procId != System_getSelfProcId()) && (procId != SYSTEM_PROC_INVALID))
    {
        status = System_ipcMsgQSendMsg(linkId, cmd, NULL, 0, FALSE);
    }
    else
    {
        status = System_sendLinkCmd_local(linkId, cmd);
    }

    return status;
}

Int32 System_linkGetInfo(UInt32 linkId, System_LinkInfo * info)
{
    Int32 status;
    UInt32 procId;

    procId = SYSTEM_GET_PROC_ID(linkId);

    UTILS_assert(procId < SYSTEM_PROC_MAX);

    if ((procId != System_getSelfProcId()) && (procId != SYSTEM_PROC_INVALID))
    {
        status =
            System_ipcMsgQSendMsg(linkId, SYSTEM_CMD_GET_INFO, info,
                                  sizeof(*info), TRUE);
    }
    else
    {
        status = System_linkGetInfo_local(linkId, info);
    }

    return status;
}

Int32 System_getLinksFullFrames(UInt32 linkId, UInt16 queId,
                                FVID2_FrameList * pFrameList)
{
    System_LinkObj *pTsk;

    linkId = SYSTEM_GET_LINK_ID(linkId);

    UTILS_assert(linkId < SYSTEM_LINK_ID_MAX);

    pTsk = &gSystem_objCommon.linkObj[linkId];

    if (pTsk->linkGetFullFrames != NULL)
        return pTsk->linkGetFullFrames(pTsk->pTsk, queId, pFrameList);

    return FVID2_EFAIL;
}

Int32 System_putLinksEmptyFrames(UInt32 linkId, UInt16 queId,
                                 FVID2_FrameList * pFrameList)
{
    System_LinkObj *pTsk;

    linkId = SYSTEM_GET_LINK_ID(linkId);

    UTILS_assert(linkId < SYSTEM_LINK_ID_MAX);

    pTsk = &gSystem_objCommon.linkObj[linkId];

    if (pTsk->linkPutEmptyFrames != NULL)
        return pTsk->linkPutEmptyFrames(pTsk->pTsk, queId, pFrameList);

    return FVID2_EFAIL;
}

Utils_TskHndl *System_getLinkTskHndl(UInt32 linkId)
{
    System_LinkObj *pTsk;

    linkId = SYSTEM_GET_LINK_ID(linkId);

    UTILS_assert(linkId < SYSTEM_LINK_ID_MAX);

    pTsk = &gSystem_objCommon.linkObj[linkId];

    return pTsk->pTsk;
}

Int32 System_getLinksFullBufs(UInt32 linkId, UInt16 queId,
                              Bitstream_BufList * pBufList)
{
    System_LinkObj *pTsk;

    linkId = SYSTEM_GET_LINK_ID(linkId);

    UTILS_assert(linkId < SYSTEM_LINK_ID_MAX);

    pTsk = &gSystem_objCommon.linkObj[linkId];

    if (pTsk->linkGetFullBitBufs != NULL)
        return pTsk->linkGetFullBitBufs(pTsk->pTsk, queId, pBufList);

    return FVID2_EFAIL;
}

Int32 System_putLinksEmptyBufs(UInt32 linkId, UInt16 queId,
                               Bitstream_BufList * pBufList)
{
    System_LinkObj *pTsk;

    linkId = SYSTEM_GET_LINK_ID(linkId);

    UTILS_assert(linkId < SYSTEM_LINK_ID_MAX);

    pTsk = &gSystem_objCommon.linkObj[linkId];

    if (pTsk->linkPutEmptyBitBufs != NULL)
        return pTsk->linkPutEmptyBitBufs(pTsk->pTsk, queId, pBufList);

    return FVID2_EFAIL;
}

Int32 System_registerLink(UInt32 linkId, System_LinkObj * pTskObj)
{
    linkId = SYSTEM_GET_LINK_ID(linkId);

    UTILS_assert(linkId < SYSTEM_LINK_ID_MAX);

    memcpy(&gSystem_objCommon.linkObj[linkId], pTskObj, sizeof(*pTskObj));

    return FVID2_SOK;
}
