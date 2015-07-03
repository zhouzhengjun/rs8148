#include <command.h>
#include <common.h>
#include <asm/io.h>

#define PRCM_REG_BASE 			0x48180000
#define PRCM_REG_SIZE 			(12*1024)

/*For TI816X */
#ifdef CONFIG_TI816X

#define CONFIG_720P
#define SET_BOARD()                     ti816x_set_board()
#define CTRL_MODULE_BASE_ADDR    	0x48140000
#define CM_CLKOUT_CTRL           	(PRCM_REG_BASE + 0x100)

#define VIDEOPLL_CTRL            	(CTRL_MODULE_BASE_ADDR + 0x470)
#define VIDEOPLL_PWD             	(CTRL_MODULE_BASE_ADDR + 0x474)
#define VIDEOPLL_FREQ1           	(CTRL_MODULE_BASE_ADDR + 0x478)
#define VIDEOPLL_DIV1            	(CTRL_MODULE_BASE_ADDR + 0x47C)
#define VIDEOPLL_FREQ2           	(CTRL_MODULE_BASE_ADDR + 0x480)
#define VIDEOPLL_DIV2            	(CTRL_MODULE_BASE_ADDR + 0x484)
#define VIDEOPLL_FREQ3          	(CTRL_MODULE_BASE_ADDR + 0x488)
#define VIDEOPLL_DIV3            	(CTRL_MODULE_BASE_ADDR + 0x48C)

/*HDVPSS*/
#define CM_HDDSS_CLKSTCTRL       	(PRCM_REG_BASE + 0x0404)
#define CM_HDMI_CLKSTCTRL        	(PRCM_REG_BASE + 0x0408)
#define CM_ACTIVE_HDDSS_CLKCTRL  	(PRCM_REG_BASE + 0x0424)
#define CM_SYSCLK13_CLKSEL       	(PRCM_REG_BASE + 0x0334)
#define CM_SYSCLK15_CLKSEL       	(PRCM_REG_BASE + 0x0338)
#define CM_ACTIVE_HDMI_CLKCTRL   	(PRCM_REG_BASE + 0x0428)

#endif

/*For TI814X */
#ifdef CONFIG_TI814X

#define CONFIG_480P
#define SET_BOARD()                     ti814x_set_board()
#define CM_HDVPSS_CLKSTCTRL       	(PRCM_REG_BASE + 0x0800)
#define CM_HDVPSS_HDVPSS_CLK_CTRL   	(PRCM_REG_BASE + 0x0820)
#define CM_HDVPSS_HDMI_CLKCTRL      	(PRCM_REG_BASE + 0x0824)
#define PM_HDVPSS_PWRSTCTRL         	(PRCM_REG_BASE + 0x0E00)
#define PM_HDVPSS_PWRSTST           	(PRCM_REG_BASE + 0x0E04)
#define RM_HDVPSS_RSTCTRL               (PRCM_REG_BASE + 0x0E10)
#define RM_HDVPSS_RSTST             	(PRCM_REG_BASE + 0x0E14)

#define PLL_REG_BASE			0x481C5000

#define CLKCTRL   			0x04
#define TENABLE                        0x08
#define TENABLEDIV			0x0C
#define M2NDIV				0x10
#define MN2DIV				0x14
#define STATUS				0x24
#define OSC_FREQ			20

