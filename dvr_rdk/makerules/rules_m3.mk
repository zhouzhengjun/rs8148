# Filename: rules_m3.mk
#
# Make rules for M3 - This file has all the common rules and defines required
#                     for Cortex-M3 ISA
#
# This file needs to change when:
#     1. Code generation tool chain changes (currently it uses TMS470)
#     2. Internal switches (which are normally not touched) has to change
#     3. XDC specific switches change
#     4. a rule common for M3 ISA has to be added or modified

# Set compiler/archiver/linker commands and include paths
CODEGEN_INCLUDE = $(CODEGEN_PATH_M3)/include
CC = $(CODEGEN_PATH_M3)/bin/cl470
AR = $(CODEGEN_PATH_M3)/bin/ar470
LNK = $(CODEGEN_PATH_M3)/bin/lnk470

# Derive a part of RTS Library name based on ENDIAN: little/big
ifeq ($(ENDIAN),little)
  RTSLIB_ENDIAN = le
else
  RTSLIB_ENDIAN = be
endif

# Derive compiler switch and part of RTS Library name based on FORMAT: COFF/ELF
ifeq ($(FORMAT),COFF)
  CSWITCH_FORMAT = ti_arm9_abi
  RTSLIB_FORMAT = tiarm9
endif
ifeq ($(FORMAT),ELF)
  CSWITCH_FORMAT = eabi
  RTSLIB_FORMAT = eabi
endif

# Internal CFLAGS - normally doesn't change
CFLAGS_INTERNAL = -c -qq -pdsw225 --endian=$(ENDIAN) -mv7M3 --abi=$(CSWITCH_FORMAT) -eo.$(OBJEXT) -ea.$(ASMEXT) --embed_inline_assembly  --pch
CFLAGS_DIROPTS = -fr=$(OBJDIR) -fs=$(OBJDIR) -ft=$(OBJDIR) --pch_dir=$(OBJDIR)

ifeq ($(TREAT_WARNINGS_AS_ERROR), yes)

CFLAGS_INTERNAL += --emit_warnings_as_errors

endif

ifneq ($(findstring $(ENABLE_FXN_PROFILE), TRUE true),)
CFLAGS_INTERNAL  += --entry_hook=FNPROF_entryHook --entry_parm=address --exit_hook=FNPROF_exitHook --exit_parm=address --remove_hooks_when_inlining
CFLAGS_APP_DEFINES += -DENABLE_FXN_PROFILE
endif

XDC_HFILE_NAME = $(basename $(XDC_CFG_FILE_$(CORE)))
# CFLAGS based on profile selected
ifeq ($(PROFILE_$(CORE)), debug)
 CFLAGS_XDCINTERNAL = -Dxdc_target_name__=M3 -Dxdc_target_types__=ti/targets/arm/elf/std.h -Dxdc_bld__profile_debug -Dxdc_bld__vers_1_0_4_6_1 -D_DEBUG_=1 --symdebug:dwarf
 ifndef MODULE_NAME
  CFLAGS_XDCINTERNAL += -Dxdc_cfg__header__='$(CONFIGURO_DIR)/package/cfg/$(XDC_HFILE_NAME)_pem3.h' 
 endif
 LNKFLAGS_INTERNAL_PROFILE =
endif
ifeq ($(PROFILE_$(CORE)), whole_program_debug)
 CFLAGS_XDCINTERNAL = -Dxdc_target_name__=M3 -Dxdc_target_types__=ti/targets/arm/elf/std.h -Dxdc_bld__profile_whole_program_debug -Dxdc_bld__vers_1_0_4_6_1 -ms -oe -O3 -k -op0 -os --symdebug:dwarf --optimize_with_debug --inline_recursion_limit=20
 ifndef MODULE_NAME
  CFLAGS_XDCINTERNAL += -Dxdc_cfg__header__='$(CONFIGURO_DIR)/package/cfg/$(XDC_HFILE_NAME)_pem3.h' 
 endif
 LNKFLAGS_INTERNAL_PROFILE = --opt='--endian=$(ENDIAN) -mv7M3 --abi=$(CSWITCH_FORMAT) -qq -pdsw225 $(CFLAGS_GLOBAL_$(CORE)) -oe --symdebug:dwarf -ms -op2 -O3 -k -os --optimize_with_debug --inline_recursion_limit=20 --diag_suppress=23000' --strict_compatibility=on
