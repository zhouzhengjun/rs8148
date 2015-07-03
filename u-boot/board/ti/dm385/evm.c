/*
 * Copyright (C) 2009, Texas Instruments, Incorporated
 *
 * See file CREDITS for list of people who contributed to this
 * project.
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

#include <common.h>
#include <asm/cache.h>
#include <asm/arch/cpu.h>
#include <asm/arch/ddr_defs.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/mmc_host_def.h>
#include <asm/arch/clock.h>
#include <asm/arch/mem.h>
#include <asm/arch/nand.h>
#include <linux/mtd/nand.h>
#include <nand.h>
#include <net.h>
#include <miiphy.h>
#include <netdev.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_DM385_CONFIG_DDR
static void cmd_macro_config(u32 ddr_phy, u32 inv_clk_out,
			 u32 ctrl_slave_ratio_cs0, u32 cmd_dll_lock_diff)
{
	u32 ddr_phy_base = DDR0_PHY_BASE_ADDR;

	__raw_writel(inv_clk_out,
		 ddr_phy_base + CMD1_REG_PHY_INVERT_CLKOUT_0);
	__raw_writel(inv_clk_out,
		 ddr_phy_base + CMD0_REG_PHY_INVERT_CLKOUT_0);
	__raw_writel(inv_clk_out,
		 ddr_phy_base + CMD2_REG_PHY_INVERT_CLKOUT_0);

	__raw_writel(((ctrl_slave_ratio_cs0 << 10) | ctrl_slave_ratio_cs0),
		ddr_phy_base + CMD0_REG_PHY_CTRL_SLAVE_RATIO_0);
	__raw_writel(((ctrl_slave_ratio_cs0 << 10) | ctrl_slave_ratio_cs0),
		ddr_phy_base + CMD1_REG_PHY_CTRL_SLAVE_RATIO_0);
	__raw_writel(((ctrl_slave_ratio_cs0 << 10) | ctrl_slave_ratio_cs0),
		 ddr_phy_base + CMD2_REG_PHY_CTRL_SLAVE_RATIO_0);

	__raw_writel(cmd_dll_lock_diff,
		 ddr_phy_base + CMD0_REG_PHY_DLL_LOCK_DIFF_0);
	__raw_writel(cmd_dll_lock_diff,
		 ddr_phy_base + CMD1_REG_PHY_DLL_LOCK_DIFF_0);
	__raw_writel(cmd_dll_lock_diff,
		 ddr_phy_base + CMD2_REG_PHY_DLL_LOCK_DIFF_0);
}

static void data_macro_config(u32 macro_num, u32 emif, u32 rd_dqs_cs0,
		u32 wr_dqs_cs0, u32 fifo_we_cs0, u32 wr_data_cs0)
{
	/* 0xA4 is size of each data macro mmr region.
	 * phy1 is at offset 0x400 from phy0
	 */
	u32 base = (macro_num * 0xA4) + (emif * 0x400);

	__raw_writel(((rd_dqs_cs0 << 10) | rd_dqs_cs0),
		(DATA0_REG_PHY0_RD_DQS_SLAVE_RATIO_0 + base));
	__raw_writel(((wr_dqs_cs0 << 10) | wr_dqs_cs0),
		(DATA0_REG_PHY0_WR_DQS_SLAVE_RATIO_0 + base));
	__raw_writel(((PHY_WRLVL_INIT_CS1_DEFINE << 10) |
		PHY_WRLVL_INIT_CS0_DEFINE),
		(DATA0_REG_PHY0_WRLVL_INIT_RATIO_0 + base));
	__raw_writel(((PHY_GATELVL_INIT_CS1_DEFINE << 10) |
		PHY_GATELVL_INIT_CS0_DEFINE),
		(DATA0_REG_PHY0_GATELVL_INIT_RATIO_0 + base));
	__raw_writel(((fifo_we_cs0 << 10) | fifo_we_cs0),
		(DATA0_REG_PHY0_FIFO_WE_SLAVE_RATIO_0 + base));
	__raw_writel(((wr_data_cs0 << 10) | wr_data_cs0),
		(DATA0_REG_PHY0_WR_DATA_SLAVE_RATIO_0 + base));
	__raw_writel(PHY_DLL_LOCK_DIFF_DEFINE,
		(DATA0_REG_PHY0_DLL_LOCK_DIFF_0 + base));
}
#endif

static void pll_config(u32, u32, u32, u32, u32);
#if 0
static void pcie_pll_config(void);
#endif
static void audio_pll_config(void);
static void sata_pll_config(void);
static void modena_pll_config(void);
static void l3_pll_config(void);
static void ddr_pll_config(void);
static void iss_pll_config(void);
static void iva_pll_config(void);
static void usb_pll_config(void);

static void unlock_pll_control_mmr(void);
#ifdef CONFIG_DRIVER_TI_CPSW
static void cpsw_pad_config(void);
#endif
static void nor_pad_config_mux(void);
/*
 * spinning delay to use before udelay works
 */
static inline void delay(unsigned long loops)
{
	__asm__ volatile ("1:\n" "subs %0, %1, #1\n"
		"bne 1b" : "=r" (loops) : "0"(loops));
}

/*
 * Basic board specific setup
 */
