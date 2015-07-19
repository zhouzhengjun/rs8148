/*
 * Code for TI8148 EVM.
 *
 * Copyright (C) 2010 Texas Instruments, Inc. - http://www.ti.com/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/spi/spi.h>
#include <linux/spi/flash.h>
#include <linux/spi/slic_spi.h>
#include <linux/platform_device.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/partitions.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/phy.h>
#include <linux/i2c/at24.h>
#include <linux/i2c/qt602240_ts.h>
#include <linux/i2c/pcf857x.h>
#include <linux/regulator/machine.h>
#include <linux/mfd/tps65910.h>
#ifdef CONFIG_WL12XX_PLATFORM_DATA
#include <linux/wl12xx.h>
#endif
#include <linux/clk.h>
#include <linux/err.h>

#include <mach/hardware.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/map.h>

#include <plat/mcspi.h>
#include <plat/irqs.h>
#include <plat/board.h>
#include <plat/common.h>
#include <plat/asp.h>
#include <plat/usb.h>
#include <plat/mmc.h>
#include <plat/gpmc.h>
#include <plat/nand.h>
#include <plat/hdmi_lib.h>
#include <mach/board-ti814x.h>

#include "board-flash.h"
#include "clock.h"
#include "mux.h"
#include "hsmmc.h"
#include "control.h"
#include "cm81xx.h"

#define GPIO_TSC               31

#ifdef CONFIG_OMAP_MUX
static struct omap_board_mux board_mux[] __initdata = {
#if defined(CONFIG_MACH_TI8148_BELLLITE)
	{									
		.reg_offset = (TI814X_CONTROL_PADCONF_MLBP_SIG_IO_CTRL_OFFSET),	
		.value		= 0x3C,
	},
	{									
		.reg_offset = (TI814X_CONTROL_PADCONF_MLBP_DAT_IO_CTRL_OFFSET),	
		.value		= 0x3C,
	},
#endif	
	{ .reg_offset = OMAP_MUX_TERMINATOR },
};
#else
#define board_mux     NULL
#endif
//GP0[1]
#define GPIO_PCIE_RESET	((1))

static struct omap2_hsmmc_info mmc[] = {
	{
#ifdef CONFIG_WL12XX_PLATFORM_DATA
#if 0
/* WLAN_EN is GP2[22] */
#define GPIO_WLAN_EN	((2 * 32) + 22)
/* WLAN_IRQ is GP2[24] */
#define GPIO_WLAN_IRQ	((2 * 32) + 24)
#else	//bell
/* WLAN_EN is GP1[09] */
#define GPIO_WLAN_EN	((1 * 32) + 9)
/* WLAN_IRQ is GP0[16] */
#define GPIO_WLAN_IRQ	((0 * 32) + 16)
#define GPIO_BT_EN	((1 * 32) + 8)
#define GPIO_BT_TEST	((1 * 32) + 10)
#define GPIO_BT_WAKEUP	((0 * 32) + 17)

#endif
		.mmc		= 1,
		.caps		= MMC_CAP_4_BIT_DATA | MMC_CAP_POWER_OFF_CARD,
		.gpio_cd	= -EINVAL, /* Dedicated pins for CD and WP */
		.gpio_wp	= -EINVAL,


		.ocr_mask	= MMC_VDD_165_195 ,
		.nonremovable	= true,

	},

	{
		.mmc            = 2,
#else
		.mmc		= 1,
#endif
		.caps		= MMC_CAP_4_BIT_DATA,
               /* modify mark 15-3-24*/
               .gpio_cd	= -EINVAL, /* Dedicated pins for CD and WP */
			   .gpio_wp	= -EINVAL,
               .ocr_mask       = MMC_VDD_33_34,
       },







	{}	/* Terminator */
};

#define GPIO_LCD_PWR_DOWN	0

static int setup_gpio_ioexp(struct i2c_client *client, int gpio_base,
	 unsigned ngpio, void *context) {
	int ret = 0;
	int gpio = gpio_base + GPIO_LCD_PWR_DOWN;

	ret = gpio_request(gpio, "lcd_power");
	if (ret) {
		printk(KERN_ERR "%s: failed to request GPIO for LCD Power"
			": %d\n", __func__, ret);
		return ret;
	}

	gpio_export(gpio, true);
	gpio_direction_output(gpio, 0);

	return 0;
}

/* IO expander data */
static struct pcf857x_platform_data io_expander_data = {
	.gpio_base	= 4 * 32,
	.setup		= setup_gpio_ioexp,
};
static struct i2c_board_info __initdata ti814x_i2c_boardinfo1[] = {
	{
		I2C_BOARD_INFO("pcf8575_1", 0x21),
	},

};

#define VPS_VC_IO_EXP_RESET_DEV_MASK        (0x0Fu)
#define VPS_VC_IO_EXP_SEL_VIN0_S1_MASK      (0x04u)
#define VPS_VC_IO_EXP_THS7368_DISABLE_MASK  (0x10u)
#define VPS_VC_IO_EXP_THS7368_BYPASS_MASK   (0x20u)
#define VPS_VC_IO_EXP_THS7368_FILTER1_MASK  (0x40u)
#define VPS_VC_IO_EXP_THS7368_FILTER2_MASK  (0x80u)
#define VPS_VC_IO_EXP_THS7368_FILTER_SHIFT  (0x06u)


static const struct i2c_device_id pcf8575_video_id[] = {
	{ "pcf8575_1", 0 },
	{ }
};
static struct i2c_client *pcf8575_client;
static unsigned char pcf8575_port[2] = {0x4F, 0x7F};
int vps_ti814x_select_video_decoder(int vid_decoder_id);

