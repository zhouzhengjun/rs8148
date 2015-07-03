#include <stdlib.h>
#include <xdc/std.h>
#include <xdc/runtime/System.h>
#include <xdc/runtime/Memory.h>
#include <ti/xdais/ialg.h>
#include <ti/xdais/ires.h>
#include <ti/sdo/utils/trace/gt.h>
#include <ti/sdo/fc/ires/hdvicp/iresman_hdvicp.h>
#include <ti/sdo/fc/ires/hdvicp/ires_hdvicp2.h>
#include <ti/sdo/fc/ires/tiledmemory/iresman_tiledmemory.h>
#include <ti/sdo/fc/ires/tiledmemory/ires_tiledmemory.h>
#include <ti/sdo/fc/ires/tiledmemory/_tiledmemory.h>
#include <ti/sdo/fc/ires/tiledmemory/iresman_protocol_tiledmemory.h>
/* tiler header files */
#include <ti/sdo/tiler/MemMgr.h>
#include <mcfw/src_bios6/utils/utils_trace.h>
#include <mcfw/src_bios6/utils/utils_mem.h>
#include <mcfw/interfaces/link_api/system_tiler.h>
#include <mcfw/src_bios6/utils/utils_tiler.h>
#include <mcfw/interfaces/link_api/system_debug.h>
#include <ti/sysbios/hal/Hwi.h>

#include "iresman_tiledmemory.h"

#define TILEDMEMORY_MAXRESOURCES 1

/** Enable below define to print malloc/free info */
//#define IRESMAN_TILEDMEMORY_MALLOC_DEBUG

Bool iresman_AllocDummy(IALG_MemRec * memTab, Int numRecs);
void iresman_FreeDummy(IALG_MemRec * memTab, Int numRecs);

#define IRESMAN_TILEDMEMORY_SYSTEMADDR_START                       (0x60000000)
#define IRESMAN_TILEDMEMORY_SYSTEMADDR_END                         (0x7FFFFFFF)

/* TiledMemory_Gethandle Plugin Implementation */
extern IRES_Handle IRESMAN_TILEDMEMORY_getHandles_Plugin(IALG_Handle algHandle,
                                                         IRES_ResourceDescriptor
                                                         * resDesc,
                                                         Int scratchGroupId,
                                                         IRES_Status * status);

extern IRES_Status IRESMAN_TILEDMEMORY_freeHandles_Plugin(IALG_Handle algHandle,
                                                          IRES_Handle
                                                          algResourceHandle,
                                                          IRES_ResourceDescriptor
                                                          * resDesc,
                                                          Int scratchGroupId);

static inline Void freeRes(Void * memoryBaseAddress, Int size,
                           IALG_Handle alg, Int scratchGroupId, Bool isTiled)
{
    /* IALG_MemRec freeMemRec; */

    if ((isTiled) &&
        ((UInt32) memoryBaseAddress >= IRESMAN_TILEDMEMORY_SYSTEMADDR_START) &&
        ((UInt32) memoryBaseAddress <= IRESMAN_TILEDMEMORY_SYSTEMADDR_END))
    {
        SystemTiler_free((UInt32) memoryBaseAddress);

        #ifdef IRESMAN_TILEDMEMORY_MALLOC_DEBUG
        Vps_printf(" IRES: MEM FREE, TILED, addr = 0x%08x, size = %d x %d", memoryBaseAddress,size&0xFFFF,size>>16);
        #endif

    }
    else
    {
        #ifdef IRESMAN_TILEDMEMORY_MALLOC_DEBUG
        Vps_printf(" IRES: MEM FREE, NON-TILED, addr = 0x%08x, size = %d bytes", memoryBaseAddress,size);
        #endif
        Utils_memFree(memoryBaseAddress, size);
    }
}

