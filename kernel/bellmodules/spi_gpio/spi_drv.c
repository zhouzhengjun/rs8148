/*
 * linux/drivers/spi/spi_davinci_client.c
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
 */

#include <linux/proc_fs.h>
#include <linux/moduleparam.h>
#include <linux/module.h>


#include "spi_davinci_api.h"
#include "spi_drv_api.h"

typedef enum
{
	SPI_DRV_OPEN=1,
	SPI_DRV_CLOSE
} spi_drv_state_e;

struct __spi_drv
{
	int  spi_state;
	void *spi_handle;
} spi_drv;

/*--------------------------------------------------------------------------------------*/

void *SPI_Open(spi_client_params_t *param)
{
	spi_davinci_client_params_t spi_davinci_param;

	if (spi_drv.spi_state == SPI_DRV_OPEN)
	{
		SPI_Close(spi_drv.spi_handle);
	}
	spi_davinci_param.SpiCB     = param->WspiCB;
	spi_davinci_param.SpiHandle = param->WspiHandle;

	spi_drv.spi_handle = spi_davinci_open(&spi_davinci_param);

	spi_drv.spi_state  = SPI_DRV_OPEN;

  return spi_drv.spi_handle;
	
} /* SPI_Open() */

/*--------------------------------------------------------------------------------------*/

int SPI_Close(void *spi_handle)
{
	if (spi_drv.spi_state == SPI_DRV_OPEN)
	{
		spi_drv.spi_state  = SPI_DRV_CLOSE;
		return spi_davinci_close(spi_handle);
	}
	else
	{
		return -1;
	}

} /* SPI_Close() */

int SPI_WriteSync8(void *spi_handle, u8  *data, u32 length, int more)
{
//      printk("the data is %d,%d.\n",data[0],length);
        return spi_davinci_write_u8_sync(spi_handle, data, length);

} /* SPI_WriteSync() */

/*--------------------------------------------------------------------------------------*/

int SPI_ReadSync8(void *spi_handle, u8  *data, u32 length, int more)
{

        return spi_davinci_read_u8_sync(spi_handle, data, length);


} /* SPI_WriteSync() */

/*--------------------------------------------------------------------------------------*/

int __init  spi_init(void)
{
	printk("spi_init\n");
	memset(&spi_drv, 0, sizeof(spi_drv));
	return 0;

} /* spi_init() */

/*--------------------------------------------------------------------------------------*/

static void __exit spi_exit(void)
{
  if (spi_drv.spi_state == SPI_DRV_OPEN)
  {
    SPI_Close(spi_drv.spi_handle);
  }
  printk("spi_exit\n");	

} /* spi_exit() */

void SPI_ResetSi3217x(void)
{
	reset_si3217x();
}
/*--------------------------------------------------------------------------------------*/

MODULE_AUTHOR("Texas Instruments");
MODULE_DESCRIPTION("DaVinci SPI TNETW Interface");
MODULE_LICENSE("GPL");

module_init(spi_init);
module_exit(spi_exit);

EXPORT_SYMBOL(SPI_Open);
EXPORT_SYMBOL(SPI_Close);
EXPORT_SYMBOL(SPI_WriteSync8);
EXPORT_SYMBOL(SPI_ReadSync8);
EXPORT_SYMBOL(SPI_ResetSi3217x);