static int pcf8575_video_probe(struct i2c_client *client,
				const struct i2c_device_id *id)
{
	pcf8575_client = client;
	vps_ti814x_select_video_decoder(0);
	return 0;
}

static int __devexit pcf8575_video_remove(struct i2c_client *client)
{
	pcf8575_client = NULL;
	return 0;
}

static struct i2c_driver pcf8575_driver = {
	.driver = {
		.name   = "pcf8575_1",
	},
	.probe          = pcf8575_video_probe,
	.remove         = pcf8575_video_remove,
	.id_table       = pcf8575_video_id,
};

int ti814x_pcf8575_init(void)
{
	i2c_add_driver(&pcf8575_driver);
	return 0;
}
EXPORT_SYMBOL(ti814x_pcf8575_init);

int ti814x_pcf8575_exit(void)
{
	i2c_del_driver(&pcf8575_driver);
	return 0;
}
EXPORT_SYMBOL(ti814x_pcf8575_exit);
#define VPS_VC_IO_EXP_RESET_DEV_MASK        (0x0Fu)
#define VPS_VC_IO_EXP_SEL_VIN0_S1_MASK      (0x04u)
#define VPS_VC_IO_EXP_THS7368_DISABLE_MASK  (0x10u)
#define VPS_VC_IO_EXP_THS7368_BYPASS_MASK   (0x20u)
#define VPS_VC_IO_EXP_THS7368_FILTER1_MASK  (0x40u)
#define VPS_VC_IO_EXP_THS7368_FILTER2_MASK  (0x80u)
#define VPS_VC_IO_EXP_THS7368_FILTER_SHIFT  (0x06u)
int vps_ti814x_select_video_decoder(int vid_decoder_id)
{
	int ret = 0;
	struct i2c_msg msg = {
			.addr = pcf8575_client->addr,
			.flags = 0,
			.len = 2,
		};
	msg.buf = pcf8575_port;
	if (VPS_SEL_TVP7002_DECODER == vid_decoder_id)
		pcf8575_port[1] &= ~VPS_VC_IO_EXP_SEL_VIN0_S1_MASK;
	else
		pcf8575_port[1] |= VPS_VC_IO_EXP_SEL_VIN0_S1_MASK;
	ret = (i2c_transfer(pcf8575_client->adapter, &msg, 1));
	if (ret < 0)
		printk(KERN_ERR "I2C: Transfer failed at %s %d with error code: %d\n",
			__func__, __LINE__, ret);
	return ret;
}
EXPORT_SYMBOL(vps_ti814x_select_video_decoder);

#define I2C_RETRY_COUNT 10u
int vps_ti814x_set_tvp7002_filter(enum fvid2_standard standard)
{
	int filter_sel;
	int ret;
	struct i2c_msg msg = {
			.addr = pcf8575_client->addr,
			.flags = 0,
			.len = 2,
		};

	pcf8575_port[0] &= ~(VPS_VC_IO_EXP_THS7368_DISABLE_MASK
		| VPS_VC_IO_EXP_THS7368_BYPASS_MASK
		| VPS_VC_IO_EXP_THS7368_FILTER1_MASK
		| VPS_VC_IO_EXP_THS7368_FILTER2_MASK);
	switch (standard) {
	case FVID2_STD_1080P_60:
	case FVID2_STD_1080P_50:
	case FVID2_STD_SXGA_60:
	case FVID2_STD_SXGA_75:
	case FVID2_STD_SXGAP_60:
	case FVID2_STD_SXGAP_75:
	case FVID2_STD_UXGA_60:
		filter_sel = 0x03u;  /* Filter2: 1, Filter1: 1 */
		break;
	case FVID2_STD_1080I_60:
	case FVID2_STD_1080I_50:
	case FVID2_STD_1080P_24:
	case FVID2_STD_1080P_30:
	case FVID2_STD_720P_60:
	case FVID2_STD_720P_50:
	case FVID2_STD_SVGA_60:
	case FVID2_STD_SVGA_72:
	case FVID2_STD_SVGA_75:
	case FVID2_STD_SVGA_85:
	case FVID2_STD_XGA_60:
	case FVID2_STD_XGA_70:
	case FVID2_STD_XGA_75:
	case FVID2_STD_XGA_85:
	case FVID2_STD_WXGA_60:
	case FVID2_STD_WXGA_75:
	case FVID2_STD_WXGA_85:
		filter_sel = 0x01u;  /* Filter2: 0, Filter1: 1 */
		break;
	case FVID2_STD_480P:
	case FVID2_STD_576P:
	case FVID2_STD_VGA_60:
	case FVID2_STD_VGA_72:
	case FVID2_STD_VGA_75:
	case FVID2_STD_VGA_85:
		filter_sel = 0x02u;  /* Filter2: 1, Filter1: 0 */
		break;
	case FVID2_STD_NTSC:
	case FVID2_STD_PAL:
	case FVID2_STD_480I:
	case FVID2_STD_576I:
	case FVID2_STD_D1:
		filter_sel = 0x00u;  /* Filter2: 0, Filter1: 0 */
		break;

	default:
		filter_sel = 0x01u;  /* Filter2: 0, Filter1: 1 */
		break;
	}
	pcf8575_port[0] |=
		(filter_sel << VPS_VC_IO_EXP_THS7368_FILTER_SHIFT);
	msg.buf = pcf8575_port;
	ret =  (i2c_transfer(pcf8575_client->adapter, &msg, 1));
	if (ret < 0) {
		printk(KERN_ERR "I2C: Transfer failed at %s %d with error code: %d\n",
			__func__, __LINE__, ret);
		return ret;
	}
	return ret;
}
EXPORT_SYMBOL(vps_ti814x_set_tvp7002_filter);
/* Touchscreen platform data */
static struct qt602240_platform_data ts_platform_data = {
	.x_line		= 18,
	.y_line		= 12,
	.x_size		= 800,
	.y_size		= 480,
	.blen		= 0x01,
	.threshold	= 30,
	.voltage	= 2800000,
	.orient		= QT602240_HORIZONTAL_FLIP,
};

