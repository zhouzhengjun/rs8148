#include <linux/fs.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/unistd.h>
#include <linux/vmalloc.h>
#include <linux/version.h>
#include <linux/slab.h>
#include <linux/proc_fs.h>
#include <linux/ctype.h>
#include <linux/delay.h>
#include <linux/moduleparam.h>
#include <asm/uaccess.h>
#include <asm/pgtable.h>

#include <linux/err.h>
#include <linux/errno.h>
#include <linux/device.h>
#include <linux/mutex.h>
#include <linux/math64.h>
#include <linux/sched.h>
#include <linux/mod_devicetable.h>

#include <linux/spi/spi.h>
#include <linux/spi/slic_spi.h>


//#include "spi_davinci.h"                                                                                                 
#include "si3217x_spi.h"
//#include "spi_drv_api.h"
#include "si3217x_irq.h"


// Flag to indicate if the tsd_GetRawPoints succeeded or not.
#define SUCCESSFUL        1
#define NOT_SUCCESSFUL    0
#define SAMPLE_VALID      3
//static void          *spi_handle;
//static int g_valid=0;

//#define PRINT_TSD_INFO
#ifdef PRINT_TSD_INFO
#define tsd_info(format, arg...)  printk(KERN_INFO MOD_NAME ": " format "\n",  ## arg )
#else
#define tsd_info(format, arg...)  NULL
#endif


#define MOD_NAME    "si3217x"
       
MODULE_AUTHOR("BroadEng Bell");
MODULE_DESCRIPTION("SI3217X SPI driver");
MODULE_LICENSE("GPL");
                                                                                                 
static int    gMajor = 0;
static int     si3217x_open   (struct inode *inode, struct file *filp);
static int     si3217x_release(struct inode *inode, struct file *filp);
static ssize_t si3217x_read   (struct file *filp, char *buf, size_t count,loff_t * l);
static ssize_t si3217x_write  (struct file *filp, const char *, size_t, loff_t * l);
static long si3217x_ioctl(struct file *filp, unsigned int cmd,unsigned long arg);
extern void reset_si3217x(void);

static struct spi_device * spi_si32171;
struct si3217x_state {
	u8				*tx;
	u8				*rx;
};

static struct si3217x_state ss;

static struct si3217x_state *st = &ss;

struct file_operations touch_fops = {
    .owner   = THIS_MODULE,
    .open    = si3217x_open,
    .read    = si3217x_read,
    .write   = si3217x_write,
    .release = si3217x_release,
    .unlocked_ioctl   = si3217x_ioctl,
    //.compat_ioctl   = si3217x_ioctl,
};

//static wait_queue_head_t gpio_wait;
//static void enable_gpio_interrupt(void)
//{
    //*DAVINCI_GPIO_SET_FAL_TRIG |= (1 << 6);
    //*DAVINCI_GPIO_BINTEN |= 0x7;
    //mdelay ( DRV_GPIO_DELAY );
//}

static void disable_gpio_interrupt(void)
{
    //*DAVINCI_GPIO_BINTEN &= 0;
    //*DAVINCI_GPIO_INSTAT &= 0;
    //*DAVINCI_GPIO_CLR_FAL_TRIG &= 0; //|=(1<<6);
    //mdelay ( DRV_GPIO_DELAY );
}

static int si3217x_open (struct inode *inode, struct file *filp)
{
    //GPIO_Hardware_Init();
    //init_waitqueue_head(&gpio_wait)£

#if defined(CONFIG_SPI_DEBUG)
		printk("--->>>%s()\n",__FUNCTION__);
#endif

    return 0;
}

static ssize_t si3217x_read (struct file *filp, char *buf, size_t count, loff_t * l)
{
	long ret=0;
		
//	SPI_ReadSync8(spi_handle, buf, count, 0);
    
    //*DAVINCI_GPIO_INSTAT &= 0;
//    enable_gpio_interrupt();
//    si3217x_GetSamples(spi_handle,&g_ScreenPoint);

//    interruptible_sleep_on(&gpio_wait);
//    memcpy(buf,&g_ScreenPoint,sizeof(TSD_POINT_T));
//    disable_gpio_interrupt();
    //interruptible_sleep_on_timeout(&gpio_wait,40);	

#if defined(CONFIG_SPI_DEBUG)
	printk("->%s(0x%x,%d)\n",__FUNCTION__,(unsigned int)buf,count);
#endif

	spi_read(spi_si32171,st->rx,count);

	ret=copy_to_user(buf, st->rx,count);

#if defined(CONFIG_SPI_DEBUG)
	printk("->%s():0x%x\n",__FUNCTION__,(unsigned int)buf[0]);
#endif

	return count;
 
}

static ssize_t si3217x_write (struct file *filp, const char *buf, size_t count, loff_t * l)
{
	long ret=0;
    //int i;
    //SPI_WriteSync8(spi_handle, buf, count, 0);

#if defined(CONFIG_SPI_DEBUG)
	//printk("--->>>%s(0x%x:0x%x,%d)\n",__FUNCTION__,buf,buf[0],count);
#endif

	ret=copy_from_user(st->tx, buf, count);
	//do{
		spi_write(spi_si32171,st->tx,count);	
	//}while(1);
	return 0;
}