static UInt32 iresman_tiler_map_pixelfmt_2_tilecontainer(pixel_fmt_t fmt)
{
    Uint32 systemTilerCntMode = SYSTEM_TILER_CNT_MAX;

    switch (fmt)
    {
        case PIXEL_FMT_8BIT:
            systemTilerCntMode = SYSTEM_TILER_CNT_8BIT;
            break;
        case PIXEL_FMT_16BIT:
            systemTilerCntMode = SYSTEM_TILER_CNT_16BIT;
            break;
        case PIXEL_FMT_32BIT:
            systemTilerCntMode = SYSTEM_TILER_CNT_32BIT;
            break;
        default:
            break;
    }
    return systemTilerCntMode;
}
static Void *retryMemAlloc(MemAllocBlock * tilerDescPtr,
                           pixel_fmt_t * unitPtr, Bool * isTiled,
                           unsigned int size,
                           Bool forceDisableTiler)
{
    Void *tilerAddr;
    IALG_MemRec requestMemRec;
    UInt32 systemTilerAllocatorCntMode;

    /* First try and alloc the requested tiler block */
    tilerDescPtr->pixelFormat = *unitPtr;
    tilerDescPtr->stride = 0;
    tilerDescPtr->ptr = 0;
    tilerDescPtr->reserved = 0;

    if (PIXEL_FMT_PAGE == tilerDescPtr->pixelFormat)
    {
        /* round len up to next page (4KB) boundary */
        tilerDescPtr->dim.len =
            (4096 - 1 + tilerDescPtr->dim.len) & ~(4096 - 1);
        size = tilerDescPtr->dim.len;
    }

    systemTilerAllocatorCntMode =
        iresman_tiler_map_pixelfmt_2_tilecontainer(tilerDescPtr->pixelFormat);
    if (systemTilerAllocatorCntMode == SYSTEM_TILER_CNT_MAX)
    {
        *isTiled = FALSE;
    }
    if (!(*isTiled))
    {

        /* RAW memory requested */
        requestMemRec.size = size;
        requestMemRec.alignment = 32;
        requestMemRec.space = IALG_EXTERNAL;
        requestMemRec.attrs = IALG_PERSIST;

        requestMemRec.base =
            Utils_memAlloc(requestMemRec.size, requestMemRec.alignment);

        #ifdef IRESMAN_TILEDMEMORY_MALLOC_DEBUG
        Vps_printf(" IRES: MEM ALLOC, NON-TILED, addr = 0x%08x, size = %d bytes",
            requestMemRec.base,
            requestMemRec.size
            );
        #endif

        tilerAddr = (void *) (requestMemRec.base);
        return (tilerAddr);
    }

    UTILS_assert((systemTilerAllocatorCntMode == SYSTEM_TILER_CNT_8BIT) ||
                 (systemTilerAllocatorCntMode == SYSTEM_TILER_CNT_16BIT));
    #ifdef SYSTEM_USE_TILER
    if (FALSE == forceDisableTiler)
    {
        #ifdef IRESMAN_TILEDMEMORY_MALLOC_DEBUG
        Vps_printf(" IRES: MEM ALLOC, TILED, addr = 0x%08x, size = %d x %d",
                        tilerAddr,
                        tilerDescPtr->dim.area.width,
                        tilerDescPtr->dim.area.height
                  );
        #endif

        tilerAddr = (Void *) SystemTiler_alloc(systemTilerAllocatorCntMode,
                                               tilerDescPtr->dim.area.width,
                                               tilerDescPtr->dim.area.height);
    }
    else
    {
        tilerAddr = (Void *)SYSTEM_TILER_INVALID_ADDR;
    }
    #else
    tilerAddr = (Void *)SYSTEM_TILER_INVALID_ADDR;
    #endif

    if (SYSTEM_TILER_INVALID_ADDR != (UInt32)tilerAddr)
    {
        return ((Void *) (Utils_tilerAddr2CpuAddr((UInt32) tilerAddr)));
    }
    /* If it Fails */
    if (PIXEL_FMT_PAGE == tilerDescPtr->pixelFormat)
    {
        *isTiled = FALSE;
    }
    else if (PIXEL_FMT_8BIT == tilerDescPtr->pixelFormat)
    {
        /* convert the dimensions if required */
        tilerDescPtr->dim.len = tilerDescPtr->dim.area.width *
                                tilerDescPtr->dim.area.height;
        size = tilerDescPtr->dim.len;
        *unitPtr = PIXEL_FMT_PAGE;
    }
    else if (PIXEL_FMT_16BIT == tilerDescPtr->pixelFormat)
    {
        tilerDescPtr->dim.area.width *= 2;
        *unitPtr = PIXEL_FMT_8BIT;
        /* convert the dimensions if required */
    }
    else if (PIXEL_FMT_32BIT == tilerDescPtr->pixelFormat)
    {
        tilerDescPtr->dim.area.width *= 2;
        *unitPtr = PIXEL_FMT_16BIT;
        /* convert the dimensions if required */
    }

    tilerAddr = retryMemAlloc(tilerDescPtr, unitPtr, isTiled, size,
                              forceDisableTiler);

    return (tilerAddr);
}

