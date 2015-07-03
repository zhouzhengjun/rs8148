
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/timer.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>

#include <asm/dma.h>
#include <asm/mach-types.h>

#ifndef CONFIG_ARCH_TI81XX
#include <mach/asp.h>
#include <mach/edma.h>
#include <mach/mux.h>
#else
#include <plat/asp.h>
#include <asm/hardware/edma.h>
#endif


#include "davinci-pcm.h"
#include "davinci-i2s.h"
#include "davinci-mcasp.h"


#define gs2971a


#define AUDIO_FORMAT SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_CBM_CFM | SND_SOC_DAIFMT_NB_NF
//#define AUDIO_FORMAT SND_SOC_DAIFMT_CBM_CFM

static int evm_hw_params(struct snd_pcm_substream *substream,
			 struct snd_pcm_hw_params *params)
{
	printk("<0>""evm_hw_params is called!\n");

	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->codec_dai;
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
	int ret = 0;
	unsigned sysclk;
#if 0
	/* set codec DAI configuration */
	ret = snd_soc_dai_set_fmt(codec_dai, AUDIO_FORMAT);
	if (ret < 0)
		return ret;
#endif
	/* set cpu DAI configuration */
	ret = snd_soc_dai_set_fmt(cpu_dai, AUDIO_FORMAT);

	if (ret < 0)
		return ret;
#if 0

        sysclk = 8000000;
        ret = snd_soc_dai_set_sysclk(codec_dai, 0, sysclk, SND_SOC_CLOCK_IN);

	if (ret < 0)
		return ret;
#endif
	printk("<0>""evm_hw_params execu success!\n");
	return 0;
}


static struct snd_soc_ops evm_ops = {
	.hw_params = evm_hw_params,
};


#ifdef gs2971a
/* Logic for a gs2971Audio as connected on a davinci-evm */
static int evm_gs2971Audio_init(struct snd_soc_pcm_runtime *rtd)
{
	//likely nothing need to do, because pinmux has been set well in the function ti814x_mcasp0_pinmux_init.

	return 0;
}
#endif



static struct snd_soc_dai_link ti81xx_evm_dai[] = {
#if 0
	{
		.name = "TLV320AIC3X",
		.stream_name = "AIC3X",
		.codec_dai_name = "tlv320aic3x-hifi",
		.codec_name = "tlv320aic3x-codec.1-0018",
		.platform_name = "davinci-pcm-audio",
		.init = evm_aic3x_init,
		.ops = &evm_ops,
	},


        {
                .name = "TLV320AIC3X",
                .stream_name = "AIC3X",
                .cpu_dai_name = "davinci-mcasp.1",
                .codec_dai_name = "tlv320aic3x-hifi",
                .codec_name = "tlv320aic3x-codec.1-0018",
                .platform_name = "davinci-pcm-audio",
                .init = evm_aic3x_init,
                .ops = &evm_ops,
        },
#endif 

#ifdef gs2971a
		{
                .name = "GS2971A",
                .stream_name = "GS2971A_0",
                .cpu_dai_name = "dm8148-mcasp",
                .codec_dai_name = "gs2971a-aout",
                .codec_name = "gs2971a-codec",
                .platform_name = "davinci-pcm-audio",
                .init = evm_gs2971Audio_init,//need modify mark
                .ops = &evm_ops,//need modify mark
        },
#endif


};




static struct snd_soc_card ti81xx_snd_soc_card = {
	.name = "DM8148 EVM",
	.dai_link = ti81xx_evm_dai,
	.num_links = ARRAY_SIZE(ti81xx_evm_dai),
};



static struct platform_device *evm_snd_device;
static int __init evm_init(void)
{
	struct snd_soc_card *evm_snd_dev_data;
	int index;
	int ret;

	
	evm_snd_dev_data = &ti81xx_snd_soc_card;
	//index = 0;
	index = -1; //not use
	

	evm_snd_device = platform_device_alloc("soc-audio", index);
	if (!evm_snd_device)
		return -ENOMEM;

	platform_set_drvdata(evm_snd_device, evm_snd_dev_data);
	ret = platform_device_add(evm_snd_device);
	if (ret)
		platform_device_put(evm_snd_device);

	return ret;
}

static void __exit evm_exit(void)
{
	platform_device_unregister(evm_snd_device);
}

module_init(evm_init);
module_exit(evm_exit);

MODULE_AUTHOR("Bo Wen");
MODULE_DESCRIPTION("TI DM8148 EVM ASoC driver");
MODULE_LICENSE("GPL");

