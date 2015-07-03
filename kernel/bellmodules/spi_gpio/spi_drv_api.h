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

#ifndef _SPI_DRV_API_H_
#define _SPI_DRV_API_H_

typedef struct
{
	void (*WspiCB)(void* WspiHandle, int status);
	void* WspiHandle;
} spi_client_params_t;

/* WSPI API */

#ifdef WL404
typedef void (*request_callback_t) (void *data, int status);

void* SPI_Open(void);
int SPI_WriteSync(void* spi_handle, u8 *data, u32 length);
int SPI_ReadSync(void *spi_handle, u16  *data, u32 length);
#else
void *SPI_Open(spi_client_params_t *param);
int SPI_WriteSync (void *spi_handle, u16  *data, unsigned int length, int more);
int SPI_ReadSync  (void *spi_handle, u16  *data, unsigned int length, int more);
int SPI_WriteSync8 (void *spi_handle, u8  *data, unsigned int length, int more);
int SPI_ReadSync8  (void *spi_handle, u8  *data, unsigned int length, int more);
#endif
int SPI_Close(void *spi_handle);
int SPI_ReadAsync (void *spi_handle, u8  *data, unsigned int length, int more);
int SPI_WriteAsync(void *spi_handle, u8  *data, unsigned int length, int more);

void SPI_ResetSi3217x(void);
#endif	/* _SPI_DRV_API_H_ */