IRES_Status IRESMAN_TiledMemoryResourceRegister()
{
    IRESMAN_Params rman_params;

    UTILS_assert(RMAN_init() == IRES_OK);

    rman_params.size = sizeof(IRESMAN_Params);
    rman_params.allocFxn = iresman_AllocDummy;
    rman_params.freeFxn = iresman_FreeDummy;

    /* Unregister Tiler Resource with the RMAN so that we can register with
     * the new plugins (gethandle & freehandle functions) */
    UTILS_assert(RMAN_unregister(&IRESMAN_TILEDMEMORY) == IRES_OK);
    /* Overwrite the Tiler Memory allocations */
    IRESMAN_TILEDMEMORY.getHandle = IRESMAN_TILEDMEMORY_getHandles_Plugin;
    IRESMAN_TILEDMEMORY.freeHandle = IRESMAN_TILEDMEMORY_freeHandles_Plugin;

    /* Register the Tiler with the new Params */
    UTILS_assert(RMAN_register(&IRESMAN_TILEDMEMORY, &rman_params) == IRES_OK);

    return IRES_OK;
}

IRES_Status IRESMAN_TiledMemoryResourceUnregister()
{
    UTILS_assert(RMAN_unregister(&IRESMAN_TILEDMEMORY) == IRES_OK);
    return IRES_OK;
}

#define IRESMAN_TILEDMEMORY_MAX_NUM_TILE_DISABLE_CHANNELS              (128u)

typedef struct IRESMAN_TiledMemoryDisableInfo
{
    UInt32 numHandles;
    IALG_Handle algHandle[IRESMAN_TILEDMEMORY_MAX_NUM_TILE_DISABLE_CHANNELS];

} IRESMAN_TiledMemoryDisableInfo;

IRESMAN_TiledMemoryDisableInfo IRESMAN_TiledMemoryDisableInfoTbl =
{
    .numHandles = 0,
};


/**
 * @fn IRESMAN_TiledMemoryForceDisableTileAlloc_Register
 * @brief Function to register forcibly disable tiler allocation for a particular algHandle
 */
IRES_Status IRESMAN_TiledMemoryForceDisableTileAlloc_Register(IALG_Handle algHandle)
{
    UInt key;

    key = Hwi_disable();
    UTILS_assert(IRESMAN_TiledMemoryDisableInfoTbl.numHandles <
                 UTILS_ARRAYSIZE(IRESMAN_TiledMemoryDisableInfoTbl.algHandle));
    IRESMAN_TiledMemoryDisableInfoTbl.algHandle[IRESMAN_TiledMemoryDisableInfoTbl.numHandles] =
        algHandle;
    IRESMAN_TiledMemoryDisableInfoTbl.numHandles++;
    Hwi_restore(key);
    return IRES_OK;
}

/**
 * @fn IRESMAN_TiledMemoryForceDisableTileAlloc_UnRegister
 * @brief Function to unregister forcibly disable tiler allocation for a particular algHandle
 *
 */
IRES_Status IRESMAN_TiledMemoryForceDisableTileAlloc_UnRegister(IALG_Handle algHandle)
{
    UInt key;
    Int i,j;

    key = Hwi_disable();
    for (i = 0; i < IRESMAN_TiledMemoryDisableInfoTbl.numHandles;i++)
    {
        if (IRESMAN_TiledMemoryDisableInfoTbl.algHandle[i] == algHandle)
        {
            break;
        }
    }
    if (i < IRESMAN_TiledMemoryDisableInfoTbl.numHandles)
    {
        for (j = i + 1; j < IRESMAN_TiledMemoryDisableInfoTbl.numHandles; j++)
        {
            IRESMAN_TiledMemoryDisableInfoTbl.algHandle[j - 1]
              = IRESMAN_TiledMemoryDisableInfoTbl.algHandle[j];
        }
        IRESMAN_TiledMemoryDisableInfoTbl.numHandles--;
    }
    else
    {
        if (IRESMAN_TiledMemoryDisableInfoTbl.numHandles)
        {
            UTILS_warn("IRES_TILER:Unregistering handle not previously "
                       "registered,%s:%d",__FILE__, __LINE__);
        }
    }
    Hwi_restore(key);
    return IRES_OK;
}

