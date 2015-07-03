#include <logo.h>

static void ti816x_hdvpss_clk_enable(void)
{
	uint delay;
	int i;

	__raw_writel(2, CM_HDDSS_CLKSTCTRL); /*Enable Power Domain Transition for HDVPSS */
	__raw_writel(0, CM_ACTIVE_HDDSS_CLKCTRL); /*Enable HDVPSS Clocks*/
	delay = 1000;
	while(delay-- > 0);
	__raw_writel(2, CM_ACTIVE_HDDSS_CLKCTRL); /*Enable HDVPSS Clocks*/

	/*Check clocks are active*/
	for (i = 0;i < 1000; i++) { 
		if (((__raw_readl(CM_HDDSS_CLKSTCTRL) & 0xFF00) >> 8) == 0xFF)
			break;
		else 
			udelay(10);
	}

	/* Check to see module is functional */
	for (i = 0;i < 1000; i++) { 
		if (((__raw_readl(CM_ACTIVE_HDDSS_CLKCTRL) & 0x70000) >> 16) == 0)
			break;
		else 
			udelay(10);
	}
}

static void ti816x_hdmi_clk_enable(void)
{
	int i;
	
	__raw_writel(2, CM_HDMI_CLKSTCTRL); /*Enable Power Domain Transition for HDMI */
	__raw_writel(2, CM_ACTIVE_HDMI_CLKCTRL); /*Enable HDMI Clocks*/

	/*Check clocks are active*/
	for (i = 0;i < 1000; i++) { 
		if (((__raw_readl(CM_HDMI_CLKSTCTRL) & 0x300) >> 8) == 0x3)
			break;
		else 
			udelay(10);
	}

	/* Check to see module is functional */
	for (i = 0;i < 1000; i++) { 
		if (((__raw_readl(CM_ACTIVE_HDMI_CLKCTRL) & 0x70000) >> 16) == 0)
			break;
		else 
			udelay(10);
	}
}

static void ti816x_video_pll(uint video_n, uint video_p, uint video_intfreq1, uint 
video_fracfreq1, uint video_mdiv1, uint video_intfreq2, uint video_fracfreq2, 
uint video_mdiv2, uint video_intfreq3, uint video_fracfreq3, uint video_mdiv3, 
uint b1, unsigned c2)
{
	uint video_pll_ctrl = 0;
	int i;

	/*Put the Video PLL in Bypass Mode*/
	video_pll_ctrl = __raw_readl(VIDEOPLL_CTRL);
	video_pll_ctrl &= 0xFFFFFFFB;
	video_pll_ctrl |= 4;
	__raw_writel(video_pll_ctrl, VIDEOPLL_CTRL);

	/*Bring Video PLL out of Power Down Mode*/
	video_pll_ctrl = __raw_readl(VIDEOPLL_CTRL);
	video_pll_ctrl &= 0xFFFFFFF7;
	video_pll_ctrl |= 8;
	__raw_writel(video_pll_ctrl, VIDEOPLL_CTRL);

	/*Program the Video PLL Multiplier and Pre-dividr value*/
	video_pll_ctrl = __raw_readl(VIDEOPLL_CTRL);
	video_pll_ctrl &= 0xFF;
	video_pll_ctrl |= (video_n << 16) | (video_p << 8);
	__raw_writel(video_pll_ctrl, VIDEOPLL_CTRL);

	/*Bring the Video PLL Individual output clocks out of Power Down Mode
	  1->Power Down
	  0->Normal Mode i.e., out of Power Down Mode
	 */
	__raw_writel(0x0, VIDEOPLL_PWD);

	/*Program the Video PLL to generate SYSCLK11 (RF Modulator), SYSCLK17 (SD_VENC), and STC1 source clocks*/
	__raw_writel((1 << 31) | (1 << 28) | (video_intfreq1 << 24) |
						video_fracfreq1, VIDEOPLL_FREQ1);

	/*Program Video PLL SYSCLK11 Post divider register*/
	__raw_writel((1 << 8) | video_mdiv1, VIDEOPLL_DIV1);

	/*Program the PLL to generate SYSCLK13 (HD_VENC_D clock) and STC0/STC1 source clocks */
	__raw_writel((1 << 31) | (1 << 28) | (video_intfreq2 << 24) |
						video_fracfreq2, VIDEOPLL_FREQ2);
	__raw_writel((1 << 8) | video_mdiv2, VIDEOPLL_DIV2);

	/*Program the PLL to generate SYSCLK13 and STC0/STC1 source clocks */
	__raw_writel((1 << 31) | (1 << 28) | (video_intfreq3 << 24) |
						video_fracfreq3, VIDEOPLL_FREQ3);
	__raw_writel((1 << 8) | video_mdiv3, VIDEOPLL_DIV3);

	__raw_writel(c2, CM_SYSCLK13_CLKSEL);
	__raw_writel(b1, CM_SYSCLK15_CLKSEL);

	/*Wait for PLL to Lock*/
	for (i = 0;i < 1000; i++) { 
		if ((__raw_readl(VIDEOPLL_CTRL) & 0x80) == 0x80 )
			break;
		else 
			udelay(10);
	}

	/*Put the Main PLL in Normal(PLL) Mode*/
	video_pll_ctrl = __raw_readl(VIDEOPLL_CTRL);
	video_pll_ctrl &= 0xFFFFFFFB;
	__raw_writel(video_pll_ctrl, VIDEOPLL_CTRL);
}

