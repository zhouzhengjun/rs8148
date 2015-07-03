#include <linux/init.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/mutex.h>
#include <linux/math64.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/mod_devicetable.h>

#include <linux/spi/spi.h>
#include <linux/spi/flash.h>

#include <linux/kernel.h>
#include <linux/cdev.h>
#include <asm/io.h>


#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/pm.h>

#include <linux/regulator/consumer.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <sound/initval.h>



//Now, I am not sure if it's right, I will make sure late to reference the datasheet.
#define GS2971_RATES	SNDRV_PCM_RATE_8000_96000    //48kHz
#define GS2971_FORMATS	(SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S20_3LE | \
			 SNDRV_PCM_FMTBIT_S24_3LE)


//for gs2971a, HD and 3G audio core registers:200h - 296h
#define GS2971_REG_NUM 0x96

/* 所有寄存器的默认值 */
static const short gs2971_reg[GS2971_REG_NUM] = {   
    0x0000, 
};

struct spi_device *gs2971Spi;


#if 0

/*-------------------------------------------------------------------------*/
struct spidev_data {
	dev_t			devt;
	spinlock_t		spi_lock;
	struct spi_device	*spi;
	struct list_head	device_entry;

	/* buffer is NULL unless this device is open (users > 0) */
	struct mutex		buf_lock;
	unsigned		users;
	u8			*buffer;
};

static LIST_HEAD(device_list);
static DEFINE_MUTEX(device_list_lock);

static unsigned bufsiz = 4096;
module_param(bufsiz, uint, S_IRUGO);
MODULE_PARM_DESC(bufsiz, "data bytes in biggest supported SPI message");

/*-------------------------------------------------------------------------*/

/*
 * We can't use the standard synchronous wrappers for file I/O; we
 * need to protect against async removal of the underlying spi_device.
 */
static void spidev_complete(void *arg)
{
	complete(arg);
}

static ssize_t
spidev_sync(struct spidev_data *spidev, struct spi_message *message)
{
	DECLARE_COMPLETION_ONSTACK(done);
	int status;

	message->complete = spidev_complete;
	message->context = &done;

	spin_lock_irq(&spidev->spi_lock);
	if (spidev->spi == NULL)
		status = -ESHUTDOWN;
	else
		status = spi_async(spidev->spi, message);
	spin_unlock_irq(&spidev->spi_lock);

	if (status == 0) {
		wait_for_completion(&done);
		status = message->status;
		if (status == 0)
			status = message->actual_length;
	}
	return status;
}

static inline ssize_t
spidev_sync_write(struct spidev_data *spidev, size_t len)
{
	struct spi_transfer	t = {
			.tx_buf		= spidev->buffer,
			.len		= len,
		};
	struct spi_message	m;

	spi_message_init(&m);
	spi_message_add_tail(&t, &m);
	return spidev_sync(spidev, &m);
}

static inline ssize_t
spidev_sync_read(struct spidev_data *spidev, size_t len)
{
	struct spi_transfer	t = {
			.rx_buf		= spidev->buffer,
			.len		= len,
		};
	struct spi_message	m;

	spi_message_init(&m);
	spi_message_add_tail(&t, &m);
	return spidev_sync(spidev, &m);
}

/*-------------------------------------------------------------------------*/

#endif





#if 1
static int gs2971_read_test(struct spi_device *spi, u8 addr[], u8 rxbuf[])
{
	int	status;
	struct spi_transfer t[2];
	struct spi_message m;
	uint8_t command[2];
	command[0] = addr[0];
	command[1] = (addr[1] & 0x0f) | 0x80;

	spi_message_init(&m);
	memset(t, 0, (sizeof t));
	
	
#if 0	
	u8 txbuf0[2] = {0x05, 0x80};
	u8 txbuf1[2] = {0x00, 0x00};
	u8 rxbuf0[2] = {0xff, 0xff};
	u8 rxbuf1[2] = {0xff, 0xff};

	u8 rxbuf2[2] = {0xff, 0xff};


	t[0].tx_buf = txbuf0;
	//t[0].rx_buf = rxbuf0;
	t[0].len = 2;
	t[0].bits_per_word = 16;
	t[0].delay_usecs = 10;
	spi_message_add_tail(&t[0], &m);
	

	//t[1].tx_buf = txbuf1;
	t[1].rx_buf = rxbuf1;
	t[1].len = 2;
	t[1].bits_per_word = 16;
	t[1].delay_usecs = 10;
	spi_message_add_tail(&t[1], &m);
#endif

	t[0].tx_buf = command;
	//t[0].rx_buf = rxbuf0;
	t[0].len = 2;
	t[0].bits_per_word = 16;
	t[0].delay_usecs = 10;
	spi_message_add_tail(&t[0], &m);
	

	//t[1].tx_buf = txbuf1;
	t[1].rx_buf = rxbuf;
	t[1].len = 2;
	t[1].bits_per_word = 16;
	t[1].delay_usecs = 10;
	spi_message_add_tail(&t[1], &m);

	status = spi_sync(spi, &m);
	
	
	printk("<0>""the rxbuf value is: %x,%x\n", rxbuf[1], rxbuf[0]);

	return status;


}