int board_init(void)
{
	u32 regVal;

	/* Do the required pin-muxing before modules are setup */
	set_muxconf_regs();

	nor_pad_config_mux();

	/* setup RMII_REFCLK to be sourced from audio_pll */
	__raw_writel(0x4, RMII_REFCLK_SRC);

	/*program GMII_SEL register for RGMII mode */
	__raw_writel(0x30a, GMII_SEL);

	/* Get Timer and UART out of reset */
	/* UART softreset */
	regVal = __raw_readl(UART_SYSCFG);
	regVal |= 0x2;
	__raw_writel(regVal, UART_SYSCFG);
	while ((__raw_readl(UART_SYSSTS) & 0x1) != 0x1)
		;

	/* Disable smart idle */
	regVal = __raw_readl(UART_SYSCFG);
	regVal |= (1<<3);
	__raw_writel(regVal, UART_SYSCFG);

	/* mach type passed to kernel */
	gd->bd->bi_arch_number = MACH_TYPE_DM385EVM;

	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_DRAM_1 + 0x100;
	gpmc_init();

#ifndef CONFIG_NOR
	/* GPMC will come up with default buswidth configuration,
	 * we will override it based on BW pin CONFIG_STATUS register.
	 * This is currently required only for NAND/NOR to
	 * support 8/16 bit NAND/NOR part. Also we always use chipselect 0
	 * for NAND/NOR boot.
	 *
	 * NOTE: This code is DM8168 EVM specific, hence we are using CS 0.
	 * Also, even for other boot modes user is expected to
	 * on/off the BW pin on the EVM.
	 */
	gpmc_set_cs_buswidth(0, get_sysboot_bw());
#endif
	return 0;
}

/*
 * sets uboots idea of sdram size
 */
int dram_init(void)
{
	/* Fill up board info */
	gd->bd->bi_dram[0].start = PHYS_DRAM_1;
	gd->bd->bi_dram[0].size = PHYS_DRAM_1_SIZE;

	return 0;
}


int misc_init_r(void)
{
	#ifdef CONFIG_DM385_MIN_CONFIG
	printf("The 2nd stage U-Boot will now be auto-loaded\n");
	printf("Please do not interrupt the countdown till "
		"DM385_EVM prompt if 2nd stage is already flashed\n");
	#endif

#ifdef CONFIG_DM385_ASCIIART
	int i = 0, j = 0;

	char dm385[22][67] = {
"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@",
"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@",
"@@                                                               @@",
"@@                                                               @@",
"@@                                                               @@",
"@@                                                               @@",
"@@      @@@        @.   @.         @@@@    @@@@  G@@@@@@         @@",
"@@      @@@@@     L@@  @@@        @@@@@@  @@@@@i @@@@@@          @@",
"@@      @@ @@@.   @@@  @@@            @@  @,  @@ :@@             @@",
"@@      @@   @@   @@@  @@@          @@@@  @@;@@: C@@@@@.         @@",
"@@      @@    @@  @@@.l@L@         G@@@   ,@@@@  @@@  @@         @@",
"@@      @@    @@ l@ @@@@ @.          l@@  @@ L@@  @   G@         @@",
"@@      @@    @@ @@ @@@@ @@           @@ C@   @@      @@         @@",
"@@      @@   ,@C @@ @@@  @@       @i  @@ C@   @@ @@   @@         @@",
"@@      @@@@@@@  @@  @@  @@       @@@@@l  @@@@@@ L@@@@@          @@",
"@@       @@@@@   @   @    @        L@@     @@@,   ,@@G           @@",
"@@                                                               @@",
"@@                                                               @@",
"@@                                                               @@",
"@@                                                               @@",
"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@",
"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@",};

	for (i = 0; i < 22; i++) {
		for (j = 0; j < 67; j++)
			printf("%c", dm385[i][j]);
			printf("\n");
	}
	printf("\n");
#endif
	return 0;
}
#ifdef CONFIG_DM385_CONFIG_DDR
static void config_dm385_ddr(void)
{
	int macro, emif = 0;

	/* Enable the Power Domain Transition of L3 Fast Domain Peripheral */
	__raw_writel(0x2, CM_DEFAULT_FW_CLKCTRL);
	/* Enable the Power Domain Transition of L3 Fast Domain Peripheral */
	__raw_writel(0x2, CM_DEFAULT_L3_FAST_CLKSTCTRL);
	__raw_writel(0x2, CM_DEFAULT_EMIF_0_CLKCTRL); /* Enable EMIF0 Clock */
	__raw_writel(0x2, CM_DEFAULT_DMM_CLKCTRL);

	/* Poll for L3_FAST_GCLK  & DDR_GCLK  are active */
	while ((__raw_readl(CM_DEFAULT_L3_FAST_CLKSTCTRL) & 0x300) != 0x300)
		;
	/* Poll for Module is functional */
	while ((__raw_readl(CM_DEFAULT_EMIF_0_CLKCTRL)) != 0x2)
		;
	while ((__raw_readl(CM_DEFAULT_DMM_CLKCTRL)) != 0x2)
		;

	cmd_macro_config(DDR_PHY0, DDR3_PHY_INVERT_CLKOUT_OFF,
			DDR3_PHY_CTRL_SLAVE_RATIO_CS0_DEFINE,
			PHY_CMD0_DLL_LOCK_DIFF_DEFINE);

	for (macro = 0; macro <= DATA_MACRO_3; macro++) {
		data_macro_config(macro, DDR_PHY0,
			DDR3_PHY_RD_DQS_CS0_DEFINE,
			DDR3_PHY_WR_DQS_CS0_DEFINE,
			DDR3_PHY_RD_DQS_GATE_CS0_DEFINE,
			DDR3_PHY_WR_DATA_CS0_DEFINE);
	}

	/* DDR IO CTRL config */
	__raw_writel(DDR0_IO_CTRL_DEFINE, DDR0_IO_CTRL);

	__raw_writel(__raw_readl(VTP0_CTRL_REG) | 0x00000040 , VTP0_CTRL_REG);

	/* Write 0 to CLRZ bit */
	__raw_writel(__raw_readl(VTP0_CTRL_REG) & 0xfffffffe , VTP0_CTRL_REG);

	/* Write 1 to CLRZ bit */
	__raw_writel(__raw_readl(VTP0_CTRL_REG) | 0x00000001 , VTP0_CTRL_REG);

	/* Read VTP control registers & check READY bits */
	while ((__raw_readl(VTP0_CTRL_REG) & 0x00000020) != 0x20)
		;

	/*
	 * Program the DDR3 DMM to Access EMIF0
	 * 1G contiguous section with no interleaving
	 */
	__raw_writel(DDR3_DMM_LISA_MAP__0, DMM_LISA_MAP__0);
	__raw_writel(DDR3_DMM_LISA_MAP__1, DMM_LISA_MAP__1);
	__raw_writel(DDR3_DMM_LISA_MAP__2, DMM_LISA_MAP__2);
	__raw_writel(DDR3_DMM_LISA_MAP__3, DMM_LISA_MAP__3);

	while (__raw_readl(DMM_LISA_MAP__0) != DDR3_DMM_LISA_MAP__0)
		;
	while (__raw_readl(DMM_LISA_MAP__1) != DDR3_DMM_LISA_MAP__1)
		;
	while (__raw_readl(DMM_LISA_MAP__2) != DDR3_DMM_LISA_MAP__2)
		;
	while (__raw_readl(DMM_LISA_MAP__3) != DDR3_DMM_LISA_MAP__3)
		;

	__raw_writel(0x80000000, DMM_PAT_BASE_ADDR);

	/* Program EMIF0 CFG Registers */
	__raw_writel(DDR3_EMIF_READ_LATENCY, EMIF4_0_DDR_PHY_CTRL_1);
	__raw_writel(DDR3_EMIF_READ_LATENCY, EMIF4_0_DDR_PHY_CTRL_1_SHADOW);
	__raw_writel(DDR3_EMIF_TIM1, EMIF4_0_SDRAM_TIM_1);
	__raw_writel(DDR3_EMIF_TIM1, EMIF4_0_SDRAM_TIM_1_SHADOW);
	__raw_writel(DDR3_EMIF_TIM2, EMIF4_0_SDRAM_TIM_2);
	__raw_writel(DDR3_EMIF_TIM2, EMIF4_0_SDRAM_TIM_2_SHADOW);
	__raw_writel(DDR3_EMIF_TIM3, EMIF4_0_SDRAM_TIM_3);
	__raw_writel(DDR3_EMIF_TIM3, EMIF4_0_SDRAM_TIM_3_SHADOW);
	__raw_writel(DDR3_EMIF_SDRAM_CONFIG, EMIF4_0_SDRAM_CONFIG);

	__raw_writel(DDR_EMIF_REF_CTRL | DDR_EMIF_REF_TRIGGER,
					 EMIF4_0_SDRAM_REF_CTRL);
	__raw_writel(DDR_EMIF_REF_CTRL, EMIF4_0_SDRAM_REF_CTRL_SHADOW);
	__raw_writel(DDR3_EMIF_SDRAM_ZQCR, EMIF4_0_SDRAM_ZQCR);
	__raw_writel(DDR_EMIF_REF_CTRL, EMIF4_0_SDRAM_REF_CTRL);
	__raw_writel(DDR_EMIF_REF_CTRL, EMIF4_0_SDRAM_REF_CTRL_SHADOW);

	__raw_writel(DDR3_EMIF_REF_CTRL, EMIF4_0_SDRAM_REF_CTRL);
	__raw_writel(DDR3_EMIF_REF_CTRL, EMIF4_0_SDRAM_REF_CTRL_SHADOW);
}
#endif
static void audio_pll_config()
{
	pll_config(AUDIO_PLL_BASE,
			AUDIO_N, AUDIO_M,
			AUDIO_M2, AUDIO_CLKCTRL);
}