static void ti816x_hdvpss_vencD297_vencA297(void)
{
	/*SYS_CLKOUT selection*/
	__raw_writel(0xA2, CM_CLKOUT_CTRL);  /* /5 divider is selected*/
	ti816x_video_pll(0x6E, 0x2, 0xB, 0x0, 0x5, 0xA, 0x0, 0x2, 0xA, 0x0, 0x2, 0x3, 0x3);
}

static void ti816x_vps_init(void)
{
	/* enable clocks on all units */
	__raw_writel(0x01031FFF, VPS_CLKC_ENABLE);
	__raw_writel(0xC010E, VPS_CLKC_VENC_CLK_SELECT);
	/* enable all video encoders */
	__raw_writel(0xF, VPS_CLKC_VENC_ENABLE);
	/* enable comp units */
	__raw_writel(0 | (1 << 0), VPS_COMP_STATUS);   /* HDMI enable */
	/* set background color */
	__raw_writel(LOGO_BGCOLOR, VPS_COMP_BGCOLOR);
}

static void ti816x_set_pinmux(void)
{
	__raw_writel(2, CTRL_MODULE_BASE_ADDR + 0x0814);
	__raw_writel(2, CTRL_MODULE_BASE_ADDR + 0x0818);
	__raw_writel(2, CTRL_MODULE_BASE_ADDR + 0x081c);
	__raw_writel(2, CTRL_MODULE_BASE_ADDR + 0x0820);
	__raw_writel(2, CTRL_MODULE_BASE_ADDR + 0x0824);
	__raw_writel(1, CTRL_MODULE_BASE_ADDR + 0x0828);
	__raw_writel(1, CTRL_MODULE_BASE_ADDR + 0x082c);
	__raw_writel(1, CTRL_MODULE_BASE_ADDR + 0x0830);
	__raw_writel(1, CTRL_MODULE_BASE_ADDR + 0x0834);
	__raw_writel(1, CTRL_MODULE_BASE_ADDR + 0x0838);
	__raw_writel(2, CTRL_MODULE_BASE_ADDR + 0x083c);
	__raw_writel(2, CTRL_MODULE_BASE_ADDR + 0x0840);
	__raw_writel(2, CTRL_MODULE_BASE_ADDR + 0x0844);
	__raw_writel(2, CTRL_MODULE_BASE_ADDR + 0x0848);
	__raw_writel(2, CTRL_MODULE_BASE_ADDR + 0x084c);
	__raw_writel(2, CTRL_MODULE_BASE_ADDR + 0x0850);
	__raw_writel(1, CTRL_MODULE_BASE_ADDR + 0x0854);
	__raw_writel(1, CTRL_MODULE_BASE_ADDR + 0x0858);
	__raw_writel(1, CTRL_MODULE_BASE_ADDR + 0x085c);
	__raw_writel(1, CTRL_MODULE_BASE_ADDR + 0x0860);
	__raw_writel(1, CTRL_MODULE_BASE_ADDR + 0x0864);
	__raw_writel(1, CTRL_MODULE_BASE_ADDR + 0x0868);
	__raw_writel(1, CTRL_MODULE_BASE_ADDR + 0x086c);
	__raw_writel(1, CTRL_MODULE_BASE_ADDR + 0x0870);
	__raw_writel(1, CTRL_MODULE_BASE_ADDR + 0x0874);
	__raw_writel(2, CTRL_MODULE_BASE_ADDR + 0x0878);
	__raw_writel(1, CTRL_MODULE_BASE_ADDR + 0x087c);
	__raw_writel(1, CTRL_MODULE_BASE_ADDR + 0x0880);
	__raw_writel(1, CTRL_MODULE_BASE_ADDR + 0x0884);
	__raw_writel(1, CTRL_MODULE_BASE_ADDR + 0x0888);
	__raw_writel(1, CTRL_MODULE_BASE_ADDR + 0x088c);
	__raw_writel(1, CTRL_MODULE_BASE_ADDR + 0x0890);
	__raw_writel(1, CTRL_MODULE_BASE_ADDR + 0x0894);
	__raw_writel(1, CTRL_MODULE_BASE_ADDR + 0x0898);
	__raw_writel(1, CTRL_MODULE_BASE_ADDR + 0x089c);
	__raw_writel(2, CTRL_MODULE_BASE_ADDR + 0x08b4);
	__raw_writel(2, CTRL_MODULE_BASE_ADDR + 0x08b8);
	__raw_writel(2, CTRL_MODULE_BASE_ADDR + 0x08bc);
	__raw_writel(2, CTRL_MODULE_BASE_ADDR + 0x08c0);
	__raw_writel(2, CTRL_MODULE_BASE_ADDR + 0x08c4);
	__raw_writel(0, CTRL_MODULE_BASE_ADDR + 0x09d4);
	__raw_writel(0, CTRL_MODULE_BASE_ADDR + 0x09d8);
	__raw_writel(0, CTRL_MODULE_BASE_ADDR + 0x09dc);
	__raw_writel(0, CTRL_MODULE_BASE_ADDR + 0x09e0);
	__raw_writel(0, CTRL_MODULE_BASE_ADDR + 0x09e4);
	__raw_writel(0, CTRL_MODULE_BASE_ADDR + 0x09e8);
	__raw_writel(0, CTRL_MODULE_BASE_ADDR + 0x09ec);
	__raw_writel(0, CTRL_MODULE_BASE_ADDR + 0x09f0);
}

