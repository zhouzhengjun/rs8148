////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//            Copyright (C) 2003-2007, Coolsand Technologies, Inc.            //
//                            All Rights Reserved                             //
//                                                                            //
//      This source code is the property of Coolsand Technologies and is      //
//      confidential.  Any  modification, distribution,  reproduction or      //
//      exploitation  of  any content of this file is totally forbidden,      //
//      except  with the  written permission  of  Coolsand Technologies.      //
#ifndef _HAL_SPI_H_
#define _HAL_SPI_H_

#include "tsd_m.h"
typedef enum
{
    /// invalid IO type
    HAL_GPIO_TYPE_INVALID = 0,

    /// no IO type
    HAL_GPIO_TYPE_NONE,

    /// GPIO type
    HAL_GPIO_TYPE_IO,

    /// GPO type
    HAL_GPIO_TYPE_O,

    /// TCO type
    HAL_GPIO_TYPE_TCO,

    HAL_GPIO_TYPE_QTY
} HAL_GPIO_TYPE_T;

typedef enum
{
    /// To be used in configuration when the field for this GPIO is not used
    HAL_GPIO_NONE = HAL_GPIO_TYPE_NONE << 16,
    // Only the first eight ones can trig interrupts
    /// Can cause an interrupt
    HAL_GPIO_0 = HAL_GPIO_TYPE_IO << 16,
    /// Can cause an interrupt
    HAL_GPIO_1,
    /// Can cause an interrupt
    HAL_GPIO_2,
    /// Can cause an interrupt
    HAL_GPIO_3,
    /// Can cause an interrupt
    HAL_GPIO_4,
    /// Can cause an interrupt
    HAL_GPIO_5,
    /// Can cause an interrupt
    HAL_GPIO_6,
    /// Can cause an interrupt
    HAL_GPIO_7,
    // Those following are not interruptible
    HAL_GPIO_8,
    HAL_GPIO_9,
    HAL_GPIO_10,
    HAL_GPIO_11,
    HAL_GPIO_12,
    HAL_GPIO_13,
    HAL_GPIO_14,
    HAL_GPIO_15,
    HAL_GPIO_16,
    HAL_GPIO_17,
    HAL_GPIO_18,
    HAL_GPIO_19,
    HAL_GPIO_20,
    HAL_GPIO_21,
    HAL_GPIO_22,
    HAL_GPIO_23,
    HAL_GPIO_24,
    HAL_GPIO_25,
    HAL_GPIO_26,
    HAL_GPIO_27,
    HAL_GPIO_28,
    HAL_GPIO_29,
    HAL_GPIO_30,
    HAL_GPIO_31,
    HAL_GPIO_QTY = 32
} HAL_GPIO_GPIO_ID_T;

// =============================================================================
//  MACROS
// =============================================================================
/// Defines the SPI minimum frame size
#define HAL_SPI_MIN_FRAME_SIZE          4

/// Defines the SPI maximum frame size
#define HAL_SPI_MAX_FRAME_SIZE          32


/// Defines the maximum number of Chip Select
/// possible on a same given SPI.
#define     HAL_SPI_CS_MAX_QTY      4


typedef struct
{
    /// Defines whether an interruption will be triggered on a rising
    /// edge on the GPIO
    bool rising;

    /// Defines whether an interruption will be triggered on a falling
    /// edge on the GPIO
    bool falling;

    /// Defines if the GPIO's signal will be debounced before the interrupt
    /// is triggered
    bool debounce;

    /// Defines if the interruption is on level (\c TRUE), or on edge (\c FALSE)
    bool level;
} HAL_GPIO_IRQ_MASK_T;

typedef enum
{
    HAL_GPIO_DIRECTION_INPUT,
    HAL_GPIO_DIRECTION_OUTPUT,

    HAL_GPIO_DIRECTION_QTY
} HAL_GPIO_DIRECTION_T;

typedef void (*HAL_GPIO_IRQ_HANDLER_T)(void);