#if 0
static void pcie_pll_config()
{
	/* Powerdown both reclkp/n single ended receiver */
	__raw_writel(0x00000002, SERDES_REFCLK_CTRL);

	__raw_writel(0x00000000, PCIE_PLLCFG0);

	/* PCIe(2.5GHz) mode, 100MHz refclk, MDIVINT = 25,
	 * disable (50,100,125M) clks
	 */
	__raw_writel(0x00640000, PCIE_PLLCFG1);

	/* SSC Mantissa and exponent = 0 */
	__raw_writel(0x00000000, PCIE_PLLCFG2);

	/* TBD */
	__raw_writel(0x004008E0, PCIE_PLLCFG3);

	/* TBD */
	__raw_writel(0x0000609C, PCIE_PLLCFG4);

	/* pcie_serdes_cfg_misc */
	/* TODO: verify the address over here
	 * (CTRL_BASE + 0x6FC = 0x481406FC ???)
	 */
	/* __raw_writel(0x00000E7B, 0x48141318); */
	delay(3);

	/* Enable PLL LDO */
	__raw_writel(0x00000004, PCIE_PLLCFG0);
	delay(3);

	/* Enable DIG LDO, PLL LD0 */
	__raw_writel(0x00000014, PCIE_PLLCFG0);
	delay(3);

	/* Enable DIG LDO, ENBGSC_REF, PLL LDO */
	__raw_writel(0x00000016, PCIE_PLLCFG0);
	delay(3);
	__raw_writel(0x30000016, PCIE_PLLCFG0);
	delay(3);
	__raw_writel(0x70000016, PCIE_PLLCFG0);
	delay(3);

	/* Enable DIG LDO, SELSC, ENBGSC_REF, PLL LDO */
	__raw_writel(0x70000017, PCIE_PLLCFG0);
	delay(3);

	/* wait for ADPLL lock */
	while (__raw_readl(PCIE_PLLSTATUS) != 0x1)
		;

}
#endif