static void ti816x_configure_hdvenc720P60(int useEmbeddedSync)
{
	volatile uint *vencHd_D_Base = NULL;

	vencHd_D_Base = (volatile uint *)0x48106000;

	if (useEmbeddedSync != 0x0)
		*vencHd_D_Base = 0x4002A033; 
	else
		*vencHd_D_Base = 0x40032033;    

	vencHd_D_Base++;
	*vencHd_D_Base = 0x1FD01E24;
	vencHd_D_Base++;
	*vencHd_D_Base = 0x02DC020C;
	vencHd_D_Base++;
	*vencHd_D_Base = 0x00DA004A;
	vencHd_D_Base++;
	*vencHd_D_Base = 0x020C1E6C;
	vencHd_D_Base++;
	*vencHd_D_Base = 0x02001F88;
	vencHd_D_Base++;
	*vencHd_D_Base = 0x00200000;
	vencHd_D_Base++;
	*vencHd_D_Base = 0x1B6C0C77;
	vencHd_D_Base++;
	*vencHd_D_Base = 0x1C0C0C30;
	vencHd_D_Base++;
	*vencHd_D_Base = 0x1C0C0C30;
	vencHd_D_Base++;
	*vencHd_D_Base = 0x842EE672;
	vencHd_D_Base++;
	*vencHd_D_Base = 0x3F000018;
	vencHd_D_Base++;
	*vencHd_D_Base = 0x50500103;
	vencHd_D_Base++;
	*vencHd_D_Base = 0x000002E8;
	vencHd_D_Base++;
	*vencHd_D_Base = 0x000C39E7;
	vencHd_D_Base++;
	*vencHd_D_Base = 0x50500172;
	vencHd_D_Base++;
	*vencHd_D_Base = 0x0001A64B;
	vencHd_D_Base++;
	*vencHd_D_Base = 0x002D0000;
	vencHd_D_Base++;
	*vencHd_D_Base = 0x05000000;
	vencHd_D_Base++;
	*vencHd_D_Base = 0x00003000;
	vencHd_D_Base++;
	*vencHd_D_Base = 0x00000000;
	vencHd_D_Base++;
	*vencHd_D_Base = 0x5050016A;
	vencHd_D_Base++;
	*vencHd_D_Base = 0x0001A64B;
	vencHd_D_Base++;
	*vencHd_D_Base = 0x002D0000;
	vencHd_D_Base++;
	*vencHd_D_Base = 0x05000000;
	vencHd_D_Base++;
	*vencHd_D_Base = 0x00003000;
	vencHd_D_Base++;
	*vencHd_D_Base = 0x00000000;
	vencHd_D_Base++;
	*vencHd_D_Base = 0x00000000;
}

