# Copyright Texas Instruments

# Default build environment, windows or linux
ifeq ($(OS), )
  OS := Linux
endif

IMGS_ID := IMGS_VIMICRO_VC0356
#IMGS_ID := IMGS_MICRON_MT9J003

PLATFORM := ti814x-evm

# Configure to build Iss Examples since conflict of Utils with RDK
ISS_EXAMPLES_DIR := YES

dvr_rdk_BASE     := $(shell pwd)/../..
TI_SW_ROOT       := $(dvr_rdk_BASE)/ti_tools

TOOLS_INSTALL_DIR := $(TI_SW_ROOT)
ISS_TOOL_DIR	  := $(TI_SW_ROOT)
ISS_REL_TAG		 := iss_03_50_00_00

CODEGEN_PATH_M3   := $(TOOLS_INSTALL_DIR)/cgt_m3/cgt470_4_9_0
iss_PATH          := $(ISS_TOOL_DIR)/$(ISS_REL_TAG)
iss_algo_PATH      := $(iss_PATH)/packages/ti/psp/iss/alg
edma3lld_PATH     := $(TOOLS_INSTALL_DIR)/edma3lld/edma3_lld_02_11_02_04
fc_PATH           := $(TOOLS_INSTALL_DIR)/framework_components/framework_components_3_22_00_05
bios_PATH         := $(TOOLS_INSTALL_DIR)/bios/bios_6_33_02_31
xdc_PATH          := $(TOOLS_INSTALL_DIR)/xdc/xdctools
ipc_PATH 	       := $(TOOLS_INSTALL_DIR)/ipc/ipc_1_24_02_27
syslink_PATH      := $(TOOLS_INSTALL_DIR)/syslink/syslink_2_10_02_17_patched
hdvpss_PATH       := $(ISS_TOOL_DIR)/hdvpss/hdvpss_01_00_01_37_patched
xdais_PATH        := $(TOOLS_INSTALL_DIR)/xdais/xdais_7_22_00_03
TIMMOSAL_PATH      := $(iss_PATH)/packages/ti/psp/iss/timmosal

ISS_INSTALL_DIR  := $(iss_PATH)

ROOTDIR := $(iss_PATH)

ifeq ($(CORE), )
  CORE := m3vpss
endif

# Default platform
ifeq ($(PLATFORM), )
  PLATFORM := ti814x-evm
endif

# Default profile
ifeq ($(PROFILE_$(CORE)), )
  #PROFILE_$(CORE) := debug
  #PROFILE_$(CORE)	:= whole_program_debug
  PROFILE_$(CORE) := release
endif

XDCPATH = $(bios_PATH)/packages;$(xdc_PATH)/packages;$(ipc_PATH)/packages;$(iss_PATH)/packages;$(hdvpss_PATH)/packages;$(xdais_PATH)/packages;$(edma3lld_PATH)/packages;

# Default klockwork build flag
ifeq ($(KW_BUILD), )
  KW_BUILD := no
endif

export OS
export PLATFORM
export CORE
export PROFILE_$(CORE)
export CODEGEN_PATH_M3
export bios_PATH
export edma3lld_PATH
export xdc_PATH
export iss_PATH
export iss_algo_PATH
export ipc_PATH
export hdvpss_PATH
export ROOTDIR
export XDCPATH
export xdais_PATH
export KW_BUILD
export IMGS_ID
export TIMMOSAL_PATH

include $(ROOTDIR)/makerules/build_config.mk
include $(ROOTDIR)/makerules/env.mk
include $(ROOTDIR)/makerules/platform.mk
include $(iss_PATH)/component.mk