# LNKFLAGS_INTERNAL_PROFILE =
endif

ifeq ($(PROFILE_$(CORE)), release)
 CFLAGS_XDCINTERNAL = -Dxdc_target_name__=M3 -Dxdc_target_types__=ti/targets/arm/elf/std.h -Dxdc_bld__profile_release -Dxdc_bld__vers_1_0_4_6_1 -D_DEBUG_=1 -ms -oe -O3 -k -op0 -os --symdebug:dwarf --optimize_with_debug --inline_recursion_limit=20 --opt_for_speed=4
 ifndef MODULE_NAME
  CFLAGS_XDCINTERNAL += -Dxdc_cfg__header__='$(CONFIGURO_DIR)/package/cfg/$(XDC_HFILE_NAME)_pem3.h' 
 endif
 LNKFLAGS_INTERNAL_PROFILE =
endif


# Following 'if...' block is for an application; to add a #define for each
#   component in the build. This is required to know - at compile time - which
#   components are on which core.
ifndef MODULE_NAME
  # Derive list of all packages from each of the components needed by the app
  PKG_LIST_M3_LOCAL = $(foreach COMP,$(COMP_LIST_$(CORE)),$($(COMP)_PKG_LIST))

  # Defines for the app and cfg source code to know which components/packages
  # are included in the build for the local CORE...
  CFLAGS_APP_DEFINES = $(foreach PKG,$(PKG_LIST_M3_LOCAL),-D_LOCAL_$(PKG)_)
  CFLAGS_APP_DEFINES += $(foreach PKG,$(PKG_LIST_M3_LOCAL),-D_BUILD_$(PKG)_)

  ifeq ($(CORE),m3vpss)
    PKG_LIST_M3_REMOTE = $(foreach COMP,$(COMP_LIST_m3video),$($(COMP)_PKG_LIST))
    CFLAGS_APP_DEFINES += -D_LOCAL_CORE_m3vpss_
  endif
  ifeq ($(CORE),m3video)
    PKG_LIST_M3_REMOTE = $(foreach COMP,$(COMP_LIST_m3vpss),$($(COMP)_PKG_LIST))
    CFLAGS_APP_DEFINES += -D_LOCAL_CORE_m3video_
  endif
  PKG_LIST_A8_REMOTE = $(foreach COMP,$(COMP_LIST_a8host),$($(COMP)_PKG_LIST))

  # Defines for the app and cfg source code to know which components/packages
  # are included in the build for the remote CORE...
  CFLAGS_APP_DEFINES += $(foreach PKG,$(PKG_LIST_M3_REMOTE),-D_REMOTE_$(PKG)_)
  CFLAGS_APP_DEFINES += $(foreach PKG,$(PKG_LIST_M3_REMOTE),-D_BUILD_$(PKG)_)
  CFLAGS_APP_DEFINES += $(foreach PKG,$(PKG_LIST_A8_REMOTE),-D_REMOTE_$(PKG)_)
  CFLAGS_APP_DEFINES += $(foreach PKG,$(PKG_LIST_A8_REMOTE),-D_BUILD_$(PKG)_)
  CFLAGS_APP_DEFINES += $(foreach PKG,$(PKG_LIST_C674_REMOTE),-D_REMOTE_$(PKG)_)
  CFLAGS_APP_DEFINES += $(foreach PKG,$(PKG_LIST_C674_REMOTE),-D_BUILD_$(PKG)_)  
endif