typedef struct
{
    /// Direction of the GPIO
    HAL_GPIO_DIRECTION_T direction;

    /// Initial value (if this GPIO is an output)
    bool value;

    /// IRQ mask, only valid if applied to an interruptible GPIO
    HAL_GPIO_IRQ_MASK_T irqMask;

    /// IRQ handler, only valid if applied to an interruptible GPIO
    HAL_GPIO_IRQ_HANDLER_T irqHandler;
} HAL_GPIO_CFG_T;




// =============================================================================
//  TYPES
// =============================================================================


// =============================================================================
// HAL_SPI_ID_T
// -----------------------------------------------------------------------------
/// Defines which SPI is used by a function of the SPI. Depending on the
/// hardware you are running on, some of those SPIs might not exist
// =============================================================================
typedef enum
{
    HAL_SPI,
    HAL_SPI_2,
    HAL_SPI_QTY = 2
} HAL_SPI_ID_T;

        


// =============================================================================
// HAL_SPI_DELAY_T
// -----------------------------------------------------------------------------
/// Delays
/// Used to define the configuration delays
// =============================================================================
typedef enum
{
    /// Delay of 0 half-period
    HAL_SPI_HALF_CLK_PERIOD_0, 
    /// Delay of 1 half-period
    HAL_SPI_HALF_CLK_PERIOD_1, 
    /// Delay of 2 half-period
    HAL_SPI_HALF_CLK_PERIOD_2, 
    /// Delay of 3 half-period
    HAL_SPI_HALF_CLK_PERIOD_3, 

    HAL_SPI_HALF_CLK_PERIOD_QTY
} HAL_SPI_DELAY_T;



// =============================================================================
// HAL_SPI_CS_T
// -----------------------------------------------------------------------------
/// Chip Select
/// Used to select a Chip Select
// =============================================================================
typedef enum
{
    /// Chip Select 0
    HAL_SPI_CS0 = 0,
    /// Chip Select 1
    HAL_SPI_CS1,
    /// Chip Select 2
    HAL_SPI_CS2,
    /// Chip Select 3 - It doesn't necessarily exists for a given SPI.
    HAL_SPI_CS3,

    HAL_SPI_CS_QTY
 } HAL_SPI_CS_T;



// =============================================================================
// HAL_SPI_RX_TRIGGER_CFG_T
// -----------------------------------------------------------------------------
/// Reception  trigger (or treshold) level.
/// The SPI module can be setup to generate an interrupt when the reception FIFO 
/// is above a configurable threshold (Rx fifo trigger)
// =============================================================================
typedef enum
{
    /// 1 Data received in the Rx FIFO
    HAL_SPI_RX_TRIGGER_1_BYTE, 
    /// 2 Data received in the Rx FIFO
    HAL_SPI_RX_TRIGGER_4_BYTE, 
    /// 3 Data received in the Rx FIFO
    HAL_SPI_RX_TRIGGER_8_BYTE, 
    /// 4 Data received in the Rx FIFO
    HAL_SPI_RX_TRIGGER_12_BYTE,

    HAL_SPI_RX_TRIGGER_QTY
} HAL_SPI_RX_TRIGGER_CFG_T;



// =============================================================================
// HAL_SPI_TX_TRIGGER_CFG_T
// -----------------------------------------------------------------------------
/// Tranmission FIFO trigger (or treshold) level. 
/// The SPI module can be setup to generate an interrupt when the emission FIFO 
/// is above a configurable threshold (Tx FIFO trigger)
// =============================================================================
typedef enum{
    /// 1 Data spot is empty in the Tx FIFO  
    HAL_SPI_TX_TRIGGER_1_EMPTY, 
    /// 2 Data spots are empty in the Tx FIFO
    HAL_SPI_TX_TRIGGER_4_EMPTY, 
    /// 8 Data spots are empty in the Tx FIFO
    HAL_SPI_TX_TRIGGER_8_EMPTY, 
    /// 12 Data spots are empty in the Tx FIFO
    HAL_SPI_TX_TRIGGER_12_EMPTY,

    HAL_SPI_TX_TRIGGER_QTY
} HAL_SPI_TX_TRIGGER_CFG_T;



