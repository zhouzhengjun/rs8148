/*
 * gpio_en.c
 *
 * Kernel module to enable Bluetooth core on WL1271
 *
 * Copyright(c) 2010 Texas Instruments. All rights reserved.      
 * All rights reserved.                                                  
 *                                                                       
 * Redistribution and use in source and binary forms, with or without    
 * modification, are permitted provided that the following conditions    
 * are met:                                                              
 *                                                                       
 *  * Redistributions of source code must retain the above copyright     
 *    notice, this list of conditions and the following disclaimer.      
 *  * Redistributions in binary form must reproduce the above copyright  
 *    notice, this list of conditions and the following disclaimer in    
 *    the documentation and/or other materials provided with the         
 *    distribution.                                                      
 *  * Neither the name Texas Instruments nor the names of its            
 *    contributors may be used to endorse or promote products derived    
 *    from this software without specific prior written permission.      
 *                                                                       
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS   
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT     
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT  
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT      
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT   
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/delay.h>
#include <linux/gpio.h>

/* Convert GPIO signal to GPIO pin number */
#define GPIO_TO_PIN(bank, gpio) (32 * (bank) + (gpio))
#define GPIO_BT_EN   ((1 * 32) + 8)
#define GPIO_BT_TEST   ((1 * 32) + 10)


/* 
   Define wl12xx_platform_data prototype here for using with first kernel versions
   that didn't pass the enable pins elements 
*/
struct wl12xx_platform_data {
	void (*set_power)(bool enable);
	/* SDIO only: IRQ number if WLAN_IRQ line is used, 0 for SDIO IRQs */
	int irq;
	bool use_eeprom;
	int board_ref_clock;
	int board_tcxo_clock;
	unsigned long platform_quirks;
	int bt_enable_gpio;
	int wlan_enable_gpio;
};

const struct wl12xx_platform_data *wl12xx_get_platform_data(void);

unsigned char bt_enable_pin;

static int __init gpio_test_init(void)
{
	const struct wl12xx_platform_data *wlan_data;
	wlan_data = wl12xx_get_platform_data();

	bt_enable_pin = wlan_data->bt_enable_gpio;

	/* Default to alpha EVM in case the element is not initialized */
	if (! bt_enable_pin)
	{
		pr_info("BT Enable pin is not initialized, defaulting to EVM Rev 1.0A.\n");
		bt_enable_pin = GPIO_TO_PIN(1, 8); 
	}

	printk("Gpio value is :%d\n", bt_enable_pin);
	gpio_set_value(bt_enable_pin, 0);
//	gpio_set_value(GPIO_BT_EN, 0);
	gpio_set_value(GPIO_BT_TEST, 0);
	msleep(1);
	printk("WL1271: BT Enable\n");
#if 0
	gpio_set_value(GPIO_BT_EN, 1);
	gpio_set_value(GPIO_BT_TEST,1 );
	gpio_set_value(bt_enable_pin, 1);
	msleep(10);
	gpio_set_value(GPIO_BT_EN, 0);
	msleep(1);
	gpio_set_value(GPIO_BT_EN, 1);
#endif 
	 gpio_set_value(GPIO_BT_EN, 1);
	return 0;
}

static void __exit gpio_test_exit(void)
{
	printk("WL1271: BT Disable\n");
	gpio_set_value(GPIO_BT_EN, 0);
        gpio_set_value(bt_enable_pin, 0);
}



module_init(gpio_test_init);
module_exit(gpio_test_exit);
MODULE_LICENSE("GPL");