static void sata_pll_config()
{
	__raw_writel(0xC12C003C, SATA_PLLCFG1);
	__raw_writel(0x004008E0, SATA_PLLCFG3);
	delay(0xFFFF);

	__raw_writel(0x80000004, SATA_PLLCFG0);
	delay(0xFFFF);

	/* Enable PLL LDO */
	__raw_writel(0x80000014, SATA_PLLCFG0);
	delay(0xFFFF);

	/* Enable DIG LDO, ENBGSC_REF, PLL LDO */
	__raw_writel(0x80000016, SATA_PLLCFG0);
	delay(0xFFFF);

	__raw_writel(0xC0000017, SATA_PLLCFG0);
	delay(0xFFFF);

	/* wait for ADPLL lock */
	while (((__raw_readl(SATA_PLLSTATUS) & 0x01) == 0x0))
		;

}

static void usb_pll_config()
{
	pll_config(USB_PLL_BASE,
			USB_N, USB_M,
			USB_M2, USB_CLKCTRL);
}

static void modena_pll_config()
{
	pll_config(MODENA_PLL_BASE,
			MODENA_N, MODENA_M,
			MODENA_M2, MODENA_CLKCTRL);
}

static void l3_pll_config()
{
	pll_config(L3_PLL_BASE,
			L3_N, L3_M,
			L3_M2, L3_CLKCTRL);
}

static void ddr_pll_config()
{
	pll_config(DDR_PLL_BASE,
			DDR_N, DDR_M,
			DDR_M2, DDR_CLKCTRL);
}

static void iss_pll_config()
{
	pll_config(ISS_PLL_BASE,
			ISS_N, ISS_M,
			ISS_M2, ISS_CLKCTRL);
}

static void iva_pll_config()
{
	pll_config(IVA_PLL_BASE,
			IVA_N, IVA_M,
			IVA_M2, IVA_CLKCTRL);
}

/*
 * configure individual ADPLLJ
 */
static void pll_config(u32 base, u32 n, u32 m, u32 m2, u32 clkctrl_val)
{
	u32 m2nval, mn2val, read_clkctrl = 0;

	m2nval = (m2 << 16) | n;
	mn2val = m;

	/* by-pass pll */
	read_clkctrl = __raw_readl(base + ADPLLJ_CLKCTRL);
	__raw_writel((read_clkctrl | 0x00800000), (base + ADPLLJ_CLKCTRL));
	while ((__raw_readl(base + ADPLLJ_STATUS) & 0x101) != 0x101)
		;
	read_clkctrl = __raw_readl(base + ADPLLJ_CLKCTRL);
	__raw_writel((read_clkctrl & 0xfffffffe), (base + ADPLLJ_CLKCTRL));


	/*
	 * ref_clk = 20/(n + 1);
	 * clkout_dco = ref_clk * m;
	 * clk_out = clkout_dco/m2;
	*/

	__raw_writel(m2nval, (base + ADPLLJ_M2NDIV));
	__raw_writel(mn2val, (base + ADPLLJ_MN2DIV));

	/* Load M2, N2 dividers of ADPLL */
	__raw_writel(0x1, (base + ADPLLJ_TENABLEDIV));
	__raw_writel(0x0, (base + ADPLLJ_TENABLEDIV));

	/* Loda M, N dividers of ADPLL */
	__raw_writel(0x1, (base + ADPLLJ_TENABLE));
	__raw_writel(0x0, (base + ADPLLJ_TENABLE));

	read_clkctrl = __raw_readl(base + ADPLLJ_CLKCTRL);

	if (MODENA_PLL_BASE == base)
		__raw_writel((read_clkctrl & 0xff7fffff) | clkctrl_val,
			base + ADPLLJ_CLKCTRL);
	else
		__raw_writel((read_clkctrl & 0xff7fe3ff) | clkctrl_val,
			base + ADPLLJ_CLKCTRL);
	/* Wait for phase and freq lock */
	while ((__raw_readl(base + ADPLLJ_STATUS) & 0x600) != 0x600)
		;

}

/*
 * Enable the clks & power for perifs (TIMER1, UART0,...)
 */