static struct at24_platform_data eeprom_info = {
	.byte_len       = (256*1024) / 8,
	.page_size      = 64,
	.flags          = AT24_FLAG_ADDR16,
};

static struct regulator_consumer_supply ti8148evm_mpu_supply =
	REGULATOR_SUPPLY("mpu", NULL);

/*
 * DM814x/AM387x (TI814x) devices have restriction that none of the supply to
 * the device should be turned of.
 *
 * NOTE: To prevent turning off regulators not explicitly consumed by drivers
 * depending on it, ensure following:
 *	1) Set always_on = 1 for them OR
 *	2) Avoid calling regulator_has_full_constraints()
 *
 * With just (2), there will be a warning about incomplete constraints.
 * E.g., "regulator_init_complete: incomplete constraints, leaving LDO8 on"
 *
 * In either cases, the supply won't be disabled.
 *
 * We are taking approach (1).
 */
static struct regulator_init_data tps65911_reg_data[] = {
	/* VRTC */
	{
		.constraints = {
			.min_uV = 1800000,
			.max_uV = 1800000,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE |
						REGULATOR_CHANGE_STATUS,
			.always_on = 1,
		},
	},

	/* VIO -VDDA 1.8V */
	{
		.constraints = {
			.min_uV = 1500000,
			.max_uV = 1500000,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE |
						REGULATOR_CHANGE_STATUS,
			.always_on = 1,
		},
	},

	/* VDD1 - MPU */
	{
		.constraints = {
			.min_uV = 600000,
			.max_uV = 1500000,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE,
			.always_on = 1,
		},
		.num_consumer_supplies	= 1,
		.consumer_supplies	= &ti8148evm_mpu_supply,
	},

	/* VDD2 - DSP */
	{
		.constraints = {
			.min_uV = 600000,
			.max_uV = 1500000,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE |
						REGULATOR_CHANGE_STATUS,
			.always_on = 1,
		},
	},

	/* VDDCtrl - CORE */
	{
		.constraints = {
			.min_uV = 600000,
			.max_uV = 1400000,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE,
			.always_on = 1,
		},
	},

	/* LDO1 - VDAC */
	{
		.constraints = {
			.min_uV = 1100000,
			.max_uV = 3300000,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE |
						REGULATOR_CHANGE_STATUS,
			.always_on = 1,
		},
	},

	/* LDO2 - HDMI */
	{
		.constraints = {
			.min_uV = 1100000,
			.max_uV = 3300000,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE |
						REGULATOR_CHANGE_STATUS,
			.always_on = 1,
		},
	},

	/* LDO3 - GPIO 3.3V */
	{
		.constraints = {
			.min_uV = 1100000,
			.max_uV = 3300000,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE |
						REGULATOR_CHANGE_STATUS,
			.always_on = 1,
		},
	},

	/* LDO4 - PLL 1.8V */
	{
		.constraints = {
			.min_uV = 1100000,
			.max_uV = 3300000,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE,
			.always_on = 1,
		},
	},

	/* LDO5 - SPARE */
	{
		.constraints = {
			.min_uV = 1100000,
			.max_uV = 3300000,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE |
						REGULATOR_CHANGE_STATUS,
			.always_on = 1,
		},
	},

	/* LDO6 - CDC */
	{
		.constraints = {
			.min_uV = 1100000,
			.max_uV = 3300000,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE,
			.always_on = 1,
		},
	},

	/* LDO7 - SPARE */
	{
		.constraints = {
			.min_uV = 1100000,
			.max_uV = 3300000,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE |
						REGULATOR_CHANGE_STATUS,
			.always_on = 1,
		},
	},

	/* LDO8 - USB 1.8V */
	{
		.constraints = {
			.min_uV = 1100000,
			.max_uV = 3300000,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE |
						REGULATOR_CHANGE_STATUS,
			.always_on = 1,
		},
	},
};

static struct tps65910_board __refdata tps65911_pdata = {
	.irq				= 0,	/* No support currently */
	.gpio_base			= 0,	/* No support currently */
	.tps65910_pmic_init_data	= tps65911_reg_data,
};

static struct i2c_board_info __initdata ti814x_i2c_boardinfo[] = {

	{
		I2C_BOARD_INFO("eeprom", 0x50),
		.platform_data	= &eeprom_info,
	},
	{
		I2C_BOARD_INFO("cpld", 0x23),
	},

	{
		I2C_BOARD_INFO("tlv320aic3x", 0x18),
	},
	{
		I2C_BOARD_INFO("si9135", 0x60),
	},	
		

};

static void __init ti814x_tsc_init(void)
{
	int error;

	omap_mux_init_signal("mlb_clk.gpio0_31", TI814X_PULL_DIS | (1 << 18));

	error = gpio_request(GPIO_TSC, "ts_irq");
	if (error < 0) {
		printk(KERN_ERR "%s: failed to request GPIO for TSC IRQ"
			": %d\n", __func__, error);
		return;
	}

	gpio_direction_input(GPIO_TSC);
	ti814x_i2c_boardinfo[6].irq = gpio_to_irq(GPIO_TSC);

	gpio_export(31, true);
}


static void __init ti814x_pcf8574_init(void)
{
	int error;

	omap_mux_init_signal("mcasp4_fsx.gpio0_22_mux1", TI814X_PULL_DIS | TI814X_INPUT_EN);

	error = gpio_request(22, "pcf8574_irq");
	if (error < 0) {
		printk(KERN_ERR "%s: failed to request GPIO for pcf8574 IRQ"
			": %d\n", __func__, error);
		return;
	}

	gpio_direction_input(22);
	ti814x_i2c_boardinfo[3].irq = gpio_to_irq(22);

	gpio_export(22, true);
}


