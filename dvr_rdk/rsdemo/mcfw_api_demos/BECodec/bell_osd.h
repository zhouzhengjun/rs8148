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
#ifndef _BELL_OSD_H_
#define _BELL_OSD_H_

#include "bell_header.h"

/* The below part is temporary for OSD specific items */
#define BELL_OSD_WIN_MAX_WIDTH   (320)
#define BELL_OSD_WIN_MAX_HEIGHT  (64)

#define BELL_OSD_NUM_WINDOWS        (4)
#define BELL_OSD_NUM_BLIND_WINDOWS  (0)
#define BELL_OSD_WIN_WIDTH       (224)
#define BELL_OSD_WIN_HEIGHT      (30)
#define BELL_OSD_WIN0_STARTX     (16)
#define BELL_OSD_WIN0_STARTY     (16)

#define BELL_OSD_WIN_PITCH_H     (224)
#define BELL_OSD_WIN_PITCH_V     (30)
#define BELL_OSD_TRANSPARENCY    (1)
#define BELL_OSD_GLOBAL_ALPHA    (0x80)
#define BELL_OSD_ENABLE_WIN      (1)

extern AlgLink_OsdChWinParams g_osdChParam[];

Int32 Bell_osdInit(UInt32 numCh, UInt8 *osdFormat);
Void  Bell_osdDeinit();
#endif