/**
 * @fn IRESMAN_TiledMemoryForceDisableTileAlloc_UnRegister
 * @brief Function to unregister forcibly disable tiler allocation for a particular algHandle
 *
 */
static
Bool IRESMAN_TiledMemoryForceDisableTileAlloc_isDisabled(IALG_Handle algHandle)
{
    Bool forceDisabled = FALSE;
    UInt key;
    Int i;

    key = Hwi_disable();
    for (i = 0; i < IRESMAN_TiledMemoryDisableInfoTbl.numHandles;i++)
    {
        if (IRESMAN_TiledMemoryDisableInfoTbl.algHandle[i] == algHandle)
        {
            break;
        }
    }
    if (i < IRESMAN_TiledMemoryDisableInfoTbl.numHandles)
    {
        forceDisabled = TRUE;
    }
    Hwi_restore(key);

    return forceDisabled;
}


Bool iresman_AllocDummy(IALG_MemRec memTab[], Int n)
{
    Int i;

#ifdef MEMORYSTATS_DEBUG
    Memory_Stats stats;
#endif
    for (i = 0; i < n; i++)
    {

#ifdef MEMORYSTATS_DEBUG
        Memory_getStats(NULL, &stats);
#endif

        memTab[i].base = (Void *) Memory_alloc(NULL,
                                               memTab[i].size,
                                               memTab[i].alignment, NULL);

        if (memTab[i].base == NULL)
        {
            iresman_FreeDummy(memTab, i);
            return (FALSE);
        }
    }
    return (TRUE);
}

void iresman_FreeDummy(IALG_MemRec memTab[], Int n)
{
    Int i;

    for (i = 0; i < n; i++)
    {
        if (memTab[i].base != NULL)
        {
            Memory_free(NULL, memTab[i].base, memTab[i].size);

        }
    }
}

#define MEMSET(addr, val, count)        \
{                                       \
    UInt i;                             \
    UInt8 *ptr = (UInt8 *)(addr);       \
                                        \
    for (i = 0; i < count; i++) {       \
        *ptr++ = val;                   \
    }                                   \
}

