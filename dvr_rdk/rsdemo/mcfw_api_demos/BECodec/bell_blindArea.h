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

#ifndef _BELL_BLINDAREA_H_
#define _BELL_BLINDAREA_H_

#include "bell_header.h"

#define BELL_BLIND_AREA_NUM_WINDOWS     (4)
#define BELL_BLIND_AREA_WIN_WIDTH       (50)
#define BELL_BLIND_AREA_WIN_HEIGHT      (20)
#define BELL_BLIND_AREA_WIN0_STARTX     (512)
#define BELL_BLIND_AREA_WIN0_STARTY     (16)

extern AlgLink_OsdChBlindWinParams g_osdChBlindParam[];
extern CaptureLink_BlindInfo g_blindAreaChParam[];

Int32 Bell_blindAreaInit(UInt32 numCh);
#endif