static int gs2971_write_test(struct spi_device *spi, u8 addr[], u8 txbuf[])
{
	int	status;
	struct spi_transfer t[2];
	struct spi_message m;
	uint8_t command[2];
	command[0] = addr[0];
	command[1] = (addr[1] & 0x0f) | 0x00;
	
	spi_message_init(&m);
	memset(t, 0, (sizeof t));
	

	t[0].tx_buf = command;
	//t[0].rx_buf = rxbuf0;
	t[0].len = 2;
	t[0].bits_per_word = 16;
	t[0].delay_usecs = 10;
	spi_message_add_tail(&t[0], &m);

	t[1].tx_buf = txbuf;
	//t[1].rx_buf = rxbuf1;
	t[1].len = 2;
	t[1].bits_per_word = 16;
	t[1].delay_usecs = 10;
	spi_message_add_tail(&t[1], &m);
	
	status = spi_sync(spi, &m);

	return status;

}



/*
static unsigned int gs2971_read(struct snd_soc_codec *codec, unsigned int reg)
{

   return 0;

}
static int gs2971_write(struct snd_soc_codec *codec, unsigned int reg, unsigned value)
{

   return 0;
}

static int audio_data_format_DS(void)
{
	//1. 首先判断是否locked.
	
	return 0;
}
*/
#endif

#if 0
//I will perfect it later.
static int gs2971_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params,
	struct snd_soc_dai *dai)
{
    /* 根据params的值,设置GS2971A的寄存器 
     * 比如时钟设置,格式,一定需要spi相关的读写函数*
     */
     
#if 1 //If video format: HD and 3G Audio.
	//为了测试，先将gs2971A: 200h :  0xa f4
	uint8_t addr_200[2] = {0x00, 0x2};
	uint8_t addr_201[2] = {0x01, 0x2};
	uint8_t addr_202[2] = {0x02, 0x2};
	uint8_t trdata[2] = {0xf4, 0xa};  //set word length 16bit
	//uint8_t trdata[2] = {0xf4, 0x0}; 
	uint8_t rcdata[2] = {0x0, 0x0};
	gs2971_write_test(gs2971Spi, addr_200, trdata);
	gs2971_read_test(gs2971Spi, addr_200, rcdata);
	gs2971_read_test(gs2971Spi, addr_201, rcdata);
	gs2971_read_test(gs2971Spi, addr_202, rcdata);
#endif	
    return 0;
}
#endif
//modify 2015-4-20
static int gs2971_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params,
	struct snd_soc_dai *dai)
{
    /* 根据params的值,设置GS2971A的寄存器 
     * 比如时钟设置,格式,一定需要spi相关的读写函数*
     */
  
#if 0 //If video format: HD and 3G Audio.
	//为了测试，先将gs2971A: 200h :  0xa f4
	uint8_t addr_200[2] = {0x00, 0x2};
	uint8_t addr_201[2] = {0x01, 0x2};
	uint8_t addr_202[2] = {0x02, 0x2};
	uint8_t trdata[2] = {0xf4, 0xa};  //set word length 16bit
	//uint8_t trdata[2] = {0xf4, 0x0}; 
	uint8_t rcdata[2] = {0x0, 0x0};
	gs2971_write_test(gs2971Spi, addr_200, trdata);
	gs2971_read_test(gs2971Spi, addr_200, rcdata);
	gs2971_read_test(gs2971Spi, addr_201, rcdata);
	gs2971_read_test(gs2971Spi, addr_202, rcdata);
#endif	
	uint8_t addr_200[2] = {0x00, 0x2};
	uint8_t addr_22[2] = {0x22, 0x0};
	uint8_t addr_408[2] = {0x08, 0x4};
	uint8_t rcdata[2] = {0x0, 0x0};
	uint8_t addr_201[2] = {0x01, 0x2};
	uint8_t addr_202[2] = {0x02, 0x2};

	uint8_t trdata_HD3G[2] = {0xf4, 0xa};  //set word length 16bit
	
	
	uint8_t trdata_SD[2] = {0xff, 0xaa};  //set word length 16bit
	
