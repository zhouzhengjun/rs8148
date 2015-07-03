# Copyright Texas Instruments
ifeq ($(dvr_rdk_PATH), )

# Board type can be one of the following
#	1. DM816X_DVR
#	2. DM816X_EVM
#	3. DM814X_EVM
#	4. DM810X_EVM
#	5. DM810X_DVR

ifeq ($(DVR_RDK_BOARD_TYPE ), )
  DVR_RDK_BOARD_TYPE := DM814X_EVM
endif

# Default build environment, windows or linux
ifeq ($(OS), )
  OS := Linux
endif

dvr_rdk_RELPATH = dvr_rdk

ifeq ($(OS),Windows_NT)
  dvr_rdk_BASE     := $(CURDIR)/..
  TI_SW_ROOT       := D:/ti_software
endif

ifeq ($(OS),Linux)
  dvr_rdk_BASE     := $(shell pwd)/..
#  TI_SW_ROOT       := $(dvr_rdk_BASE)/ti_tools
  TI_SW_ROOT       := /home/ruisu/dvrrdk-rs8148/ti_tools
endif

dvr_rdk_PATH     := $(dvr_rdk_BASE)/$(dvr_rdk_RELPATH)
#CODEGEN_PATH_A8  := $(TI_SW_ROOT)/cgt_a8/arm-2009q1
#just use arago tool, avoid toolchains install
CODEGEN_PATH_A8  := $(TI_SW_ROOT)/cgt_a8/arago/linux-devkit
CODEGEN_PATH_DSP := $(TI_SW_ROOT)/cgt_dsp/cgt6x_7_3_1/
CODEGEN_PATH_M3  := $(TI_SW_ROOT)/cgt_m3/cgt470_4_9_0/

ifeq ($(DVR_RDK_BOARD_TYPE),DM816X_EVM)
	hdvpss_PATH      := $(TI_SW_ROOT)/hdvpss/hdvpss_01_00_01_37_patched
endif
ifeq ($(DVR_RDK_BOARD_TYPE),DM816X_DVR)
	hdvpss_PATH      := $(TI_SW_ROOT)/hdvpss/hdvpss_01_00_01_37_patched
endif
ifeq ($(DVR_RDK_BOARD_TYPE),DM814X_EVM)
	hdvpss_PATH      := $(TI_SW_ROOT)/hdvpss/hdvpss_01_00_01_37_rs8148
endif
ifeq ($(DVR_RDK_BOARD_TYPE),DM814X_DVR)
	hdvpss_PATH      := $(TI_SW_ROOT)/hdvpss/hdvpss_01_00_01_36
endif
ifeq ($(DVR_RDK_BOARD_TYPE),DM810X_EVM)
	hdvpss_PATH      := $(TI_SW_ROOT)/hdvpss/hdvpss_01_00_01_37_patched
endif
ifeq ($(DVR_RDK_BOARD_TYPE),DM810X_DVR)
	hdvpss_PATH      := $(TI_SW_ROOT)/hdvpss/hdvpss_01_00_01_37_patched
endif

bios_PATH        := $(TI_SW_ROOT)/bios/bios_6_33_02_31
xdc_PATH         := $(TI_SW_ROOT)/xdc/xdctools_3_23_01_43

syslink_PATH     := $(TI_SW_ROOT)/syslink/syslink_2_10_02_17_patched

ifeq ($(DVR_RDK_BOARD_TYPE),DM810X_EVM)
syslink_PATH     := $(TI_SW_ROOT)/syslink/syslink_2_10_02_17_ti810x_patched
endif

ifeq ($(DVR_RDK_BOARD_TYPE),DM810X_DVR)
syslink_PATH     := $(TI_SW_ROOT)/syslink/syslink_2_10_02_17_ti810x_patched
endif

iss_PATH		 := $(TI_SW_ROOT)/iss_03_50_00_00

