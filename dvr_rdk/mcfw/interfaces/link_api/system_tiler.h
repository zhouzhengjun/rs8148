/*******************************************************************************
 *                                                                             *
 * Copyright (c) 2009 Texas Instruments Incorporated - http://www.ti.com/      *
 *                        ALL RIGHTS RESERVED                                  *
 *                                                                             *
 ******************************************************************************/

/**
    \ingroup LINK_API

    \defgroup SYSTEM_TILER_API Tiler allocator API

    @{
*/

/**
    \file system_tiler.h
    \brief  Tiler allocator API
*/

#ifndef _SYSTEM_TILER_H_
#define _SYSTEM_TILER_H_

#include <mcfw/interfaces/link_api/system.h>
#include <mcfw/interfaces/link_api/systemLink_common.h>

#define SYSTEM_TILER_CNT_FIRST  (0)
#define SYSTEM_TILER_CNT_8BIT   (SYSTEM_TILER_CNT_FIRST)
#define SYSTEM_TILER_CNT_16BIT  (1)
#define SYSTEM_TILER_CNT_32BIT  (2)
#define SYSTEM_TILER_CNT_LAST   (SYSTEM_TILER_CNT_32BIT)
#define SYSTEM_TILER_CNT_MAX    (SYSTEM_TILER_CNT_LAST + 1)

#define SYSTEM_TILER_INVALID_ADDR              ((UInt32)~(0u))

#define SYSTEM_TILER_ALLOCATOR_PROC_ID              (SYSTEM_PROC_M3VPSS)

UInt32  SystemTiler_alloc(UInt32 cntMode, UInt32 width, UInt32 height);
Int32  SystemTiler_free(UInt32 tileAddr);
Int32  SystemTiler_freeAll(void);
UInt32  SystemTiler_isAllocatorDisabled();
Int32  SystemTiler_disableAllocator(void);
Int32  SystemTiler_enableAllocator(void);
Ptr SystemTiler_allocRaw(UInt32 size,UInt32 align);
Int32 SystemTiler_freeRaw(Ptr addr, UInt32 size);
Int32 SystemTiler_getFreeSize(SystemCommon_TilerGetFreeSize *pPrm);

#endif /* _SYSTEM_TILER_H_ */


/* @} */


