/*
 * linux/drivers/spi/spi_davinci.h
 *
 * Copyright (C) 2004 Texas Instruments.
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
 Modifications:
 ver. 1.0: Feb 2005, Vinod Mistral
 - 
 *
 */

#ifndef _SPI_DAVINCI_H_
#define _SPI_DAVINCI_H_

#include <linux/types.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/device.h>       /* for struct device */
//#include <asm/semaphore.h>
//#include <asm/arch/hardware.h>
//#include <asm/arch/irqs.h>

#include "spi_davinci_api.h"
#include "spi_debug.h"

#define DAVINCI_SPI_BASE_ADDR		IO_ADDRESS(DAVINCI_SPI_BASE)
#define DAVINCI_SPI_IOSIZE			0x80


/**************************************************************************\
* Register Overlay Structure
\**************************************************************************/
typedef struct  {
	volatile u32 SPIGCR0;
	volatile u32 SPIGCR1;
	volatile u32 SPIINT;
	volatile u32 SPILVL;
	volatile u32 SPIFLG;
	volatile u32 SPIPC0;
	volatile u32 SPIPC1;
	volatile u32 SPIPC2;
	volatile u32 SPIPC3;
	volatile u32 SPIPC4;
	volatile u32 SPIPC5;
	volatile u32 SPIPC6;
	volatile u32 SPIPC7;
	volatile u32 SPIPC8;
	volatile u32 SPIDAT0;
	volatile u32 SPIDAT1;
	volatile u32 SPIBUF;
	volatile u32 SPIEMU;
	volatile u32 SPIDELAY;
	volatile u32 SPIDEF;
	volatile u32 SPIFMT[4];
	volatile u32 TGINTVEC[2];
	volatile u8  RSVD0[8];
	volatile u32 MIBSPIE;
} spiregs;

/**************************************************************************\
* Overlay structure typedef definition
\**************************************************************************/
typedef volatile spiregs             *spiregsovly;

/* SPI device structure */
struct spi_davinci_device 
{
	void				*dma_handle;
	char				*buf;
	spiregsovly			regs;
};

#endif	/* _SPI_DAVINCI_H_ */