# Assemble CFLAGS from all other CFLAGS definitions
_CFLAGS = $(CFLAGS_INTERNAL) $(CFLAGS_GLOBAL_$(CORE)) $(CFLAGS_XDCINTERNAL) $(CFLAGS_LOCAL_COMMON) $(CFLAGS_LOCAL_$(CORE)) $(CFLAGS_LOCAL_$(PLATFORM)) $(CFLAGS_LOCAL_$(SOC)) $(CFLAGS_APP_DEFINES) $(CFLAGS_COMP_COMMON) $(CFLAGS_GLOBAL_$(PLATFORM))

# Object file creation
# The first $(CC) generates the dependency make files for each of the objects
# The second $(CC) compiles the source to generate object
$(OBJ_PATHS): $(OBJDIR)/%.$(OBJEXT): %.c
	$(ECHO) \# Compiling $(PLATFORM):$(CORE):$(PROFILE_$(CORE)):$(APP_NAME)$(MODULE_NAME): $<
	$(CC) -ppd=$(DEPFILE).P $(_CFLAGS) $(INCLUDES) $(CFLAGS_DIROPTS) -fc $<
	$(CC) $(_CFLAGS) $(INCLUDES) $(CFLAGS_DIROPTS) -fc $<

# Archive flags - normally doesn't change
ARFLAGS = rq

# Archive/library file creation
$(LIBDIR)/$(MODULE_NAME).$(LIBEXT) : $(OBJ_PATHS)
	$(ECHO) \#
	$(ECHO) \# Archiving $(PLATFORM):$(CORE):$(PROFILE_$(CORE)):$(MODULE_NAME)
	$(ECHO) \#
	$(AR) $(ARFLAGS) $@ $(OBJ_PATHS)

# Linker options and rules
LNKFLAGS_INTERNAL_COMMON = -w -q -u _c_int00 --silicon_version=7M3 -c --dynamic

# Assemble Linker flags from all other LNKFLAGS definitions
_LNKFLAGS = $(LNKFLAGS_INTERNAL_COMMON) $(LNKFLAGS_INTERNAL_PROFILE) $(LNKFLAGS_GLOBAL_$(CORE)) $(LNKFLAGS_LOCAL_COMMON) $(LNKFLAGS_LOCAL_$(CORE))

# Path of the RTS library - normally doesn't change for a given tool-chain
RTSLIB_PATH = $(CODEGEN_PATH_M3)/lib/rtsv7M3_T_$(RTSLIB_ENDIAN)_$(RTSLIB_FORMAT).lib
LIB_PATHS += $(RTSLIB_PATH) $(h264dec_PATH)/Lib/h264vdec_ti.lib $(mpeg4dec_PATH)/Lib/mpeg4vdec_ti_host.lib $(h264enc_PATH)/Lib/h264enc_ti_host.lib $(jpegenc_PATH)/lib/jpegenc_ti_host.lib $(jpegdec_PATH)/lib/jpegvdec_ti_host.lib $(hdvicplib_PATH)/lib/ivahd_ti_api_vM3.lib

DVR_RDK_ISS_LIBS = $(iss_PATH)/packages/ti/psp/iss/alg/aewb/ti2a/awb/lib/release/iss_ti2a_awb.aem3 \
		$(iss_PATH)/packages/ti/psp/iss/alg/aewb/ti2a/fd/lib/release/iss_ti2a_fd.aem3 \
		$(iss_PATH)/packages/ti/psp/iss/alg/aewb/appro2a/lib/release/iss_appro2A.aem3 \