// =============================================================================
// HAL_SPI_TRANSFERT_MODE_T
// -----------------------------------------------------------------------------
/// Data transfert mode: via DMA or direct. 
/// To allow for an easy use of the SPI modules, a non blocking Hardware 
/// Abstraction Layer interface is provided. Each transfer direction 
/// (send/receive) can be configured as:
// =============================================================================
typedef enum {
    /// Direct polling: The application sends/receives the data directly to/from 
    /// the hardware module. The number of bytes actually sent/received is 
    /// returned. No Irq is generated.
    HAL_SPI_DIRECT_POLLING = 0,

    /// Direct IRQ: The application sends/receives the data directly to/from 
    /// the hardware module. The number of bytes actually sent/received is 
    /// returned. An Irq can be generated when the Tx/Rx FIFO reaches the 
    /// pre-programmed level.
    HAL_SPI_DIRECT_IRQ, 

    /// DMA polling: The application sends/receives the data through a DMA to 
    /// the hardware module. The function returns 0 when no DMA channel is 
    /// available. No bytes are sent. The function returns the number of bytes 
    /// to send when a DMA resource is available. They will all be sent. A 
    /// function allows to check if the previous DMA transfer is finished. No 
    /// new DMA transfer in the same direction will be allowed before the end 
    /// of the previous transfer.
    HAL_SPI_DMA_POLLING, 

    /// DMA IRQ: The application sends/receives the data through a DMA to the 
    /// hardware module. The function returns 0 when no DMA channel is
    /// available. No bytes are sent. The function returns the number of bytes 
    /// to send when a DMA resource is available. They will all be sent. An 
    /// IRQ is generated when the current transfer is finished. No new DMA 
    /// transfer in the same direction will be allowed before the end of the 
    /// previous transfer.
    HAL_SPI_DMA_IRQ, 
    
    /// The SPI is off
    HAL_SPI_OFF, 
    
    HAL_SPI_TM_QTY
} HAL_SPI_TRANSFERT_MODE_T;



// =============================================================================
// HAL_SPI_IRQ_STATUS_T
// -----------------------------------------------------------------------------
/// This structure is used to represent the IRQ status and mask 
/// of the SPI module.
// =============================================================================
typedef struct
{
    /// receive FIFO overflow irq
    u32 rxOvf:1;
    /// transmit FIFO threshold irq
    u32 txTh:1;
    /// transmit Dma Done irq
    u32 txDmaDone:1;
    /// receive FIFO threshold irq
    u32 rxTh:1;
    /// receive Dma Done irq
    u32 rxDmaDone:1;
} HAL_SPI_IRQ_STATUS_T;



// =============================================================================
// HAL_SPI_IRQ_HANDLER_T
// -----------------------------------------------------------------------------
/// Type of the user IRQ handler
// =============================================================================
typedef void (*HAL_SPI_IRQ_HANDLER_T)(HAL_SPI_IRQ_STATUS_T);