void per_clocks_enable(void)
{
	u32 temp;

	__raw_writel(0x2, CM_ALWON_L3_SLOW_CLKSTCTRL);

#if 0
	/* TIMER 1 */
	__raw_writel(0x2, CM_ALWON_TIMER_1_CLKCTRL);
#endif
	/* Selects OSC0 (20MHz) for DMTIMER1 */
	temp = __raw_readl(DMTIMER_CLKSRC);
	temp &= ~(0x7 << 3);
	temp |= (0x4 << 3);
	__raw_writel(temp, DMTIMER_CLKSRC);

#if 0
	while (((__raw_readl(CM_ALWON_L3_SLOW_CLKSTCTRL) & (0x80000<<1)) >>
			(19+1)) != 1)
		;
	while (((__raw_readl(CM_ALWON_TIMER_1_CLKCTRL) & 0x30000)>>16) != 0)
		;
#endif
	__raw_writel(0x2, (DM_TIMER1_BASE + 0x54));
	while (__raw_readl(DM_TIMER1_BASE + 0x10) & 1)
		;

	__raw_writel(0x1, (DM_TIMER1_BASE + 0x38));

	/* UARTs */
	__raw_writel(0x2, CM_ALWON_UART_0_CLKCTRL);
	while (__raw_readl(CM_ALWON_UART_0_CLKCTRL) != 0x2)
		;

	__raw_writel(0x2, CM_ALWON_UART_1_CLKCTRL);
	while (__raw_readl(CM_ALWON_UART_1_CLKCTRL) != 0x2)
		;

	__raw_writel(0x2, CM_ALWON_UART_2_CLKCTRL);
	while (__raw_readl(CM_ALWON_UART_2_CLKCTRL) != 0x2)
		;

	while ((__raw_readl(CM_ALWON_L3_SLOW_CLKSTCTRL) & 0x2100) != 0x2100)
		;

	/* SPI */
	__raw_writel(0x2, CM_ALWON_SPI_CLKCTRL);
	while (__raw_readl(CM_ALWON_SPI_CLKCTRL) != 0x2)
		;

	/* I2C0 and I2C2 */
	__raw_writel(0x2, CM_ALWON_I2C_0_CLKCTRL);
	while (__raw_readl(CM_ALWON_I2C_0_CLKCTRL) != 0x2)
		;
	/* Ethernet */
	__raw_writel(0x2, CM_ETHERNET_CLKSTCTRL);
	__raw_writel(0x2, CM_ALWON_ETHERNET_0_CLKCTRL);
	while ((__raw_readl(CM_ALWON_ETHERNET_0_CLKCTRL) & 0x30000) != 0)
		;
	__raw_writel(0x2, CM_ALWON_ETHERNET_1_CLKCTRL);
	/* HSMMC */
	__raw_writel(0x2, CM_ALWON_HSMMC_CLKCTRL);
	while (__raw_readl(CM_ALWON_HSMMC_CLKCTRL) != 0x2)
		;

	/*
	 * McASP2
	 * select mcasp2 clk from sys_clk_22 (OSC 0)
	 * so that audio clk (sys_clk_20) can be used for RMII
	 * ToDo :
	 * This can be removed once kernel exports set_parent()
	 */
	__raw_writel(0x2, CM_AUDIOCLK_MCASP1_CLKSEL);
	while (__raw_readl(CM_AUDIOCLK_MCASP1_CLKSEL) != 0x2)
		;

	/* WDT */
	/* For WDT to be functional, it needs to be first stopped by writing
	 * the pattern 0xAAAA followed by 0x5555 in the WDT start/stop register.
	 * After that a write-once register in Control module needs to be
	 * configured to unfreeze the timer.
	 * Note: It is important to stop the watchdog before unfreezing it
	*/
	__raw_writel(0xAAAA, WDT_WSPR);
	while (__raw_readl(WDT_WWPS) != 0x0)
		;
	__raw_writel(0x5555, WDT_WSPR);
	while (__raw_readl(WDT_WWPS) != 0x0)
		;

	/* Unfreeze WDT */
	__raw_writel(0x13, WDT_UNFREEZE);
}

/*
 * inits clocks for PRCM as defined in clocks.h
 */
void prcm_init(u32 in_ddr)
{
	/* Enable the control module */
	__raw_writel(0x2, CM_ALWON_CONTROL_CLKCTRL);

	/* Setup the various plls */
	audio_pll_config();
	sata_pll_config();
#if 0
	pcie_pll_config();
#endif
	modena_pll_config();
	l3_pll_config();
	ddr_pll_config();
	iva_pll_config();
	iss_pll_config();

	usb_pll_config();

	/*  With clk freqs setup to desired values,
	 *  enable the required peripherals
	 */
	per_clocks_enable();
}

#ifdef CONFIG_DRIVER_TI_CPSW

#define PADCTRL_BASE 0x48140000

#define PAD204_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0B2c))
#define PAD205_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0B30))
#define PAD206_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0B34))
#define PAD207_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0B38))
#define PAD208_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0B3c))
#define PAD209_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0B40))
#define PAD210_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0B44))
#define PAD211_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0B48))
#define PAD212_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0B4c))
#define PAD213_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0B50))
#define PAD214_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0B54))
#define PAD215_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0B58))
#define PAD216_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0B5c))
#define PAD217_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0B60))
#define PAD218_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0B64))
#define PAD219_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0B68))
#define PAD220_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0B6c))
#define PAD221_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0B70))
#define PAD222_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0B74))
#define PAD223_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0B78))
#define PAD224_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0B7c))
#define PAD225_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0B80))
#define PAD226_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0B84))
#define PAD227_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0B88))

#define PAD232_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0B9C))
#define PAD233_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0BA0))
#define PAD234_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0BA4))
#define PAD235_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0BA8))
#define PAD236_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0BAC))
#define PAD237_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0BB0))
#define PAD238_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0BB4))
#define PAD239_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0BB8))
#define PAD240_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0BBC))
#define PAD241_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0BC0))
#define PAD242_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0BC4))
#define PAD243_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0BC8))
#define PAD244_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0BCC))
#define PAD245_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0BD0))
#define PAD246_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0BD4))
#define PAD247_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0BD8))
#define PAD248_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0BDC))
#define PAD249_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0BE0))
#define PAD250_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0BE4))
#define PAD251_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0BE8))
#define PAD252_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0BEC))
#define PAD253_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0BF0))
#define PAD254_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0BF4))
#define PAD255_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0BF8))
#define PAD256_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0BFC))
#define PAD257_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0C00))
#define PAD258_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0C04))

