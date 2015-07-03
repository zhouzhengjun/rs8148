/*
 * Header for SPI Driver
 *
 *
 */

#ifndef _SPI_LINUX_H_
#define _SPI_LINUX_H_

#ifdef __KERNEL__

#include "osTIType.h"
#include <linux/proc_fs.h>
#include <asm/io.h>
#include <asm/irq.h>
//#include "dma.h"

/***********/
/* defines */
/***********/
#define TRUE 1
#define FALSE 0

#define READ_U32(addr)\
	omap_readl(addr)       /* inl(addr) */
#define WRITE_U32(addr,val)\
	omap_writel(val,addr)  /* outl(val,addr) */
#define READ_U16(addr)\
	omap_readw(addr)        /* inw(addr) */
#define WRITE_U16(addr,val)\
	omap_writew(val, addr)     /* outw(val,addr) */

#define os_memCopy(pDstPtr,pSrcPtr,Size)\
	memcpy(pDstPtr,pSrcPtr,Size)

#define va2pa(addr)\
	__pa(addr)


/*********/
/* DMA support */
/*********/

/* DMA control register structure, without LCD registers */
typedef struct {
    volatile u16 csdp;      // channel s/d parameters -- working set (current transfer)
    volatile u16 ccr;       // channel control -- working set
    volatile u16 cicr;      // channel interrupt control -- working set
    volatile u16 csr;       // channel status -- working set
    volatile u16 cssa_l;    // source lower bits -- programming set (next transfer)
    volatile u16 cssa_u;    // source upper bits -- programming set
    volatile u16 cdsa_l;    // destn lower bits -- programming set
    volatile u16 cdsa_u;    // destn upper bits -- programming set
    volatile u16 cen;       // channel element number -- programming set
    volatile u16 cfn;       // channel frame number -- programming set
    volatile u16 csfi;      // channel source frame index -- programming set
    volatile u16 csei;      // channel source element index -- programming set
    volatile u16 csac;      // channel source address counter
    volatile u16 cdac;      // channel dest. address counter
    volatile u16 cdei;      // channel dest. element index -- programming set
    volatile u16 cdfi;      // channel dest. frame index -- programming set
    volatile u16 color_l;   // graphics&LCD channels color - lower bits
    volatile u16 color_u;   // graphics&LCD channels color - upper bits
    volatile u16 ccr2;      // channel control 2
    volatile u16 reserved;  // reserved
    volatile u16 clink_ctrl;// link control
    volatile u16 lch_ctrl;  // channel type control
    volatile u16 null[10];  // for alignment
} dma_regs_t;

/*
  Defines for register state.
*//*
  Defines for register state.
  */

#define DCSDP_DST_BURST_EN_BIT 14
#define DCSDP_SRC_BURST_EN_BIT  7
#define   NO_BURST 0x0
#define   BURST_4  0x2
#define   BURST_8  0x3
#define DCSDP_DST_PACK         (1<<13)
#define DCSDP_DEST_PORT_BIT     9
#define DCSDP_SRC_PORT_BIT      2
#define   PORT_EMIFF  0x0
#define   PORT_EMIFS  0x1     //old EMIF
#define   PORT_OCP_T1 0x2     //old IMIF
#define   PORT_TIPB   0x3
#define   PORT_OCP_T2 0x4     //old LOCAL
#define   PORT_MPUI   0x5     //old TIPB_MPUI
#define DCSDP_SRC_PACK          (1<<6)
#define DCSDP_DATA_TYPE_BIT     0
#define   DATA_TYPE_S8  0x0
#define   DATA_TYPE_S16 0x1
#define   DATA_TYPE_S32 0x2


#define DCSR_ERROR  0x3
#define DCSR_SYNC_SET (1 << 6)

#define DCCR_FS      (1 << 5)
#define DCCR_PRIO    (1 << 6)
#define DCCR_EN      (1 << 7)
#define DCCR_AI      (1 << 8)
#define DCCR_REPEAT  (1 << 9)
#define DCCR_N31COMP (1 << 10) // if 0 the channel works in 3.1 compatible mode
#define DCCR_EP      (1 << 11)
#define DCCR_SRC_AMODE_BIT  12
#define DCCR_SRC_AMODE_MASK (0x3<<12)
#define DCCR_DST_AMODE_BIT  14
#define DCCR_DST_AMODE_MASK (0x3<<14)
#define   AMODE_CONST        0x0
#define   AMODE_POST_INC     0x1
#define   AMODE_SINGLE_INDEX 0x2
#define   AMODE_DOUBLE_INDEX 0x3

