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

#include "spi_davinci.h"

#include <linux/module.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <mach/io.h>
//#include <mach/cputype.h>
#include <mach/hardware.h>
#include "spi_davinci.h"
#include <linux/clk.h>
//#include <mach/dm365.h>
//#include <mach/mux.h>

#define MODULE_NAME      "DM8148 RAW GPIO SPI"
//#define DM365_GPIO33      33      // GPIO33  CS
//#define DM365_GPIO32      32      // GPIO32  PCLK
//#define DM365_GPIO31     31      // GPIO31   MISO
//#define DM365_GPIO30     30      // GPIO30   MOSI

//#define DM365_GPIO24     24      // GPIO24   INTTERUPT
//#define DM365_GPIO29     29      //GPIO29    RESET
//#define DM365_GPIO93     93      //GPIO93    RESET

#define GPIO_SPI1_SCS0	(32*1+16)	//nCS
#define GPIO_SPI1_SCLK	(32*1+17)	//SCLK
#define GPIO_SPI1_D1		(32*1+18)	//MISO
#define GPIO_SPI1_D0		(32*1+26)	//MOSI

#define GPIO_VOICE_INT	(32*0+31)	//GP0[31]
#define GPIO_RST_PERI	(32*1+ 8)	//W2: GP1[8] MUX1 
//#define GPIO_RST_PERI	(32*0+29)//	//U4: GP0[29] 	

#if 0
#define __REG(addr)   (*(volatile unsigned int *)IO_ADDRESS(addr))
#define REG_PINMUX4     __REG(0x01c40010)

#define BINTEN	IO_ADDRESS(0x01C67008)

//0-1 REGISTER 30---30, 31--31
//2-3 REGISTER 32---0,  33--1

//GPIO DIR Register
#define REGDIR0	IO_ADDRESS(0x01C67010)  //GPIO30,GPIO31
#define REGDIR2 IO_ADDRESS(0x01C67038)  //GPIO32,GPIO33

//GPIO SET DATA Register
#define REGOUT0 IO_ADDRESS(0x01C67014)  //GPIO30,GPIO31
#define REGOUT2 IO_ADDRESS(0x01C6703C)  //GPIO32,GPIO33

//GPIO SET DATA Register
#define REGSET0 IO_ADDRESS(0x01C67018)  //GPIO30,GPIO31
#define REGSET2 IO_ADDRESS(0x01C67040)  //GPIO32,GPIO33

//GPIO CLR DATA Register
#define REGCLR0 IO_ADDRESS(0x01C67030)  //GPIO30,GPIO31
#define REGCLR2 IO_ADDRESS(0x01C67044)  //GPIO32,GPIO33

//GPIO GET DATA Register
#define REGGET0 IO_ADDRESS(0x01C67020)  //GPIO30,GPIO31
#define REGGET2 IO_ADDRESS(0x01C67048)  //GPIO32,GPIO33
#endif
int spi_davinci_debug = 4;

static int g_delay=8;
static int g_halfdelay=2;
void spi_delay(int iCount);

/*
static unsigned int READREG (unsigned int r)
{
	return *((volatile unsigned int *) r);
}

static void WRITEREG (unsigned int r, unsigned int val)
{
	*((volatile unsigned int *) r) = val;
}

static void setreg (unsigned int regist, unsigned int flush, unsigned int set)
{
	unsigned int value;
	value = READREG (regist);
	value &= flush;
	value |= set;
	WRITEREG (regist, value);
}*/
/*-----------------------------------------------------------------------------------------------*/

/*
 * This function performs hardware setup of SPI. The hardware
 * setup structure is populated by the user and passed to this 
 * function. If hardware setup structure is not passed, SPI will
 * be initialized with the default hardware setup structure.
 */

