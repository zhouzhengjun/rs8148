/*******************************************************************************
 *                                                                             *
 * Copyright (c) 2009 Texas Instruments Incorporated - http://www.ti.com/      *
 *                        ALL RIGHTS RESERVED                                  *
 *                                                                             *
 ******************************************************************************/

#include <mcfw/src_bios6/utils/utils_bit_buf.h>

Int32 Utils_bitbufCreate(Utils_BitBufHndl * pHndl,
                         Bool blockOnGet, Bool blockOnPut, UInt32 numAllocPools)
{
    Int32 status;
    UInt32 flags;
    Int i;

    flags = UTILS_QUE_FLAG_NO_BLOCK_QUE;

    if (blockOnGet)
        flags |= UTILS_QUE_FLAG_BLOCK_QUE_GET;
    if (blockOnPut)
        flags |= UTILS_QUE_FLAG_BLOCK_QUE_PUT;

    for (i = 0; i < numAllocPools; i++)
    {
        status = Utils_queCreate(&(pHndl->emptyQue[i]),
                                 UTILS_BUF_MAX_QUE_SIZE,
                                 pHndl->emptyQueMem[i], flags);
        UTILS_assert(status == FVID2_SOK);
    }
    pHndl->numAllocPools = numAllocPools;
    status = Utils_queCreate(&pHndl->fullQue,
                             UTILS_BUF_MAX_QUE_SIZE, pHndl->fullQueMem, flags);
    UTILS_assert(status == FVID2_SOK);

    return status;
}

Int32 Utils_bitbufDelete(Utils_BitBufHndl * pHndl)
{
    Int i;

    for (i = 0; i < pHndl->numAllocPools; i++)
    {
        Utils_queDelete(&(pHndl->emptyQue[i]));
    }
    pHndl->numAllocPools = 0;
    Utils_queDelete(&pHndl->fullQue);

    return FVID2_SOK;
}

Int32 Utils_bitbufGetEmpty(Utils_BitBufHndl * pHndl,
                           Bitstream_BufList * pBufList, UInt32 allocPoolID,
                           UInt32 timeout)
{
    UInt32 idx, maxBufs;
    Int32 status;

    UTILS_assert(pHndl != NULL);
    UTILS_assert(pBufList != NULL);

    if (timeout == BIOS_NO_WAIT)
        maxBufs = VIDBITSTREAM_MAX_BITSTREAM_BUFS;
    else
        maxBufs = pBufList->numBufs;

    UTILS_assert(maxBufs <= VIDBITSTREAM_MAX_BITSTREAM_BUFS);
    UTILS_assert(allocPoolID < pHndl->numAllocPools);

    for (idx = 0; idx < maxBufs; idx++)
    {
        status = Utils_queGet(&(pHndl->emptyQue[allocPoolID]),
                              (Ptr *) & pBufList->bufs[idx], 1, timeout);
        if (status != FVID2_SOK)
            break;
        pBufList->bufs[idx]->allocPoolID = allocPoolID;
    }

    pBufList->numBufs = idx;

    return FVID2_SOK;
}

Int32 Utils_bitbufGetEmptyBuf(Utils_BitBufHndl * pHndl,
                              Bitstream_Buf ** pBuf,
                              UInt32 allocPoolID, UInt32 timeout)
{
    Int32 status;

    UTILS_assert(pHndl != NULL);
    UTILS_assert(pBuf != NULL);

    *pBuf = NULL;
    UTILS_assert(allocPoolID < pHndl->numAllocPools);
    status = Utils_queGet(&(pHndl->emptyQue[allocPoolID]),
                          (Ptr *) pBuf, 1, timeout);
    if (status == FVID2_SOK)
    {
        (*pBuf)->allocPoolID = allocPoolID;
    }
    return status;
}

Int32 Utils_bitbufPutEmpty(Utils_BitBufHndl * pHndl,
                           Bitstream_BufList * pBufList)
{
    UInt32 idx;
    Int32 status;

    UTILS_assert(pHndl != NULL);
    UTILS_assert(pBufList != NULL);
    UTILS_assert(pBufList->numBufs <= VIDBITSTREAM_MAX_BITSTREAM_BUFS);

    for (idx = 0; idx < pBufList->numBufs; idx++)
    {
        UInt32 allocPoolID = pBufList->bufs[idx]->allocPoolID;

        UTILS_assert(pBufList->bufs[idx]->allocPoolID < pHndl->numAllocPools);
        status = Utils_quePut(&(pHndl->emptyQue[allocPoolID]),
                              pBufList->bufs[idx], BIOS_NO_WAIT);
        UTILS_assert(status == FVID2_SOK);
    }

    return FVID2_SOK;
}