#if defined(CONFIG_MACH_TI8148_BELLLITE)	
static void __init ti814x_spi_gpio_init(void)
{
	omap_mux_init_signal("spi1_cs0.gpio1_16_mux1", TI814X_PULL_UP | TI814X_INPUT_EN);
	omap_mux_init_signal("spi1_sclk.gpio1_17_mux1", TI814X_PULL_UP | TI814X_INPUT_EN);
	omap_mux_init_signal("spi1_d1.gpio1_18_mux1", TI814X_PULL_UP | TI814X_INPUT_EN);
	omap_mux_init_signal("spi1_d0.gpio1_26_mux1", TI814X_PULL_UP | TI814X_INPUT_EN);

	omap_mux_init_signal("mlbp_sig_n.gpio1_08_mux1", TI814X_SLEW_SLOW);

	omap_mux_init_signal("mlb_clk.gpio0_31", TI814X_PULL_DIS |TI814X_INPUT_EN);	//VOICE_INT

	omap_mux_init_signal("mlb_sig.gpio0_29", TI814X_INPUT_EN);					//PHY_INT->RST_PERI

 // add by wangzhaofeng for led_mux
	omap_mux_init_signal("gpmc_a19.gpio1_14_mux0", TI814X_INPUT_EN | TI814X_PULL_UP);				
	omap_mux_init_signal("gpmc_a18.gpio1_13_mux0", TI814X_INPUT_EN |TI814X_PULL_UP );				
	omap_mux_init_signal("gpmc_a16.gpio2_05_mux0", TI814X_INPUT_EN | TI814X_PULL_UP);				
	omap_mux_init_signal("gpmc_a17.gpio2_06_mux0", TI814X_INPUT_EN | TI814X_PULL_UP );				


}
static void __init ti814x_mcasp2_pinmux_init(void)
{
	omap_mux_init_signal("xref_clk2", TI814X_SLEW_SLOW | TI814X_INPUT_EN);
	omap_mux_init_signal("mcasp2_aclkx", TI814X_PULL_UP | TI814X_INPUT_EN);
	omap_mux_init_signal("mcasp2_fsx",   TI814X_SLEW_SLOW | TI814X_PULL_UP | TI814X_INPUT_EN);
	omap_mux_init_signal("mcasp2_axr0", TI814X_SLEW_SLOW | TI814X_PULL_UP | TI814X_INPUT_EN);
	omap_mux_init_signal("mcasp2_axr1", TI814X_SLEW_SLOW | TI814X_PULL_UP | TI814X_INPUT_EN);	
}
static void __init ti814x_mcasp0_pinmux_init(void)
{	
	/* modify for gs2971a */
	/*SDIIN_ACLK  SDIIN_WCLK  SDIIN_AOUT0 - SDIIN_AOUT3*/
	
	//omap_mux_init_signal("mcasp0_aclkx", TI814X_PULL_UP | TI814X_INPUT_EN);not use
	omap_mux_init_signal("mcasp0_aclkr", TI814X_PULL_UP | TI814X_INPUT_EN);
	
	//omap_mux_init_signal("mcasp0_fsx",   TI814X_SLEW_SLOW | TI814X_PULL_UP | TI814X_INPUT_EN);not use
	omap_mux_init_signal("mcasp0_fsr",   TI814X_SLEW_SLOW | TI814X_PULL_UP | TI814X_INPUT_EN);

	
	omap_mux_init_signal("mcasp0_axr0", TI814X_SLEW_SLOW | TI814X_PULL_UP | TI814X_INPUT_EN);
	omap_mux_init_signal("mcasp0_axr1", TI814X_SLEW_SLOW | TI814X_PULL_UP | TI814X_INPUT_EN);
	omap_mux_init_signal("mcasp0_axr2", TI814X_SLEW_SLOW | TI814X_PULL_UP | TI814X_INPUT_EN);
	omap_mux_init_signal("mcasp0_axr3", TI814X_SLEW_SLOW | TI814X_PULL_UP | TI814X_INPUT_EN);


	
}
static void __init ti814x_mcasp1_pinmux_init(void)
{
	omap_mux_init_signal("xref_clk1", TI814X_SLEW_SLOW | TI814X_INPUT_EN);
	omap_mux_init_signal("mcasp1_aclkx", TI814X_PULL_UP | TI814X_INPUT_EN);
	omap_mux_init_signal("mcasp1_fsx",   TI814X_SLEW_SLOW | TI814X_PULL_UP | TI814X_INPUT_EN);
	omap_mux_init_signal("mcasp1_axr0", TI814X_SLEW_SLOW | TI814X_PULL_UP | TI814X_INPUT_EN);
	omap_mux_init_signal("mcasp1_axr1", TI814X_SLEW_SLOW | TI814X_PULL_UP | TI814X_INPUT_EN);	
}

//add by wangzhaofengZ
static void __init ti814x_mcasp4_pinmux_init(void)
{
	omap_mux_init_signal("mcasp4_aclkx", TI814X_PULL_UP | TI814X_INPUT_EN);
	omap_mux_init_signal("mcasp4_fsx",   TI814X_SLEW_SLOW | TI814X_PULL_UP | TI814X_INPUT_EN);
	omap_mux_init_signal("mcasp4_axr0", TI814X_SLEW_SLOW | TI814X_PULL_UP | TI814X_INPUT_EN);
	omap_mux_init_signal("mcasp4_axr1", TI814X_SLEW_SLOW | TI814X_PULL_UP | TI814X_INPUT_EN);	
}

#endif

