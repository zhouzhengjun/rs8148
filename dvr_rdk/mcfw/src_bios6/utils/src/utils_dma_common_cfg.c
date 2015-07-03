
/*
    EDMA resource allocation across different cores

    - Que Priority set from M3 VPSS side

        Que 0 - Pri 4
        Que 1 - Pri 4
        Que 2 - Pri 4
        Que 3 - Pri 4

    - TC to Que mapping is 1:1

    HOST A8 - Defined in <linux kernel>\arch\arm\mach-omap2\devices.c, ti81xx_dma_rsv_chans, ti81xx_dma_rsv_slots,
    =======
    Region    : 0
    Default Q : NA
    EDMA CHs  : 2-25, 32-47, 52-55
    TCCs      : Same as EDMA CHs
    QDMA Chs  : NOT USED
    PaRAMs    : Same as EDMA CHs, 192-511

    DSP - Defined in utils_dma_c6xdsp_cfg.c
    =======
    Region    : 1                           (defined in FC_RMAN_IRES_c6xdsp.cfg)
    Default Q : 3                           (defined in FC_RMAN_IRES_c6xdsp.cfg)
    EDMA CHs  : 0-1, 26-31, 48-51, 56-57
    TCCs      : Same as EDMA CHs
    QDMA Chs  : 0-3
    PaRAMs    : Same as EDMA CHs, 68-191

    M3 VPSS - Defined in utils_dma_m3vpss_cfg.c
    =======
    Region    : 4
    Default Q : 3                           (defined in utils_dma.h)
    EDMA CHs  : 58-63
    TCCs      : Same as EDMA CHs
    QDMA Chs  : NOT USED
    PaRAMs    : Same as EDMA CHs, 64-67

    M3 Video
    ========
    EDMA NOT USED ON M3 Video

    NOTE,

    - when region is changed EDMA3_CC_XFER_COMPLETION_INT should also be changed in utils_dma_m3vpss_cfg.c

    - When SIMCOP is enabled, the SIMCOP library inits the EDMA hence the #define UTILS_DMA_INIT_DONE_BY_SIMCOP_LIB
      is defined.

      In this case EDMA config is pixked from the file utils_dma_m3vpss_simcop_cfg.c

    - M3 VPSS region should not be changed from 4 when SIMCOP is enabled.
*/

/*
    This file in included in utils_dma_c6xdsp_cfg.c and
    utils_dma_m3vpss_cfg.c
*/


/**
 * Variable which will be used internally for referring transfer completion
 * interrupt.
 */
unsigned int gUtils_ccXferCompInt[EDMA3_MAX_REGIONS] = {
                            EDMA3_CC_XFER_COMPLETION_INT, EDMA3_CC_XFER_COMPLETION_INT, EDMA3_CC_XFER_COMPLETION_INT, EDMA3_CC_XFER_COMPLETION_INT,
                            EDMA3_CC_XFER_COMPLETION_INT, EDMA3_CC_XFER_COMPLETION_INT, EDMA3_CC_XFER_COMPLETION_INT, EDMA3_CC_XFER_COMPLETION_INT,
                        };

/**
 * Variable which will be used internally for referring channel controller's
 * error interrupt.
 */
unsigned int gUtils_ccErrorInt = EDMA3_CC_ERROR_INT;

/**
 * Variable which will be used internally for referring transfer controllers'
 * error interrupts.
 */
unsigned int gUtils_tcErrorInt[EDMA3_MAX_REGIONS] =
                                {
                                EDMA3_TC0_ERROR_INT, EDMA3_TC1_ERROR_INT,
                                EDMA3_TC2_ERROR_INT, EDMA3_TC3_ERROR_INT,
                                EDMA3_TC4_ERROR_INT, EDMA3_TC5_ERROR_INT,
                                EDMA3_TC6_ERROR_INT, EDMA3_TC7_ERROR_INT,
                                };


/* Driver Object Initialization Configuration */
EDMA3_DRV_GblConfigParams gUtils_dmaGblCfgParams =

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
			0u,
	        4u,
	        4u,
	        4u,
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
            8u, 9u, 10u, 11u,
            12u, 13u, 14u, 15u,
            16u, 17u, 18u, 19u,
            20u, 21u, 22u, 23u,
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
};