static void spi_davinci_hw_init(void)
{
	int error;

#if 0

	//PINMUX4 set GPIO30,GPIO31,GPIO32,GPIO33 function
	u32 temp = REG_PINMUX4;
//        //Set GPIO29 function
        temp &=0xFFFFFFCF;	

	//Set GPIO30 function
	temp &=0xFFFFFF3F;

	//Set GPIO31 function
	temp &=0xFFFFFCFF;

	//Set GPIO32 function
	temp &=0xFFFFF3FF;
	
	//Set GPIO33 function
	temp &=0xFFFFCFFF;

	//Set GPIO29 funciton
	temp &=0xFFFFFFCF;
	
	REG_PINMUX4 = temp;

	//set GPIO input and output function
	//Set GPIO30 output
	setreg(REGDIR0,(~0x40000000),0x0);

	//Set GPIO31 input
	setreg(REGDIR0,(~0x0),0x80000000);

	//Set GPIO32 output
	setreg(REGDIR2,(~0x1),0x0);

	//Set GPIO33 output
	setreg(REGDIR2,(~0x2),0x0);

        //Set GPIO24 input
        setreg(REGDIR0,(~0x0),0x01000000);


//add for interrupt
	gpio_request(DM365_GPIO24, "INTERRUPT");
        gpio_direction_input(DM365_GPIO24);

#else

//add for reset
        error = gpio_request(GPIO_RST_PERI, "VOICE_RESET");
	if (error < 0) {
		printk(KERN_ERR "%s: failed to request GPIO1_8 for VOICE_RESET"
			": %d\n", __func__, error);
		//return;
	}

	gpio_direction_output(GPIO_RST_PERI, 0);
	//gpio_direction_input(GPIO_RST_PERI);
	spi_delay(50000);

	error = gpio_request(GPIO_VOICE_INT, "VOICE_INT");
	if (error < 0) {
		printk(KERN_ERR "%s: failed to request GPIO0_31 for VOICE_INT"
			": %d\n", __func__, error);
		//return;
	}

	gpio_direction_input(GPIO_VOICE_INT);
	//ti814x_i2c_boardinfo[6].irq = gpio_to_irq(GPIO_VOICE_INT);

	gpio_export(GPIO_VOICE_INT, true);

#endif
	//MOSI
        error = gpio_request(GPIO_SPI1_D0, "SPI1_MOSI");
	if (error < 0) {
		printk(KERN_ERR "%s: failed to request GPIO1_26 for SPI1_MOSI"
			": %d\n", __func__, error);
		//return;
	}

	gpio_direction_output(GPIO_SPI1_D0, 1);

        //MISO
        error = gpio_request(GPIO_SPI1_D1, "SPI1_MISO");
	if (error < 0) {
		printk(KERN_ERR "%s: failed to request GPIO1_18 for SPI1_MISO"
			": %d\n", __func__, error);
		//return;
	}

	gpio_direction_input(GPIO_SPI1_D1);


	//CS
        error = gpio_request(GPIO_SPI1_SCS0, "SPI1_SCS0");
	if (error < 0) {
		printk(KERN_ERR "%s: failed to request GPIO1_16 for SPI1_SCS0"
			": %d\n", __func__, error);
		//return;
	}

	gpio_direction_output(GPIO_SPI1_SCS0, 1);

	//MCLK
        error = gpio_request(GPIO_SPI1_SCLK, "SPI1_SCLK");
	if (error < 0) {
		printk(KERN_ERR "%s: failed to request GPIO1_17 for SPI1_SCLK"
			": %d\n", __func__, error);
		//return;
	}

	gpio_direction_output(GPIO_SPI1_SCLK, 1);

	reset_si3217x();
	//printk("the dir is %x,%x\n",READREG(REGDIR0),READREG(REGDIR2));
	//printk("the pinmux4 is 0x%x.\n",REG_PINMUX4);


} /* spi_davinci_hw_init() */

/*-----------------------------------------------------------------------------------------------*/

void reset_si3217x(void)
{

#if defined(CONFIG_SPI_DEBUG)	
	printk("--->>>%s()\n",__FUNCTION__);	
#endif
	//return;
        gpio_set_value(GPIO_RST_PERI, 0);
        spi_delay(2000000);
        gpio_set_value(GPIO_RST_PERI, 1);
	spi_delay(2000000);
}

#define nsdelay(n)							\
	(__builtin_constant_p(n) ?					\
	  ((n) > (MAX_UDELAY_MS * 1000*1000) ? __bad_udelay() :		\
			__const_udelay((n) * ((2199023U*HZ)>>11)/1000)) :	\
	  __udelay(n/1000))
	  

void spi_delay(int iCount)
{
    int i;
    for(i=0;i<iCount;i++)
	//udelay(1); 
	nsdelay(50);
}
#if 0
//Set chip select -- low
//Set sclk high
//Set sdo high
void spi_enable()  
{  
	//chip is disable
	WRITEREG(REGSET2,(READREG(REGSET2) | 0x2));

	//clock is high
	WRITEREG(REGSET2,(READREG(REGSET2) | 0x1));

	
	//MOSI is high
	WRITEREG(REGSET0,(READREG(REGSET0) | 0x40000000));
	spi_delay(g_delay); 
	    
        WRITEREG(REGCLR2,(READREG(REGCLR2) | (0x2)));
	spi_delay(g_delay);  
}  
#endif
void spi_clk_start(void)
{
	int i;
        gpio_set_value(GPIO_SPI1_SCLK, 1);
        spi_delay(g_delay);
	spi_delay(g_delay);

	for(i=0;i<4;i++)
	{
		gpio_set_value(GPIO_SPI1_SCLK, 1);
		spi_delay(g_delay);  
		gpio_set_value(GPIO_SPI1_SCLK, 0);
		spi_delay(g_delay); 
	}
	
         
} /* spi_davinci_write_u16() */

void spi_clk_end(void)
{
        int i;

        for(i=0;i<4;i++)
        {
                gpio_set_value(GPIO_SPI1_SCLK, 1);
                spi_delay(g_delay);
                gpio_set_value(GPIO_SPI1_SCLK, 0);
                spi_delay(g_delay); 
        }
	gpio_set_value(GPIO_SPI1_SCLK, 1);
	spi_delay(g_delay);
	gpio_set_value(GPIO_SPI1_SCLK, 1);

} /* spi_davinci_write_u16() */
  

