/*******************************************************************************
 *                                                                             *
 * Copyright (c) 2009 Texas Instruments Incorporated - http://www.ti.com/      *
 *                        ALL RIGHTS RESERVED                                  *
 *                                                                             *
 ******************************************************************************/

/**
    \ingroup UTILS_API
    \defgroup UTILS_BUF_API Frame buffer exchange API

    APIs defined in this file are used by links-and-chains example to exchange
    frames between two tasks

    Internally this consists of two queues
    - empty or input queue
    - full or output queue

    The queue implementation uses fixed size array based queue data structure,
    with mutual exclusion built inside the queue implementation.

    Optional blocking of Get and/or Put operation is possible

    The element that can be inserted/extracted from the queue is of
    type FVID2_Frame *

    The basic operation in the example is as below

    - When a producer task needs to output some data, it first 'gets' a empty frame
      to output the data from the buffer handle.
    - The task outputs the data to the empty frame
    - The task then 'puts' this data as full data into the buffer handle
    - The consumer task, then 'gets' this full frame from the buffer handle
    - After using or consuming this frame, it 'puts' this frame as empty frame into
      this buffer handle.
    - This way frames are exchanged between a producer and consumer.

    @{
*/

/**
    \file utils_buf.h
    \brief Frame buffer exchange API
*/

#ifndef _UTILS_BUF_H_
#define _UTILS_BUF_H_

#include <mcfw/src_bios6/utils/utils_que.h>

#define UTILS_BUF_MAX_QUE_SIZE       (384)

/**
    \brief Frame Buffer Handle
*/
typedef struct {

    Utils_QueHandle emptyQue;
    /**< Empty or input queue */

    Utils_QueHandle fullQue;
    /**< Full or output queue */

    FVID2_Frame *emptyQueMem[UTILS_BUF_MAX_QUE_SIZE];
    /**< Memory for empty que data - video frames */

    FVID2_Frame *fullQueMem[UTILS_BUF_MAX_QUE_SIZE];
    /**< Memory for empty que data - video frames */

} Utils_BufHndl;

/**
    \brief Create a frame buffer handle

    When blockOnGet/blockOnPut is TRUE a semaphore gets allocated internally.
    In order to reduce resource usuage keep this as FALSE if application
    doesnt plan to use the blocking API feature.

    \param pHndl        [OUT] Created handle
    \param blockOnGet   [IN]  Enable blocking on 'get' API
    \param blockOnPut   [IN]  Enable blocking on 'put' API

    \return FVID2_SOK on success, else failure
*/
Int32 Utils_bufCreate(Utils_BufHndl * pHndl, Bool blockOnGet, Bool blockOnPut);

/**
    \brief Delete frame buffer handle

    Free's resources like semaphore allocated during create

    \param pHndl    [IN] Buffer handle

    \return FVID2_SOK on success, else failure
*/
Int32 Utils_bufDelete(Utils_BufHndl * pHndl);

/**
    \brief Get frame's from empty queue

    This API is used to get multiple frames in a single API call.
    FVID2_FrameList.numFrames is set to number of frames
    that are returned.

    When during create
    - 'blockOnGet' = TRUE
      - timeout can be BIOS_WAIT_FOREVER or BIOS_NO_WAIT
    - 'blockOnGet' = FALSE
      - timeout must be BIOS_NO_WAIT

    \param pHndl        [IN] Buffer handle
    \param pFrameList   [OUT] Frame's returned by the API
    \param timeout      [IN] BIOS_NO_WAIT or BIOS_WAIT_FOREVER

    \return FVID2_SOK on success, else failure
*/
Int32 Utils_bufGetEmpty(Utils_BufHndl * pHndl,
                        FVID2_FrameList * pFrameList, UInt32 timeout);

/**
    \brief Put frame's into full queue

    This API is used to return multiple frames in a single API call.
    FVID2_FrameList.numFrames is set to number of frames
    that are to be returned.

    When during create
    - 'blockOnPut' = TRUE
      - API will block until space is available in the queue to put the frames
    - 'blockOnPut' = FALSE
      - API will return error in case space is not available in the queue
        to put the frames

    \param pHndl        [IN] Buffer handle
    \param pFrameList   [IN] Frame's to be put
    \param timeout      [IN] BIOS_NO_WAIT or BIOS_WAIT_FOREVER

    \return FVID2_SOK on success, else failure
*/
Int32 Utils_bufPutFull(Utils_BufHndl * pHndl, FVID2_FrameList * pFrameList);