ipc_PATH         := $(TI_SW_ROOT)/ipc/ipc_1_24_02_27
fc_PATH          := $(TI_SW_ROOT)/framework_components/framework_components_3_22_00_05
xdais_PATH       := $(TI_SW_ROOT)/xdais/xdais_7_22_00_03
h264dec_DIR      := $(TI_SW_ROOT)/codecs/REL.500.V.H264AVC.D.HP.IVAHD.02.00.04.01
h264enc_DIR      := $(TI_SW_ROOT)/codecs/REL.500.V.H264AVC.E.IVAHD.02.00.02.01
h264dec_PATH     := $(h264dec_DIR)/500.V.H264AVC.D.HP.IVAHD.02.00/IVAHD_001
h264enc_PATH     := $(h264enc_DIR)/500.V.H264AVC.E.IVAHD.02.00/IVAHD_001
jpegdec_DIR      := $(TI_SW_ROOT)/codecs/REL.500.V.MJPEG.D.IVAHD.01.00.04.00
jpegenc_DIR      := $(TI_SW_ROOT)/codecs/REL.500.V.MJPEG.E.IVAHD.01.00.02.00
jpegdec_PATH     := $(jpegdec_DIR)/500.V.MJPEG.D.IVAHD.01.00/IVAHD_001
jpegenc_PATH     := $(jpegenc_DIR)/500.V.MJPEG.E.IVAHD.01.00/IVAHD_001
mpeg4dec_DIR     := $(TI_SW_ROOT)/codecs/REL.500.V.MPEG4.D.IVAHD.01.00.06.00
mpeg4dec_PATH    := $(mpeg4dec_DIR)/500.V.MPEG4.D.ASP.IVAHD.01.00/IVAHD_001
hdvicplib_PATH   := $(TI_SW_ROOT)/ivahd_hdvicp/hdvicp20
linuxdevkit_PATH := $(TI_SW_ROOT)/linux_devkit/arm-none-linux-gnueabi
edma3lld_PATH    := $(TI_SW_ROOT)/edma3lld/edma3_lld_02_11_02_04

# Audio framework (RPE) and Codecs
rpe_PATH         := $(TI_SW_ROOT)/rpe/remote-processor-execute
aaclcdec_PATH    := $(TI_SW_ROOT)/codecs/c674x_aaclcdec_01_41_00_00_elf
aaclcenc_PATH    := $(TI_SW_ROOT)/codecs/c674x_aaclcenc_01_00_01_00_elf_patched

dvrapp_PATH      := $(dvr_rdk_PATH)/dvrapp
qt_PATH          := /usr/local/Trolltech/QtEmbedded-4.6.2-arm

TFTP_HOME     := $(dvr_rdk_BASE)/tftphome
TARGET_FS     := $(dvr_rdk_BASE)/target/rfs
TARGET_FS_DIR := $(dvr_rdk_PATH)/bin/ti816x

ROOTDIR := $(dvr_rdk_PATH)

ifeq ($(DVR_RDK_BOARD_TYPE),DM814X_EVM)
TARGET_FS_DIR := $(dvr_rdk_PATH)/bin/ti814x
#TARGET_FS     := $(dvr_rdk_BASE)/target/rfs_814x
TARGET_FS     := $(dvr_rdk_BASE)/target/rfs_814x
endif

ifeq ($(DVR_RDK_BOARD_TYPE),DM814X_DVR)
TARGET_FS_DIR := $(dvr_rdk_PATH)/bin/ti814x
TARGET_FS     := $(dvr_rdk_BASE)/target/rfs_814x
endif

ifeq ($(DVR_RDK_BOARD_TYPE),DM816X_EVM)
TARGET_FS_DIR := $(dvr_rdk_PATH)/bin/ti816x
TARGET_FS     := $(dvr_rdk_BASE)/target/rfs_816x
endif

ifeq ($(DVR_RDK_BOARD_TYPE),DM816X_DVR)
TARGET_FS_DIR := $(dvr_rdk_PATH)/bin/ti816x
TARGET_FS     := $(dvr_rdk_BASE)/target/rfs_816x
endif

ifeq ($(DVR_RDK_BOARD_TYPE),DM810X_EVM)
TARGET_FS_DIR := $(dvr_rdk_PATH)/bin/ti810x
TARGET_FS     := $(dvr_rdk_BASE)/target/rfs_810x
endif

ifeq ($(DVR_RDK_BOARD_TYPE),DM810X_DVR)
TARGET_FS_DIR := $(dvr_rdk_PATH)/bin/ti810x
TARGET_FS     := $(dvr_rdk_BASE)/target/rfs_810x
endif

# Default DDR Size
ifeq ($(DVR_RDK_BOARD_TYPE),DM816X_DVR)
ifeq ($(DDR_MEM), )
  DDR_MEM := DDR_MEM_1024M