$(iss_PATH)/packages/ti/psp/iss/alg/vstab/lib/release/iss_vstab.aem3 \
		$(iss_PATH)/packages/ti/psp/iss/alg/glbce/lib/release/iss_glbce.aem3 \
		$(DVR_RDK_ISS_LIB_PATH)/cpis_core.aem3 \
		$(DVR_RDK_ISS_LIB_PATH)/cpis_imgproc.aem3 \
		$(DVR_RDK_ISS_LIB_PATH)/cpis_vision.aem3 \
		$(DVR_RDK_ISS_LIB_PATH)/gpp_core.aem3 \
		$(DVR_RDK_ISS_LIB_PATH)/gpp_imgproc.aem3 \
		$(DVR_RDK_ISS_LIB_PATH)/gpp_vision.aem3 \
		$(DVR_RDK_ISS_LIB_PATH)/iss_ip_run2_bios.aem3 \
		$(DVR_RDK_ISS_LIB_PATH)/iss_ip_run2_sched.aem3 \
		$(DVR_RDK_ISS_LIB_PATH)/iss_timmosal.aem3 \
		$(DVR_RDK_ISS_LIB_PATH)/swosd.aem3 \
		$(DVR_RDK_ISS_LIB_PATH)/scd.aem3 \
		$(DVR_RDK_ISS_LIB_PATH)/iss_imx.aem3 \
		$(DVR_RDK_ISS_LIB_PATH)/iss_devices.aem3 \
		$(DVR_RDK_ISS_LIB_PATH)/iss_drivers.aem3 \
		$(DVR_RDK_ISS_LIB_PATH)/iss_platform.aem3 \
		
ifeq ($(SYSTEM_SIMCOP_ENABLE),yes)
LIB_PATHS += $(DVR_RDK_ISS_LIBS)
endif

LNK_LIBS = $(addprefix -l,$(LIB_PATHS))

# Linker - to create executable file

ifeq ($(LOCAL_APP_NAME),)
 EXE_NAME = $(BINDIR)/$(APP_NAME)_$(CORE)_$(PROFILE_$(CORE)).$(EXEEXT)
else
 ifeq ($(PROFILE_$(CORE)),whole_program_debug)
  EXE_NAME = $(BINDIR)/$(LOCAL_APP_NAME).$(EXEEXT)
 else
  EXE_NAME = $(BINDIR)/$(LOCAL_APP_NAME)_$(PROFILE_$(CORE)).$(EXEEXT)
 endif
endif


ifeq ($(CORE),m3video)
	ifeq ($(SOC),ti816x)
		MISC_LNKCMD_INCLUDE = $(dvr_rdk_PATH)/mcfw/src_bios6/cfg/ti816x/link_codec.cmd
	endif
	ifeq ($(SOC),ti814x)
		MISC_LNKCMD_INCLUDE = 
	endif
	ifeq ($(SOC),ti8107)
		MISC_LNKCMD_INCLUDE = $(dvr_rdk_PATH)/mcfw/src_bios6/cfg/ti810x/link_codec.cmd
	endif
endif

ifeq ($(CORE),m3vpss)
	ifeq ($(SOC),ti816x)
		MISC_LNKCMD_INCLUDE = $(dvr_rdk_PATH)/mcfw/src_bios6/cfg/ti816x/link_vpss.cmd
	endif
	ifeq ($(SOC),ti814x)
		MISC_LNKCMD_INCLUDE = 
	endif
	ifeq ($(SOC),ti8107)
		MISC_LNKCMD_INCLUDE = $(dvr_rdk_PATH)/mcfw/src_bios6/cfg/ti810x/link_hdvpss.cmd
	endif
endif


$(EXE_NAME) : $(OBJ_PATHS) $(LIB_PATHS) $(LNKCMD_FILE) $(MISC_LNKCMD_INCLUDE) $(OBJDIR)/$(CFG_COBJ_XDC)
	$(ECHO) \# Linking into $(EXE_NAME)...
	$(ECHO) \#
	$(CP) $(OBJDIR)/$(CFG_COBJ_XDC) $(CONFIGURO_DIR)/package/cfg
ifeq ($(PROFILE_$(CORE)),whole_program_debug)
	$(LNK) $(_LNKFLAGS) $(OBJ_PATHS) $(OBJDIR)/$(CFG_COBJ_XDC) $(LNKCMD_FILE) -o $@ -m $@.map $(LNK_LIBS) $(MISC_LNKCMD_INCLUDE)
else
	$(LNK) $(_LNKFLAGS) $(OBJ_PATHS) $(LNKCMD_FILE) -o $@ -m $@.map $(LNK_LIBS) $(MISC_LNKCMD_INCLUDE)