IRES_Handle IRESMAN_TILEDMEMORY_getHandles_Plugin(IALG_Handle algHandle,
                                                  IRES_ResourceDescriptor *
                                                  resDesc,
                                                  Int scratchGroupId,
                                                  IRES_Status * status)
{
    IALG_MemRec protocolMemRecs[TILEDMEMORY_MAXRESOURCES];
    Int numMemRecs = 0;
    Int sizeTiledMemory = 0;
    IRESMAN_TILEDMEMORY_ConstructHandleArgs constructArgs;
    IRES_Handle resourceHandle = (IRES_Handle) NULL;
    MemAllocBlock tilerDesc;
    Void *tilerAddr = NULL;
    pixel_fmt_t unit;
    IRES_TILEDMEMORY_AccessUnit accessUnit;
    Bool isTiled = TRUE;
    IRES_ProtocolArgs *protocolArgs;

    /*
     * Cast the protocolArgs as the request descriptor for Tiledmemory resource
     */
    IRES_TILEDMEMORY_ProtocolArgs *requestDescArgs;

    if (resDesc == NULL)
    {

        *status = IRES_ENORESOURCE;
        System_printf
            ("Error:Invalid Input parameters to IRESMAN_TILEDMEMORY_getHandles()"
             "\n");
        return (NULL);
    }

    protocolArgs = resDesc->protocolArgs;

    if ((protocolArgs == NULL) || (algHandle == NULL))
    {

        *status = IRES_ENORESOURCE;
        System_printf
            ("Error:Invalid Input parameters to IRESMAN_TILEDMEMORY_getHandles()"
             " \n");
        return (NULL);
    }

    requestDescArgs = (IRES_TILEDMEMORY_ProtocolArgs *) protocolArgs;

    if (requestDescArgs->base.mode == IRES_PERSISTENT)
    {
        scratchGroupId = -1;
    }
    else
    {

        *status = IRES_ENORESOURCE;

        System_printf
            ("Error:IRESMAN_TILEDMEMORY_getHandles> Request for IRES_SCRATCH"
              "TILEDMEMORY resources, not supported.\n");

        return (NULL);

    }

    /* clear tiler descriptor */
    MEMSET(&tilerDesc, 0, sizeof(tilerDesc));

    /*
     * Error checking for accessDimension and accessUnit comobinations
     */
    if (requestDescArgs->accessDimension == IRES_TILEDMEMORY_1D)
    {

        if (requestDescArgs->sizeDim0 <= 0)
        {
            *status = IRES_EALG;

            System_printf("IRESMAN_TILEDMEMORY_getHandles> Incorrect value "
                          "specified for sizeDim0 or sizeDim1 in codec's "
                          "protocolArgs.\n");

            return (NULL);
        }

        tilerDesc.dim.len = requestDescArgs->sizeDim0;

        if (requestDescArgs->accessUnit == IRES_TILEDMEMORY_PAGE)
        {
            unit = PIXEL_FMT_PAGE;
        }
        else if (requestDescArgs->accessUnit == IRES_TILEDMEMORY_RAW)
        {
            isTiled = FALSE;
            sizeTiledMemory = requestDescArgs->sizeDim0;
        }
        else
        {
            *status = IRES_ENORESOURCE;

            System_printf("IRESMAN_TILEDMEMORY_getHandles> Did not recognize "
                          "requested access Unit %d\n",
                          requestDescArgs->accessUnit);

            return (NULL);
        }

    }
    else if (requestDescArgs->accessDimension == IRES_TILEDMEMORY_2D)
    {

        if ((requestDescArgs->sizeDim0 <= 0) ||
            (requestDescArgs->sizeDim1 <= 0) ||
            (requestDescArgs->sizeDim0 > 0xFFFF) ||
            (requestDescArgs->sizeDim1 > 0xFFFF))
        {

            *status = IRES_EALG;

            System_printf("IRESMAN_TILEDMEMORY_getHandles> Incorrect value "
                          "specified for sizeDim0 or sizeDim1 in codec's "
                          "protocolArgs.\n");

            return (NULL);

        }

        tilerDesc.dim.len = requestDescArgs->sizeDim0 *
            requestDescArgs->sizeDim1;

        if (requestDescArgs->accessUnit == IRES_TILEDMEMORY_8BIT)
        {
            unit = PIXEL_FMT_8BIT;
            tilerDesc.dim.area.width = requestDescArgs->sizeDim0;
            tilerDesc.dim.area.height = requestDescArgs->sizeDim1;
        }
        else if (requestDescArgs->accessUnit == IRES_TILEDMEMORY_16BIT)
        {
            unit = PIXEL_FMT_16BIT;
            tilerDesc.dim.area.width = requestDescArgs->sizeDim0;
            tilerDesc.dim.area.height = requestDescArgs->sizeDim1;
        }
        else if (requestDescArgs->accessUnit == IRES_TILEDMEMORY_32BIT)
        {
            unit = PIXEL_FMT_32BIT;
            tilerDesc.dim.area.width = requestDescArgs->sizeDim0;
            tilerDesc.dim.area.height = requestDescArgs->sizeDim1;
        }
        else if (requestDescArgs->accessUnit == IRES_TILEDMEMORY_PAGE)
        {

            unit = PIXEL_FMT_PAGE;
            tilerDesc.dim.len = requestDescArgs->sizeDim0 *
                requestDescArgs->sizeDim1;
        }
        else if (requestDescArgs->accessUnit == IRES_TILEDMEMORY_RAW)
        {
            isTiled = FALSE;
            tilerDesc.dim.len = requestDescArgs->sizeDim0 *
                /* size is required only for RAW allocations */
                requestDescArgs->sizeDim1;
            sizeTiledMemory = requestDescArgs->sizeDim0 *
                requestDescArgs->sizeDim1;
        }
        else
        {
            *status = IRES_ENORESOURCE;

            System_printf("IRESMAN_TILEDMEMORY_getHandles> Did not recognize "
                          "request type %d\n", requestDescArgs->accessUnit);

            return (NULL);

        }
    }
    else
    {
        *status = IRES_ENORESOURCE;

        System_printf("IRESMAN_TILEDMEMORY_getHandles> Did not recognize "
                      "requested accessDimension%d\n",
                      requestDescArgs->accessDimension);

        return (NULL);
    }

    /* sizeTiledMemory gets set only for RAW case. Therefore, setting it here
     * for all other types as well, namely, PAGE, 8BIT & 16BIT. This is
     * needed so that the resource handle constructed now gets the size set
     * correcly as well (else it remains 0). This size is needed in the
     * freeHandle plugin invoked in the free path */
    if (sizeTiledMemory == 0)
        sizeTiledMemory = tilerDesc.dim.len;

    /* Call to tiler Alloc */
    tilerAddr = retryMemAlloc(&tilerDesc, &unit, &isTiled, sizeTiledMemory,
                              IRESMAN_TiledMemoryForceDisableTileAlloc_isDisabled(algHandle));

    /* Error if returned address is NULL */
    if (tilerAddr == (Void *) NULL)
    {

        *status = IRES_ENORESOURCE;

        System_printf("IRESMAN_TILEDMEMORY_getHandles> Error obtaining "
                      "TILEDMEMORY resource.");

        System_printf("IRESMAN_TILEDMEMORY_getHandles> Exit (handle=NULL, "
                      "status=IRES_ENORESOURCE)");

        return (NULL);
    }

    /* sizeTiledMemory gets set only for RAW case. Therefore, setting it here
     * for all other types as well, namely, PAGE, 8BIT & 16BIT. This is
     * needed so that the resource handle constructed now gets the size set
     * correcly as well (else it remains 0). This size is needed in the
     * freeHandle plugin invoked in the free path */
    if ((sizeTiledMemory != tilerDesc.dim.len) && (isTiled == TRUE))
        sizeTiledMemory = tilerDesc.dim.len;

    if ((sizeTiledMemory != tilerDesc.dim.len) && (isTiled == FALSE))
    {
        sizeTiledMemory = tilerDesc.dim.len;
    }
    if (isTiled)
    {

        /* TILED memory was allocated */
        switch (unit)
        {
            case PIXEL_FMT_32BIT:
                accessUnit = IRES_TILEDMEMORY_32BIT;
                break;
            case PIXEL_FMT_16BIT:
                accessUnit = IRES_TILEDMEMORY_16BIT;
                break;
            case PIXEL_FMT_8BIT:
                accessUnit = IRES_TILEDMEMORY_8BIT;
                break;
            case PIXEL_FMT_PAGE:
                accessUnit = IRES_TILEDMEMORY_PAGE;
                break;
        }
    }
    else
    {
        /* RAW memory was returned */
        accessUnit = IRES_TILEDMEMORY_RAW;
    }

    /*
     * Get IRES_TILEDMEMORY resource handle's memory requirements
     */
    numMemRecs = IRESMAN_TILEDMEMORY_CONSTRUCTFXNS.getNumMemRecs
        ((IRES_ProtocolArgs *) protocolArgs);

    if (numMemRecs > TILEDMEMORY_MAXRESOURCES)
    {
        System_printf
            ("IRESMAN_TILEDMEMORY_getHandles> Error - numMemRecs exceeds TILEDMEMORY_MAXRESOURCES");
        return (NULL);
    }

    /*
     * Populate the construct args
     */
    (constructArgs.args).size = sizeof(IRESMAN_TILEDMEMORY_ConstructHandleArgs);
    constructArgs.isTiledMemory = isTiled;
    constructArgs.systemSpaceBaseAddress = tilerAddr;
    constructArgs.memBaseAddr = tilerAddr;                 /* MMU set up for
                                                            * 0x0 offset */
    // constructArgs.tilerBaseAddr = (void
    // *)TILEDMEMORY_PARAMS.tiledmemoryBaseAddress;
    constructArgs.tilerBaseAddr = (void *) IRESMAN_TILEDMEMORY_SYSTEMADDR_START;
    constructArgs.unit = accessUnit;
    constructArgs.sizeTiledMemory = sizeTiledMemory;

    // TODO RSVD for later ?!
    constructArgs.info = NULL;

    if (-1 == scratchGroupId)
    {
        constructArgs.persistent = TRUE;
    }
    else
    {
        constructArgs.persistent = FALSE;
    }

    /*
     * Obtain memory resources to build the handle
     */
    IRESMAN_TILEDMEMORY_CONSTRUCTFXNS.getMemRecs((IRES_Handle) NULL,
                                                 (IRES_ProtocolArgs *)
                                                 protocolArgs, protocolMemRecs);

    /*
     * Allocate that memory
     */
    if (FALSE == iresman_AllocDummy(protocolMemRecs, numMemRecs))
    {

        *status = IRES_ENOMEM;

        System_printf("IRESMAN_TILEDMEMORY_getHandles> Error allocating memory "
                      "for constructing handle of size 0x%x, alignment 0x%x, space "
                      "IALG_EXTERNAL, attrs IALG_PERSIST",
                      (IArg) (protocolMemRecs->size),
                      (IArg) (protocolMemRecs->alignment));

        System_printf("IRESMAN_TILEDMEMORY_getHandles> Exit (handle=NULL, "
                      "status=IRES_ENOMEM)");

        /* Error so free acquired resource */
        freeRes(tilerAddr, sizeTiledMemory, algHandle, scratchGroupId, isTiled);

        return (NULL);
    }

    /*
     * Use the CONSTRUCT interface of the protocol implementation to
     * construct the handle within the allocated resource
     */
    resourceHandle =
        IRESMAN_TILEDMEMORY_CONSTRUCTFXNS.constructHandle(protocolArgs,
                                                          protocolMemRecs,
                                                          (IRESMAN_ConstructArgs
                                                           *) & constructArgs,
                                                          status);

    if (IRES_OK != *status)
    {

        System_printf("IRESMAN_TILEDMEMORY_getHandles> Error creating "
                      "handle\n");

        System_printf("IRESMAN_TILEDMEMORY_getHandles> Exit (handle=NULL, "
                      "status=IRES_EFAIL)");

        /* Error so free acquired resource */
        freeRes(tilerAddr, constructArgs.sizeTiledMemory,
                algHandle, scratchGroupId, isTiled);

        return (NULL);
    }
    else
    {

        resDesc->handle = (IRES_Obj *) resourceHandle;

        resDesc->revision->Major = 1;
        resDesc->revision->Source = 0;
        resDesc->revision->Radius = 0;

        return (resourceHandle);
    }
}