static void cpsw_pad_config()
{
	volatile u32 val = 0;

	/*configure pin mux for rmii_refclk,mdio_clk,mdio_d */
	val = PAD232_CNTRL;
	PAD232_CNTRL = (volatile unsigned int) (BIT(18) | BIT(0));
	val = PAD233_CNTRL;
	PAD233_CNTRL = (volatile unsigned int) (BIT(19) | BIT(17) | BIT(0));
	val = PAD234_CNTRL;
	PAD234_CNTRL = (volatile unsigned int) (BIT(19) | BIT(18) | BIT(17) |
			BIT(0));

	/*setup rgmii0/rgmii1 pins here*/
	/* In this case we enable rgmii_en bit in GMII_SEL register and
	 * still program the pins in gmii mode: gmii0 pins in mode 1*/
	val = PAD235_CNTRL; /*rgmii0_rxc*/
	PAD235_CNTRL = (volatile unsigned int) (BIT(18) | BIT(0));
	val = PAD236_CNTRL; /*rgmii0_rxctl*/
	PAD236_CNTRL = (volatile unsigned int) (BIT(18) | BIT(0));
	val = PAD237_CNTRL; /*rgmii0_rxd[2]*/
	PAD237_CNTRL = (volatile unsigned int) (BIT(18) | BIT(0));
	val = PAD238_CNTRL; /*rgmii0_txctl*/
	PAD238_CNTRL = (volatile unsigned int) BIT(0);
	val = PAD239_CNTRL; /*rgmii0_txc*/
	PAD239_CNTRL = (volatile unsigned int) BIT(0);
	val = PAD240_CNTRL; /*rgmii0_txd[0]*/
	PAD240_CNTRL = (volatile unsigned int) BIT(0);
	val = PAD241_CNTRL; /*rgmii0_rxd[0]*/
	PAD241_CNTRL = (volatile unsigned int) (BIT(18) | BIT(0));
	val = PAD242_CNTRL; /*rgmii0_rxd[1]*/
	PAD242_CNTRL = (volatile unsigned int) (BIT(18) | BIT(0));
	val = PAD243_CNTRL; /*rgmii1_rxctl*/
	PAD243_CNTRL = (volatile unsigned int) (BIT(18) | BIT(0));
	val = PAD244_CNTRL; /*rgmii0_rxd[3]*/
	PAD244_CNTRL = (volatile unsigned int) (BIT(18) | BIT(0));
	val = PAD245_CNTRL; /*rgmii0_txd[3]*/
	PAD245_CNTRL = (volatile unsigned int) BIT(0);
	val = PAD246_CNTRL; /*rgmii0_txd[2]*/
	PAD246_CNTRL = (volatile unsigned int) BIT(0);
	val = PAD247_CNTRL; /*rgmii0_txd[1]*/
	PAD247_CNTRL = (volatile unsigned int) BIT(0);
	val = PAD248_CNTRL; /*rgmii1_rxd[1]*/
	PAD248_CNTRL = (volatile unsigned int) (BIT(18) | BIT(0));
	val = PAD249_CNTRL; /*rgmii1_rxc*/
	PAD249_CNTRL = (volatile unsigned int) (BIT(18) | BIT(0));
	val = PAD250_CNTRL; /*rgmii1_rxd[3]*/
	PAD250_CNTRL = (volatile unsigned int) (BIT(18) | BIT(0));
	val = PAD251_CNTRL; /*rgmii1_txd[1]*/
	PAD251_CNTRL = (volatile unsigned int) (BIT(0));
	val = PAD252_CNTRL; /*rgmii1_txctl*/
	PAD252_CNTRL = (volatile unsigned int) (BIT(0));
	val = PAD253_CNTRL; /*rgmii1_txd[0]*/
	PAD253_CNTRL = (volatile unsigned int) (BIT(0));
	val = PAD254_CNTRL; /*rgmii1_txd[2]*/
	PAD254_CNTRL = (volatile unsigned int) (BIT(0));
	val = PAD255_CNTRL; /*rgmii1_txc*/
	PAD255_CNTRL = (volatile unsigned int) (BIT(0));
	val = PAD256_CNTRL; /*rgmii1_rxd[0]*/
	PAD256_CNTRL = (volatile unsigned int) (BIT(18) | BIT(0));
	val = PAD257_CNTRL; /*rgmii1_txd[3]*/
	PAD257_CNTRL = (volatile unsigned int) (BIT(0));
	val = PAD258_CNTRL; /*rgmii1_rxd[2]*/
	PAD258_CNTRL = (volatile unsigned int) (BIT(18) | BIT(0));
}
#endif /* CONFIG_DRIVER_TI_CPSW */

struct nor_pad_config {
	unsigned int offset;
	unsigned int value;
};

static struct nor_pad_config nor_pad_cfg[] = {
		{GPMC_D0, MODE(1) | INPUT_EN | PULL_DIS},
		{GPMC_D1, MODE(1) | INPUT_EN | PULL_DIS},
		{GPMC_D2, MODE(1) | INPUT_EN | PULL_DIS},
		{GPMC_D3, MODE(1) | INPUT_EN | PULL_DIS},
		{GPMC_D4, MODE(1) | INPUT_EN | PULL_DIS},
		{GPMC_D5, MODE(1) | INPUT_EN | PULL_DIS},
		{GPMC_D6, MODE(1) | INPUT_EN | PULL_DIS},
		{GPMC_D7, MODE(1) | INPUT_EN | PULL_DIS},
		{GPMC_D8, MODE(1) | INPUT_EN | PULL_DIS},
		{GPMC_D9, MODE(1) | INPUT_EN | PULL_DIS},
		{GPMC_D10, MODE(1) | INPUT_EN | PULL_DIS},
		{GPMC_D11, MODE(1) | INPUT_EN | PULL_DIS},
		{GPMC_D12, MODE(1) | INPUT_EN | PULL_DIS},
		{GPMC_D13, MODE(1) | INPUT_EN | PULL_DIS},
		{GPMC_D14, MODE(1) | INPUT_EN | PULL_DIS},
		{GPMC_D15, MODE(1) | INPUT_EN | PULL_DIS},
		{GPMC_A1, MODE(2) | PULL_UP_EN},
		{GPMC_A2, MODE(2) | PULL_UP_EN},
		{GPMC_A3, MODE(2) | PULL_UP_EN},
		{GPMC_A4, MODE(2) | PULL_UP_EN},
		{GPMC_A5, MODE(5) | PULL_UP_EN},
		{GPMC_A6, MODE(5)},
		{GPMC_A7, MODE(5)},
		{GPMC_A8, MODE(5)},
		{GPMC_A9, MODE(5)},
		{GPMC_A10, MODE(5) | PULL_UP_EN},
		{GPMC_A11, MODE(5)},
		{GPMC_A12, MODE(5)},
		{GPMC_A13, MODE(2) | PULL_UP_EN},
		{GPMC_A14, MODE(2) | PULL_UP_EN},
		{GPMC_A15, MODE(2)},
		{GPMC_A16, MODE(1)},
		{GPMC_A17, MODE(1)},
		{GPMC_A18, MODE(1)},
		{GPMC_A19, MODE(1)},
		{GPMC_A20, MODE(1) | PULL_UP_EN},
		{GPMC_A21, MODE(1)},
		{GPMC_A22, MODE(1) | PULL_UP_EN},
		{GPMC_A23, MODE(1)},
		{GPMC_A24, MODE(2) | PULL_UP_EN},
		{GPMC_A25, MODE(2)},
		{GPMC_A27, MODE(8) | PULL_UP_EN},
		{GPMC_CS0_REG, MODE(1) | PULL_UP_EN},
		{GPMC_OEN, MODE(1) | PULL_UP_EN},
		{GPMC_WEN, MODE(1) | PULL_UP_EN},
		{0},
};

