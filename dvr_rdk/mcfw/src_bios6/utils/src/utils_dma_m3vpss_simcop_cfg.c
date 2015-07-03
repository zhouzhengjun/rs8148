
#include <ti/sdo/edma3/drv/edma3_drv.h>


/* Number of EDMA3 controllers present in the system */
#define NUM_EDMA3_INSTANCES			1u

/* Determine the region id used for this processor */
unsigned short determineProcId()
{
    extern unsigned short Utils_getRegionId();

    return Utils_getRegionId();
}

unsigned short isGblConfigRequired(unsigned int dspNum)
{
    extern unsigned short Utils_isGblConfigRequired(unsigned int dspNum);

    return Utils_isGblConfigRequired(dspNum);
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

// All EDMA channels allocated in region 2 will be assigned as follow:
// Channels used for transfers from DDR will be assigned to queue VICP_EDMA3_FROM_DDR_queue
// Channels used for transfers from DDR will be assigned to queue VICP_EDMA3_TO_DDR_queue
EDMA3_RM_EventQueue VICP_EDMA3_FROM_DDR_queue= 3;
EDMA3_RM_EventQueue VICP_EDMA3_TO_DDR_queue= 3;

/**
 * Variable which will be used internally for referring transfer completion
 * interrupt.
 */
unsigned int VICP_EDMA3_ccXferCompInt[NUM_EDMA3_INSTANCES][EDMA3_MAX_REGIONS] = {
        {
                EDMA3_CC_XFER_COMPLETION_INT, EDMA3_CC_XFER_COMPLETION_INT, EDMA3_CC_XFER_COMPLETION_INT, EDMA3_CC_XFER_COMPLETION_INT,
                EDMA3_CC_XFER_COMPLETION_INT, EDMA3_CC_XFER_COMPLETION_INT, EDMA3_CC_XFER_COMPLETION_INT, EDMA3_CC_XFER_COMPLETION_INT,
        },
};

/**
 * Variable which will be used internally for referring channel controller's
 * error interrupt.
 */
unsigned int VICP_EDMA3_ccErrorInt[NUM_EDMA3_INSTANCES] = {EDMA3_CC_ERROR_INT};

/**
 * Variable which will be used internally for referring transfer controllers'
 * error interrupts.
 */
unsigned int VICP_EDMA3_tcErrorInt[NUM_EDMA3_INSTANCES][8] =    {
        {
                EDMA3_TC0_ERROR_INT, EDMA3_TC1_ERROR_INT,
                EDMA3_TC2_ERROR_INT, EDMA3_TC3_ERROR_INT,
                EDMA3_TC4_ERROR_INT, EDMA3_TC5_ERROR_INT,
                EDMA3_TC6_ERROR_INT, EDMA3_TC7_ERROR_INT,
        }
};


/* Driver Object Initialization Configuration */
EDMA3_DRV_GblConfigParams vicpEdma3GblCfgParams[NUM_EDMA3_INSTANCES] =
{
        {
                /** Total number of DMA Channels supported by the EDMA3 Controller */
                64u,
                /** Total number of QDMA Channels supported by the EDMA3 Controller */
                8u,
                /** Total number of TCCs supported by the EDMA3 Controller */
                64u,
                /** Total number of PaRAM Sets supported by the EDMA3 Controller */
                512u,
                /** Total number of Event Queues in the EDMA3 Controller */
                4u,
                /** Total number of Transfer Controllers (TCs) in the EDMA3 Controller */
                4u,
                /** Number of Regions on this EDMA3 controller */
                6u,

                /**
                 * \brief Channel mapping existence
                 * A value of 0 (No channel mapping) implies that there is fixed association
                 * for a channel number to a parameter entry number or, in other words,
                 * PaRAM entry n corresponds to channel n.
                 */
                1u,

                /** Existence of memory protection feature */
                1u,

                /** Global Register Region of CC Registers */
                (void *)0x49000000u,
                /** Transfer Controller (TC) Registers */
                {
                        (void *)0x49800000u,
                        (void *)0x49900000u,
                        (void *)0x49A00000u,
                        (void *)0x49B00000u,
                        (void *)NULL,
                        (void *)NULL,
                        (void *)NULL,
                        (void *)NULL
                },
                /** Interrupt no. for Transfer Completion */
                EDMA3_CC_XFER_COMPLETION_INT,
                /** Interrupt no. for CC Error */
                EDMA3_CC_ERROR_INT,
                /** Interrupt no. for TCs Error */
                {
                        EDMA3_TC0_ERROR_INT,
                        EDMA3_TC1_ERROR_INT,
                        EDMA3_TC2_ERROR_INT,
                        EDMA3_TC3_ERROR_INT,
                        EDMA3_TC4_ERROR_INT,
                        EDMA3_TC5_ERROR_INT,
                        EDMA3_TC6_ERROR_INT,
                        EDMA3_TC7_ERROR_INT
                },

                /**
                 * \brief EDMA3 TC priority setting
                 *
                 * User can program the priority of the Event Queues
                 * at a system-wide level.  This means that the user can set the
                 * priority of an IO initiated by either of the TCs (Transfer Controllers)
                 * relative to IO initiated by the other bus masters on the
                 * device (ARM, DSP, USB, etc)
                 */
                {
                        2u,
                        2u,
                        2u,
                        2u,
                        0u,
                        0u,
                        0u,
                        0u
                },
                /**
                 * \brief To Configure the Threshold level of number of events
                 * that can be queued up in the Event queues. EDMA3CC error register
                 * (CCERR) will indicate whether or not at any instant of time the
                 * number of events queued up in any of the event queues exceeds
                 * or equals the threshold/watermark value that is set
                 * in the queue watermark threshold register (QWMTHRA).
                 */
                {
                        16u,
                        16u,
                        16u,
                        16u,
                        0u,
                        0u,
                        0u,
                        0u
                },

                /**
                 * \brief To Configure the Default Burst Size (DBS) of TCs.
                 * An optimally-sized command is defined by the transfer controller
                 * default burst size (DBS). Different TCs can have different
                 * DBS values. It is defined in Bytes.
                 */
                {
                        16u,
                        16u,
                        16u,
                        16u,
                        0u,
                        0u,
                        0u,
                        0u
                },

                /**
                 * \brief Mapping from each DMA channel to a Parameter RAM set,
                 * if it exists, otherwise of no use.
                 */
                {
                        0u, 1u, 2u, 3u, 4u, 5u, 6u, 7u,
                        8u, 9u, 10u, 11u, 12u, 13u, 14u, 15u,
                        16u, 17u, 18u, 19u, 20u, 21u, 22u, 23u,
                        24u, 25u, 26u, 27u, 28u, 29u, 30u, 31u,
                        32u, 33u, 34u, 35u, 36u, 37u, 38u, 39u,
                        40u, 41u, 42u, 43u, 44u, 45u, 46u, 47u,
                        48u, 49u, 50u, 51u, 52u, 53u, 54u, 55u,
                        56u, 57u, 58u, 59u, 60u, 61u, 62u, 63u
                },

                /**
                 * \brief Mapping from each DMA channel to a TCC. This specific
                 * TCC code will be returned when the transfer is completed
                 * on the mapped channel.
                 */
                {
                        0u, 1u, 2u, 3u,
                        4u, 5u, 6u, 7u,
                        8u, 9u, EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP,
                        12u, 13u, 14u, 15u,
                        16u, 17u, 18u, 19u,
                        20u, 21u, EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP,
                        24u, 25u, 26u, 27u,
                        28u, 29u, 30u, 31u,
                        32u, 33u, 34u, 35u,
                        36u, 37u, 38u, 39u,
                        40u, 41u, 42u, 43u,
                        44u, 45u, 46u, 47u,
                        48u, 49u, 50u, 51u,
                        52u, 53u, 54u, 55u,
                        56u, 57u, 58u, 59u,
                        60u, 61u, 62u, 63u
                },


                /**
                 * \brief Mapping of DMA channels to Hardware Events from
                 * various peripherals, which use EDMA for data transfer.
                 * All channels need not be mapped, some can be free also.
                 */
                {
                        0x00000000u,
                        0x00000000u
                },
        },
};


/* Driver Instance Initialization Configuration */
EDMA3_DRV_InstanceInitConfig vicpInstInitConfig[NUM_EDMA3_INSTANCES][EDMA3_MAX_REGIONS] =
{
        /* EDMA3 INSTANCE# 0 */
        {
                /* Resources owned/reserved by region 0 */
                {
                    /* 31     0     63    32     95    64     127   96 */
                    {0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
                    /* 159  128     191  160     223  192     255  224 */
                     0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
                    /* 287  256     319  288     351  320     383  352 */
                     0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
                    /* 415  384     447  416     479  448     511  480 */
                     0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,},

                    /* ownDmaChannels */
                    /* 31     0     63    32 */
                    {0x00000000u, 0x00000000u},

                    /* ownQdmaChannels */
                    /* 31     0 */
                    {0x00000000u},

                    /* ownTccs */
                    /* 31     0     63    32 */
                    {0x00000000u, 0x00000000u},

                    /* Resources reserved by Region 0 */
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
                },

                /* Resources owned/reserved by region 1 */
                {
                    /* 31     0     63    32     95    64     127   96 */
                    {0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
                    /* 159  128     191  160     223  192     255  224 */
                     0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
                    /* 287  256     319  288     351  320     383  352 */
                     0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
                    /* 415  384     447  416     479  448     511  480 */
                     0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,},

                    /* ownDmaChannels */
                    /* 31     0     63    32 */
                    {0x00000000u, 0x00000000u},

                    /* ownQdmaChannels */
                    /* 31     0 */
                    {0x00000000u},

                    /* ownTccs */
                    /* 31     0     63    32 */
                    {0x00000000u, 0x00000000u},

                    /* Resources reserved by Region 0 */
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
                },

                /* Resources owned/reserved by region 2 */
                {
                    /* 31     0     63    32     95    64     127   96 */
                    {0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
                    /* 159  128     191  160     223  192     255  224 */
                     0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
                    /* 287  256     319  288     351  320     383  352 */
                     0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
                    /* 415  384     447  416     479  448     511  480 */
                     0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,},

                    /* ownDmaChannels */
                    /* 31     0     63    32 */
                    {0x00000000u, 0x00000000u},

                    /* ownQdmaChannels */
                    /* 31     0 */
                    {0x00000000u},

                    /* ownTccs */
                    /* 31     0     63    32 */
                    {0x00000000u, 0x00000000u},

                    /* Resources reserved by Region 0 */
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
                },

                /* Resources owned/reserved by region 3 */
                {
                    /* 31     0     63    32     95    64     127   96 */
                    {0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
                    /* 159  128     191  160     223  192     255  224 */
                     0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
                    /* 287  256     319  288     351  320     383  352 */
                     0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
                    /* 415  384     447  416     479  448     511  480 */
                     0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,},

                    /* ownDmaChannels */
                    /* 31     0     63    32 */
                    {0x00000000u, 0x00000000u},

                    /* ownQdmaChannels */
                    /* 31     0 */
                    {0x00000000u},

                    /* ownTccs */
                    /* 31     0     63    32 */
                    {0x00000000u, 0x00000000u},

                    /* Resources reserved by Region 0 */
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
                },

                /* Resources owned/reserved by region 4 */
                {
                    /* ownPaRAMSets */
                    /* 31     0     63    32     95    64     127   96 */
                   {0xFCFF0000u, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
                    /* 159  128     191  160     223  192     255  224 */
                    0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
                    /* 287  256     319  288     351  320     383  352 */
                    0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
                    /* 415  384     447  416     479  448     511  480 */
                    0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,},

                    /* ownDmaChannels */
                    /* 31     0     63    32 */
                    {0xFCFF0000u, 0xFFFFFFFFu},

                    /* ownQdmaChannels */
                    /* 31     0 */
                    {0x00000000u},

                    /* ownTccs */
                    /* 31     0     63    32 */
                    {0xFCFF0000u, 0xFFFFFFFFu},

                    /* Resources reserved by Region 4 */
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
                    /* 31     0     63    32 */
                    {0x00000000u, 0x00000000u},

                    /* resvdQdmaChannels */
                    /* 31     0 */
                    {0x00000000u},

                    /* resvdTccs */
                    /* 31     0     63    32 */
                    {0x00000000u, 0x00000000u},
                },

                /* Resources owned/reserved by region 5 */
                {
                    /* ownPaRAMSets */
                    /* 31     0     63    32     95    64     127   96 */
                   {0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
                    /* 159  128     191  160     223  192     255  224 */
                    0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
                    /* 287  256     319  288     351  320     383  352 */
                    0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
                    /* 415  384     447  416     479  448     511  480 */
                    0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,},

                    /* ownDmaChannels */
                    /* 31     0     63    32 */
                    {0x00000000u, 0x00000000u},

                    /* ownQdmaChannels */
                    /* 31     0 */
                    {0x00000000u},

                    /* ownTccs */
                    /* 31     0     63    32 */
                    {0x00000000u, 0x00000000u},

                    /* Resources reserved by Region 4 */
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
                    /* 31     0     63    32 */
                    {0x00000000u, 0x00000000u},

                    /* resvdQdmaChannels */
                    /* 31     0 */
                    {0x00000000u},

                    /* resvdTccs */
                    /* 31     0     63    32 */
                    {0x00000000u, 0x00000000u},
                },

                /* Resources owned/reserved by region 6 */
                {
                    /* 31     0     63    32     95    64     127   96 */
                    {0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
                    /* 159  128     191  160     223  192     255  224 */
                     0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
                    /* 287  256     319  288     351  320     383  352 */
                     0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
                    /* 415  384     447  416     479  448     511  480 */
                     0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,},

                    /* ownDmaChannels */
                    /* 31     0     63    32 */
                    {0x00000000u, 0x00000000u},

                    /* ownQdmaChannels */
                    /* 31     0 */
                    {0x00000000u},

                    /* ownTccs */
                    /* 31     0     63    32 */
                    {0x00000000u, 0x00000000u},

                    /* Resources reserved by Region 0 */
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
                },
        },
};



/* End of File */