//Clear chip select -- high
void spi_disable(void)  
{  
	//chip is select
	//WRITEREG(REGSET2,(READREG(REGSET2) | 0x2));
	gpio_set_value(GPIO_SPI1_SCS0, 1);
	spi_delay(g_delay);  

}  
 
void spi_davinci_wait(struct spi_davinci_device *spi_davinci_dev, u32 val)
{
	spi_delay(val);
	return;

} /* spi_davinci_wait() */

int spi_davinci_write_u8_sync(void *spi_handle, u8 *buf, u32 nwrite)
{
 	int i = 0;  
	//u32 temp;
	u8 send_bit;
	u8 send_data = buf[0];  
	     
	u16 mask[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x100, 0x200, 0x400, 0x800, 0x1000, 0x2000, 0x4000, 0x8000};  
	
	spi_clk_start();      

	gpio_set_value(GPIO_SPI1_SCS0, 0);

	for (i=7; i>=0; i--)  
	{  

		send_bit=(send_data & mask[i]) >> i;

		if(send_bit)
			gpio_set_value(GPIO_SPI1_D0, 1);
		else
			gpio_set_value(GPIO_SPI1_D0, 0);

		spi_delay(g_halfdelay);

		//clk high
		gpio_set_value(GPIO_SPI1_SCLK, 1);

                spi_delay(g_delay);

		//clk low
		gpio_set_value(GPIO_SPI1_SCLK, 0);
                spi_delay(g_halfdelay);
	}

	gpio_set_value(GPIO_SPI1_SCS0, 1);
	
	spi_delay(g_halfdelay);

	spi_clk_end();

	return 0;
} /* spi_davinci_write_u16() */

int spi_davinci_read_u8_sync(void *spi_handle, u8 *buf, u32 bufSize)
{
 	int i = 0;  
	u8 read_bit=0;
	u8 read_data=0;  
	//u32 temp=0;
	      
	spi_clk_start();
	gpio_set_value(GPIO_SPI1_SCS0, 0);
	read_data = 0;
	for (i=7; i>=0; i--)  
	{ 
 
		spi_delay(g_halfdelay);

                //clk high
                gpio_set_value(GPIO_SPI1_SCLK, 0);

                spi_delay(g_halfdelay);
#if 0				
		temp = READREG(REGGET0);
		read_bit = (temp>>31)&0x01;
#else
		read_bit =gpio_get_value(GPIO_SPI1_D1);
#endif
//		printk("bit is %x\n",temp);
		if (0 != read_bit)  
		{  
			read_data |= (1<<i);
		}  


                spi_delay(g_halfdelay);

                //clk low
                gpio_set_value(GPIO_SPI1_SCLK, 1);
                spi_delay(g_halfdelay);
		   
	}
        gpio_set_value(GPIO_SPI1_SCS0, 1);

        spi_delay(g_halfdelay);

        spi_clk_end();
//	printk("the read is %x.\n",read_data);
	buf[0]=read_data;

	return 0;

} 

int spi_davinci_close(void *spi_handle)
{
	struct  spi_davinci_device * spi_davinci_dev = (struct spi_davinci_device *)spi_handle;

	spi_disable();

	if (!spi_davinci_dev)
	{
		return -1;
	}

	kfree(spi_davinci_dev);

	gpio_free(GPIO_RST_PERI);
	printk("%s(): free GPIO_RST_PERI\n", __func__);	
	gpio_free(GPIO_VOICE_INT);
	printk("%s(): free GPIO_VOICE_INT\n", __func__);	
	gpio_free(GPIO_SPI1_D0);
	printk("%s(): free GPIO_SPI1_D0\n", __func__);	
	gpio_free(GPIO_SPI1_D1);
	printk("%s(): free GPIO_SPI1_D1\n", __func__);	
	gpio_free(GPIO_SPI1_SCS0);
	printk("%s(): free GPIO_SPI1_SCS0\n", __func__);	
	gpio_free(GPIO_SPI1_SCLK);
	printk("%s(): free GPIO_SPI1_SCLK\n", __func__);	

	return 0;

} /* spi_davinci_close() */

/*-----------------------------------------------------------------------------------------------*/
 
void *spi_davinci_open(spi_davinci_client_params_t *param)
{
	//int i;
	u8 temp[2];
	struct   spi_davinci_device *spi_davinci_dev=NULL;
	//struct   device *dev = NULL;
	spi_davinci_hw_init();

	temp[0]=0x55;
	spi_disable();

	spi_davinci_dev = kmalloc(sizeof(struct spi_davinci_device),GFP_KERNEL);

	if (!spi_davinci_dev) 
	{
		spi_err("could not allocate memory\n");
		goto init_error;
	}
	memset(spi_davinci_dev, 0, sizeof(struct spi_davinci_device));
	
//	spi_enable();
//	for(i=0;i<300;i++)
//		spi_davinci_write_u8_sync(NULL,temp,1);
		//spi_clk_start();
//	spi_disable();

	return spi_davinci_dev;

init_error:
	kfree(spi_davinci_dev);

	return NULL;
	
} /* spi_davinci_open() */

/*-----------------------------------------------------------------------------------------------*/