#define PLL_HDVPSS_PWRCTRL        	(PLL_REG_BASE + 0x0170)
#define PLL_HDVPSS_BASE           	PLL_HDVPSS_PWRCTRL
#define PLL_VIDEO2_PWRCTRL        	(PLL_REG_BASE + 0x0200)
#define PLL_VIDEO2_BASE           	PLL_VIDEO2_PWRCTRL
#define PLL_VIDEO2_CLKCTRL        	(PLL_REG_BASE + 0x0204)
#define PLL_VIDEO2_TENABLE        	(PLL_REG_BASE + 0x0208)
#define PLL_VIDEO2_TENABLEDIV     	(PLL_REG_BASE + 0x020C)
#define PLL_VIDEO2_M2NDIV         	(PLL_REG_BASE + 0x0210)
#define PLL_VIDEO2_MN2DIV        	(PLL_REG_BASE + 0x0214)
#define PLL_VIDEO2_FRACDIV        	(PLL_REG_BASE + 0x0218)
#define PLL_VIDEO2_BWCTRL         	(PLL_REG_BASE + 0x021C)
#define PLL_VIDEO2_FRACCTRL       	(PLL_REG_BASE + 0x0220)
#define PLL_VIDEO2_STATUS         	(PLL_REG_BASE + 0x0224)
#define PLL_VIDEO2_PINMUX         	(PLL_REG_BASE + 0x02C8)
#define PLL_OSC_SRC_CTRL          	(PLL_REG_BASE + 0x02C0)

#endif

/* Common for TI814X and TI816X */
#define HDMI_REG_BASE 0x46C00000

#define FRAME_BUFFER_ADDRESS		0x96D00000
#define VPDMA_REG_BASE 			0x4810D000

#define vpdma_read32(off)            *(volatile int*)(VPDMA_REG_BASE+(off))
#define vpdma_write32(off, value)    (*(volatile int*)(VPDMA_REG_BASE+(off)) =(value))

#define DATA_TYPE                       0x6
#define VPDMA_DESC_BUFFER               0x81600000  /* Descriptor buffer */
#define VPDMA_LIST_ADDR                 (VPDMA_REG_BASE + 0x004)
#define VPDMA_LIST_ATTR                 (VPDMA_REG_BASE + 0x008)
#define VPDMA_GRPX0_DATA_CSTAT          (VPDMA_REG_BASE + 0x37c)
#define VPDMA_GRPX1_DATA_CSTAT          (VPDMA_REG_BASE + 0x380)
#define VPDMA_GRPX2_DATA_CSTAT          (VPDMA_REG_BASE + 0x384)
#define VPDMA_LIST_NUMBER               2 
#define LOGO_TIME_MULTIPLIER            60
#define DEFAULT_LOGO_TIME               56
#define MAX_LOGO_TIME                   56
#define HDMI_TIMING_NUMBER              100

#define GRPX0                           0
#define GRPX1                           1
#define GRPX2                           2
#define GRPX0_CHANNEL_NUMBER            29
#define GRPX1_CHANNEL_NUMBER            30
#define GRPX2_CHANNEL_NUMBER            31

#define VPS_REG_BASE                    0x48100000

#define VPS_CLKC_ENABLE                 (VPS_REG_BASE + 0x0100)
#define VPS_CLKC_VENC_CLK_SELECT        (VPS_REG_BASE + 0x0114)
#define VPS_CLKC_VENC_ENABLE            (VPS_REG_BASE + 0x0118)
#define VPS_COMP_STATUS                 (VPS_REG_BASE + 0x5200)
#define VPS_COMP_BGCOLOR                (VPS_REG_BASE + 0x5214)
#define VPS_CLKC_RESET                  (VPS_REG_BASE + 0x0104)
#define VPS_COMP_HDMI_VOUT1             (VPS_REG_BASE + 0x5204)

#define LOGO_BGCOLOR                    0x0	/*Black*/

#ifdef CONFIG_480P
#define WIDTH                           720
#define HEIGHT                          480
#elif defined CONFIG_720P
#define WIDTH                           1280
#define HEIGHT                          720
#else
#define WIDTH                           1920
#define HEIGHT                          1080
#endif

#define STRIDE                          3 * WIDTH
#define FRAME_SIZE                      (STRIDE * HEIGHT)

struct pll_config_t
{
	u32 n;
	u32 m;
	u32 m2; 
	u32 clk_ctrl;
};

enum vps_module_t
{       
	VPS_MODULE_VPDMA = 0
};

#ifdef CONFIG_TI816X
void ti816x_set_board(void);
#endif

#ifdef CONFIG_TI814X
void ti814x_set_board(void);
#endif