/*********************************************************************
 *
 * nor_pad_config_mux - configure the pin mux for NOR
 *
 *********************************************************************/
static void nor_pad_config_mux(void)
{
	u8 i = 0;

	while (nor_pad_cfg[i].offset != 0x0) {
		*(volatile u32 *)(nor_pad_cfg[i].offset) =
			nor_pad_cfg[i].value;
		i++;
	}
}

/*
 * baord specific muxing of pins
 */
void set_muxconf_regs(void)
{
	u32 i, add, val;
	u32 pad_conf[] = {
#include "mux.h"
	};

	for (i = 0; i < N_PINS; i++) {
		add = PIN_CTRL_BASE + (i*4);
		val = __raw_readl(add);
		val |= pad_conf[i];
		__raw_writel(val, add);
	}
	/* MMC/SD pull-down enable */
	__raw_writel(0x000C0040, 0x48140928);
}

void unlock_pll_control_mmr()
{
	/* ??? */
	__raw_writel(0x1EDA4C3D, 0x481C5040);
	__raw_writel(0x2FF1AC2B, 0x48140060);
	__raw_writel(0xF757FDC0, 0x48140064);
	__raw_writel(0xE2BC3A6D, 0x48140068);
	__raw_writel(0x1EBF131D, 0x4814006c);
	__raw_writel(0x6F361E05, 0x48140070);

}

/*
 * early system init of muxing and clocks.
 */
void s_init(u32 in_ddr)
{
	/* TODO: Revisit enabling of I/D-cache in 1st stage */
#if 0
	icache_enable();
	dcache_enable();
#endif

	/*
	 * Disable Write Allocate on miss to avoid starvation of other masters
	 * (than A8).
	 *
	 * Ref DM385 Erratum: TODO
	 */
	l2_disable_wa();

	/* Can be removed as A8 comes up with L2 enabled */
	l2_cache_enable();
	unlock_pll_control_mmr();
	/* Setup the PLLs and the clocks for the peripherals */
	prcm_init(in_ddr);
#ifdef CONFIG_DM385_CONFIG_DDR
	if (!in_ddr)
		config_dm385_ddr();	/* Do DDR settings */
#endif
}

/*
 * Reset the board
 */
void reset_cpu(ulong addr)
{
	addr = __raw_readl(PRM_DEVICE_RSTCTRL);
	addr &= ~BIT(1);
	addr |= BIT(1);
	__raw_writel(addr, PRM_DEVICE_RSTCTRL);
}

#ifdef CONFIG_DRIVER_TI_CPSW

#define SMA1_CPSW_CLOCK_MASK		0xfffffd0f
#define SMA1_CPSW_SEL_SATA1_SRC		(1 << 10)
#define SMA1_CPSW_SEL_CPTS_GMII_SRC	(1 << 6)
#define SMA1_CPSW_SEL_CPTS_RMII_SRC	(1 << 4)

/* TODO : Check for the board specific PHY */
static void phy_init(char *name, int addr)
{
	unsigned short val;
	unsigned int   cntr = 0;

	miiphy_reset(name, addr);

	udelay(100000);

	/* Enable Autonegotiation */
	if (miiphy_read(name, addr, PHY_BMCR, &val) != 0) {
		printf("failed to read bmcr\n");
		return;
	}
	val |= PHY_BMCR_DPLX | PHY_BMCR_AUTON | PHY_BMCR_100_MBPS;
	if (miiphy_write(name, addr, PHY_BMCR, val) != 0) {
		printf("failed to write bmcr\n");
		return;
	}
	miiphy_read(name, addr, PHY_BMCR, &val);

	/* Setup GIG advertisement */
	miiphy_read(name, addr, PHY_1000BTCR, &val);
	val |= PHY_1000BTCR_1000FD;
	val &= ~PHY_1000BTCR_1000HD;
	miiphy_write(name, addr, PHY_1000BTCR, val);
	miiphy_read(name, addr, PHY_1000BTCR, &val);

	/* Setup general advertisement */
	if (miiphy_read(name, addr, PHY_ANAR, &val) != 0) {
		printf("failed to read anar\n");
		return;
	}
	val |= (PHY_ANLPAR_10 | PHY_ANLPAR_10FD | PHY_ANLPAR_TX |
		PHY_ANLPAR_TXFD);
	if (miiphy_write(name, addr, PHY_ANAR, val) != 0) {
		printf("failed to write anar\n");
		return;
	}
	miiphy_read(name, addr, PHY_ANAR, &val);

	/* Restart auto negotiation*/
	miiphy_read(name, addr, PHY_BMCR, &val);
	val |= PHY_BMCR_RST_NEG;
	miiphy_write(name, addr, PHY_BMCR, val);

	/*check AutoNegotiate complete - it can take upto 3 secs*/
	do {
		udelay(40000);
		cntr++;

		if (!miiphy_read(name, addr, PHY_BMSR, &val)) {
			if (val & PHY_BMSR_AUTN_COMP)
				break;
		}
	} while (cntr < 250);

	if (!miiphy_read(name, addr, PHY_BMSR, &val)) {
		if (!(val & PHY_BMSR_AUTN_COMP))
			printf("Auto negotitation failed\n");
	}
}