static void __init ti814x_evm_i2c_init(void)
{
	/* There are 4 instances of I2C in TI814X but currently only one
	 * instance is being used on the TI8148 EVM
	 */
	omap_register_i2c_bus(1, 100, ti814x_i2c_boardinfo,
				ARRAY_SIZE(ti814x_i2c_boardinfo));
//	omap_register_i2c_bus(3, 100, ti814x_i2c_boardinfo1,
//				ARRAY_SIZE(ti814x_i2c_boardinfo1));
}

static u8 ti8148_iis_serializer_direction[] = {
	//add by wangzhaofeng
	TX_MODE,	RX_MODE,	INACTIVE_MODE,	INACTIVE_MODE,
	INACTIVE_MODE,	INACTIVE_MODE,	INACTIVE_MODE,	INACTIVE_MODE,
	INACTIVE_MODE,	INACTIVE_MODE,	INACTIVE_MODE,	INACTIVE_MODE,
	INACTIVE_MODE,	INACTIVE_MODE,	INACTIVE_MODE,	INACTIVE_MODE,

	RX_MODE,	TX_MODE,	INACTIVE_MODE,	INACTIVE_MODE,
	INACTIVE_MODE,	INACTIVE_MODE,	INACTIVE_MODE,	INACTIVE_MODE,
	INACTIVE_MODE,	INACTIVE_MODE,	INACTIVE_MODE,	INACTIVE_MODE,
	INACTIVE_MODE,	INACTIVE_MODE,	INACTIVE_MODE,	INACTIVE_MODE,
};


static struct snd_platform_data ti8148_evm_snd_data[] = {
	{
	.tx_dma_offset	= 0x4A1AB000,
	.rx_dma_offset	= 0x4A1AB000,
	.op_mode	= DAVINCI_MCASP_IIS_MODE,
	.num_serializer = 16,
	.tdm_slots	= 2,
	.serial_dir	= &ti8148_iis_serializer_direction[0],
	.asp_chan_q	= EVENTQ_2,
	.version	= MCASP_VERSION_2,
	.txnumevt	= 1,
	.rxnumevt	= 1,
	},
	{
	.tx_dma_offset	= 0x46800000,
	.rx_dma_offset	= 0x46800000,
	.op_mode	= DAVINCI_MCASP_IIS_MODE,
	.num_serializer = 16,
	.tdm_slots	= 32,
	.serial_dir	= &ti8148_iis_serializer_direction[16],
	.asp_chan_q	= EVENTQ_2,
	.version	= MCASP_VERSION_2,
	.txnumevt	= 1,
	.rxnumevt	= 1,
	},

};

/* NOR Flash partitions */
static struct mtd_partition ti814x_evm_norflash_partitions[] = {
	/* bootloader (U-Boot, etc) in first 5 sectors */
	{
		.name		= "bootloader",
		.offset		= 0,
		.size		= 2 * SZ_128K,
		.mask_flags	= MTD_WRITEABLE, /* force read-only */
	},
	/* bootloader params in the next 1 sectors */
	{
		.name		= "env",
		.offset		= MTDPART_OFS_APPEND,
		.size		= SZ_128K,
		.mask_flags	= 0,
	},
	/* kernel */
	{
		.name		= "kernel",
		.offset		= MTDPART_OFS_APPEND,
		.size		= 2 * SZ_2M,
		.mask_flags	= 0
	},
	/* file system */
	{
		.name		= "filesystem",
		.offset		= MTDPART_OFS_APPEND,
		.size		= 25 * SZ_2M,
		.mask_flags	= 0
	},
	/* reserved */
	{
		.name		= "reserved",
		.offset		= MTDPART_OFS_APPEND,
		.size		= MTDPART_SIZ_FULL,
		.mask_flags	= 0
	}
};

#if defined(CONFIG_MACH_TI8148_BELL)
/* NAND flash information */
static struct mtd_partition ti814x_nand_partitions[] = {
/* All the partition sizes are listed in terms of NAND block size */
	{
		.name           = "U-Boot-min",
		.offset         = 0,    /* Offset = 0x0 */
		.size           = SZ_128K,
	},
	{
		.name           = "U-Boot",
		.offset         = MTDPART_OFS_APPEND,/* Offset = 0x0 + 128K */
		.size           = 18 * SZ_128K,
	},
	{
		.name           = "U-Boot Env",
		.offset         = MTDPART_OFS_APPEND,   /* Offset = 0x260000 */
		.size           = 1 * SZ_128K,
	},
	{
		.name           = "Kernel",
		.offset         = MTDPART_OFS_APPEND,   /* Offset = 0x280000 */
		.size           = 34 * SZ_128K,
	},
	{
		.name           = "File System",
		.offset         = MTDPART_OFS_APPEND,   /* Offset = 0x6C0000 */
		.size           = 500*SZ_1M,//300*SZ_1M,//1601 * SZ_128K,
	},
	{
		.name           = "Reserved",
		.offset         = MTDPART_OFS_APPEND,   /* Offset = 0xCEE0000 */
		.size           = MTDPART_SIZ_FULL,
	},
};
#else
/* NAND flash information */
static struct mtd_partition ti814x_nand_partitions[] = {
/* All the partition sizes are listed in terms of NAND block size */
	{
		.name           = "U-Boot-min",
		.offset         = 0,    /* Offset = 0x0 */
		.size           = SZ_128K,
	},
	{
		.name           = "U-Boot",
		.offset         = MTDPART_OFS_APPEND,/* Offset = 0x0 + 128K */
		.size           = 18 * SZ_128K,
	},
	{
		.name           = "U-Boot Env",
		.offset         = MTDPART_OFS_APPEND,   /* Offset = 0x260000 */
		.size           = 1 * SZ_128K,
	},
	{
		.name           = "Kernel",
		.offset         = MTDPART_OFS_APPEND,   /* Offset = 0x280000 */
		.size           = 34 * SZ_128K,
	},
	{
		.name           = "File System",
		.offset         = MTDPART_OFS_APPEND,   /* Offset = 0x6C0000 */
		.size           = 1601 * SZ_128K,
	},
	{
		.name           = "Reserved",
		.offset         = MTDPART_OFS_APPEND,   /* Offset = 0xCEE0000 */
		.size           = MTDPART_SIZ_FULL,
	},
};
#endif

