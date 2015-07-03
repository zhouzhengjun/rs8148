# File: component.mk
#       This file is component include make file of DVR_RDK.

# List of variables set in this file and their purpose:
# <mod>_RELPATH        - This is the relative path of the module, typically from
#                        top-level directory of the package
# <mod>_PATH           - This is the absolute path of the module. It derives from
#                        absolute path of the top-level directory (set in env.mk)
#                        and relative path set above
# <mod>_INCLUDE        - This is the path that has interface header files of the
#                        module. This can be multiple directories (space separated)
# <mod>_PKG_LIST       - Names of the modules (and sub-modules) that are a part
#                        part of this module, including itself.
# <mod>_PLATFORM_DEPENDENCY - "yes": means the code for this module depends on
#                             platform and the compiled obj/lib has to be kept
#                             under <platform> directory
#                             "no" or "" or if this variable is not defined: means
#                             this module has no platform dependent code and hence
#                             the obj/libs are not kept under <platform> dir.
# <mod>_CORE_DEPENDENCY     - "yes": means the code for this module depends on
#                             core and the compiled obj/lib has to be kept
#                             under <core> directory
#                             "no" or "" or if this variable is not defined: means
#                             this module has no core dependent code and hence
#                             the obj/libs are not kept under <core> dir.
# <mod>_APP_STAGE_FILES     - List of source files that belongs to the module
#                             <mod>, but that needs to be compiled at application
#                             build stage (in the context of the app). This is
#                             primarily for link time configurations or if the
#                             source file is dependent on options/defines that are
#                             application dependent. This can be left blank or
#                             not defined at all, in which case, it means there
#                             no source files in the module <mod> that are required
#                             to be compiled in the application build stage.
#

dvr_rdk_PKG_LIST = dvr_rdk_bios6

#
# DVR_RDK Modules
#

# VSI SDK BIOS6 libraries
dvr_rdk_bios6_RELPATH = mcfw/src_bios6
dvr_rdk_bios6_PATH = $(dvr_rdk_PATH)/$(dvr_rdk_bios6_RELPATH)
dvr_rdk_bios6_INCLUDE = $(dvr_rdk_PATH)
dvr_rdk_bios6_PKG_LIST = dvr_rdk_bios6
dvr_rdk_bios6_PLATFORM_DEPENDENCY = yes
dvr_rdk_bios6_CORE_DEPENDENCY = yes

# Component specific CFLAGS
CFLAGS_COMP_COMMON +=

SYSTEM_SIMCOP_ENABLE=yes

# CFLAGS based on platform selected
ifeq ($(PLATFORM),ti814x-evm)
 DVR_RDK_CFLAGS = -DTI_814X_BUILD -DPLATFORM_EVM_SI
endif
ifeq ($(PLATFORM),ti814x-sim)
 DVR_RDK_CFLAGS = -DTI_814X_BUILD -DPLATFORM_SIM
endif
ifeq ($(PLATFORM),ti816x-evm)
 DVR_RDK_CFLAGS = -DTI_816X_BUILD -DPLATFORM_EVM_SI
endif
ifeq ($(PLATFORM),ti816x-sim)
 DVR_RDK_CFLAGS = -DTI_816X_BUILD -DPLATFORM_SIM
endif
ifeq ($(PLATFORM),ti810x-evm)
 DVR_RDK_CFLAGS = -DTI_8107_BUILD -DPLATFORM_EVM_SI
 SYSTEM_SIMCOP_ENABLE=yes
endif
ifeq ($(PLATFORM),ti810x-sim)
 DVR_RDK_CFLAGS = -DTI_8107_BUILD -DPLATFORM_SIM
endif

DVR_RDK_CFLAGS += -DUTILS_ASSERT_ENABLE 

ifeq ($(SYSTEM_SIMCOP_ENABLE),yes)
	DVR_RDK_CFLAGS += -DSYSTEM_SIMCOP_ENABLE
endif

#
#  DVR_RDK_BOARD_TYPE definition
#

ifeq ($(DVR_RDK_BOARD_TYPE), DM816X_DVR)
DVR_RDK_CFLAGS += -DTI816X_DVR -DENABLE_FB -D_GNU_SOURCE
ifeq ($(DDR_MEM),DDR_MEM_2048M)
DVR_RDK_CFLAGS += -DTI816X_2G_DDR
endif
endif

ifeq ($(DVR_RDK_BOARD_TYPE), DM816X_EVM)
DVR_RDK_CFLAGS += -DTI816X_EVM
endif

ifeq ($(DVR_RDK_BOARD_TYPE),DM814X_EVM)
DVR_RDK_CFLAGS += -DTI814X_EVM
ifeq ($(VS_CARD),WITHOUT_VS_CARD)
DVR_RDK_CFLAGS += -DTI814X_EVM_WITHOUT_VS_CARD
endif
endif
ifeq ($(DVR_RDK_BOARD_TYPE),DM810X_EVM)
DVR_RDK_CFLAGS += -DTI8107_EVM 
ifeq ($(VS_CARD),WITHOUT_VS_CARD)
DVR_RDK_CFLAGS += -DTI8107_EVM_WITHOUT_VS_CARD 
endif
endif
ifeq ($(DVR_RDK_BOARD_TYPE),DM810X_DVR)
DVR_RDK_CFLAGS += -DTI8107_DVR 
endif

ifeq ($(DVR_RDK_BOARD_TYPE), DM814X_DVR)
DVR_RDK_CFLAGS += -DTI814X_DVR 
endif

ifeq ($(DDR_MEM),DDR_MEM_256M)
DVR_RDK_CFLAGS += -DTI816X_256M_DDR
endif

ifeq ($(DISABLE_AUDIO),yes)
DVR_RDK_CFLAGS += -DSYSTEM_DISABLE_AUDIO
endif

export DVR_RDK_LIBS
export DVR_RDK_EXAMPLE_LIBS
export DVR_RDK_CFLAGS

# Nothing beyond this point