IRES_Status IRESMAN_TILEDMEMORY_freeHandles_Plugin(IALG_Handle algHandle,
                                                   IRES_Handle
                                                   algResourceHandle,
                                                   IRES_ResourceDescriptor *
                                                   resDesc, Int scratchGroupId)
{
    IALG_MemRec resourceMemRecs[TILEDMEMORY_MAXRESOURCES];
    Int numMemRecs;
    IRES_Status status = IRES_OK;
    Bool isTiled = TRUE;
    IRES_ProtocolArgs *protocolArgs = resDesc->protocolArgs;
    IRES_TILEDMEMORY_IntObj *resourceHandle = (IRES_TILEDMEMORY_IntObj *)
        algResourceHandle;

    if ((protocolArgs == NULL) || (algHandle == NULL))
    {

        status = IRES_ENORESOURCE;
        System_printf
            ("Error:Invalid Input parameters to IRESMAN_TILEDMEMORY_freeHandles()"
              "\n");
        return (status);
    }

    if (resourceHandle->resObj.isTiledMemory)
    {
        isTiled = TRUE;
    }
    else
    {
        isTiled = FALSE;
    }

    /*
     * Free the memory allocated for the tiledmemory resource.
     */
    freeRes(((IRES_TILEDMEMORY_Handle) resourceHandle)->memoryBaseAddress,
            resourceHandle->sizeTiledMemory, algHandle, scratchGroupId,
            isTiled);

    /*
     * Obtain handle memory resources to free and free them
     */
    numMemRecs = IRESMAN_TILEDMEMORY_CONSTRUCTFXNS.getNumMemRecs
        ((IRES_ProtocolArgs *) protocolArgs);

    IRESMAN_TILEDMEMORY_CONSTRUCTFXNS.getMemRecs((IRES_Handle) NULL,
                                                 (IRES_ProtocolArgs *)
                                                 protocolArgs, resourceMemRecs);

    resourceMemRecs[0].base = algResourceHandle;

    /*
     * Use IRES_TILEDMEMORY_RESOURCEPROTOCOL to de-init the resource protocol
     * if required
     */
    IRESMAN_TILEDMEMORY_CONSTRUCTFXNS.destructHandle(algResourceHandle);

    /*
     * Free the memory for the handles
     */
    iresman_FreeDummy(resourceMemRecs, numMemRecs);

    return (status);
}