#if !defined(CONFIG_MACH_TI8148_BELLLITE)
/* SPI fLash information */
struct mtd_partition ti8148_spi_partitions[] = {
	/* All the partition sizes are listed in terms of erase size */
	{
		.name		= "U-Boot-min",
		.offset		= 0,	/* Offset = 0x0 */
		.size		= 32 * SZ_4K,
		.mask_flags	= MTD_WRITEABLE, /* force read-only */
	},
	{
		.name		= "U-Boot",
		.offset		= MTDPART_OFS_APPEND, /* 0x0 + (32*4K) */
		.size		= 64 * SZ_4K,
		.mask_flags	= MTD_WRITEABLE, /* force read-only */
	},
	{
		.name		= "U-Boot Env",
		.offset		= MTDPART_OFS_APPEND, /* 0x40000 + (32*4K) */
		.size		= 2 * SZ_4K,
	},
	{
		.name		= "Kernel",
		.offset		= MTDPART_OFS_APPEND, /* 0x42000 + (32*4K) */
		.size		= 640 * SZ_4K,
	},
	{
		.name		= "File System",
		.offset		= MTDPART_OFS_APPEND, /* 0x2C2000 + (32*4K) */
		.size		= MTDPART_SIZ_FULL, /* size ~= 1.1 MiB */
	}
};

const struct flash_platform_data ti8148_spi_flash = {
	.type		= "w25x32",
	.name		= "spi_flash",
	.parts		= ti8148_spi_partitions,
	.nr_parts	= ARRAY_SIZE(ti8148_spi_partitions),
};

struct spi_board_info __initdata ti8148_spi_slave_info[] = {
	{
		.modalias	= "m25p80",
		.platform_data	= &ti8148_spi_flash,
		.irq		= -1,
		.max_speed_hz	= 75000000,
		.bus_num	= 1,
		.chip_select	= 0,
	},
};
#else

struct spi_board_info __initdata ti8148_spi_slave_info[] = {
	{
		.modalias	= "gs2971",
		//.platform_data	= &ti8148_spi_gs2971,
		.irq		= -1,
		//Min: 16.67ns; 16.67ns < 1/speed, speed < 59988002;spi controller:48000000
		.max_speed_hz	= 3000000, //I select the fast
		.bus_num	= 1,
		.chip_select	= 0,
	},
};
#endif

void __init ti8148_spi_init(void)
{
	spi_register_board_info(ti8148_spi_slave_info,
				ARRAY_SIZE(ti8148_spi_slave_info));
}

static struct omap_musb_board_data musb_board_data = {
	.interface_type		= MUSB_INTERFACE_ULPI,
#ifdef CONFIG_USB_MUSB_OTG
	.mode           = MUSB_OTG,
#elif defined(CONFIG_USB_MUSB_HDRC_HCD)
	.mode           = MUSB_HOST,
#elif defined(CONFIG_USB_GADGET_MUSB_HDRC)
	.mode           = MUSB_PERIPHERAL,
#endif
	.power		= 500,
	.instances	= 1,
};

static void __init ti8148_evm_init_irq(void)
{
	omap2_init_common_infrastructure();
	omap2_init_common_devices(NULL, NULL);
	omap_init_irq();
	gpmc_init();
}

#if defined(CONFIG_SND_SOC_WL1271BT)

static struct platform_device ti8148_wl1271bt_codec_device = {
   .name   = "wl1271bt-dummy-codec",
   .id     = -1,
};


#endif



#ifdef CONFIG_SND_SOC_TI81XX_HDMI
static struct snd_hdmi_platform_data ti8148_snd_hdmi_pdata = {
	.dma_addr = TI81xx_HDMI_WP + HDMI_WP_AUDIO_DATA,
	.channel = 53,
	.data_type = 4,
	.acnt = 4,
	.fifo_level = 0x20,
};

static struct platform_device ti8148_hdmi_audio_device = {
	.name   = "hdmi-dai",
	.id     = -1,
	.dev = {
		.platform_data = &ti8148_snd_hdmi_pdata,
	}
};

static struct platform_device ti8148_hdmi_codec_device = {
	.name   = "hdmi-dummy-codec",
	.id     = -1,
};

static struct platform_device *ti8148_devices[] __initdata = {
	&ti8148_hdmi_audio_device,
	&ti8148_hdmi_codec_device,
	&ti8148_wl1271bt_codec_device,
};

/*
 * HDMI Audio Auto CTS MCLK configuration.
 * sysclk20, sysclk21, sysclk21 and CLKS(external)
 * setting sysclk20 as the parent of hdmi_i2s_ck
 * ToDo:
 */
void __init ti8148_hdmi_clk_init(void)
{
	int ret = 0;
	struct clk *parent, *child;

	/* modify the clk name to choose diff clk*/
	parent = clk_get(NULL, "sysclk20_ck");
	if (IS_ERR(parent))
		pr_err("Unable to get [sysclk20_ck] clk\n");

	child = clk_get(NULL, "hdmi_i2s_ck");
	if (IS_ERR(child))
		pr_err("Unable to get [hdmi_i2s_ck] clk\n");

	ret = clk_set_parent(child, parent);
	if (ret < 0)
		pr_err("Unable to set parent clk [hdmi_i2s_ck]\n");

	clk_put(child);
	clk_put(parent);
	pr_debug("{{HDMI Audio MCLK setup completed}}\n");
}

