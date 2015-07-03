/*
 * TI816X/TI814X PCIe Endpoint (EP) boot driver.
 *
 * Note: References common across TI816X and TI814X devices are fererred as
 * TI81XX/ti81xx.
 *
 * Copyright (C) 2010 Texas Instruments, Incorporated
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/uaccess.h>
#include <linux/ioport.h>
#include <linux/io.h>
#include <linux/pci.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/interrupt.h>
#include <linux/version.h>
#include <asm/irq.h>

#include "ti81xx_pcie_bootdrv.h"

/* PCIe application registers virtual address (mapped to PCI window) */
#define PCI_REGV(reg)           (reg_virt + reg)

struct pci_dev *ti81xx_pci_dev;

/*
 * TODO: Check for the possibility of avoiding the following static variables
 * and make it part of driver/device data or at least group in one structure.
 */

/*
 * TI81XX access ranges. Note that all the addresses below translate to PCIe
 * access.
 */
static unsigned int reg_phys, reg_virt, reg_len;
static unsigned int ocmc1_phys, ocmc1_virt, ocmc1_len;
static unsigned int ddr_phys, ddr_virt, ddr_len;

static int ti81xx_pci_major;
static struct cdev ti81xx_pci_cdev;
static struct class *ti81xx_pci_class;
static dev_t ti81xx_dev;

/* List of TI81XX family devices supported by this driver */
static unsigned int supported_dev_list[] = {
	TI816X_PCI_DEVICE_ID,
	TI814X_PCI_DEVICE_ID,
};

/* These values change depending upon the actual TI81XX device detected */
static unsigned int pci_device_id;
static unsigned int bootflag_offset;
static unsigned int bar1_ib_offset;

/**
 * ti81xx_ep_find_device() - Look-up for first available TI816X/TI814X Endpoint
 * @device_id: PCI Device ID of the device to search
 *
 * Since we could even be running on another TI81XX device acting as RC, we need
 * to skip it - this is done based on checking device class to be set as "PCI
 * Bridge" for RC as the RC driver does this setting during enumeration.
 *
 * Note: This checking needs to be updated if RC driver is changed to set (or
 * not to set) class differently.
 *
 * Returns pointer of pci device structure on success, NULL otherwise.
 */
static struct pci_dev *ti81xx_ep_find_device(unsigned int device_id)
{
	struct pci_dev *dev = pci_get_device(TI81XX_PCI_VENDOR_ID,
						device_id, NULL);

	while (NULL != dev) {
		if ((dev->class >> 8) == PCI_CLASS_BRIDGE_PCI) {
			pr_warning(TI81XX_PCIE_MODFILE
					": skipping TI81xx PCIe RC...\n");
			dev = pci_get_device(TI81XX_PCI_VENDOR_ID,
						device_id, dev);
			continue;
		}

		pr_info(TI81XX_PCIE_MODFILE ": Found TI81xx PCIe EP @0x%08x, "
				"DEVICE ID = %04x\n", (int)dev, device_id);

		return dev;
	}

	pr_err(TI81XX_PCIE_MODFILE ": No TI81xx PCIe EP [0x%04x] found\n",
			device_id);
	return NULL;
}

/**
 * ti81xx_ep_setup_bar() - Setup specified BAR on TI81XX EP for inbound
 * @bar_num: BAR to set inbound access to
 * @addr: Address for inbound translation to TI816X internal region
 *
 * On TI81XX, BAR0 is hrdwired and hence skipped for inbound configuration and a
 * maximum of 4 inbound translations are allowed for 32-bit BARs.
 *
 * TODO: Add 64-bit addressing support.
 */