// =============================================================================
// HAL_SPI_CFG_T
// -----------------------------------------------------------------------------
/// Structure for configuration. 
/// A configuration structure allows to open or change the SPI with the desired 
/// parameters.
// =============================================================================
typedef struct
{
    /// Select the Chip Select
    HAL_SPI_CS_T enabledCS; 

    /// Polarity of this CS
    bool csActiveLow;

    /// When \c TRUE, the emission commands will fill the Rx FIFO with read
    /// data, thus enabling the ability to receive data. \n
    /// When \c FALSE, nothing is written in the Rx FIFO when data are sent.
    /// It is not possible to read received data, which are discarded.
    bool inputEn;

    /// If the first edge after the CS activation is a falling edge, set to 
    /// \c TRUE.\n Otherwise, set to \c FALSE.
    bool clkFallEdge;

    /// The delay between the CS activation and the first clock edge,
    /// can be 0 to 2 half clocks.
    HAL_SPI_DELAY_T clkDelay; 

    /// The delay between the CS activation and the output of the data, 
    /// can be 0 to 2 half clocks.
    HAL_SPI_DELAY_T doDelay; 

    /// The delay between the CS activation and the sampling of the input data,
    /// can be 0 to 3 half clocks.
    HAL_SPI_DELAY_T diDelay; 

    /// The delay between the end of transfer and the CS deactivation, can be 
    /// 0 to 3 half clocks.
    HAL_SPI_DELAY_T csDelay;

    /// The time when the CS must remain deactivated before a new transfer, 
    /// can be 0 to 3 half clocks.
    HAL_SPI_DELAY_T csPulse; 

    /// Frame size in bits
    u32 frameSize;

    /// OE ratio - Value from 0 to 31 is the number of data out to transfert 
    /// before the SPI_DO pin switches to input. When 0m the SPI_DO pin switching 
    /// direction mode is not enabled.
    u8 oeRatio;
    
    /// SPI maximum clock frequency: the SPI clock will be the highest
    /// possible value inferior to this parameter.
    u32 spiFreq;

    /// Value for the reception FIFO above which an interrupt may be generated.
    HAL_SPI_RX_TRIGGER_CFG_T rxTrigger; 

    /// Value for the emission FIFO above which an interrupt may be generated.
    HAL_SPI_TX_TRIGGER_CFG_T txTrigger; 

    /// Reception transfer mode
    HAL_SPI_TRANSFERT_MODE_T rxMode;

    /// Emission transfer mode
    HAL_SPI_TRANSFERT_MODE_T txMode;

    /// IRQ mask for this CS
    HAL_SPI_IRQ_STATUS_T mask;

    /// IRQ handler for this CS;
    HAL_SPI_IRQ_HANDLER_T handler;

} HAL_SPI_CFG_T;



// =============================================================================
// HAL_SPI_PATTERN_MODE_T
// -----------------------------------------------------------------------------
/// The pattern mode can be used in two: record data after the pattern has been
/// received on the SPI, or data are not recorded while the pattern is seens on
/// the bus. HAL_SPI_PATTERN_WHILE feature is not available for every chip.
// =============================================================================
typedef enum
{
    /// Standard mode, we don't care about the pattern
    HAL_SPI_PATTERN_NO,
    /// No data written until the pattern is read on the bus
    HAL_SPI_PATTERN_UNTIL,
    /// No data are written while the pattern is read on the bus
    HAL_SPI_PATTERN_WHILE
} HAL_SPI_PATTERN_MODE_T;

// =============================================================================
// HAL_SPI_INFINITE_TX_T
// -----------------------------------------------------------------------------
/// That type describes the characteristics and configuration of an infinite 
/// transfer on a SPI CS. That is typically used for devices like MMC which use
/// the pattern mode read feature. (The infinite transfer provides the clock
/// coping with the trashing reads before the pattern is finally seen).
/// When the pattern mode is enabled, data received on the SPI bus will only
/// be put in the Rx FIFO after a given pattern has been received.
/// The SPI will continue transfering data (zeros or ones, depending on the
/// configuration of sendOne) after the Tx fifo is empty (i.e. the IFC DMA
/// is finished) and until the Rx transfer is completed (i.e. the pattern
/// has been received and the Rx transfer size is reached).
/// After you start the inifinite write mode, you have to use the usual
/// send and get data functions.
/// The infinite transfert can be stopped by two ways:
/// - manually, by a call to #hal_SpiStopInfiniteWrite;
/// - automatically (prefered behaviour): the infinite transfer is stopped
/// by the RxDmaDone IRQ generated by the end of the reception done parallelly,
/// if the Rx mode is configured as #HAL_SPI_DMA_POLLING or #HAL_SPI_DMA_IRQ.
// =============================================================================
typedef struct
{
    /// \c TRUE if the infinite transfer is done by sending ones,
    /// \c FALSE if it is done by sending zeroes.
    bool sendOne;

    /// \c TRUE enable the auto-stopping feature of the infinite transfer.
    bool autoStop;

    /// Described the pattern mode. That is the read buffer 
    /// will only be filled with data received after a given pattern, 
    /// specified by the following field, is seen on the line,
    /// or after the pattern is no more read on it (while the pattern
    /// is seen, no data is recorded).
    HAL_SPI_PATTERN_MODE_T patternMode;

    /// Value of the pattern waited before useful data are received.
    u32 pattern;
} HAL_SPI_INFINITE_TX_T;

