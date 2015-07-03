/*
 *
 * Copyright (C) 2006 Texas Instruments.
 *
 * ----------------------------------------------------------------------------
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 * ----------------------------------------------------------------------------
 *
 */

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


#include <linux/interrupt.h>
#include <linux/cdev.h>
#include <linux/gpio.h>
#include <mach/io.h>
#include <mach/hardware.h>
#include <linux/clk.h>


#define MODULE_NAME      "davinci_led"

#define GPIO_LED0  (32*1+14)   
#define GPIO_LED1  (32*1+13)   
#define GPIO_LED2  (32*2+6)   
#define GPIO_LED3  (32*2+5)   




#define CMD_LED_NETWORK		0x1  //led2
#define CMD_LED_REGISTER	0x22 //led1
#define CMD_LED_CALLING		0x3  //led0

#define MAJOR_NUM	312
/*-----------------------------------------------------------------------------------------------*/

/*
 * This function performs hardware setup of SPI. The hardware
 * setup structure is populated by the user and passed to this 
 * function. If hardware setup structure is not passed, SPI will
 * be initialized with the default hardware setup structure.
 */

static int led_open(struct inode *inode, struct file *filp);
static int led_release (struct inode *inode, struct file *filp);
static int led_ioctl ( struct file *file, unsigned int cmd, unsigned long arg);
static ssize_t led_read (struct file *filp, char *buf, size_t count, loff_t * l);
static ssize_t led_write (struct file *filp, const char *buf, size_t count, loff_t * l);

struct file_operations led_fops = {
	.owner = THIS_MODULE,
	.open = led_open,
	.read = led_read,
	.write = led_write,
	.release = led_release,
	.unlocked_ioctl = led_ioctl,
};

static int led_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static int led_release (struct inode *inode, struct file *filp)
{
	return 0;
}


static ssize_t led_write (struct file *filp, const char *buf, size_t count, loff_t * l)
{
	return 0;
}

static ssize_t led_read (struct file *filp, char *buf, size_t count, loff_t * l)
{
	return 0;
}

static int led_ioctl ( struct file *file, unsigned int cmd, unsigned long arg)
{
	int led_enable = 0;
	int led_num = -1;

	if(copy_from_user(&led_enable, (const char*)arg, sizeof(unsigned int)) != 0)
	{
		
		printk("copy_from_user error in led\n");
		return -1;
	}
			
	switch(cmd)
	{
		
		case CMD_LED_NETWORK:
		{
			led_num = GPIO_LED2;
			break;
		}
		
		case CMD_LED_REGISTER:
		{
			led_num = GPIO_LED1;
			break;
		}
	
		case CMD_LED_CALLING:
		{
			led_num = GPIO_LED0;
			break;
		}
		
		default:
			break;
	}
	
	if(led_enable)
	{
		gpio_direction_output(led_num, 1);

	}
	else
	{
		gpio_direction_output(led_num, 0);

	}

	return 0;
}

static int led_control_init(void)
{
	//PINMUX4 set GPIO30,GPIO31,GPIO32,GPIO33 function
	int result;

	result = register_chrdev (MAJOR_NUM, MODULE_NAME, &led_fops);

	if (result)
	{
		printk ("uart2_dev : can't get major number \n");
		return result;
	}


        gpio_request(GPIO_LED2, "LED1");
        gpio_direction_output(GPIO_LED2, 1);
                                                                                                               
        gpio_request(GPIO_LED1, "LED2");
        gpio_direction_output(GPIO_LED1, 1);
                                                                                                               
        gpio_request(GPIO_LED0, "LED3");
        gpio_direction_output(GPIO_LED0, 1);
	return 0;
}

static void led_control_exit(void)
{
	unregister_chrdev (MAJOR_NUM, MODULE_NAME);
	return 0;
}

MODULE_AUTHOR("Texas Instruments");
MODULE_DESCRIPTION("DaVinci LED Control");
MODULE_LICENSE("GPL");

module_init(led_control_init);
module_exit(led_control_exit);