static int ti81xx_ep_setup_bar(u32 bar_num, u32 addr)
{
	u32 bar_val, ib_num;

	if ((bar_num == 0) || (bar_num > 4))
		return -1;

	ib_num = bar_num - 1;

	bar_val = pci_resource_start(ti81xx_pci_dev, bar_num);

	__raw_writel(0, PCI_REGV(IB_BAR(ib_num)));
	__raw_writel(bar_val, PCI_REGV(IB_START_LO(ib_num)));
	__raw_writel(0, PCI_REGV(IB_START_HI(ib_num)));
	__raw_writel(addr, PCI_REGV(IB_OFFSET(ib_num)));
	__raw_writel(bar_num, PCI_REGV(IB_BAR(ib_num)));

	return 0;
}

/**
 * ti81xx_pci_get_resources() - Read BARs as set by Host and reserve resources
 *
 * Only reads first 3 BARs. Expects the BAR sizes are already set by the boot
 * ROM on TI81XX EP with minimum sizes as follows:
 * - BAR0 = 4KB
 * - BAR1 = 256KB
 * - BAR2 = 8MB
 *
 * FIXME: Presently this function maps all 3 BARs but in reality we only need to
 * map BAR0 and rest can be relied upon the application (mmap).
 */
static int ti81xx_pci_get_resources(void)
{
	int index;
	u32  bar_start[3];
	u32  bar_len[3];
	u32  bar_flags[3];

	dev_info(&ti81xx_pci_dev->dev, "BAR Configuration -\n\t   "
			"Start\t|\tLength\t|\tFlags\n");
	for (index = 0; index < 3; index++) {
		bar_start[index] = pci_resource_start(ti81xx_pci_dev, index);
		bar_len[index] = pci_resource_len(ti81xx_pci_dev, index);
		bar_flags[index] = pci_resource_flags(ti81xx_pci_dev, index);

		if (bar_flags[index] & IORESOURCE_IO) {
			dev_err(&ti81xx_pci_dev->dev,
				"This driver does not support PCI IO.\n");
			return -1;
		}

		dev_info(&ti81xx_pci_dev->dev, "\t0x%08x\t|\t%d\t|\t0x%08x\n",
				(int)bar_start[index], (int)bar_len[index],
				(int)bar_flags[index]);
	}

	reg_phys = bar_start[0];
	reg_len = bar_len[0];

	if (NULL == request_mem_region(reg_phys, reg_len, "ti81xx_reg")) {
		dev_err(&ti81xx_pci_dev->dev, "Failed reserve reg resource\n");
		return -1;
	}

	reg_virt = (unsigned int)ioremap_nocache(reg_phys, reg_len);
	if (!reg_virt) {
		dev_err(&ti81xx_pci_dev->dev, "Failed remapping registers\n");
		goto err_regremap;
	}

	dev_info(&ti81xx_pci_dev->dev, "TI81XX registers mapped to 0x%08x\n",
			(int)reg_virt);

	ocmc1_phys = bar_start[1];
	ocmc1_len = bar_len[1];

	if (NULL == request_mem_region(ocmc1_phys, ocmc1_len, "ti81xx_ocmc1")) {
		dev_err(&ti81xx_pci_dev->dev, "Failed reserve ocmc resource\n");
		goto err_ocmc1res;
	}

	ocmc1_virt = (unsigned int)ioremap_nocache(ocmc1_phys, ocmc1_len);
	if (!ocmc1_virt) {
		dev_err(&ti81xx_pci_dev->dev, "Failed remapping OCMC\n");
		goto err_ocmc1remap;
	}

	dev_info(&ti81xx_pci_dev->dev, "TI81XX OCMC mapped to 0x%08x\n",
			(int)ocmc1_virt);

	ddr_phys = bar_start[2];
	ddr_len = bar_len[2];

	if (NULL == request_mem_region(ddr_phys, ddr_len, "ti81xx_ram")) {
		dev_err(&ti81xx_pci_dev->dev, "Failed remapping RAM\n");
		goto err_ramres;
	}

	ddr_virt = (unsigned int)ioremap_nocache(ddr_phys, ddr_len);
	if (!ddr_virt) {
		dev_err(&ti81xx_pci_dev->dev, "Failed remapping RAM\n");
		goto err_ramremap;
	}

	dev_info(&ti81xx_pci_dev->dev, "TI81XX DDR mapped to 0x%08x\n",
			(int)ddr_virt);

	return 0;
err_ramremap:
	release_mem_region(ddr_phys, ddr_len);
err_ramres:
	iounmap((void *)ocmc1_virt);
err_ocmc1remap:
	release_mem_region(ocmc1_phys, ocmc1_len);
err_ocmc1res:
	iounmap((void *)reg_virt);
err_regremap:
	release_mem_region(reg_phys, reg_len);

	return -1;
}