// =============================================================================
// HAL_SPI_PIN_T
// -----------------------------------------------------------------------------
/// SPI Pin
///
/// Used to specify an SPI pin.
// =============================================================================
typedef enum
{
    /// Chip select 0 pin
    HAL_SPI_PIN_CS0 = 0, 
    /// Chip select 1 pin
    HAL_SPI_PIN_CS1,  
    /// Chip select 2 pin, if available
    HAL_SPI_PIN_CS2, 
    /// Chip select 3 pin, if available
    HAL_SPI_PIN_CS3, 
    /// Clock pin
    HAL_SPI_PIN_CLK, 
    /// Data out pin
    HAL_SPI_PIN_DO, 
    /// Data in pin
    HAL_SPI_PIN_DI, 
    HAL_SPI_PIN_LIMIT 
} HAL_SPI_PIN_T;



// =============================================================================
// HAL_SPI_PIN_STATE_T
// -----------------------------------------------------------------------------
/// SPI Pin State
///
/// Used to specify the state of an SPI pin. It can be
/// zero, one or Z (high impedence, tri-state).
// =============================================================================
typedef enum
{
    HAL_SPI_PIN_STATE_SPI   =   0,
    HAL_SPI_PIN_STATE_Z     =   1,
    HAL_SPI_PIN_STATE_0     =   2,
    HAL_SPI_PIN_STATE_1     =   3
} HAL_SPI_PIN_STATE_T;



// =============================================================================
//  FUNCTIONS
// =============================================================================


// =============================================================================
// hal_SpiIrqSetHandler
// -----------------------------------------------------------------------------
/// Set the user irq handler function
/// @param id Identifier of the SPI for which the function is called.
/// @param handler User function called in case of SPI-related IRQ.
// =============================================================================
void hal_SpiIrqSetHandler(HAL_SPI_ID_T id, HAL_SPI_CS_T csNum, HAL_SPI_IRQ_HANDLER_T handler);



// =============================================================================
// hal_SpiIrqSetMask
// -----------------------------------------------------------------------------
/// Set the irq mask
/// @param id Identifier of the SPI for which the function is called.
/// @param mask Mask to set.
// =============================================================================
void hal_SpiIrqSetMask(HAL_SPI_ID_T id, HAL_SPI_CS_T csNum, HAL_SPI_IRQ_STATUS_T mask);



// =============================================================================
// hal_SpiIrqGetMask
// -----------------------------------------------------------------------------
/// Get the irq mask.
/// @param id Identifier of the SPI for which the function is called.
/// @return The IRQ mask.
// =============================================================================
HAL_SPI_IRQ_STATUS_T hal_SpiIrqGetMask(HAL_SPI_ID_T id, HAL_SPI_CS_T csNum);



// =============================================================================
// hal_SpiOpen
// -----------------------------------------------------------------------------
/// Open a SPI CS driver. 
/// This function enables the SPI in the mode selected by \c spiCfg.
/// <B> The polarity of both the Chip Select must be set in the configuration
/// structure of HAL in the tgt_BoardConfig structure. The knowledge about those 
/// two polarities
/// is needed by the driver, and it cannot operate without them. </B>
///
/// @param id Identifier of the SPI for which the function is called.
/// @param csNum Chip Select for which this configuration applies.
/// @param spiCfg The configuration for SPI
// =============================================================================
void hal_SpiOpen(HAL_SPI_ID_T id, HAL_SPI_CS_T csNum, const HAL_SPI_CFG_T* spiCfg);