Int32 Utils_bitbufPutEmptyBuf(Utils_BitBufHndl * pHndl, Bitstream_Buf * pBuf)
{
    Int32 status;

    UTILS_assert(pHndl != NULL);
    UTILS_assert(pBuf->allocPoolID < pHndl->numAllocPools);

    status = Utils_quePut(&(pHndl->emptyQue[pBuf->allocPoolID]),
                          pBuf, BIOS_NO_WAIT);
    UTILS_assert(status == FVID2_SOK);

    return FVID2_SOK;
}

Int32 Utils_bitbufGetFull(Utils_BitBufHndl * pHndl,
                          Bitstream_BufList * pBufList, UInt32 timeout)
{
    UInt32 idx, maxBufs;
    Int32 status;

    UTILS_assert(pHndl != NULL);
    UTILS_assert(pBufList != NULL);

    if (timeout == BIOS_NO_WAIT)
        maxBufs = FVID2_MAX_FVID_FRAME_PTR;
    else
        maxBufs = pBufList->numBufs;

    UTILS_assert(maxBufs <= FVID2_MAX_FVID_FRAME_PTR);

    for (idx = 0; idx < maxBufs; idx++)
    {
        status = Utils_queGet(&pHndl->fullQue, (Ptr *) & pBufList->bufs[idx], 1,
                              timeout);
        if (status != FVID2_SOK)
            break;
    }

    pBufList->numBufs = idx;

    return FVID2_SOK;
}

Int32 Utils_bitbufGetFullBuf(Utils_BitBufHndl * pHndl,
                             Bitstream_Buf ** pBuf, UInt32 timeout)
{
    Int32 status;

    UTILS_assert(pHndl != NULL);
    UTILS_assert(pBuf != NULL);

    *pBuf = NULL;

    status = Utils_queGet(&pHndl->fullQue, (Ptr *) pBuf, 1, timeout);

    return status;
}

Int32 Utils_bitbufPutFull(Utils_BitBufHndl * pHndl,
                          Bitstream_BufList * pBufList)
{
    UInt32 idx;
    Int32 status;

    UTILS_assert(pHndl != NULL);
    UTILS_assert(pBufList != NULL);
    UTILS_assert(pBufList->numBufs <= FVID2_MAX_FVID_FRAME_PTR);

    for (idx = 0; idx < pBufList->numBufs; idx++)
    {
        status =
            Utils_quePut(&pHndl->fullQue, pBufList->bufs[idx], BIOS_NO_WAIT);
        UTILS_assert(status == FVID2_SOK);
    }

    return FVID2_SOK;
}

Int32 Utils_bitbufPutFullBuf(Utils_BitBufHndl * pHndl, Bitstream_Buf * pBuf)
{
    Int32 status;

    UTILS_assert(pHndl != NULL);

    status = Utils_quePut(&pHndl->fullQue, pBuf, BIOS_NO_WAIT);
    if (status != FVID2_SOK)
    {
#if 0
        Vps_rprintf
            ("%d: ERROR: In Utils_bitbufPutFullBuf(), Utils_quePut() failed !!!\n",
             Utils_getCurTimeInMsec());
#endif
    }

    return status;
}


Void Utils_bitbufPrintStatus(UInt8 *str, Utils_BitBufHndl * pHndl)
{
    Uint8 i;

    Vps_printf(" %s BitBuf Q Status\n", str);
    for (i=0; i<pHndl->numAllocPools; i++)
    {
        Vps_printf("Empty Q %d -> count %d, wrPtr %d, rdPtr %d\n", i, pHndl->emptyQue[i].count, pHndl->emptyQue[i].curWr, pHndl->emptyQue[i].curRd);
    }
    Vps_printf("Full Q -> count %d, wrPtr %d, rdPtr %d\n", pHndl->fullQue.count, pHndl->fullQue.curWr, pHndl->fullQue.curRd);
}