/**
 * ti81xx_pci_set_master() - Set ti81Xx EP to be master.
 *
 * Also sets latency timer value.
 *
 * _NOTE_: As of now, it is not mandatory to set ti81xx device as master for
 * boot operation and this may be skipped. It is advisable to retain some of the
 * other configurations this function does though.
 */
static void ti81xx_pci_set_master(void)
{
	s32   ret_val ;
	u16  cmd_val ;
	struct pci_dev *dev = ti81xx_pci_dev;

	pci_set_master(dev);
	pci_write_config_byte(dev, PCI_LATENCY_TIMER, 0x80);

	/* Add support memory write invalidate */
	ret_val = pci_set_mwi(dev);

	pci_read_config_word(dev, PCI_COMMAND, (u16 *) &cmd_val);

	/* and set the master bit in command register. */
	cmd_val |= (PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER | PCI_COMMAND_SERR);

	pci_write_config_word(dev, PCI_COMMAND, cmd_val);
}

/**
 * ti81xx_pcie_ioctl() - Application interface for booting TI81XX EP
 *
 * Provides the interface to the application code to transfer data to PCI mapped
 * memory spaces. It will be used to initially transfer the U-Boot binary and
 * file. This code largely depends on the handshaking done using the Boot
 * Complete flag written/read in TI816X/TI814X device's OCMC RAM by this driver
 * as well as the boot ROM and U-Boot running on the EP device.
 */
long ti81xx_pcie_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int ret = 0 ;

	switch (cmd) {
	case TI81XX_PCI_SET_DWNLD_DONE:
	{
		/* Set the download complete flag */
		__raw_writel(0x1, (ocmc1_virt + bootflag_offset));

		if (arg) {
			unsigned int msec_loops = (unsigned int) arg * 1000;

			/* Wait until it is cleared  */
			while (__raw_readl((ocmc1_virt + bootflag_offset))) {
				udelay(1000);

				if (!--msec_loops)
					break;
			}

			if (!msec_loops) {
				dev_warn(&ti81xx_pci_dev->dev,
						"Bootflag clear timed out\n");
				ret = -1;
			} else {
				dev_info(&ti81xx_pci_dev->dev,
						"Bootflag cleared\n");
			}
		}

	}

	break;

	case TI81XX_PCI_SET_BAR_WINDOW:
	{
		struct ti81xx_bar_info *bar
			= (struct ti81xx_bar_info *) arg;

		if (ti81xx_ep_setup_bar(bar->num, bar->addr)) {
			dev_err(&ti81xx_pci_dev->dev,
					"Setting inbound for BAR%d failed\n",
					(int)bar->num);
			ret = -1;
		}
	}

	break;

	case TI81XX_PCI_GET_BAR_INFO:
	{
		struct ti81xx_bar_info *bar
			= (struct ti81xx_bar_info *) arg;

		bar->addr = pci_resource_start(ti81xx_pci_dev, bar->num);
		bar->size = pci_resource_len(ti81xx_pci_dev, bar->num);
	}

	break;

	case TI81XX_PCI_GET_DEVICE_ID:
		*(unsigned int *) arg = pci_device_id;
		break;

	default:
		ret = -1;
	}

	return ret;
}

/* Check if requested address range lies within those supported */
#define is_in_ocmc1(a, l)	\
	((a >= ocmc1_phys) && ((a + l) <= (ocmc1_phys + ocmc1_len)))