// =============================================================================
// hal_SpiActivateCs
// -----------------------------------------------------------------------------
/// Activate a Chip Select previously opened by a call to #hal_SpiOpen. Once 
/// the CS is no more used, it must be deactivated by a call to 
/// #hal_SpiDeActivateCs.
///
/// This is a non blocking function, the return value *MUST* be checked
/// 
/// If an attempt is made to activate a previously opened CS, two case can
/// happen:
/// - the SPI bus is free: the CS will be activated immediatly and 
/// the function returns TRUE.
/// - another CS is activated: the new activation is not done and
/// the function returns FALSE, the calling task must retry later.
///
/// The hal_SpiActivateCs function will always exit immediately.
///
/// A CS must be activated before being used, and only one CS at a time can be
/// activated.
///
/// This function requests a resource corresponding to the needed frequency.
///
/// @param id Identifier of the SPI for which the function is called.
/// @param csNum CS to activate.
/// @return \c TRUE if the cs has been successfully activated
/// \c FALSE when another cs is already active
// =============================================================================
bool hal_SpiActivateCs(HAL_SPI_ID_T id, HAL_SPI_CS_T csNum);




// =============================================================================
// hal_SpiDeActivateCs
// -----------------------------------------------------------------------------
/// Deactivate a Chip Select. This is the opposite operation to #hal_SpiActivateCs.
/// It must be called after #hal_SpiActivateCs to allow the activation of another
/// CS. The deactivation of a non-activated CS will trig an ASSERT.
///
/// This function release the resource to #HAL_SYS_FREQ_32K.
///
/// @param id Identifier of the SPI for which the function is called.
/// @param csNum CS to deactivate.
// =============================================================================
void hal_SpiDeActivateCs(HAL_SPI_ID_T id, HAL_SPI_CS_T csNum);

// =============================================================================
// hal_SpiRxIfcChannelRelease
// -----------------------------------------------------------------------------
/// Force the release of the RxIfc channel owned by 
/// the request HAL_IFC_SPI_RX.
// =============================================================================
void hal_SpiRxIfcChannelRelease(HAL_SPI_ID_T id);


// =============================================================================
// hal_SpiTxIfcChannelRelease
// -----------------------------------------------------------------------------
/// Force the release of the TxIfc channel owned by 
/// the request HAL_IFC_SPI_TX.
// =============================================================================
void hal_SpiTxIfcChannelRelease(HAL_SPI_ID_T id);


// =============================================================================
// hal_SpiClose
// -----------------------------------------------------------------------------
/// Close the SPI CS \c csNum.  To use it again,
/// it must be opened and activated again.
/// 
/// @param id Identifier of the SPI for which the function is called.
/// @param csNum SPI CSto close.
// =============================================================================
void hal_SpiClose(HAL_SPI_ID_T id, HAL_SPI_CS_T csNum);



// =============================================================================
// hal_SpiSendData
// -----------------------------------------------------------------------------
/// Send a bunch of data. 
/// This functions sends \c length bytes starting from the address \c 
/// start_address. The number returned is the number of bytes actually sent. 
/// In DMA mode, this function returns 0 when no DMA channel is available, it 
/// returns length otherwise. This function is not to be used for the infinite
/// mode. Use instead the dedicated driver function #hal_SpiStartInfiniteWrite.
///
/// @param id Identifier of the SPI for which the function is called.
/// @param csNum The CS to use to send the data. This cs must be activated before
/// sending data.
/// @param startAddress Pointer on the buffer to send
/// @param length number of bytes to send (Up to 4 kB) .
/// @return When in DMA mode, returns 0 if no DMA channel is available. \n
///         In direct mode or DMA mode with an available channel, returns the 
///         number of sent bytes. 
// =============================================================================
u32 hal_SpiSendData(
                HAL_SPI_ID_T id,
                HAL_SPI_CS_T csNum,
                const u8* startAddress,
                u32 length);