#define LCH_TYPE_2D 0  // non-iterleaved transfers
#define LCH_TYPE_G  1  // graphical
#define LCH_TYPE_P  2  // periferial - most common type
#define LCH_TYPE_D  4  // LCD
#define LCH_TYPE_PD 15 // dedicated channel

#define DCSDP_DST_BURST_EN_BIT 14
#define DCSDP_SRC_BURST_EN_BIT  7
#define   NO_BURST 0x0
#define   BURST_4  0x2
#define   BURST_8  0x3
#define DCSDP_DST_PACK         (1<<13)
#define DCSDP_DEST_PORT_BIT     9
#define DCSDP_SRC_PORT_BIT      2
#define   PORT_EMIFF  0x0
#define   PORT_EMIFS  0x1     //old EMIF
#define   PORT_OCP_T1 0x2     //old IMIF
#define   PORT_TIPB   0x3
#define   PORT_OCP_T2 0x4     //old LOCAL
#define   PORT_MPUI   0x5     //old TIPB_MPUI
#define DCSDP_SRC_PACK          (1<<6)
#define DCSDP_DATA_TYPE_BIT     0
#define   DATA_TYPE_S8  0x0
#define   DATA_TYPE_S16 0x1
#define   DATA_TYPE_S32 0x2


#define DCSR_ERROR  0x3
#define DCSR_SYNC_SET (1 << 6)

#define DCCR_FS      (1 << 5)
#define DCCR_PRIO    (1 << 6)
#define DCCR_EN      (1 << 7)
#define DCCR_AI      (1 << 8)
#define DCCR_REPEAT  (1 << 9)
#define DCCR_N31COMP (1 << 10) // if 0 the channel works in 3.1 compatible mode
#define DCCR_EP      (1 << 11)
#define DCCR_SRC_AMODE_BIT  12
#define DCCR_SRC_AMODE_MASK (0x3<<12)
#define DCCR_DST_AMODE_BIT  14
#define DCCR_DST_AMODE_MASK (0x3<<14)
#define   AMODE_CONST        0x0
#define   AMODE_POST_INC     0x1
#define   AMODE_SINGLE_INDEX 0x2
#define   AMODE_DOUBLE_INDEX 0x3

#define LCH_TYPE_2D 0  // non-iterleaved transfers
#define LCH_TYPE_G  1  // graphical
#define LCH_TYPE_P  2  // periferial - most common type
#define LCH_TYPE_D  4  // LCD
#define LCH_TYPE_PD 15 // dedicated channel

/*********/
/* enums */
/*********/

/*********/
/* types */
/*********/
typedef void (*dma_CB_t)(int lch, UINT16 ch_status, void *data);

typedef struct SPI_DMAData_s
{
	dma_regs_t*	pTxDMARegs;
	dma_regs_t*	pRxDMARegs;

	dma_addr_t	buf_dma_phys_Tx; /* pre-allocate DMA Tx buffer physical address */
    void*		buf_dma_virt_Tx; /* pre-allocate DMA Tx buffer virtual address */
	dma_addr_t	buf_dma_phys_Rx; /* pre-allocate DMA Rx buffer physical address */
    void*		buf_dma_virt_Rx; /* pre-allocate DMA Rx buffer virtual address */

	dma_CB_t	cbFunc_DMADone_Tx;
	void*		cbArg_DMADone_Tx;
	dma_CB_t	cbFunc_DMADone_Rx;
	void*		cbArg_DMADone_Rx;

    int         channelTxInd;
    int         channelRxInd;

} SPI_DMAData_t;

/*************/
/* functions */
/*************/
int os_AllocMem(int size,u8** mem);
int os_FreeMem(u8* mem);
int os_AllocDMA(SPI_DMAData_t* pDMAdata,int sizeOfDMABlock);
int os_FreeDMA(SPI_DMAData_t* pDMAdata,int sizeOfDMABlock);

/*********/
/* DEBUG */
/*********/
enum DEBUG_LEVELS
{
	LEVEL0 = 0,
	LEVEL1
};

//#define CONFIG_WSPI_DEBUG

#undef DEBUG
#ifdef CONFIG_WSPI_DEBUG
extern int g_wspi_debug;		/* nick +++ */
#define DEBUG(n, fmt, args...)\
	if (n <=  g_wspi_debug) printk(KERN_INFO "\n  %s(%d): %s: " fmt,__FILE__,__LINE__,__FUNCTION__ , ## args);
#else
#define DEBUG(n, args...)
#endif // CONFIG_WSPI_DEBUG

#endif /* #ifdef __KERNEL__ */

#endif /* OMAP_SDIO_H */