#define is_in_ddr(a, l)		\
	((a >= ddr_phys) && ((a + l) <= (ddr_phys + ddr_len)))

/**
 * ti81xx_ep_pcie_mmap() - Provide userspace mapping for specified kernel memory
 * @filp: File private data - ignored
 * @vma: User virtual memory area to map to
 *
 * At present, only allows mapping BAR1 & BAR2 spaces. It is assumed that these
 * BARs are internally translated to access ti81Xx OCMC and DDR RAM
 * respectively (application can ensure this using TI81XX_PCI_SET_BAR_WINDOW
 * ioctl to setup proper translation on ti81xx EP).
 *
 * Note that the application has to get the physical BAR address as assigned by
 * the host code. One way to achieve this is to use ioctl
 * TI81XX_PCI_GET_BAR_INFO.
 */
int ti81xx_ep_pcie_mmap(struct file *filp, struct vm_area_struct *vma)
{
	int ret = -EINVAL;
	unsigned long sz = vma->vm_end - vma->vm_start;
	unsigned int addr = (unsigned int)vma->vm_pgoff << PAGE_SHIFT;

	dev_info(&ti81xx_pci_dev->dev, "Mapping %#lx bytes from address %#x\n",
			sz, addr);

	if (is_in_ocmc1(addr, (unsigned int)sz)
		|| is_in_ddr(addr, (unsigned int)sz)) {
		vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

		ret = remap_pfn_range(vma, vma->vm_start,
				vma->vm_pgoff,
				sz, vma->vm_page_prot);
	}

	return ret;
}

/**
 * ti81xx_pci_fops - Declares supported file access functions
 */
static const struct file_operations ti81xx_pci_fops = {
	.owner		= THIS_MODULE,
	.mmap		= ti81xx_ep_pcie_mmap,
	.unlocked_ioctl = ti81xx_pcie_ioctl,
};

/**
 * ti81xx_ep_pci_init() - Initialize TI81XX PCIe EP
 *
 * Checks if an TI816X/TI814X Endpoint is detected in the system. Initialize the
 * PCI configuration for the device to enable access for downloading the images
 * and booting.
 *
 * Note: Currently supports only booting the first TI816X or, if not, then first
 * TI814X EP detected. The sequence is to first search for TI816X devices and if
 * none found, search for TI814X endpoint.
 */