#endif




#define LSI_PHY_ID		0x0282F014
#define LSI_PHY_MASK		0xffffffff
#define PHY_CONFIG_REG		22

static int ti8148_evm_lsi_phy_fixup(struct phy_device *phydev)
{
	unsigned int val;

	/* This enables TX_CLK-ing in case of 10/100MBps operation */
	val = phy_read(phydev, PHY_CONFIG_REG);
	phy_write(phydev, PHY_CONFIG_REG, (val | BIT(5)));

	return 0;
}

#ifdef CONFIG_WL12XX_PLATFORM_DATA

static struct wl12xx_platform_data wlan_data __initdata = {
	.irq = OMAP_GPIO_IRQ(GPIO_WLAN_IRQ),
	/* COM6 (127x) uses FREF */
	.board_ref_clock = WL12XX_REFCLOCK_38_XTAL,
	/* COM7 (128x) uses TCXO */
	.board_tcxo_clock = WL12XX_TCXOCLOCK_26,
};

static int wl12xx_set_power(struct device *dev, int slot, int power_on,
			    int vdd)
{
	static bool power_state;

	//pr_debug("Powering %s wl12xx", power_on ? "on" : "off");
	printk(KERN_ERR"Powering %s wl12xx", power_on ? "on" : "off");

	if (power_on == power_state)
		return 0;
	power_state = power_on;

	if (power_on) {
		/* Power up sequence required for wl127x devices */
		gpio_set_value(GPIO_WLAN_EN, 1);
		usleep_range(15000, 15000);
		gpio_set_value(GPIO_WLAN_EN, 0);
		usleep_range(1000, 1000);
		gpio_set_value(GPIO_WLAN_EN, 1);
		msleep(70);
	} else {
		gpio_set_value(GPIO_WLAN_EN, 0);
	}

	return 0;
}
static void __init ti814x_wl12xx_wlan_init(void)
{
	struct device *dev;
	struct omap_mmc_platform_data *pdata;
	int ret;

	/* Set up mmc0 muxes */
	omap_mux_init_signal("mmc0_clk", TI814X_INPUT_EN | TI814X_PULL_UP);
	omap_mux_init_signal("mmc0_cmd", TI814X_INPUT_EN | TI814X_PULL_UP);
	omap_mux_init_signal("mmc0_dat0", TI814X_INPUT_EN | TI814X_PULL_UP);
	omap_mux_init_signal("mmc0_dat1", TI814X_INPUT_EN | TI814X_PULL_UP);
	omap_mux_init_signal("mmc0_dat2", TI814X_INPUT_EN | TI814X_PULL_UP);
	omap_mux_init_signal("mmc0_dat3", TI814X_INPUT_EN | TI814X_PULL_UP);

    /* Set up the WLAN_EN and WLAN_IRQ muxes */
    //gpio1_15_mux1 is good for application daughter board
#if 0
    omap_mux_init_signal("gpio2_22", TI814X_PULL_DIS);
    omap_mux_init_signal("gpio2_24", TI814X_INPUT_EN | TI814X_PULL_DIS);
#else	//bell
		omap_mux_init_signal("mlbp_dat_p.gpio1_09_mux1", TI814X_PULL_DIS);
    omap_mux_init_signal("mcasp3_aclkx.gpio0_16_mux1", TI814X_INPUT_EN | TI814X_PULL_DIS);
    
    //bt-uart
    omap_mux_init_signal("uart0_dcdn.uart3_rxd_mux0", TI814X_PULL_UP | TI814X_INPUT_EN | TI814X_SLEW_SLOW);
    omap_mux_init_signal("uart0_dsrn.uart3_txd_mux0", TI814X_PULL_UP | TI814X_INPUT_EN | TI814X_SLEW_SLOW);
    omap_mux_init_signal("uart0_dtrn.uart3_ctsn_mux0", TI814X_PULL_UP | TI814X_INPUT_EN | TI814X_SLEW_SLOW);
    omap_mux_init_signal("uart0_rin.uart3_rtsn_mux0", TI814X_PULL_UP | TI814X_INPUT_EN | TI814X_SLEW_SLOW);
     omap_mux_init_signal("mcasp3_fsx.gpio0_17_mux1", TI814X_PULL_DIS);
    
    //omap_mux_init_signal("mlbp_dat_n.gpio1_10_mux1",TI814X_PULL_DIS );
    omap_mux_init_signal("mlbp_sig_n.gpio1_08_mux1", TI814X_PULL_DIS);
#endif

	/* Pass the wl12xx platform data information to the wl12xx driver */
	if (wl12xx_set_platform_data(&wlan_data)) {
		pr_err("Error setting wl12xx data\n");
		return;
	}

	/*
	 * The WLAN_EN gpio has to be toggled without using a fixed regulator,
	 * as the omap_hsmmc does not enable/disable regulators on the TI814X.
	 */
	ret = gpio_request_one(GPIO_WLAN_EN, GPIOF_OUT_INIT_LOW, "wlan_en");
	if (ret) {
		pr_err("Error requesting wlan enable gpio: %d\n", ret);
		return;
	}

#if 1
	ret = gpio_request_one(GPIO_BT_EN, GPIOF_OUT_INIT_LOW, "bt_en");
	if (ret) {
		pr_err("Error requesting wlan enable gpio: %d\n", ret);
		return;
	}

	ret = gpio_request_one(GPIO_BT_WAKEUP, GPIOF_OUT_INIT_LOW, "bt_wakup");
	if (ret) {
		pr_err("Error requesting wlan enable gpio: %d\n", ret);
		return;
	}

	#if 1
        printk(KERN_ERR "WL1271: BT Enable\n");
        gpio_set_value(GPIO_BT_EN, 1);
        msleep(10);
        gpio_set_value(GPIO_BT_EN, 0);
        msleep(1);
        gpio_set_value(GPIO_BT_EN, 1);
	#endif
#endif
	/*
	 * Set our set_power callback function which will be called from
	 * set_ios. This is requireq since, unlike other omap2+ platforms, a
	 * no-op set_power function is registered. Thus, we cannot use a fixed
	 * regulator, as it will never be toggled.
	 * Moreover, even if this was not the case, we're on mmc0, for which
	 * omap_hsmmc' set_power functions do not toggle any regulators.
	 * TODO: Consider modifying omap_hsmmc so it would enable/disable a
	 * regulator for ti814x/mmc0.
	 */
	dev = mmc[0].dev;
	if (!dev) {
		pr_err("wl12xx mmc device initialization failed\n");
		return;
	}

	pdata = dev->platform_data;
	if (!pdata) {
		pr_err("Platform data of wl12xx device not set\n");
		return;
	}

	pdata->slots[0].set_power = wl12xx_set_power;
}

