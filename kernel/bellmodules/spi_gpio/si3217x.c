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

#include "spi_davinci.h"                                                                                                 
#include "si3217x.h"
#include "spi_drv_api.h"
#include "si3217x_irq.h"


//#define DAVINCI_GPIO_BASE     0x01c67000
//#define DAVINCI_GPIO_BASE_ADDR   IO_ADDRESS(DAVINCI_SPI_BASE)

#define SAMPLES_PER_POINT    9
// With 10-bit Conversion, keeping the tenth most significant bits.
#define MAX_ADC_VAL          0x3ff
#define MAXERROR             0x50
#define X_AXIS               0xd1
#define Y_AXIS               0x91
// Flag to indicate if the tsd_GetRawPoints succeeded or not.
#define SUCCESSFUL        1
#define NOT_SUCCESSFUL    0
#define SAMPLE_VALID      3
static void          *spi_handle;
//static int g_valid=0;

//#define PRINT_TSD_INFO
#ifdef PRINT_TSD_INFO
#define tsd_info(format, arg...)  printk(KERN_INFO MOD_NAME ": " format "\n",  ## arg )
#else
#define tsd_info(format, arg...)  NULL
#endif


#define MOD_NAME    "si3217x"
       
MODULE_AUTHOR("Texas Instruments");
MODULE_DESCRIPTION("SI3217X driver");
MODULE_LICENSE("GPL");
                                                                                                 
static int    gMajor = 0;
static int     si3217x_open   (struct inode *inode, struct file *filp);
static int     si3217x_release(struct inode *inode, struct file *filp);
static ssize_t si3217x_read   (struct file *filp, char *buf, size_t count,loff_t * l);
static ssize_t si3217x_write  (struct file *filp, const char *, size_t, loff_t * l);
static long si3217x_ioctl(struct file *filp, unsigned int cmd,unsigned long arg);
extern void reset_si3217x(void);

struct file_operations touch_fops = {
    .owner   = THIS_MODULE,
    .open    = si3217x_open,
    .read    = si3217x_read,
    .write   = si3217x_write,
    .release = si3217x_release,
    .unlocked_ioctl   = si3217x_ioctl,
};

static wait_queue_head_t gpio_wait;
#if 0
static void enable_gpio_interrupt(void)
{
    //*DAVINCI_GPIO_SET_FAL_TRIG |= (1 << 6);
    //*DAVINCI_GPIO_BINTEN |= 0x7;
    mdelay ( DRV_GPIO_DELAY );
}
#endif
static void disable_gpio_interrupt(void)
{
   // *DAVINCI_GPIO_BINTEN &= 0;
    //*DAVINCI_GPIO_INSTAT &= 0;
    //*DAVINCI_GPIO_CLR_FAL_TRIG &= 0; //|=(1<<6);
    mdelay ( DRV_GPIO_DELAY );
}

static int si3217x_open (struct inode *inode, struct file *filp)
{
    GPIO_Hardware_Init();
    //init_waitqueue_head(&gpio_wait)£»
#if defined(CONFIG_SPI_DEBUG)
		printk("--->>>%s()\n",__FUNCTION__);
#endif

    return 0;
}

static ssize_t si3217x_read (struct file *filp, char *buf, size_t count, loff_t * l)
{
	SPI_ReadSync8(spi_handle, buf, count, 0);
    
    //*DAVINCI_GPIO_INSTAT &= 0;
//    enable_gpio_interrupt();
//    si3217x_GetSamples(spi_handle,&g_ScreenPoint);

//    interruptible_sleep_on(&gpio_wait);
//    memcpy(buf,&g_ScreenPoint,sizeof(TSD_POINT_T));
//    disable_gpio_interrupt();
    //interruptible_sleep_on_timeout(&gpio_wait,40);	

#if defined(CONFIG_SPI_DEBUG)
	//printk("->%s(0x%x,%d)\n",__FUNCTION__,(unsigned int)buf,count);
#endif

#if defined(CONFIG_SPI_DEBUG)
	//printk("->%s():0x%x\n",__FUNCTION__,(unsigned int)buf[0]);
#endif
    return count;
 
}

static ssize_t si3217x_write (struct file *filp, const char *buf, size_t count, loff_t * l)
{
    //int i;
	
    SPI_WriteSync8(spi_handle, (u8 *)buf, count, 0);
#if defined(CONFIG_SPI_DEBUG)
	//printk("->%s(0x%x)\n",__FUNCTION__,buf[0]);
#endif

    return 0;
}

static long si3217x_ioctl(struct file *filp,unsigned int cmd,unsigned long arg)
{

#if defined(CONFIG_SPI_DEBUG)
	printk("--->>>%s(%d,%d)\n",__FUNCTION__,cmd,(unsigned int)arg);
#endif

    switch(cmd)
    {
	case SI3217X_RESET:
		//do{
		SPI_ResetSi3217x();
		//}while(1);
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

static int si3217x_init(void)
{
    int result;
	
#if defined(CONFIG_SPI_DEBUG)	
    printk("--->>>%s()\n",__FUNCTION__);	
#endif

    spi_client_params_t param = {NULL,0};

    GPIO_Hardware_Init();
                                                                                                 
    if ((spi_handle = SPI_Open(&param)) == NULL)
    {
        printk("SPI_Open() FAILD !!\n");
        return 0;
    }

    result = register_chrdev(303, MOD_NAME, &touch_fops);
    
    if (result)
    {
        printk ("touch_dev : can't get major number \n");
        return result;
    }
    
    printk ("make node for touch with 'mknod /dev/si3217x c 303 %d '\n", result);
    if (gMajor == 0)    gMajor = 303;

    touch_irq_init();
    init_waitqueue_head(&gpio_wait);
    return 0;
}

static void si3217x_exit(void)
{

#if defined(CONFIG_SPI_DEBUG)
    printk("<<<---%s()\n",__FUNCTION__);
#endif

    touch_irq_release();
    unregister_chrdev (gMajor, MOD_NAME);
}

void GPIO_Hardware_Init( void )
{
    /* Set falling edge detection on GPIO bank 0 */
    //*DAVINCI_GPIO_SET_FAL_TRIG |= (1 << 6);

    /* Enable bank 0 interrupts */
//    *DAVINCI_GPIO_BINTEN |= 0x7;
    /* Enable bank 2 interrupts */
    /* Delay to make sure GPIOs are stable, 1ms */
    mdelay ( DRV_GPIO_DELAY );
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
    int result;
    result = request_irq (DAVINCI_GPIO_IRQ,(irq_handler_t)touch_interrupt,IRQF_DISABLED,DEV_IRQ_NAME,DEV_IRQ_ID);
    
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