static int __init ti81xx_ep_pci_init(void)
{
	int ret, val, i;

	for (i = 0; i < ARRAY_SIZE(supported_dev_list); i++) {
		struct pci_dev *tmp =
			ti81xx_ep_find_device(supported_dev_list[i]);

		if (tmp) {
			pci_device_id = supported_dev_list[i];
			ti81xx_pci_dev = tmp;
			break;
		}
	}

	if (!ti81xx_pci_dev) {
		pr_err(TI81XX_PCIE_MODFILE ": No TI81xx PCIe EP found.\n");
		return -1;
	}

	dev_warn(&ti81xx_pci_dev->dev, "This driver supports booting the "
			"first TI816x or TI814x target found on the bus\n");

	/* Set up device related parameters for actual TI81XX device */
	switch (pci_device_id) {
	case TI814X_PCI_DEVICE_ID:
		bootflag_offset = TI814X_EP_BOOTFLAG_OFFSET;
		bar1_ib_offset = TI814X_EP_UBOOT_STG1_IB_OFFSET;
		break;

	case TI816X_PCI_DEVICE_ID:
		bootflag_offset = TI816X_EP_BOOTFLAG_OFFSET;
		bar1_ib_offset = TI816X_EP_UBOOT_IB_OFFSET;
		break;

	default:
		dev_err(&ti81xx_pci_dev->dev, "Something went wrong - "
				"no valid device id, exiting.\n");
	}

	ret = alloc_chrdev_region(&ti81xx_dev, 0, 1, TI81XX_PCIE_MODFILE);
	if (ret) {
		dev_err(&ti81xx_pci_dev->dev,
				"could not allocate the character driver");
		goto err_post_pci;
	}

	ti81xx_pci_major = MAJOR(ti81xx_dev);

	cdev_init(&ti81xx_pci_cdev, &ti81xx_pci_fops);
	ti81xx_pci_cdev.owner = THIS_MODULE;
	ti81xx_pci_cdev.ops = &ti81xx_pci_fops;

	ret = cdev_add(&ti81xx_pci_cdev, ti81xx_dev, TI81XX_DEV_COUNT);
	if (ret) {
		dev_err(&ti81xx_pci_dev->dev,
				"Failed creation of node for PCI boot\n");
		unregister_chrdev_region(ti81xx_dev, TI81XX_DEV_COUNT);
		goto err_post_pci;
	}

	dev_info(&ti81xx_pci_dev->dev, "Major number %d assigned\n",
			ti81xx_pci_major);

	ti81xx_pci_class = class_create(THIS_MODULE, TI81XX_PCIE_MODFILE);
	if (!ti81xx_pci_class) {
		cdev_del(&ti81xx_pci_cdev);
		unregister_chrdev_region(ti81xx_dev, TI81XX_DEV_COUNT);
		dev_err(&ti81xx_pci_dev->dev,
				"Failed to add device to sys fs\n");
		ret = -1;
		goto err_post_pci;
	}

	device_create(ti81xx_pci_class, NULL, ti81xx_dev,
				NULL, TI81XX_PCIE_MODFILE);
	dev_info(&ti81xx_pci_dev->dev, "Added device to the sys file system\n");

	ret = pci_enable_device(ti81xx_pci_dev);
	if (ret) {
		dev_err(&ti81xx_pci_dev->dev, "Failed to enable device.\n");
		goto err_post_cdev;
	}

	ti81xx_pci_set_master();

	ret = ti81xx_pci_get_resources();
	if (ret == -1) {
		dev_err(&ti81xx_pci_dev->dev, "could not get resources\n");
		goto err_post_cdev;
	}

	/* Set up default inbound access windows */
	ti81xx_ep_setup_bar(1, bar1_ib_offset);
	ti81xx_ep_setup_bar(2, TI81XX_EP_KERNEL_IB_OFFSET);

	/* Enable inbound translation */
	val = __raw_readl(PCI_REGV(CMD_STATUS));
	val |= IB_XLAT_EN_VAL;
	__raw_writel(val, PCI_REGV(CMD_STATUS));

	return 0 ;

err_post_cdev:
	device_destroy(ti81xx_pci_class, ti81xx_dev);
	class_destroy(ti81xx_pci_class);
	cdev_del(&ti81xx_pci_cdev);
	unregister_chrdev_region(ti81xx_dev, TI81XX_DEV_COUNT);

err_post_pci:
	pci_dev_put(ti81xx_pci_dev);

	return ret;
}
module_init(ti81xx_ep_pci_init);

/**
 * ti81xx_ep_pcie_cleanup() - Perform cleanups before module unload
 */
static void __exit ti81xx_ep_pcie_cleanup(void)
{
	device_destroy(ti81xx_pci_class, ti81xx_dev);
	class_destroy(ti81xx_pci_class);
	cdev_del(&ti81xx_pci_cdev);
	unregister_chrdev_region(ti81xx_dev, TI81XX_DEV_COUNT);

	iounmap((void *)ocmc1_virt);
	release_mem_region(ocmc1_phys, ocmc1_len);

	iounmap((void *)ddr_virt);
	release_mem_region(ddr_phys, ddr_len);

	iounmap((void *)reg_virt);
	release_mem_region(reg_phys, reg_len);

	pci_disable_device(ti81xx_pci_dev);

	pci_dev_put(ti81xx_pci_dev);
}
module_exit(ti81xx_ep_pcie_cleanup);
MODULE_LICENSE("GPL");