IRES_Status IRESMAN_TILEDMEMORY_checkResourceAvail(IALG_Handle alg, UInt32 *size,
                                            IRES_Fxns * resFxns,
                                            IRES_ResourceDescriptor resDesc[])
{
    IRES_Status status = IRES_OK;
    MemAllocBlock tilerDesc;
    Int n, numResources;
    pixel_fmt_t unit;
    IRES_TILEDMEMORY_ProtocolArgs *pArg;

    UTILS_assert(resFxns != NULL);
    UTILS_assert(resDesc != NULL);
    /* clear tiler descriptor */
    memset(&tilerDesc, 0, sizeof(tilerDesc));

    /* Get resource requirements of the algorithm */
    numResources = (resFxns->numResourceDescriptors)(alg);
    status = resFxns->getResourceDescriptors(alg, resDesc);

    if (status != IRES_OK) 
    {
        status = IRES_EALG;
        System_printf("IRESMAN getResourceDescriptors call failed. \n");
        return (status);
    }
    /* For each requested resource */
    for (n = 0 ; n < numResources; n++) 
    {
        unit = PIXEL_FMT_8BIT;
        /* If someone populated the resource descriptor improperly, bail */
        if (strcmp(resDesc[n].resourceName, IRES_TILEDMEMORY_PROTOCOLNAME) == 0)
        {
            pArg = (IRES_TILEDMEMORY_ProtocolArgs*) resDesc[n].protocolArgs;
            if (pArg->accessDimension == IRES_TILEDMEMORY_1D)
            {
                if (pArg->sizeDim0 <= 0)
                {
                    status = IRES_EALG;
                    System_printf("IRESMAN_TILEDMEMORY_getHandles> Incorrect value "
                                  "specified for sizeDim0 or sizeDim1 in codec's "
                                  "protocolArgs.\n");
                    return (status);
                }
                tilerDesc.dim.len = pArg->sizeDim0;
                if (pArg->accessUnit == IRES_TILEDMEMORY_PAGE)
                {
                    unit = PIXEL_FMT_PAGE;
                    tilerDesc.dim.len = pArg->sizeDim0;
                }
                else if (pArg->accessUnit == IRES_TILEDMEMORY_RAW)
                {
                    tilerDesc.dim.len = pArg->sizeDim0;
                }
                else
                {
                    status = IRES_ENORESOURCE;
                    System_printf("IRESMAN_TILEDMEMORY_getHandles> Did not recognize "
                                  "requested access Unit %d\n",
                                  pArg->accessUnit);
                    return (status);
                }

            }
            else
            {
                status = IRES_ENORESOURCE;
                System_printf("IRESMAN_TILEDMEMORY_getHandles recognize as TiledMemory,"
                              "Tiled memory availability check is not supported,"
                              "requested accessDimension%d\n",
                               pArg->accessDimension);
                return (status);
            }
           
            if (PIXEL_FMT_PAGE == unit)
            {
                /* round len up to next page (4KB) boundary */
                tilerDesc.dim.len = (4096 - 1 + tilerDesc.dim.len) & ~(4096 - 1);
            }
            *size += VpsUtils_align (tilerDesc.dim.len, pArg->alignment);
        }
    }

    return (status);
}