void SPI_ResetSi3217x(void)
{

#if defined(CONFIG_SPI_DEBUG)
	printk("--->>>%s()\n",__FUNCTION__);
#endif

}

static long si3217x_ioctl(struct file *filp,unsigned int cmd,unsigned long arg)
{

#if defined(CONFIG_SPI_DEBUG)
	printk("--->>>%s(%d,%d)\n",__FUNCTION__,cmd,(unsigned int)arg);
#endif

    switch(cmd)
    {
	case SI3217X_RESET:
		SPI_ResetSi3217x();
		break;
	default:
		break;
    }

    return 0;
}

static int si3217x_release (struct inode *inode, struct file *filp)
{
    printk ("touch release\n");
    return 0;
}

#define SI3217X_MAX_TX 12
#define SI3217X_MAX_RX 12


static int __devinit si3217x_spi_probe(struct spi_device *spi)
{
	int ret = 0;
	const struct spi_device_id	*id = NULL;

#if defined(CONFIG_SPI_DEBUG)
	printk("--->>>%s()\n",__FUNCTION__);
#endif

	id = spi_get_device_id(spi);
	
	dev_info(&spi->dev, "[%s]\n", id->name);
	
	spi_si32171=spi;

	/* Allocate the comms buffers */
	st->rx = kzalloc(sizeof(*st->rx)*SI3217X_MAX_RX, GFP_KERNEL);
	if (st->rx == NULL) {
		ret = -ENOMEM;
		goto error;
	}
	st->tx = kzalloc(sizeof(*st->tx)*SI3217X_MAX_TX, GFP_KERNEL);
	if (st->tx == NULL) {
		ret = -ENOMEM;
		goto error;
	}

error:
	return ret;
}

static int __devexit si3217x_spi_remove(struct spi_device *spi)
{

#if defined(CONFIG_SPI_DEBUG)
	printk("<<<---%s()\n",__FUNCTION__);
#endif

	kfree(st->tx);
	kfree(st->rx);
	
	return 0;
}

static const struct spi_device_id si3217x_ids[] = {
	{ "si3217x-spi",  0 },
	{ },	
};


static struct spi_driver si32171_spi_driver = {
	.driver = {
		.name	= "si3217x-codec",
		.bus	= &spi_bus_type,
		.owner	= THIS_MODULE,
	},
	.id_table	= si3217x_ids,
	.probe	= si3217x_spi_probe,
	.remove	= __devexit_p(si3217x_spi_remove),

};

static int si3217x_init(void)
{
    int result;
	
#if defined(CONFIG_SPI_DEBUG)	
    printk("--->>>%s()\n",__FUNCTION__);	
#endif

 //   spi_client_params_t param = {NULL,0};

    //GPIO_Hardware_Init();
                                                                                                 
 //   if ((spi_handle = SPI_Open(&param)) == NULL)
 //   {
 //       printk("SPI_Open() FAILD !!\n");
 //       return 0;
 //   }

     spi_register_driver(&si32171_spi_driver);	
 

    result = register_chrdev(303, MOD_NAME, &touch_fops);
    
    if (result)
    {
        printk ("touch_dev : can't get major number \n");
        return result;
    }

#if defined(CONFIG_SPI_DEBUG)	
    printk ("make node for touch with 'mknod /dev/si3217x c 303 %d '\n", result);
#endif

    if (gMajor == 0)    gMajor = 303;

    //touch_irq_init();
    //init_waitqueue_head(&gpio_wait);

  
    return 0;
}

static void si3217x_exit(void)
{

#if defined(CONFIG_SPI_DEBUG)
    printk("<<<---%s()\n",__FUNCTION__);
#endif

    //touch_irq_release();
    spi_unregister_driver(&si32171_spi_driver);
    unregister_chrdev (gMajor, MOD_NAME);
}

void touch_interrupt (int irq, void *dev_id, struct pt_regs *regs)
{
//    if (*DAVINCI_GPIO_INSTAT & (0x1<<TNETW_GPIO_IRQ))
//    {
//        *DAVINCI_GPIO_CLR_FAL_TRIG |=(1<<6);
//        *DAVINCI_GPIO_BINTEN &= 0;
	disable_gpio_interrupt();
//	if(si3217x_GetSamples(spi_handle,&g_ScreenPoint))
//	{
//		g_valid = 1;
//    		wake_up(&gpio_wait);	
//	}
//	else
//	{
//		enable_gpio_interrupt();
//		g_valid = 0;
//	}
        //else
	//	*DAVINCI_GPIO_BINTEN |= 0x7;           
//    }
}
/*--------------------------------------------------------------------------------------*/
int touch_irq_init(void)
{
    int result=0;
    //result = request_irq (DAVINCI_GPIO_IRQ,touch_interrupt,IRQF_DISABLED,DEV_IRQ_NAME,DEV_IRQ_ID);
    
    if (result)
    {
        printk ("<1>cannot init irq\n");
        return -1;
    }
    
    return 0;
}
                                                                                                               
/*--------------------------------------------------------------------------------------*/
void touch_irq_release(void)
{
    free_irq (DAVINCI_GPIO_IRQ, DEV_IRQ_ID);
    disable_irq (DAVINCI_GPIO_IRQ);
    printk(KERN_INFO "Release interrupt 58\n");
}

module_init(si3217x_init);
module_exit(si3217x_exit);

