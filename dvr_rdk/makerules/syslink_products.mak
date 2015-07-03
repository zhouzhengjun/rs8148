#
#  ======== products.mak ========
#

# This is optional but recommended to install all products in one folder.
#
DEPOT = 

# List of supported devices (choose one): OMAP3530, TI816X, TI814X
#DEVICE = TI816X

# Host OS (choose one): Linux
HOSTOS = Linux

# Master core (GPP) OS type (choose one): Linux
#
GPPOS = Linux

# SysLink HLOS driver options variable (choose one): 0 or 1
#
SYSLINK_BUILD_DEBUG=1
SYSLINK_BUILD_OPTIMIZE=0
SYSLINK_TRACE_ENABLE=1

# Optional: choose one to override DEVICE-specific default: COFF, ELF
LOADER = ELF

# Optional: if using an SDK, choose one: EZSDK
SDK = NONE

# Optionally override SysLink driver opts here
SYSLINK_DRIVER_OPTS = 

# Define the product variables for the device you will be using.
#
ifeq ("$(DEVICE)","OMAP3530")
SYSLINK_INSTALL_DIR     = $(DEPOT)/_your_syslink_install_
IPC_INSTALL_DIR         = $(DEPOT)/_your_ipc_install_
BIOS_INSTALL_DIR        = $(DEPOT)/_your_bios_install_
XDC_INSTALL_DIR         = $(DEPOT)/_your_xdctools_install_
LINUXKERNEL             = $(DEPOT)/_your_linux_kernel_install_
CGT_ARM_INSTALL_DIR     = $(DEPOT)/_your_arm_code_gen_install_
CGT_ARM_PREFIX          = $(CGT_ARM_INSTALL_DIR)/bin/arm-arago-linux-gnueabi-
CGT_C64P_INSTALL_DIR    = $(DEPOT)/_your_c64p_code_gen_install_

else ifeq ("$(DEVICE)","TI816X")
SYSLINK_INSTALL_DIR     = $(syslink_PATH)
IPC_INSTALL_DIR         = $(ipc_PATH)
BIOS_INSTALL_DIR        = $(bios_PATH)
XDC_INSTALL_DIR         = $(xdc_PATH)
LINUXKERNEL             = $(KERNELDIR)
CGT_ARM_INSTALL_DIR     = $(CODEGEN_PATH_A8)
CGT_ARM_PREFIX          = $(CGT_ARM_INSTALL_DIR)/bin/arm-arago-linux-gnueabi-
CGT_C674_ELF_INSTALL_DIR= $(CODEGEN_PATH_DSP)
CGT_M3_ELF_INSTALL_DIR  = $(CODEGEN_PATH_M3)

else ifeq ("$(DEVICE)","TI814X")
SYSLINK_INSTALL_DIR     = $(syslink_PATH)
IPC_INSTALL_DIR         = $(ipc_PATH)
BIOS_INSTALL_DIR        = $(bios_PATH)
XDC_INSTALL_DIR         = $(xdc_PATH)
LINUXKERNEL             = $(KERNELDIR)
CGT_ARM_INSTALL_DIR     = $(CODEGEN_PATH_A8)
CGT_ARM_PREFIX          = $(CGT_ARM_INSTALL_DIR)/bin/arm-arago-linux-gnueabi-
CGT_C674_ELF_INSTALL_DIR= $(CODEGEN_PATH_DSP)
CGT_M3_ELF_INSTALL_DIR  = $(CODEGEN_PATH_M3)

else ifeq ("$(DEVICE)","OMAPL1XX")

endif


# Tip: If you install all products in one folder, say in $(DEPOT), and
# have only one version of each product, then you can use the wildcard
# function to set your product variables as illustrated below. This has
# the benefit, that when you replace a product with a newer version, this
# makefile will update itself automatically. Note the use of the ':='
# assignment operator.
#
# BIOS_INSTALL_DIR        := $(wildcard $(DEPOT)/bios_*)


# Use this goal to print your product variables.
.show-products:
	@echo "DEPOT                = $(DEPOT)"
	@echo "DEVICE               = $(DEVICE)"
	@echo "HOSTOS               = $(HOSTOS)"
	@echo "LOADER               = $(LOADER)"
	@echo "SDK                  = $(SDK)"
	@echo "SYSLINK_DRIVER_OPTS  = $(SYSLINK_DRIVER_OPTS)"
	@echo "SYSLINK_INSTALL_DIR  = $(SYSLINK_INSTALL_DIR)"
	@echo "IPC_INSTALL_DIR      = $(IPC_INSTALL_DIR)"
	@echo "BIOS_INSTALL_DIR     = $(BIOS_INSTALL_DIR)"
	@echo "XDC_INSTALL_DIR      = $(XDC_INSTALL_DIR)"
	@echo "LINUXKERNEL          = $(LINUXKERNEL)"
	@echo "CGT_ARM_PREFIX       = $(CGT_ARM_PREFIX)"
	@echo "CGT_C64P_INSTALL_DIR = $(CGT_C64P_INSTALL_DIR)"
	@echo "CGT_C674_ELF_INSTALL_DIR = $(CGT_C674_ELF_INSTALL_DIR)"
	@echo "CGT_M3_ELF_INSTALL_DIR = $(CGT_M3_ELF_INSTALL_DIR)"