/*
u16 hal_SpiSendData(
                HAL_SPI_ID_T id,
                HAL_SPI_CS_T csNum,
                u8* startAddress,
                u16 length);
*/


// =============================================================================
// hal_SpiTxFifoAvail
// -----------------------------------------------------------------------------
/// Get available data spaces in the Spi Tx FIFO.
/// This function returns the size of the available space in the Tx FIFO. 
///
/// @param id Identifier of the SPI for which the function is called.
/// @param  csNum   Chip select
/// @return Tthe size of the available space in the Tx FIFO. 
// =============================================================================
u8 hal_SpiTxFifoAvail(HAL_SPI_ID_T id, HAL_SPI_CS_T csNum);



// =============================================================================
// hal_SpiTxDmaDone
// -----------------------------------------------------------------------------
/// Check if the transmission is finished. 
///
/// This function returns \c TRUE when the last DMA transfer is finished. 
/// Before sending new data in DMA mode, the previous transfer must be finished, 
/// hence the use of this function for polling.\n
/// Note that the DMA transfer can be finished but the Tx FIFO of the SPI is 
/// not empty. Before shutting down the SPI, one must check that the SPI FIFO 
/// is empty and that the last byte has been completely sent by using 
/// #hal_SpiTxFinished.\n
/// Even if the Tx FIFO is not empty, if a previous DMA transfer is over, one
/// can start a new DMA transfert
///
/// @param id Identifier of the SPI for which the function is called.
/// @param  csNum   Chip select
/// @return \c TRUE is the previous DMA transfert is finshed.\n
///          \c FALSE otherwise.
// =============================================================================
bool hal_SpiTxDmaDone(HAL_SPI_ID_T id, HAL_SPI_CS_T csNum);



// =============================================================================
// hal_SpiTxFinished
// -----------------------------------------------------------------------------
/// Check if the last transfer is done 
/// This function returns \c TRUE when the transmit FIFO is empty and when the 
/// last byte is completely sent. It should be called before closing the SPI if 
/// the last bytes of the transfer are important.\n
/// This function should not be called between transfers, in direct or DMA mode. 
/// The @link #hal_SpiTxFifoAvail FIFO availability @endlink for direct mode and the 
/// @link #hal_SpiTxDmaDone DMA done indication @endlink for DMA allow 
/// for a more optimized transmission.
///
/// @param id Identifier of the SPI for which the function is called.
/// @param  csNum   Chip select
/// @return \c TRUE if the last tranfer is done and the Tx FIFO empty.\n
///         \c FALSE otherwise. 
// =============================================================================
bool hal_SpiTxFinished(HAL_SPI_ID_T id, HAL_SPI_CS_T csNum);


// =============================================================================
// hal_SpiGetData
// -----------------------------------------------------------------------------
/// Get a bunch of data. 
///
/// This functions gets \c length bytes from the SPI and stores them starting 
/// from the address \c dest_address. The number returned is the number of bytes 
/// actually received. In DMA mode, this function returns 0 when no DMA channel 
/// is available. It returns length otherwise.
/// A pattern mode is available.
/// When enabled, the actual reception and count of received bytes starts
/// only after a precised pattern has been read on the SPI. This pattern is not
/// copied into the reception buffer.
/// This feature is only enabled by using #hal_SpiStartInfiniteWrite.
///
/// @param id Identifier of the SPI for which the function is called.
/// @param  csNum   Chip select
/// @param destAddress Pointer on the buffer to store received data
/// @param length Number of byte to receive.
/// @return When in DMA mode, returns 0 if no DMA channel is available. \n
///         In direct mode or DMA mode with an available channel, returns the 
///         number of received bytes. 
// =============================================================================
u32 hal_SpiGetData(
                HAL_SPI_ID_T id, 
                HAL_SPI_CS_T csNum,
                u8*      destAddress,
                u32      length
                );

