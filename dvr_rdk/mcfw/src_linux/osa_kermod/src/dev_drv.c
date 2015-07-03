
#include <dev_dma.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>           /* everything... */
#include <linux/cdev.h>
#include <linux/mm.h>
#include <linux/kernel.h>       /* printk() */
#include <linux/slab.h>         /* kmalloc() */
#include <asm/uaccess.h>        /* copy_*_user */

#include <linux/io.h>


int DRV_devInit(void)
{
  int     result;

  result = DMA_devInit();
  if(result!=0) {
    return result;
  }

  printk(KERN_INFO "DRV: Module install successful\n");
  printk(KERN_INFO "DRV: Module built on %s %s \n", __DATE__, __TIME__);

  return result;
}

void DRV_devExit(void)
{
  DMA_devExit();
}

module_init(DRV_devInit);
module_exit(DRV_devExit);

MODULE_AUTHOR("Texas Instruments");
MODULE_LICENSE("GPL");