/**
    \brief Get frame's from full queue

    This API is used to get multiple frame's in a single API call.
    FVID2_FrameList.numFrames is set to number of frames
    that are returned.

    When during create
    - 'blockOnGet' = TRUE
      - timeout can be BIOS_WAIT_FOREVER or BIOS_NO_WAIT
    - 'blockOnGet' = FALSE
      - timeout must be BIOS_NO_WAIT

    \param pHndl        [IN] Buffer handle
    \param pFrameList   [OUT] Frame's returned by the API
    \param timeout      [IN] BIOS_NO_WAIT or BIOS_WAIT_FOREVER

    \return FVID2_SOK on success, else failure
*/
Int32 Utils_bufGetFull(Utils_BufHndl * pHndl,
                       FVID2_FrameList * pFrameList, UInt32 timeout);

/**
    \brief Put frame's into empty queue

    This API is used to return multiple frames in a single API call.
    FVID2_FrameList.numFrames is set to number of frames
    that are to be returned.

    When during create
    - 'blockOnPut' = TRUE
      - API will block until space is available in the queue to put the frames
    - 'blockOnPut' = FALSE
      - API will return error in case space is not available in the queue
        to put the frames

    \param pHndl        [IN] Buffer handle
    \param pFrameList   [IN] Frame's to be put
    \param timeout      [IN] BIOS_NO_WAIT or BIOS_WAIT_FOREVER

    \return FVID2_SOK on success, else failure
*/
Int32 Utils_bufPutEmpty(Utils_BufHndl * pHndl, FVID2_FrameList * pFrameList);

/**
    \brief Get a frame from empty queue

    Same as Utils_bufGetEmpty() except that only a single frame is returned

    \param pHndl        [IN] Buffer handle
    \param pFrame       [OUT] Frame that is returned by the API
    \param timeout      [IN] BIOS_NO_WAIT or BIOS_WAIT_FOREVER

    \return FVID2_SOK on success, else failure
*/
Int32 Utils_bufGetEmptyFrame(Utils_BufHndl * pHndl,
                             FVID2_Frame ** pFrame, UInt32 timeout);

/**
    \brief Get a frame from full queue

    Same as Utils_bufGetFull() except that only a single frame is returned

    \param pHndl        [IN] Buffer handle
    \param pFrame       [OUT] Frame that is returned by the API
    \param timeout      [IN] BIOS_NO_WAIT or BIOS_WAIT_FOREVER

    \return FVID2_SOK on success, else failure
*/
Int32 Utils_bufGetFullFrame(Utils_BufHndl * pHndl,
                            FVID2_Frame ** pFrame, UInt32 timeout);

/**
    \brief Put a frame into full queue

    Same as Utils_bufPutFull() except that only a single frame is put

    \param pHndl        [IN] Buffer handle
    \param pFrame       [OUT] Frame that is to be returned to the queue
    \param timeout      [IN] BIOS_NO_WAIT or BIOS_WAIT_FOREVER

    \return FVID2_SOK on success, else failure
*/
Int32 Utils_bufPutFullFrame(Utils_BufHndl * pHndl, FVID2_Frame * pFrame);

/**
    \brief Print buffer status of full & empty queues

    \param str            [IN] prefix string to print
    \param pHndl        [IN] Buffer handle

    \return None
*/
Void Utils_bufPrintStatus(UInt8 *str, Utils_BufHndl * pHndl);

/**
    \brief Put a frame into empty queue

    Same as Utils_bufPutEmpty() except that only a single frame is put

    \param pHndl        [IN] Buffer handle
    \param pFrame       [OUT] Frame that is to be returned to the queue
    \param timeout      [IN] BIOS_NO_WAIT or BIOS_WAIT_FOREVER

    \return FVID2_SOK on success, else failure
*/
Int32 Utils_bufPutEmptyFrame(Utils_BufHndl * pHndl, FVID2_Frame * pFrame);

/**
    \brief Peek into empty queue

    This only peeks at the top of the queue but does not remove the
    frame from the queue

    \param pHndl        [IN] Buffer handle

    \return frame pointer is frame is present in the queue, else NULL
*/
static inline FVID2_Frame *Utils_bufPeekEmpty(Utils_BufHndl * pHndl)
{
    FVID2_Frame *pFrame;

    Utils_quePeek(&pHndl->emptyQue, (Ptr *) & pFrame);

    return pFrame;
}

/**
    \brief Peek into full queue

    This only peeks at the top of the queue but does not remove the
    frame from the queue

    \param pHndl        [IN] Buffer handle

    \return frame pointer is frame is present in the queue, else NULL
*/
static inline FVID2_Frame *Utils_bufPeekFull(Utils_BufHndl * pHndl)
{
    FVID2_Frame *pFrame;

    Utils_quePeek(&pHndl->fullQue, (Ptr *) & pFrame);

    return pFrame;
}

#endif

/* @} */

/**
    \defgroup EXAMPLE_API Sample Example API

    The API defined in this module are utility APIs OUTSIDE of the FVID2 drivers.

    Example code makes use of these APIs to implement sample application which
    demonstrate the driver in different ways.
*/

/**
    \ingroup EXAMPLE_API
    \defgroup UTILS_API Sample Example - Utility library API
*/
