# Comment/uncomment the following line to disable/enable debugging
DEBUG = n

ARCH = arm
CROSS_COMPILE = arm-none-linux-gnueabi-
OUTPUT_DIR = ./output
KERNEL_DIR= ../../

#OBJS = src/os_linux.o src/spi_omap.o src/spi.o
#OBJS = os_linux.o  spi_davinci.o

ifneq ($(KERNELRELEASE),)

	# This Makefile has been simplified as much as possible, by putting all
	# generic material, independent of this specific directory, into
	# ./rules.make. Read that file for details
	TOPDIR  := $(PWD)
	#include $(TOPDIR)/rules.make

	# Add your debugging flag (or not) to EXTRA_CFLAGS
	ifeq ($(DEBUG),y)
		DEBFLAGS = -O -g -DUSE_SPI_DEBUG # "-O" is needed to expand inlines
	else
		DEBFLAGS =
	endif

	EXTRA_CFLAGS += $(DEBFLAGS)
	EXTRA_CFLAGS += -DUSE_SPI_DEBUG -DUSE_DMA_COPY_DATA -D__KERNEL__ -DLINUX -Dlinux -DMODULE -DEXPORT_SYMTAB
	#EXTRA_CFLAGS += -DWL404
	EXTRA_CFLAGS += -I$(TOPDIR) -I$(TOPDIR)/../inc

	obj-m :=  si3217x_spi.o

	spi-y := 
else

.PHONY: all modules test

all: modules

modules:
	$(MAKE) -C $(KERNEL_DIR) M=`pwd` ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) $@
	#mv wspi.ko $(OUTPUT_DIR)/omap_spi.o >&/dev/null

test:
	$(CROSS_COMPILE)gcc spi_test.c -o $(OUTPUT_DIR)

clean:
	$(MAKE) -C $(KERNEL_DIR) M=`pwd` ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) $@
	@rm -f spi_test.o $(OUTPUT_DIR)/omap_spi.o $(OUTPUT_DIR)

%::
	$(WARN)
	$(MAKE) -C $(KERNEL_DIR) M=`pwd` ARCH=arm CROSS_COMPILE=$(CROSS_COMPILE) $@

endif

