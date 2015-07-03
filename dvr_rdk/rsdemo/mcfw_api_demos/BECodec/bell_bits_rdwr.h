/*******************************************************************************
 *                                                                             *
 *      Copyright (c) 2013 BroadEng Beijing - http://www.broadeng.net/         *
 *                        ALL RIGHTS RESERVED                                  *
 *                                                                             * 
 *******************************************************************************
 *                                                                             *
 * Author:      Zhang Tao <zhangtao@broadeng.net>                              *
 * Version:     v1.0.0                                                         *
 *                                                                             *
 ******************************************************************************/

#ifndef _BELL_BITS_RDWR_H_
#define _BELL_BITS_RDWR_H_

#include "bell_api.h"
#include "bell_header.h"

#define IPCBITS_RESOLUTION_TYPES               (2)  /* Bit bufs can be D1 & CIF resolution in some usecases */

Int32 Bell_resetStatistics();
Int32 Bell_updateStatistics(VCODEC_BITSBUF_S *pBuf);

Int32 Bell_resetAvgStatistics();
Int32 Bell_printAvgStatistics(UInt32 elaspedTime, Bool resetStats);

void* Bell_ipcBitsInit(Bell_res resolution[]);
Int32 Bell_ipcBitsExit();

int Bell_ipcBitsGet(void *ipcHdl, VCODEC_BITSBUF_LIST_S *buflist);
int Bell_ipcBitsPut(VCODEC_BITSBUF_LIST_S *buflist);

VCODEC_BITSBUF_S* Bell_ipcGetEmptyBuf(void *ipcHdl, int size);
int Bell_ipcBitsFeed(void *ipcHdl, VCODEC_BITSBUF_S *pFullBufInfo);

#endif