static void ti816x_hdmi_enable(void)
{
	u32 temp;
	volatile ulong reset_time_out;
	
	temp = __raw_readl(HDMI_REG_BASE + 0x0008);
	temp |= 0x0001;
	__raw_writel(temp, HDMI_REG_BASE + 0x0008);
	reset_time_out = 0x000FFFFF;
	while (((__raw_readl(HDMI_REG_BASE + 0x0008)) & 0x1) == 0x1) 
	{
		reset_time_out--;
		if (reset_time_out == 0x0) 
		{
			printf("Could not reset wrapper ");
			break;
		}
	}

	/* phy settings */
	temp = __raw_readl(0x48122008);
	if ((temp & 0x00000001) != 0x00000001)
	{
		temp |= 0x00000001;
	}
	__raw_writel(temp, 0x48122008);
	temp = __raw_readl(0x48122008);
	temp &= (~((0x00000006) | (0x00000018) | (0x00000040)));
	temp |= (0x00000000 << 0x00000001);   /* 8-bit channel */
	temp |= (0x00000001 << 0x00000003);
	__raw_writel(temp, 0x48122008);
	temp = __raw_readl(0x4812200C);
	temp &= ~0x01;
	temp |= 0x004;
	__raw_writel(temp, 0x4812200C);
	/* disable 10bit encode in the PHY */
	__raw_writel(0xE0, 0x48122020);
	temp = __raw_readl(0x48122004);
	temp |= (0x10 | 0x20);
	__raw_writel(temp, 0x48122004);

	/* wrapper settings */
	/* wrapper debounce config */
	temp = __raw_readl(HDMI_REG_BASE + 0x0044) ;
	temp = temp | 0x00001414;
	__raw_writel(temp, HDMI_REG_BASE + 0x0044);
	/* cec clock divider config */
	temp = __raw_readl(HDMI_REG_BASE + 0x0070) ;
	temp = temp | 0x00000218;
	__raw_writel(temp, HDMI_REG_BASE + 0x0070);
	temp = __raw_readl(HDMI_REG_BASE + 0x0050);
	temp &= (~(0x700));
	temp |= ((0x00000001 << 0x00000008) & 0x00000700);
	temp &= (~(0x03));   /* in slave mode */
	__raw_writel(temp, HDMI_REG_BASE + 0x0050);
	/* audio wrapper disable */
	temp = __raw_readl(HDMI_REG_BASE + 0x0088);
	temp &= (~(0x80000000));
	__raw_writel(temp, HDMI_REG_BASE + 0x0088);
	__raw_writel(0x0, HDMI_REG_BASE + 0x0080);
	/* Bring the CORE out of reset */
	temp = __raw_readl(HDMI_REG_BASE + 0x0414);
	temp &= (~(0x00000001));
	__raw_writel(temp, HDMI_REG_BASE + 0x0414);
	temp = __raw_readl(HDMI_REG_BASE + 0x0420);
	temp &= (~(0x00000001));	/* Interrupts in power down mode */
	__raw_writel(temp, HDMI_REG_BASE + 0x0420);

	/* core settings */
	temp = __raw_readl(HDMI_REG_BASE + 0x09F0);
        temp &= (~(0x00000008));
	__raw_writel(temp, HDMI_REG_BASE + 0x09F0);
	temp = __raw_readl(HDMI_REG_BASE + 0x0524);
        temp &= (~(0x000000C0));
        temp |= ((0x0 << 0x00000006) & 0x000000C0);   /* 8-bits channel width */
	__raw_writel(temp, HDMI_REG_BASE + 0x0524);
        temp = __raw_readl(HDMI_REG_BASE + 0x0420);
        temp &= (~(0x00000004 | 0x00000002));
        temp |= 0x00000002;
        temp |= 0x00000004;
	__raw_writel(temp, HDMI_REG_BASE + 0x0420);
	__raw_writel(0x00000001, HDMI_REG_BASE + 0x04CC);  /* data enable control */
	temp = __raw_readl(HDMI_REG_BASE + 0x0420);     
        temp |= 0x00000020;
        temp |= 0x00000010;
	__raw_writel(temp, HDMI_REG_BASE + 0x0420);  /* enable hsync and vsync */
	__raw_writel(0x0, HDMI_REG_BASE + 0x0528);  /* vid mode config */
	__raw_writel(0x0, HDMI_REG_BASE + 0x04F8);  /* iadjust config to enable vsync */
	__raw_writel(0x10, HDMI_REG_BASE + 0x0520); /* csc is bt709 */
	__raw_writel(0x20, HDMI_REG_BASE + 0x09BC); /* enable dvi */
	__raw_writel(0x20, HDMI_REG_BASE + 0x0608); /* tmds_ctrl */
	__raw_writel(0x0, HDMI_REG_BASE + 0x0904);
	__raw_writel(0x0, HDMI_REG_BASE + 0x0950);  /* disable audio */
	__raw_writel(0x0, HDMI_REG_BASE + 0x0414);  /* keep audio  operation in reset state */

	temp = __raw_readl(HDMI_REG_BASE + 0x0420);
	temp |= 0x00000001;
	__raw_writel(temp, HDMI_REG_BASE + 0x0420);
	temp = __raw_readl(HDMI_REG_BASE + 0x0414);
	temp &= (~(0x00000001));
	__raw_writel(temp, HDMI_REG_BASE + 0x0414);
	temp = __raw_readl(HDMI_REG_BASE + 0x0050);
	temp &= (~(0x00000008));
	__raw_writel(temp, HDMI_REG_BASE + 0x0050);
	temp = __raw_readl(HDMI_REG_BASE + 0x09BC);
	temp &= (~(0x00000001));
	__raw_writel(temp, HDMI_REG_BASE + 0x09BC);
}

void ti816x_set_board(void)
{
	/* enable vps clock */
	ti816x_hdvpss_clk_enable();
	/* enable hdmi clock */
	ti816x_hdmi_clk_enable();
	ti816x_hdvpss_vencD297_vencA297();
	ti816x_vps_init();
	ti816x_set_pinmux();
	ti816x_configure_hdvenc720P60(0u);
	/* set the comp for hdmi */
	ti816x_hdmi_enable();
}