// =============================================================================
// hal_SpiRxFifoLevel
// -----------------------------------------------------------------------------
/// Get data quantity in the Spi Rx FIFO. 
///
/// Returns the number of bytes in the Rx FIFO.
/// @param id Identifier of the SPI for which the function is called.
/// @param  csNum   Chip select
/// @return The number of bytes in the Rx FIFO
// =============================================================================
u8 hal_SpiRxFifoLevel(HAL_SPI_ID_T id, HAL_SPI_CS_T csNum);



// =============================================================================
// hal_SpiRxDmaDone
// -----------------------------------------------------------------------------
/// Check if the reception is finished. 
///
/// @param id Identifier of the SPI for which the function is called.
/// @param  csNum   Chip select
/// @return This function returns \c TRUE when the last DMA transfer is finished. 
/// Before receiving new data in DMA mode, the previous transfer must be 
/// finished, hence the use of this function for polling.
// =============================================================================
bool hal_SpiRxDmaDone(HAL_SPI_ID_T id, HAL_SPI_CS_T csNum);





// =============================================================================
// hal_SpiStartInfiniteWriteMode
// -----------------------------------------------------------------------------
/// Do a stream write on the given CS.
/// @param id Identifier of the SPI for which the function is called.
/// @param csNum CS on which send the stream.
/// @param infTx Pointer to the structure describing the streaming write.
// =============================================================================
void hal_SpiStartInfiniteWriteMode(HAL_SPI_ID_T id, HAL_SPI_CS_T csNum, const HAL_SPI_INFINITE_TX_T* infTx);




// =============================================================================
// hal_SpiStopInfiniteWriteMode
// -----------------------------------------------------------------------------
/// Stop the stream write on the given CS.
/// This function is useless when the auto-stop mode is enabled.
/// @param id Identifier of the SPI for which the function is called.
/// @param csNum CS on which send the stream.
// =============================================================================
void hal_SpiStopInfiniteWriteMode(HAL_SPI_ID_T id, HAL_SPI_CS_T csNum);



// =============================================================================
// hal_SpiFlushFifos
// -----------------------------------------------------------------------------
/// Flush both SPI Fifos.
/// @param id Identifier of the SPI for which the function is called.
// =============================================================================
void hal_SpiFlushFifos(HAL_SPI_ID_T id);


// =============================================================================
// hal_SpiClearRxDmaDone
// -----------------------------------------------------------------------------
/// clear the RX DMA Done status
/// @param id Identifier of the SPI for which the function is called.
// =============================================================================
void hal_SpiClearRxDmaDone(HAL_SPI_ID_T id);


// =============================================================================
// hal_SpiClearTxDmaDone
// -----------------------------------------------------------------------------
/// clear the TX DMA Done status
/// @param id Identifier of the SPI for which the function is called.
// =============================================================================
void hal_SpiClearTxDmaDone(HAL_SPI_ID_T id);


// =============================================================================
// hal_SpiIfcTc
// -----------------------------------------------------------------------------
/// @param id Identifier of the SPI for which the function is called.
/// @return ifcTc
// =============================================================================
u16 hal_SpiGetRxIfcTc(HAL_SPI_ID_T id);




// =============================================================================
// hal_SpiForcePin
// -----------------------------------------------------------------------------
/// Force an SPI pin to a certain state or release the force mode
/// and put the pin back to normal SPI mode.
///
/// @param id Identifier of the SPI for which the function is called.
/// @param pin Identification of the pins to be forced.
/// @param state State to enforce.
// =============================================================================
void hal_SpiForcePin(HAL_SPI_ID_T id, HAL_SPI_PIN_T pin,  HAL_SPI_PIN_STATE_T state);


void hal_GpioOpen(HAL_GPIO_GPIO_ID_T gpio, const HAL_GPIO_CFG_T* cfg);



#endif //_HAL_SPI_H_