static void __init ti814x_wl12xx_init(void)
{
	ti814x_wl12xx_wlan_init();
	pr_debug("add by wangzhaofeng\n");
	
}

#else /* CONFIG_WL12XX_PLATFORM_DATA */

static void __init ti814x_wl12xx_init(void) { }

#endif
static void __init ti8148_evm_init(void)
{
	int bw; /* bus-width */
    int ret;
	ti814x_mux_init(board_mux);
	omap_serial_init();

#if defined(CONFIG_MACH_TI8148_BELLLITE)	
	ti814x_spi_gpio_init();
//	ti814x_mcasp2_pinmux_init();
	ti814x_mcasp0_pinmux_init();
	ti814x_mcasp1_pinmux_init();
//	ti814x_mcasp4_pinmux_init();
	printk(KERN_ERR"pinmux\n");
#else
	ti814x_tsc_init();
#endif

	ti814x_pcf8574_init();//before i2c_init

	ti814x_evm_i2c_init();
//changed  by wangzhaofeng 
//	ti81xx_register_mcasp(4, &ti8148_evm_snd_data[0]);
  //  ti81xx_register_mcasp(2, &ti8148_evm_snd_data[1]);

	omap2_hsmmc_init(mmc);
	
	omap_mux_init_signal("mmc0_clk.gpio0_1", TI814X_PULL_UP );
	omap_mux_init_signal("mmc0_dat2.gpio0_5", TI814X_PULL_UP );
	omap_mux_init_signal("mmc0_dat3.gpio0_6", TI814X_PULL_UP );
	
	ret = gpio_request_one(5, GPIOF_OUT_INIT_LOW, "led_0");
	if (ret) {
		pr_err("Error requesting pcie reset gpio: %d\n", ret);
		return;
	}		
    gpio_set_value(5, 0);		
	
	ret = gpio_request_one(6, GPIOF_OUT_INIT_LOW, "led_1");
	if (ret) {
		pr_err("Error requesting pcie reset gpio: %d\n", ret);
		return;
	}		
    gpio_set_value(6, 0);		
		
	ret = gpio_request_one(GPIO_PCIE_RESET, GPIOF_OUT_INIT_LOW, "pcie_reset");
	if (ret) {
		pr_err("Error requesting pcie reset gpio: %d\n", ret);
		return;
	}	
	gpio_set_value(GPIO_PCIE_RESET, 1);	
	msleep(4000);
	
	/* nand initialisation */
	if (cpu_is_ti814x()) {
		u32 *control_status = TI81XX_CTRL_REGADDR(0x40);
		if (*control_status & (1<<16))
			bw = 2; /*16-bit nand if BTMODE BW pin on board is ON*/
		else
			bw = 0; /*8-bit nand if BTMODE BW pin on board is OFF*/

		board_nand_init(ti814x_nand_partitions,
			ARRAY_SIZE(ti814x_nand_partitions), 0, bw);
	} else
		board_nand_init(ti814x_nand_partitions,
		ARRAY_SIZE(ti814x_nand_partitions), 0, NAND_BUSWIDTH_16);

	/* initialize usb */
	usb_musb_init(&musb_board_data);

	ti8148_spi_init();
#ifdef CONFIG_SND_SOC_TI81XX_HDMI
	/*setup the clokc for HDMI MCLK*/
	ti8148_hdmi_clk_init();
	__raw_writel(0x0, DSS_HDMI_RESET);
	platform_add_devices(ti8148_devices, ARRAY_SIZE(ti8148_devices));
#endif
//add wangzhaofeng
//	ti814x_wl12xx_init();

	regulator_use_dummy_regulator();
	board_nor_init(ti814x_evm_norflash_partitions,
		ARRAY_SIZE(ti814x_evm_norflash_partitions), 0);

#if !defined(CONFIG_MACH_TI8148_BELLLITE)
	/* LSI Gigabit Phy fixup */
	phy_register_fixup_for_uid(LSI_PHY_ID, LSI_PHY_MASK,
				   ti8148_evm_lsi_phy_fixup);
#endif
}

static void __init ti8148_evm_map_io(void)
{
	omap2_set_globals_ti814x();
	ti81xx_map_common_io();
}

MACHINE_START(TI8148EVM, "ti8148evm")
	/* Maintainer: Texas Instruments */
	.boot_params	= 0x80000100,
	.map_io		= ti8148_evm_map_io,
	.reserve         = ti81xx_reserve,
	.init_irq	= ti8148_evm_init_irq,
	.init_machine	= ti8148_evm_init,
	.timer		= &omap_timer,
MACHINE_END
