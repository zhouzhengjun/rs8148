# File: env.mk. This file contains all the paths and other ENV variables

#
# Module paths
#

# Destination root directory.
#   - specify the directory where you want to place the object, archive/library,
#     binary and other generated files in a different location than source tree
#   - or leave it blank to place then in the same tree as the source
DEST_ROOT = $(dvr_rdk_PATH)/build

# Utilities directory. This is required only if the build machine is Windows.
#   - specify the installation directory of utility which supports POSIX commands
#     (eg: Cygwin installation or MSYS installation).
UTILS_INSTALL_DIR := $(xdc_PATH)

# Set path separator, etc based on the OS
ifeq ($(OS),Windows_NT)
  PATH_SEPARATOR = ;
  UTILSPATH = $(UTILS_INSTALL_DIR)/bin/
else
  # else, assume it is linux
  PATH_SEPARATOR = :
  UTILSPATH = /bin/
endif

# BIOS
bios_INCLUDE = $(bios_PATH)/packages

# IPC
ipc_INCLUDE = $(ipc_PATH)/packages

# SYSLINK
syslink_INCLUDE = $(syslink_PATH)/packages

# XDC
xdc_INCLUDE = $(xdc_PATH)/packages

# ISS drivers
iss_INCLUDE = $(iss_PATH)/packages
# HDVPSS drivers
hdvpss_INCLUDE = $(hdvpss_PATH)/packages
include $(hdvpss_PATH)/component.mk
export hdvpss_PATH

dvr_rdk_INCLUDE = $(dvr_rdk_PATH)
include $(dvr_rdk_PATH)/component.mk
export dvr_rdk_PATH

#FC include
fc_INCLUDE = $(fc_PATH)/packages

#XDAIS include
xdais_INCLUDE = $(xdais_PATH)/packages

#H264ENC include
h264enc_INCLUDE = $(h264enc_PATH)/Inc

#H264DEC include
h264dec_INCLUDE = $(h264dec_PATH)/Inc

#JPEGENC include
jpegenc_INCLUDE = $(jpegenc_PATH)/Inc

#JPEGDEC include
jpegdec_INCLUDE = $(jpegdec_PATH)/Inc

#MPEG4DEC include
mpeg4dec_INCLUDE = $(mpeg4dec_PATH)/Inc

#HDVICP API
hdvicpapi_INCLUDE = $(hdvicplib_PATH)
#
# Tools paths
#

# EDMA3LLD 
edma3lld_INCLUDE = $(edma3lld_PATH)/packages

# RPE
rpe_INCLUDE = $(rpe_PATH)/include
rpei_INCLUDE = $(rpe_PATH)/src/include
aaclcdec_INCLUDE = $(aaclcdec_PATH)/packages
aaclcenc_INCLUDE = $(aaclcenc_PATH)/packages


# Commands commonly used within the make files
RM = $(UTILSPATH)rm
RMDIR = $(UTILSPATH)rm -rf
MKDIR = $(UTILSPATH)mkdir
ECHO = @$(UTILSPATH)echo

#emake provides detailed info for build analysis.
EMAKE = emake --emake-emulation=gmake --emake-subbuild-db --emake-gen-subbuild-db=1

ifeq ($(OS),Windows_NT)
  MAKE = make
#  MAKE =  $(EMAKE)
else
  MAKE = make
endif
EGREP = $(UTILSPATH)egrep
CP = $(UTILSPATH)cp
ifeq ($(OS),Windows_NT)
  CHMOD = $(UTILSPATH)echo
else
  CHMOD = $(UTILSPATH)chmod
endif

#
# XDC specific ENV variables
#
# XDC Config.bld file (required for configuro); Derives from top-level hdvpss_PATH
ifeq ($(PLATFORM),ti816x-evm)
    ifeq ($(DDR_MEM),DDR_MEM_2048M)
      CONFIG_BLD_XDC_m3     = $(dvr_rdk_PATH)/mcfw/src_bios6/cfg/ti816x/config_2G.bld
      CONFIG_BLD_XDC_c674   = $(dvr_rdk_PATH)/mcfw/src_bios6/cfg/ti816x/config_2G.bld
    else
        ifeq ($(DDR_MEM),DDR_MEM_256M)
            CONFIG_BLD_XDC_m3     = $(dvr_rdk_PATH)/mcfw/src_bios6/cfg/ti816x/config_256M.bld
            CONFIG_BLD_XDC_c674   = $(dvr_rdk_PATH)/mcfw/src_bios6/cfg/ti816x/config_256M.bld
        else
            ifeq ($(LINUX_MEM),LINUX_MEM_256M) 
              CONFIG_BLD_XDC_m3     = $(dvr_rdk_PATH)/mcfw/src_bios6/cfg/ti816x/config_1G_256MLinux.bld
              CONFIG_BLD_XDC_c674   = $(dvr_rdk_PATH)/mcfw/src_bios6/cfg/ti816x/config_1G_256MLinux.bld
            else
              CONFIG_BLD_XDC_m3     = $(dvr_rdk_PATH)/mcfw/src_bios6/cfg/ti816x/config_1G_128MLinux.bld
              CONFIG_BLD_XDC_c674   = $(dvr_rdk_PATH)/mcfw/src_bios6/cfg/ti816x/config_1G_128MLinux.bld
            endif
        endif
    endif