#  DDR_MEM := DDR_MEM_2048M
endif
endif

ifeq ($(DVR_RDK_BOARD_TYPE),DM816X_DVR)
ifeq ($(DEMOTYPE), )
#  DEMOTYPE = link_api_sd_demo
endif
endif

ifeq ($(CORE), )
  CORE := m3vpss
endif

# Default platform
ifeq ($(PLATFORM), )
  PLATFORM := ti816x-evm
ifeq ($(DVR_RDK_BOARD_TYPE),DM814X_EVM)
  PLATFORM := ti814x-evm
endif
ifeq ($(DVR_RDK_BOARD_TYPE),DM814X_DVR)
  PLATFORM := ti814x-evm
endif
ifeq ($(DVR_RDK_BOARD_TYPE),DM810X_EVM)
  PLATFORM := ti810x-evm
endif
ifeq ($(DVR_RDK_BOARD_TYPE),DM810X_DVR)
  PLATFORM := ti810x-evm
endif
endif

# Default linux mem 
ifeq ($(LINUX_MEM), )
ifeq ($(PLATFORM), ti814x-evm)
  LINUX_MEM := LINUX_MEM_128M
endif
ifeq ($(PLATFORM), ti810x-evm)
  LINUX_MEM := LINUX_MEM_128M
else
  LINUX_MEM := LINUX_MEM_256M
endif
endif


# VS_CARD can be one of the following
#	1. WITH_VS_CARD
#	2. WITHOUT_VS_CARD
ifeq ($(PLATFORM), ti814x-evm)
#  DDR_MEM := DDR_MEM_256M
#  DDR_MEM := DDR_MEM_512M
 DDR_MEM := DDR_MEM_1024M
  VS_CARD := WITH_VS_CARD
#  VS_CARD := WITHOUT_VS_CARD
endif

# VS_CARD can be one of the following
#       1. WITH_VS_CARD
#       2. WITHOUT_VS_CARD
ifeq ($(PLATFORM), ti810x-evm)
#  DDR_MEM := DDR_MEM_256M
  DDR_MEM := DDR_MEM_512M
# DDR_MEM := DDR_MEM_1024M
  VS_CARD := WITH_VS_CARD
#  VS_CARD := WITHOUT_VS_CARD
endif


# Default profile
ifeq ($(PROFILE_m3video), )
  PROFILE_m3video := release
#  PROFILE_m3video := debug
endif

ifeq ($(PROFILE_m3vpss), )
  PROFILE_m3vpss := release
#  PROFILE_m3vpss := debug
endif

ifneq ($(PLATFORM), ti810x-evm)
ifeq ($(PROFILE_c6xdsp), )
  PROFILE_c6xdsp := release
endif
endif

# Default klockwork build flag
ifeq ($(KW_BUILD), )
  KW_BUILD := no
endif

USE_SYSLINK_NOTIFY=0

XDCPATH = $(bios_PATH)/packages;$(xdc_PATH)/packages;$(ipc_PATH)/packages;$(hdvpss_PATH)/packages;$(fc_PATH)/packages;$(dvr_rdk_PATH);$(syslink_PATH)/packages;$(xdais_PATH)/packages;$(edma3lld_PATH)/packages;

ifeq ($(PLATFORM),ti816x-evm)
LSPDIR        := $(TI_SW_ROOT)/linux_lsp/linux-psp-dvr-04.04.00.01
KERNELDIR     := $(LSPDIR)/src/linux-04.04.00.01
UBOOTDIR      := $(TI_SW_ROOT)/linux_lsp/linux-psp-dvr-04.04.00.01/src/u-boot-04.04.00.01
endif

ifeq ($(PLATFORM),ti814x-evm)
ifeq ($(DVR_RDK_BOARD_TYPE),DM814X_EVM)
#LSPDIR        := $(TI_SW_ROOT)/linux_lsp/TI81XX-LINUX-PSP-04.04.00.01
#KERNELDIR     := $(LSPDIR)/src/kernel/linux-04.04.00.01
#UBOOTDIR      := $(TI_SW_ROOT)/linux_lsp/TI81XX-LINUX-PSP-04.04.00.01/src/u-boot/u-boot-04.04.00.01
LSPDIR        := $(TI_SW_ROOT)/linux_lsp/TI81XX-LINUX-PSP-rs8148
KERNELDIR     := $(LSPDIR)/src/kernel
UBOOTDIR      := $(LSPDIR)/src/u-boot/
endif