	gs2971_read_test(gs2971Spi, addr_22, rcdata);
	switch (params_format(params)) {
	case SNDRV_PCM_FORMAT_S16_LE:
		break;
	case SNDRV_PCM_FORMAT_S20_3LE:
		trdata_HD3G[1] = 0x5;
		trdata_SD[1] = 0x55;
		break;
	case SNDRV_PCM_FORMAT_S24_LE:
		trdata_HD3G[1] = 0x0;
		trdata_SD[1] = 0x0;
		break;
	default:
		break;
	}
	
	if( ((rcdata[2] >> 6) == 0) || ((rcdata[2] >> 6) == 2) )
	{
		gs2971_write_test(gs2971Spi, addr_200, trdata_HD3G);
		gs2971_read_test(gs2971Spi, addr_200, rcdata);
	}
	else
	{
		gs2971_write_test(gs2971Spi, addr_408, trdata_SD);
	}

	//for test
	gs2971_read_test(gs2971Spi, addr_201, rcdata);
	gs2971_read_test(gs2971Spi, addr_202, rcdata);
    return 0;
}


static const struct snd_soc_dai_ops gs2971_dai_ops = {
	.hw_params	= gs2971_hw_params,
};


static struct snd_soc_dai_driver gs2971a_dai = {
	.name = "gs2971a-aout",
#if 0
	.playback = {
		.stream_name = "Playback",
		.channels_min = 1,
		.channels_max = 2,
		.rates = AIC3X_RATES,
		.formats = AIC3X_FORMATS,},
#endif
	.capture = {
		.stream_name = "Capture",
		.channels_min = 2,
		.channels_max = 2,
		.rates = GS2971_RATES,
		.formats = GS2971_FORMATS,},
	.ops = &gs2971_dai_ops,
	//.symmetric_rates = 1,
};


static int gs2971a_probe(struct snd_soc_codec *codec)
{
    //gs2971_init_regs(codec);
    /* make user if need init gs2971a */
    return 0;
}


static struct snd_soc_codec_driver soc_codec_dev_gs2971a = {
	//.set_bias_level = aic3x_set_bias_level,
	.reg_cache_size = ARRAY_SIZE(gs2971_reg),
	.reg_word_size = sizeof(u8),
	.reg_cache_default = gs2971_reg,
	.probe = gs2971a_probe,
	//.remove = aic3x_remove,
	//.suspend = aic3x_suspend,
	//.resume = aic3x_resume,
	//.read =
	//.write =
};

static int gs2971a_codec_probe(struct platform_device *pdev)
{
	int ret;

	printk("<0>""gs2971a_codec_probe() is called!\n");
	
	ret = snd_soc_register_codec(&pdev->dev,
			&soc_codec_dev_gs2971a, &gs2971a_dai, 1);

	return ret;
}
static int gs2971a_codec_remove(struct platform_device *pdev)
{
	snd_soc_unregister_codec(&pdev->dev);
    return 0;
}

static void gs2971a_codec_release(struct device * dev)
{
}

static struct platform_device gs2971a_codec_dev = {
    .name         = "gs2971a-codec",
    .id       = -1,
    .dev = { 
    	.release = gs2971a_codec_release, 
	},
};
struct platform_driver gs2971a_codec_drv = {
	.probe		= gs2971a_codec_probe,
	.remove		= gs2971a_codec_remove,
	.driver		= {
		.name	= "gs2971a-codec",
	}
};

static int __devinit gs2971_probe(struct spi_device *spi)
{
	int ret = 0;
	
	
	printk("<0>""gs2971_probe() is called!\n");
	
	platform_device_register(&gs2971a_codec_dev);
    platform_driver_register(&gs2971a_codec_drv);

	gs2971Spi = spi;

	return ret;
}


static int __devexit gs2971_remove(struct spi_device *spi)
{
	printk("<0>""gs2971_remove() is called!\n");

	platform_device_unregister(&gs2971a_codec_dev);
    platform_driver_unregister(&gs2971a_codec_drv);
	
	return 0;
}


static struct spi_driver gs2971_driver = {
	.driver = {
		.name	= "gs2971",
		.bus	= &spi_bus_type,
		.owner	= THIS_MODULE,
	},
	//.id_table	= gs2971,
	.probe	= gs2971_probe,
	.remove	= __devexit_p(gs2971_remove),

	/* REVISIT: many of these chips have deep power-down modes, which
	 * should clearly be entered on suspend() to minimize power use.
	 * And also when they're otherwise idle...
	 */
};

static int __init gs2971A_init(void)
{
	return spi_register_driver(&gs2971_driver);
}

static void __exit gs2971A_exit(void)
{
	spi_unregister_driver(&gs2971_driver);
}


module_init(gs2971A_init);
module_exit(gs2971A_exit);

MODULE_DESCRIPTION("ASoC Gs2971a spi codec driver");
MODULE_AUTHOR("Bo Wen");
MODULE_LICENSE("GPL");