endif
	$(ECHO) \#
	$(ECHO) \# $@ created.
	$(ECHO) \#

# XDC specific - assemble XDC-Configuro command
CONFIGURO_CMD = $(xdc_PATH)/xs xdc.tools.configuro --generationOnly -o $(CONFIGURO_DIR) -t $(TARGET_XDC) -p $(PLATFORM_XDC) \
               -r $(PROFILE_$(CORE)) -b $(CONFIG_BLD_XDC_$(ISA)) --cfgArgs $(CFGARGS_XDC) $(XDC_CFG_FILE_NAME)
_XDC_GREP_STRING = \"$(XDC_GREP_STRING)\"
EGREP_CMD = $(EGREP) -ivw $(XDC_GREP_STRING) $(XDCLNKCMD_FILE)
GEN_SHELL_SCRIPT_SRC_PATH   := $(CONFIGURO_DIR)/env.sh
GEN_SHELL_SCRIPT_DEST_PATH  := $(TARGET_FS_DIR)/env.sh

# Invoke configuro for the rest of the components
#  NOTE: 1. String handling is having issues with various make versions when the
#           cammand is directly tried to be given below. Hence, as a work-around,
#           the command is re-directed to a file (shell or batch file) and then
#           executed
#        2. The linker.cmd file generated, includes the libraries generated by
#           XDC. An egrep to search for these and omit in the .cmd file is added
#           after configuro is done
#$(CFG_CFILE_XDC) : $(XDC_CFG_FILE)
xdc_configuro : $(XDC_CFG_FILE)
	$(ECHO) \# Invoking configuro...
	$(ECHO) -e $(CONFIGURO_CMD) > $(DEST_ROOT)/maketemp_configuro_cmd_$(CORE).bat
	$(CHMOD) a+x $(DEST_ROOT)/maketemp_configuro_cmd_$(CORE).bat
	$(DEST_ROOT)/maketemp_configuro_cmd_$(CORE).bat
	$(CP) $(XDCLNKCMD_FILE) $(LNKCMD_FILE)
#	$(ECHO) @ $(EGREP_CMD) > maketemp_egrep_cmd.bat
#	./maketemp_egrep_cmd.bat | $(CYGWINPATH)/bin/tail -n+3 > $(LNKCMD_FILE)
#	$(EGREP_CMD) > $(LNKCMD_FILE)
#	$(EGREP) -iv "$(XDC_GREP_STRING)" $(XDCLNKCMD_FILE) > $(LNKCMD_FILE)
ifeq ($(CORE),m3vpss)
ifeq ($(OS),Windows_NT)
	dos2unix $(GEN_SHELL_SCRIPT_SRC_PATH)
endif
	$(CP) $(GEN_SHELL_SCRIPT_SRC_PATH) $(GEN_SHELL_SCRIPT_DEST_PATH)
endif
	$(ECHO) \# Configuro done!

$(LNKCMD_FILE) :
#	$(CP) $(XDCLNKCMD_FILE) $(LNKCMD_FILE)
#	$(ECHO) @ $(EGREP_CMD) > maketemp_egrep_cmd.bat
#	./maketemp_egrep_cmd.bat | $(CYGWINPATH)/bin/tail -n+3 > $(LNKCMD_FILE)
#	$(EGREP_CMD) > $(LNKCMD_FILE)

ifndef MODULE_NAME
$(OBJDIR)/$(CFG_COBJ_XDC) : $(CFG_C_XDC)
	$(ECHO) \# Compiling generated $(CFG_COBJ_XDC)
	$(CC) -ppd=$(DEPFILE).P $(_CFLAGS) $(INCLUDES) $(CFLAGS_DIROPTS) -fc $(CFG_C_XDC)
	$(CC) $(_CFLAGS) $(INCLUDES) $(CFLAGS_DIROPTS) -fc $(CFG_C_XDC)
endif

# Include dependency make files that were generated by $(CC)
-include $(SRCS:%.c=$(DEPDIR)/%.P)

# Nothing beyond this point
