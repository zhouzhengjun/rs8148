

#include <ti/sdo/edma3/drv/edma3_drv.h>



/* Determine the processor id by reading DNUM register. */
unsigned short Utils_getRegionId()
    {
    return 4;
    }


unsigned short Utils_isGblConfigRequired(unsigned int dspNum)
    {
    (void) dspNum;

    return 0;
    }


/** Interrupt no. for Transfer Completion */
#define EDMA3_CC_XFER_COMPLETION_INT                    (62)
/** Interrupt no. for CC Error */
#define EDMA3_CC_ERROR_INT                              (46u)
/** Interrupt no. for TCs Error */
#define EDMA3_TC0_ERROR_INT                             (0u)
#define EDMA3_TC1_ERROR_INT                             (0u)
#define EDMA3_TC2_ERROR_INT                             (0u)
#define EDMA3_TC3_ERROR_INT                             (0u)
#define EDMA3_TC4_ERROR_INT                             (0u)
#define EDMA3_TC5_ERROR_INT                             (0u)
#define EDMA3_TC6_ERROR_INT                             (0u)
#define EDMA3_TC7_ERROR_INT                             (0u)








#include "utils_dma_common_cfg.c"


/* Driver Instance Initialization Configuration */
EDMA3_DRV_InstanceInitConfig gUtils_dmaInstInitConfig =
        {
        /* 31     0     63    32     95    64     127   96 */
        {0x00000000u, 0xFC000000u, 0x0000000Fu, 0x00000000u,
        /* 159  128     191  160     223  192     255  224 */
         0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
        /* 287  256     319  288     351  320     383  352 */
         0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
        /* 415  384     447  416     479  448     511  480 */
         0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,},

        /* ownDmaChannels */
        /* 31     0     63    32 */
        {0x00000000u, 0xFC000000u},

        /* ownQdmaChannels */
        /* 31     0 */
        {0x00000000u},

        /* ownTccs */
        /* 31     0     63    32 */
        {0x00000000u, 0xFC000000u},


        /* resvdPaRAMSets */
        /* 31     0     63    32     95    64     127   96 */
        {0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
        /* 159  128     191  160     223  192     255  224 */
         0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
        /* 287  256     319  288     351  320     383  352 */
         0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
        /* 415  384     447  416     479  448     511  480 */
         0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,},

        /* resvdDmaChannels */
        /* 31     0    63     32 */
        {0x00000000u, 0x00000000u},

        /* resvdQdmaChannels */
        /* 31     0 */
        {0x00000000u},

        /* resvdTccs */
        /* 31     0    63     32 */
        {0x00000000u, 0x00000000u},
    };



/* End of File */