ifeq ($(DVR_RDK_BOARD_TYPE),DM814X_DVR)
LSPDIR        := $(TI_SW_ROOT)/linux_lsp/TI814X_DVR/TI814X-LINUX-PSP-04.01.00.06
KERNELDIR     := $(LSPDIR)/src/kernel/linux-04.01.00.06
UBOOTDIR      := $(TI_SW_ROOT)/linux_lsp/TI814X_DVR/TI814X-LINUX-PSP-04.01.00.06/src/u-boot/u-boot-04.01.00.06
endif
endif

ifeq ($(PLATFORM),ti810x-evm)
LSPDIR        := $(TI_SW_ROOT)/linux_lsp/TI81XX-LINUX-PSP-04.04.00.01
KERNELDIR     := $(LSPDIR)/src/kernel/linux-04.04.00.01
UBOOTDIR      := $(LSPDIR)/src/u-boot/u-boot-04.04.00.01
endif

# Default klockwork build flag
ifeq ($(DISABLE_AUDIO), )
  DISABLE_AUDIO := no
endif

TREAT_WARNINGS_AS_ERROR=no


DVR_RDK_ISS_LIB_PATH=$(dvr_rdk_BASE)/dvr_rdk/mcfw/src_bios6/alg/simcop/lib
DVR_RDK_ISS_INC_PATH=$(dvr_rdk_BASE)/dvr_rdk/mcfw/src_bios6/alg/simcop/inc

endif

RPE_BUILD_VARS = ipc_PATH="${ipc_PATH}" \
	bios_PATH="${bios_PATH}" \
	xdc_PATH="${xdc_PATH}" \
	xdais_PATH="${xdais_PATH}" \
	syslink_PATH="${syslink_PATH}" \
	kernel_PATH="${KERNELDIR}" \
	CODESOURCERY_PATH="${CODEGEN_PATH_A8}" \
	CGT_ARM_PREFIX="${CSTOOL_PREFIX}" \
	CODEGEN_PATH_DSP="${CODEGEN_PATH_DSP}" \
	CODEGEN_PATH_DSPELF="${CODEGEN_PATH_DSP}" \
	ROOTDIR="${rpe_PATH}" \
	aaclcdec_PATH="${aaclcdec_PATH}" \
	aaclcenc_PATH="${aaclcenc_PATH}" 

include $(ROOTDIR)/makerules/build_config.mk
include $(ROOTDIR)/makerules/env.mk
include $(ROOTDIR)/makerules/platform.mk
include $(dvr_rdk_PATH)/component.mk

export OS
export PLATFORM
export CORE
export PROFILE_m3vpss
export PROFILE_m3video
export PROFILE_c6xdsp
export CODEGEN_PATH_M3
export CODEGEN_PREFIX
export CODEGEN_PATH_A8
export CODEGEN_PATH_DSP
export bios_PATH
export xdc_PATH
export hdvpss_PATH
export dvr_rdk_PATH
export ipc_PATH
export fc_PATH
export xdais_PATH
export h264dec_PATH
export h264enc_PATH
export jpegdec_PATH
export mpeg4dec_PATH
export jpegenc_PATH
export hdvicplib_PATH
export linuxdevkit_PATH
export edma3lld_PATH
export ROOTDIR
export XDCPATH
export KW_BUILD
export syslink_PATH
export KERNELDIR
export TARGET_FS_DIR
export UBOOTDIR
export DVR_RDK_BOARD_TYPE
export USE_SYSLINK_NOTIFY
export DEST_ROOT
export dvr_rdk_BASE
export TFTP_HOME
export LINUX_MEM
export DDR_MEM
export DISABLE_AUDIO 
export dvrapp_PATH
export qt_PATH
export TREAT_WARNINGS_AS_ERROR
export VS_CARD
export SC_SCRIPTS_BASE_DIR
export iss_PATH
export SYSTEM_SIMCOP_ENABLE
export DVR_RDK_ISS_LIB_PATH
export DVR_RDK_ISS_INC_PATH
export DEMOTYPE
export rpe_PATH    
export RPE_BUILD_VARS
export aaclcdec_PATH    
export aaclcenc_PATH    


