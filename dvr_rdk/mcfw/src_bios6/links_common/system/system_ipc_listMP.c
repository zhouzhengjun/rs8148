/*******************************************************************************
 *                                                                             *
 * Copyright (c) 2009 Texas Instruments Incorporated - http://www.ti.com/      *
 *                        ALL RIGHTS RESERVED                                  *
 *                                                                             *
 ******************************************************************************/

#include "system_priv_common.h"
#include "system_priv_ipc.h"


Int32 System_ipcListMPReset(ListMP_Handle pOutHndl, ListMP_Handle pInHndl)
{
    while (!ListMP_empty(pOutHndl))
    {
        ListMP_getHead(pOutHndl);
    }
    while (!ListMP_empty(pInHndl))
    {
        ListMP_getHead(pInHndl);
    }

    return FVID2_SOK;
}

Int32 System_ipcListMPCreate(UInt32 regionId, UInt32 linkId,
                             ListMP_Handle * pOutHndl, ListMP_Handle * pInHndl,
                             GateMP_Handle * pOutHndleGate, GateMP_Handle * pInHndleGate)
{
    Int32 status;
    char listMPOutName[64];
    char listMPInName[64];
    ListMP_Params listMPParams;
    GateMP_Params gateMPParams;

    status = System_ipcGetListMPName(linkId, listMPOutName, listMPInName);
    UTILS_assert(status == FVID2_SOK);

    GateMP_Params_init(&gateMPParams);
    gateMPParams.regionId = regionId;
    gateMPParams.remoteProtect = GateMP_RemoteProtect_CUSTOM1;
    *pOutHndleGate = GateMP_create(&gateMPParams);
    UTILS_assert(*pOutHndleGate != NULL);

    ListMP_Params_init(&listMPParams);

    listMPParams.name = listMPOutName;
    listMPParams.regionId = regionId;
    listMPParams.gate     = *pOutHndleGate;

    Vps_printf(" %d: SYSTEM: Creating ListMP [%s] in region %d ...\n",
               Utils_getCurTimeInMsec(), listMPParams.name, listMPParams.regionId);

    *pOutHndl = ListMP_create(&listMPParams);

    UTILS_assert(*pOutHndl != NULL);

    GateMP_Params_init(&gateMPParams);
    gateMPParams.regionId = regionId;
    gateMPParams.remoteProtect = GateMP_RemoteProtect_CUSTOM1;
    *pInHndleGate = GateMP_create(&gateMPParams);
    UTILS_assert(*pInHndleGate != NULL);

    ListMP_Params_init(&listMPParams);

    listMPParams.name = listMPInName;
    listMPParams.regionId = regionId;
    listMPParams.gate     = *pInHndleGate;

    Vps_printf(" %d: SYSTEM: Creating ListMP [%s] in region %d ...\n",
               Utils_getCurTimeInMsec(), listMPParams.name, listMPParams.regionId);

    *pInHndl = ListMP_create(&listMPParams);

    UTILS_assert(*pInHndl != NULL);

    return status;
}

Int32 System_ipcListMPOpen(UInt32 linkId, ListMP_Handle * pOutHndl,
                           ListMP_Handle * pInHndl)
{
    Int32 retryCount;
    Int32 status;
    char listMPOutName[64];
    char listMPInName[64];

    status = System_ipcGetListMPName(linkId, listMPOutName, listMPInName);
    UTILS_assert(status == FVID2_SOK);

    retryCount = 10;
    while (retryCount)
    {
        Vps_printf(" %d: SYSTEM: Opening ListMP [%s] ...\n",
                   Utils_getCurTimeInMsec(), listMPOutName);

        status = ListMP_open(listMPOutName, pOutHndl);
        if (status == ListMP_E_NOTFOUND)
            Task_sleep(1000);
        else if (status == ListMP_E_FAIL)
            UTILS_assert(0);
        else if (status == ListMP_S_SUCCESS)
            break;

        retryCount--;
        if (retryCount <= 0)
            UTILS_assert(0);

    }

    retryCount = 10;
    while (retryCount)
    {
        Vps_printf(" %d: SYSTEM: Opening ListMP [%s] ...\n",
                   Utils_getCurTimeInMsec(), listMPInName);

        status = ListMP_open(listMPInName, pInHndl);
        if (status == ListMP_E_NOTFOUND)
            Task_sleep(1000);
        else if (status == ListMP_E_FAIL)
            UTILS_assert(0);
        else if (status == ListMP_S_SUCCESS)
            break;

        retryCount--;
        if (retryCount <= 0)
            UTILS_assert(0);
    }

    return status;
}

Int32 System_ipcListMPClose(ListMP_Handle * pOutHndl, ListMP_Handle * pInHndl)
{
    Int32 status;

    status = ListMP_close(pOutHndl);
    UTILS_assert(status == ListMP_S_SUCCESS);

    status = ListMP_close(pInHndl);
    UTILS_assert(status == ListMP_S_SUCCESS);

    return status;
}

Int32 System_ipcListMPDelete(ListMP_Handle * pOutHndl, ListMP_Handle * pInHndl,
                             GateMP_Handle * pOutHndleGate, GateMP_Handle * pInHndleGate)
{
    Int32 status;

    status = ListMP_delete(pOutHndl);
    UTILS_assert(status == ListMP_S_SUCCESS);

    status = ListMP_delete(pInHndl);
    UTILS_assert(status == ListMP_S_SUCCESS);

    status = GateMP_delete(pOutHndleGate);
    UTILS_assert(status == GateMP_S_SUCCESS);

    status = GateMP_delete(pInHndleGate);
    UTILS_assert(status == GateMP_S_SUCCESS);

    return status;
}

UInt32 System_ipcListMPAllocListElemMem(UInt32 regionId, UInt32 size)
{
    IHeap_Handle heapHndl;
    Error_Block eb;
    UInt32 shAddr;

    heapHndl = SharedRegion_getHeap(regionId);
    UTILS_assert(heapHndl != NULL);

    Error_init(&eb);
    shAddr = (UInt32) Memory_alloc((IHeap_Handle) heapHndl, size, 0, &eb);
    UTILS_assert(shAddr != NULL);

    Vps_printf(" %d: SYSTEM: ListElem Shared Addr = 0x%08x\n",
               Utils_getCurTimeInMsec(), shAddr);

    return shAddr;
}

Int32 System_ipcListMPFreeListElemMem(UInt32 regionId, UInt32 shAddr,
                                      UInt32 size)
{
    IHeap_Handle heapHndl;

    heapHndl = SharedRegion_getHeap(regionId);
    UTILS_assert(heapHndl != NULL);

    Memory_free(heapHndl, (Ptr) shAddr, size);

    return FVID2_SOK;
}
