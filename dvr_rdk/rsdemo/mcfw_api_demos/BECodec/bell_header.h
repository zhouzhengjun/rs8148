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

#ifndef _BELL_HEADER_H_
#define _BELL_HEADER_H_

#include <osa.h>
#include <osa_thr.h>
#include <osa_sem.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>
#include <ctype.h>
#include <sys/wait.h>
#include <sys/types.h>  // For stat().
#include <sys/stat.h>   // For stat().
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>

#include "ti_vdis_common_def.h"
#include "ti_vdec_common_def.h"

#include "ti_vsys.h"
#include "ti_vcap.h"
#include "ti_venc.h"
#include "ti_vdec.h"
#include "ti_vdis.h"
#include "ti_vdis_timings.h"

/* To select if FBDEV interface is used for Graphics */
#define USE_FBDEV   1

typedef enum {
    BELL_TYPE_PROGRESSIVE,
    BELL_TYPE_INTERLACED
}bellType;

typedef struct {

    UInt32 width;
    UInt32 height;

} Bell_res;


#endif
