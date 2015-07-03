/*
 * linux/drivers/spi/spi_davinci.h
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

#ifndef _SPI_DAVINCI_API_H_
#define _SPI_DAVINCI_API_H_

typedef struct
{
	void (*SpiCB)(void* SpiHandle, int status);
	void* SpiHandle;
} spi_davinci_client_params_t;


void *spi_davinci_open(spi_davinci_client_params_t *param);
int   spi_davinci_close(void *spi_handle);
int   spi_davinci_write_u8_sync(void *spi_handle, u8 *buf, u32 bufSize);
int   spi_davinci_read_u8_sync(void *spi_handle, u8 *buf, u32 bufSize);

void reset_si3217x(void);
#endif	/* _SPI_DAVINCI_API_H_ */