endif


ifeq ($(PLATFORM),ti816x-sim)
  CONFIG_BLD_XDC_m3     = $(dvr_rdk_PATH)/mcfw/src_bios6/cfg/ti816x/config_1G_128MLinux.bld
  CONFIG_BLD_XDC_c674   = $(dvr_rdk_PATH)/mcfw/src_bios6/cfg/ti816x/config_1G_128MLinux.bld
endif
ifeq ($(PLATFORM),ti814x-evm)
	ifeq ($(DDR_MEM),DDR_MEM_512M)
	  CONFIG_BLD_XDC_m3     = $(dvr_rdk_PATH)/mcfw/src_bios6/cfg/ti814x/config_512M.bld
	  CONFIG_BLD_XDC_c674   = $(dvr_rdk_PATH)/mcfw/src_bios6/cfg/ti814x/config_512M.bld
	else 
		ifeq ($(DDR_MEM),DDR_MEM_1024M)
		  CONFIG_BLD_XDC_m3     = $(dvr_rdk_PATH)/mcfw/src_bios6/cfg/ti814x/config_1024M.bld
		  CONFIG_BLD_XDC_c674   = $(dvr_rdk_PATH)/mcfw/src_bios6/cfg/ti814x/config_1024M.bld
		else
		  CONFIG_BLD_XDC_m3     = $(dvr_rdk_PATH)/mcfw/src_bios6/cfg/ti814x/config_256M.bld
		  CONFIG_BLD_XDC_c674   = $(dvr_rdk_PATH)/mcfw/src_bios6/cfg/ti814x/config_256M.bld
		endif
	endif	
endif
ifeq ($(PLATFORM),ti814x-sim)
  CONFIG_BLD_XDC_m3     = $(dvr_rdk_PATH)/mcfw/src_bios6/cfg/ti814x/config_256M.bld
  CONFIG_BLD_XDC_c674   = $(dvr_rdk_PATH)/mcfw/src_bios6/cfg/ti814x/config_256M.bld
endif


ifeq ($(PLATFORM),ti810x-evm)
	ifeq ($(DDR_MEM),DDR_MEM_512M)
	  CONFIG_BLD_XDC_m3     = $(dvr_rdk_PATH)/mcfw/src_bios6/cfg/ti810x/config_512M.bld
#	  CONFIG_BLD_XDC_c674   = $(dvr_rdk_PATH)/mcfw/src_bios6/cfg/ti810x/config_512M.bld
	else 
		ifeq ($(DDR_MEM),DDR_MEM_1024M)
		  CONFIG_BLD_XDC_m3     = $(dvr_rdk_PATH)/mcfw/src_bios6/cfg/ti810x/config_1024M.bld
#		  CONFIG_BLD_XDC_c674   = $(dvr_rdk_PATH)/mcfw/src_bios6/cfg/ti810x/config_1024M.bld
		else
		  CONFIG_BLD_XDC_m3     = $(dvr_rdk_PATH)/mcfw/src_bios6/cfg/ti810x/config_256M.bld
#		  CONFIG_BLD_XDC_c674   = $(dvr_rdk_PATH)/mcfw/src_bios6/cfg/ti810x/config_256M.bld
		endif
	endif	
endif
ifeq ($(PLATFORM),ti810x-sim)
  CONFIG_BLD_XDC_m3     = $(dvr_rdk_PATH)/mcfw/src_bios6/cfg/ti810x/config_256M.bld
#  CONFIG_BLD_XDC_c674   = $(dvr_rdk_PATH)/mcfw/src_bios6/cfg/ti810x/config_256M.bld
endif


XDCROOT = $(xdc_PATH)
XDCTOOLS = $(xdc_PATH)
export XDCROOT
export XDCTOOLS

CGTOOLS = $(CODEGEN_PATH_M3)
export CGTOOLS

CGTOOLS_DSP = $(CODEGEN_PATH_DSP)
export CGTOOLS_DSP

CODESOURCERYCGTOOLS = $(CODEGEN_PATH_A8)
export CODESOURCERYCGTOOLS

STRIP470 = $(CODEGEN_PATH_M3)/bin/strip470 -p
STRIP6x = $(CODEGEN_PATH_DSP)/bin/strip6x -p

PATH += $(PATH_SEPARATOR)$(xdc_PATH)$(PATH_SEPARATOR)$(CODEGEN_PATH_M3)/bin $(PATH_SEPARATOR)$(CODEGEN_PATH_DSP)/bin
PATH += $(PATH_SEPARATOR)$(UBOOTDIR)/tools
export PATH

# Nothing beyond this point
