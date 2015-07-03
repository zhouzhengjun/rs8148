/*******************************************************************************
 *                                                                             *
 * Copyright (c) 2009 Texas Instruments Incorporated - http://www.ti.com/      *
 *                        ALL RIGHTS RESERVED                                  *
 *                                                                             *
 ******************************************************************************/

/**
    \ingroup LINK_API

    \defgroup SYSTEM_COMMON_LINK_API System Common Link API

    @{
*/

/**
    \file systemLink_common.h
    \brief System Common Link Link API
*/


#ifndef _SYSTEM_LINK_COMMON_H_
#define _SYSTEM_LINK_COMMON_H_

#include <mcfw/interfaces/link_api/system.h>


/*
    \param NONE
*/
#define SYSTEM_COMMON_CMD_CPU_LOAD_CALC_START       (0x9000)

/*
    \param NONE
*/
#define SYSTEM_COMMON_CMD_CPU_LOAD_CALC_STOP       (0x9001)

/*
    \param NONE
*/
#define SYSTEM_COMMON_CMD_CPU_LOAD_CALC_RESET      (0x9002)

/*
    \param NONE
*/
#define SYSTEM_COMMON_CMD_CPU_LOAD_CALC_PRINT      (0x9003)

/*
    \param SystemCommon_PrintStatus *
*/
#define SYSTEM_COMMON_CMD_PRINT_STATUS             (0x9004)

/*
    \param [IN][OUT] SystemCommon_TilerAlloc *
*/
#define SYSTEM_COMMON_CMD_TILER_ALLOC               (0x9005)

/*
    \param [IN] SystemCommon_TilerFree *
*/
#define SYSTEM_COMMON_CMD_TILER_FREE                (0x9006)

/*
    \param [IN] NONE
*/
#define SYSTEM_COMMON_CMD_TILER_FREE_ALL            (0x9007)

/*
    \param [IN] SystemVideo_Ivahd2ChMap_Tbl[] *
*/
#define SYSTEM_COMMON_CMD_SET_CH2IVAHD_MAP_TBL      (0x9008)

/*
 * \param NONE
*/
#define SYSTEM_COMMON_CMD_TILER_DISABLE_ALLOCATOR   (0x9009)


/*
 * \param NONE
*/
#define SYSTEM_COMMON_CMD_TILER_ENABLE_ALLOCATOR    (0x900A)


/*
 * \param NONE
*/
#define SYSTEM_COMMON_CMD_TILER_IS_ALLOCATOR_DISABLED  (0x900B)

/*
    \param [IN][OUT] SystemCommon_TilerAllocRaw *
*/
#define SYSTEM_COMMON_CMD_TILER_ALLOC_RAW              (0x900C)

/*
    \param [IN] SystemCommon_TilerFreeRaw *
*/
#define SYSTEM_COMMON_CMD_TILER_FREE_RAW              (0x900D)


/*
    \param [IN] SystemCommon_TilerGetFreeSize    *
*/
#define SYSTEM_COMMON_CMD_TILER_GET_FREE_SIZE         (0x900E)

/*
    \param [IN] System command to check core status    *
*/
#define SYSTEM_COMMON_CMD_CORE_STATUS         (0x900F)

typedef struct {

    UInt32 printCpuLoad;
    UInt32 printTskLoad;
    UInt32 printHeapStatus;

} SystemCommon_PrintStatus;

typedef struct {
    UInt32 cntMode;   /**< [IN] Tiler container mode 8/16/32/PG */
    UInt32 width;     /**< [IN] Frame width */
    UInt32 height;    /**< [IN] Frame height */
    UInt32 tileAddr;  /**< [OUT] Tiled buf addr */
} SystemCommon_TilerAlloc;


typedef struct {
    UInt32 tileAddr;  /**< [IN] Tiled buf addr to be freed */
} SystemCommon_TilerFree;

typedef struct SystemCommon_TilerIsDisabled {
    UInt32 isAllocatorDisabled;  /**< [OUT] Flag indicating if tiler allocator is disabled */
} SystemCommon_TilerIsDisabled;

typedef struct SystemCommon_TilerAllocRaw {
    UInt32 size;      /**< [IN] Size in bytes to be alloced */
    UInt32 align;     /**< [IN] Alignment bytes */
    UInt32 allocAddr; /**< [OUT] Allocated address */
} SystemCommon_TilerAllocRaw;

typedef struct SystemCommon_TilerFreeRaw {
    UInt32 size;      /**< [IN] Size in bytes to be alloced */
    UInt32 allocAddr; /**< [IN] Allocated address */
} SystemCommon_TilerFreeRaw;


typedef struct SystemCommon_TilerGetFreeSize {

    UInt32 freeSize8b;      /**<  [IN] Free Size in bytes for 8-bit container,  when tiler is Enabled */
    UInt32 freeSize16b;      /**< [IN] Free Size in bytes for 16-bit container, when tiler is Enabled */
    UInt32 freeSize32b;      /**< [IN] Free Size in bytes for 24-bit container, when tiler is Enabled */
    UInt32 freeSizeRaw;      /**< [IN] Free Size in bytes for tiler heap mem, when tiler is Disabled */

} SystemCommon_TilerGetFreeSize;


#endif

/* @} */