static void cpsw_control(int enabled)
{
	/* nothing for now */
	/* TODO : VTP was here before */
}

static struct cpsw_slave_data cpsw_slaves[] = {
	{
		.slave_reg_ofs	= 0x50,
		.sliver_reg_ofs	= 0x700,
		.phy_id		= 0,
	},
	{
		.slave_reg_ofs	= 0x90,
		.sliver_reg_ofs	= 0x740,
		.phy_id		= 1,
	},
};

static struct cpsw_platform_data cpsw_data = {
	.mdio_base		= TI814X_CPSW_MDIO_BASE,
	.cpsw_base		= TI814X_CPSW_BASE,
	.mdio_div		= 0xff,
	.channels		= 8,
	.cpdma_reg_ofs		= 0x100,
	.slaves			= 1,
	.slave_data		= cpsw_slaves,
	.ale_reg_ofs		= 0x600,
	.ale_entries		= 1024,
	.host_port_reg_ofs	= 0x28,
	.hw_stats_reg_ofs	= 0x400,
	.mac_control		= (1 << 5) /* MIIEN      */,
	.control		= cpsw_control,
	.phy_init		= phy_init,
	.host_port_num		= 0,
};

extern void cpsw_eth_set_mac_addr (const u_int8_t *addr);

int board_eth_init(bd_t *bis)
{
	u_int8_t mac_addr[6];
	u_int32_t mac_hi,mac_lo;
	u_int32_t eth_clock_config;

#ifdef CONFIG_DRIVER_TI_CPSW
	cpsw_pad_config();
#endif

	eth_clock_config = __raw_readl(SMA1);
	printf("Ethernet clocking: 0x%x\n", eth_clock_config);

#ifdef ETHERNET_SATA_1_CLOCK_SRC
	eth_clock_config &= SMA1_CPSW_CLOCK_MASK;
	/* By default sata0 is selected in SMA1 REG
	Changing it to Sata 1 for power optimization */
	eth_clock_config |= SMA1_CPSW_SEL_SATA1_SRC;
	__raw_writel(eth_clock_config, SMA1);
#endif

	if (!eth_getenv_enetaddr("ethaddr", mac_addr)) {
		char mac_addr_env[20];

		printf("<ethaddr> not set. Reading from E-fuse\n");
		/* try reading mac address from efuse */
		mac_lo = __raw_readl(MAC_ID0_LO);
		mac_hi = __raw_readl(MAC_ID0_HI);
		mac_addr[0] = mac_hi & 0xFF;
		mac_addr[1] = (mac_hi & 0xFF00) >> 8;
		mac_addr[2] = (mac_hi & 0xFF0000) >> 16;
		mac_addr[3] = (mac_hi & 0xFF000000) >> 24;
		mac_addr[4] = mac_lo & 0xFF;
		mac_addr[5] = (mac_lo & 0xFF00) >> 8;
		/* set the ethaddr variable with MACID detected */
		sprintf(mac_addr_env, "%02x:%02x:%02x:%02x:%02x:%02x",
			mac_addr[0], mac_addr[1], mac_addr[2],
			mac_addr[3], mac_addr[4], mac_addr[5]);
		eth_setenv_enetaddr("ethaddr", mac_addr_env);
	}

	if(is_valid_ether_addr(mac_addr)) {
		printf("Detected MACID:%x:%x:%x:%x:%x:%x\n",mac_addr[0],
			mac_addr[1], mac_addr[2], mac_addr[3],
			mac_addr[4], mac_addr[5]);
		cpsw_eth_set_mac_addr(mac_addr);
	} else {
		printf("Caution:using static MACID!! Set <ethaddr> variable\n");
	}

	return cpsw_register(&cpsw_data);
}
#endif

#ifdef CONFIG_NAND_TI81XX
/******************************************************************************
 * Command to switch between NAND HW and SW ecc
 *****************************************************************************/
extern void ti81xx_nand_switch_ecc(nand_ecc_modes_t hardware, int32_t mode);
static int do_switch_ecc(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int type = 0;
	if (argc < 2)
		goto usage;

	if (strncmp(argv[1], "hw", 2) == 0) {
		if (argc == 3)
			type = simple_strtoul(argv[2], NULL, 10);
		ti81xx_nand_switch_ecc(NAND_ECC_HW, type);
	} else if (strncmp(argv[1], "sw", 2) == 0) {
		ti81xx_nand_switch_ecc(NAND_ECC_SOFT, 0);
	} else
		goto usage;

	return 0;

usage:
	printf("Usage: nandecc %s\n", cmdtp->usage);
	return 1;
}

U_BOOT_CMD(
	nandecc, 3, 1,	do_switch_ecc,
	"Switch NAND ECC calculation algorithm b/w hardware and software",
	"[sw|hw <hw_type>] \n"
	"   [sw|hw]- Switch b/w hardware(hw) & software(sw) ecc algorithm\n"
	"   hw_type- 0 for Hamming code\n"
	"            1 for bch4\n"
	"            2 for bch8\n"
	"            3 for bch16\n"
);

#endif /* CONFIG_NAND_TI81XX */

#ifdef CONFIG_GENERIC_MMC
int board_mmc_init(bd_t *bis)
{
	omap_mmc_init(0);
	return 0;
}
#endif

