/*
 * BRIEF MODULE DESCRIPTION
 * Low-level MMC/SD functions for the OMAP1510 MMC controller
 * Based on: arch/arm/mach-sa1100/h3600_asic_mmc.c
 *
 * Copyright 2003 MontaVista Software Inc.
 * Author: MontaVista Software, Inc.
 *	   source@mvista.com
 *
 * Modified for the OMAP1610 by Alexandr Shkurenkov:
 * Copyright (c) 2003 Monta Vista Software, Inc.
 *
 *  This program is free software; you can redistribute	 it and/or modify it
 *  under  the terms of	 the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the	License, or (at your
 *  option) any later version.
 *
 *  THIS  SOFTWARE  IS PROVIDED	  ``AS	IS'' AND   ANY	EXPRESS OR IMPLIED
 *  WARRANTIES,	  INCLUDING, BUT NOT  LIMITED  TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 *  NO	EVENT  SHALL   THE AUTHOR  BE	 LIABLE FOR ANY	  DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *  NOT LIMITED	  TO, PROCUREMENT OF  SUBSTITUTE GOODS	OR SERVICES; LOSS OF
 *  USE, DATA,	OR PROFITS; OR	BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 *  ANY THEORY OF LIABILITY, WHETHER IN	 CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  You should have received a copy of the  GNU General Public License along
 *  with this program; if not, write  to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 */

#undef CONFIG_CEE
#define CONFIG_CEE

/* includes */
#include <linux/module.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/dma-mapping.h>
#include <linux/proc_fs.h>
#include <linux/fs.h>
#include <linux/slab.h>

#include <asm/arch/dma.h>
#include "os_linux.h"
#include "spi_api.h"
#include "wspi_api.h"
#include "error.h"

/* global variables */
#ifdef CONFIG_WSPI_DEBUG
int g_wspi_debug = 2;		/* nick +++ */
#endif

/* defining structs */
typedef struct s_spi_api_arg{
	char *cmd_str;
	s32 dest_addr;
	UINT32 data;
	UINT32 length;
} spi_api_arg;


int os_AllocMem(int size,UINT8** mem)
{
	*mem = (UINT8*)kmalloc(size,GFP_KERNEL|GFP_ATOMIC);
	if(!mem)
		return ERROR_OS_ALLOC_MEM;

	return OK;
}

int os_FreeMem(UINT8* mem)
{
	kfree(mem);

	return OK;
}

