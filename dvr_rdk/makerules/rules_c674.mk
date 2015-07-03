# Filename: rules_c674.mk
#
# Make rules for c674 - This file has all the common rules and defines required
#                     for DSP - c674 ISA
#
# This file needs to change when:
#     1. Code generation tool chain changes (currently it uses TI CGT)
#     2. Internal switches (which are normally not touched) has to change
#     3. XDC specific switches change
#     4. a rule common for c674 ISA has to be added or modified

# Set compiler/archiver/linker commands and include paths - Currently different
#  for ELF and COFF. In a future release of the toolchain, it would merge.
#ifeq ($(FORMAT),COFF)
 CODEGEN_PATH = $(CODEGEN_PATH_DSP)
 CODEGEN_INCLUDE = $(CODEGEN_PATH_DSP)/include
 CC = $(CODEGEN_PATH_DSP)/bin/cl6x 
 AR = $(CODEGEN_PATH_DSP)/bin/ar6x
 LNK = $(CODEGEN_PATH_DSP)/bin/lnk6x
#endif
#ifeq ($(FORMAT),ELF)
# CODEGEN_PATH = $(CODEGEN_PATH_DSPELF)
# CODEGEN_INCLUDE = $(CODEGEN_PATH_DSPELF)/include
# CC = $(CODEGEN_PATH_DSPELF)/bin/cl6x 
# AR = $(CODEGEN_PATH_DSPELF)/bin/ar6x
# LNK = $(CODEGEN_PATH_DSPELF)/bin/lnk6x
#endif


# Derive a part of RTS Library name based on ENDIAN: little/big
ifeq ($(ENDIAN),little)
  #RTSLIB_ENDIAN =
else
  RTSLIB_ENDIAN = e
endif

# Derive compiler switch and part of RTS Library name based on FORMAT: COFF/ELF
ifeq ($(FORMAT),COFF)
  CSWITCH_FORMAT = 
  #RTSLIB_FORMAT = 
  XDCINTERNAL_DEFINES = -Dxdc_target_types__=ti/targets/std.h -Dxdc_bld__vers_1_0_7_0_0
endif
ifeq ($(FORMAT),ELF)
  CSWITCH_FORMAT = --abi=elfabi 
  RTSLIB_FORMAT = _elf
  XDCINTERNAL_DEFINES = -Dxdc_target_types__=ti/targets/elf/std.h -Dxdc_bld__vers_1_0_7_2_0_10271
endif

# XDC Specific defines
ifneq ($(XDC_CFG_FILE_$(CORE)),)
  ifeq ($(PROFILE_$(CORE)),debug)
    CFG_CFILENAMEPART_XDC =p$(FORMAT_EXT)674
    #CFG_CFILENAMEPART_XDC =p$(FORMAT_EXT)674$(ENDIAN_EXT)    
  endif
  ifeq ($(PROFILE_$(CORE)),release)
    CFG_CFILENAMEPART_XDC =p$(FORMAT_EXT)674
    #CFG_CFILENAMEPART_XDC =p$(FORMAT_EXT)674$(ENDIAN_EXT)    
  endif
  ifeq ($(PROFILE_$(CORE)),whole_program_debug)
    CFG_CFILENAMEPART_XDC =p$(FORMAT_EXT)$(ISA)$(ENDIAN_EXT)
    CFG_LNKFILENAMEPART_XDC=_x
  endif
  CFG_CFILE_XDC =$(patsubst %.cfg,%_$(CFG_CFILENAMEPART_XDC).c,$(notdir $(XDC_CFG_FILE_$(CORE))))
  CFG_C_XDC = $(addprefix $(CONFIGURO_DIR)/package/cfg/,$(CFG_CFILE_XDC))
  #XDCLNKCMD_FILE =$(patsubst %.c, %$(CFG_LNKFILENAMEPART_XDC)_x.xdl, $(CFG_C_XDC))
  XDCLNKCMD_FILE =$(patsubst %.c, %$(CFG_LNKFILENAMEPART_XDC).xdl, $(CFG_C_XDC))
  CFG_COBJ_XDC = $(patsubst %.c,%.$(OBJEXT),$(CFG_CFILE_XDC))
#  OBJ_PATHS += $(CFG_COBJ_XDC)
  LNKCMD_FILE = $(CONFIGURO_DIR)/linker_mod.cmd
  SPACE := 
  SPACE += 
#  XDC_GREP_STRING = $(CONFIGURO_DIRNAME)
  XDC_GREP_STRING = $(CFG_COBJ_XDC)
#  XDC_GREP_STRING = $(subst $(SPACE),\|,$(COMP_LIST_$(CORE)))
#  XDC_GREP_STRING += \|$(CONFIGURO_DIRNAME)
endif

# Internal CFLAGS - normally doesn't change
CFLAGS_INTERNAL = -c -qq -pdsw225 -mv6740 $(CSWITCH_FORMAT) -mo -eo.$(OBJEXT) -ea.$(ASMEXT) --embed_inline_assembly  --pch
CFLAGS_DIROPTS = -fr=$(OBJDIR) -fs=$(OBJDIR) -ft=$(OBJDIR) --pch_dir=$(OBJDIR)

ifeq ($(TREAT_WARNINGS_AS_ERROR), yes)

CFLAGS_INTERNAL += --emit_warnings_as_errors

endif

XDC_HFILE_NAME = $(basename $(XDC_CFG_FILE_$(CORE)))

# CFLAGS based on profile selected
ifeq ($(PROFILE_$(CORE)), debug)
#CFLAGS_INTERNAL += --symdebug:dwarf
CFLAGS_INTERNAL += --symdebug:dwarf -O3 -ms0 --opt_for_speed=5 --optimize_with_debug
 CFLAGS_XDCINTERNAL = -Dxdc_target_name__=C674  -Dxdc_bld__profile_debug -D_DEBUG_=1 
 ifndef MODULE_NAME
 CFLAGS_XDCINTERNAL += -Dxdc_cfg__header__='$(CONFIGURO_DIR)/package/cfg/$(XDC_HFILE_NAME)_pe674.h' 
 endif
 LNKFLAGS_INTERNAL_PROFILE = 
endif
ifeq ($(PROFILE_$(CORE)), release)
 CFLAGS_XDCINTERNAL = -Dxdc_target_name__=C674 -Dxdc_bld__profile_release -DGENERIC -ms -oe -O3 -k -op0 -os --symdebug:dwarf --optimize_with_debug --inline_recursion_limit=20 --opt_for_speed=4 
# ifndef MODULE_NAME
 CFLAGS_XDCINTERNAL += -Dxdc_cfg__header__='$(CONFIGURO_DIR)/package/cfg/$(XDC_HFILE_NAME)_pe674.h' 
# endif
 LNKFLAGS_INTERNAL_PROFILE = -o2
 #LNKFLAGS_INTERNAL_PROFILE = --opt='--endian=$(ENDIAN) --abi=$(CSWITCH_FORMAT) -qq -pdsw225 $(CFLAGS_GLOBAL_$(CORE)) -oe --symdebug:dwarf -ms -op2 -O3 -k -os --optimize_with_debug --inline_recursion_limit=20 --diag_suppress=23000' --strict_compatibility=on
# LNKFLAGS_INTERNAL_PROFILE = 
endif
CFLAGS_XDCINTERNAL += $(XDCINTERNAL_DEFINES)

# Following 'if...' block is for an application; to add a #define for each
#   component in the build. This is required to know - at compile time - which
#   components are on which core.
ifndef MODULE_NAME
  # Derive list of all packages from each of the components needed by the app
  PKG_LIST_C674_LOCAL = $(foreach COMP,$(COMP_LIST_$(CORE)),$($(COMP)_PKG_LIST))
  
  # Defines for the app and cfg source code to know which components/packages
  # are included in the build for the local CORE...
  CFLAGS_APP_DEFINES = $(foreach PKG,$(PKG_LIST_C674_LOCAL),-D_LOCAL_$(PKG)_)
  CFLAGS_APP_DEFINES += $(foreach PKG,$(PKG_LIST_C674_LOCAL),-D_BUILD_$(PKG)_)
  
  ifeq ($(CORE),m3vpss)
    PKG_LIST_M3_REMOTE = $(foreach COMP,$(COMP_LIST_m3video),$($(COMP)_PKG_LIST))
    PKG_LIST_C674_REMOTE = $(foreach COMP,$(COMP_LIST_c6xdsp),$($(COMP)_PKG_LIST))    
    CFLAGS_APP_DEFINES += -D_LOCAL_CORE_m3vpss_
  endif
  ifeq ($(CORE),m3video)
    PKG_LIST_M3_REMOTE = $(foreach COMP,$(COMP_LIST_m3vpss),$($(COMP)_PKG_LIST))
    PKG_LIST_C674_REMOTE = $(foreach COMP,$(COMP_LIST_c6xdsp),$($(COMP)_PKG_LIST))    
    CFLAGS_APP_DEFINES += -D_LOCAL_CORE_m3video_
  endif
  ifeq ($(CORE),c6xdsp)
    PKG_LIST_M3_REMOTE = $(foreach COMP,$(COMP_LIST_m3video),$($(COMP)_PKG_LIST))
    PKG_LIST_M3_REMOTE += $(foreach COMP,$(COMP_LIST_m3vpss),$($(COMP)_PKG_LIST))
    CFLAGS_APP_DEFINES += -D_LOCAL_CORE_c6xdsp_
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

CFLAGS_APP_DEFINES += -D_LOCAL_CORE_c6xdsp_

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
LNKFLAGS_INTERNAL_COMMON = --warn_sections -q --silicon_version=6740 -c  --dynamic

# Assemble Linker flags from all other LNKFLAGS definitions
_LNKFLAGS = $(LNKFLAGS_INTERNAL_COMMON) $(LNKFLAGS_INTERNAL_PROFILE) $(LNKFLAGS_GLOBAL_$(CORE)) $(LNKFLAGS_LOCAL_COMMON) $(LNKFLAGS_LOCAL_$(CORE)) 

# Path of the RTS library - normally doesn't change for a given tool-chain
RTSLIB_PATH = $(CODEGEN_PATH)/lib/rts6740$(RTSLIB_ENDIAN)$(RTSLIB_FORMAT).lib
LIB_PATHS += $(RTSLIB_PATH)  \
             $(fc_INCLUDE)/ti/sdo/fc/ecpy/lib/debug/ecpy.ae674 \
             $(dvr_rdk_PATH)/mcfw/src_bios6/alg/scd/lib/scd.ae674\
             $(rpe_PATH)/lib/lib/c674/debug/rpe.ae674\
             $(aaclcdec_PATH)/packages/ti/sdo/codecs/aaclcdec/lib/aacdec_tii_lc_elf.l64P\
             $(aaclcenc_PATH)/packages/ti/sdo/codecs/aaclcenc/lib/mp4aacenc_tij_lc_elf.l67
             
LNK_LIBS = $(addprefix -l,$(LIB_PATHS))

# Linker - to create executable file 
ifeq ($(LOCAL_APP_NAME),)
 EXE_NAME = $(BINDIR)/$(APP_NAME)_$(CORE)_$(PROFILE_$(CORE)).$(EXEEXT)
 ALG_LNKCMD = $(dvr_rdk_PATH)/mcfw/src_bios6/cfg/ti816x/link_algs.cmd
else
 ifeq ($(PROFILE_$(CORE)),whole_program_debug)
  EXE_NAME = $(BINDIR)/$(LOCAL_APP_NAME).$(EXEEXT)
  ALG_LNKCMD = $(dvr_rdk_PATH)/mcfw/src_bios6/cfg/ti816x/link_algs.cmd
 else
  EXE_NAME = $(BINDIR)/$(LOCAL_APP_NAME)_$(PROFILE_$(CORE)).$(EXEEXT)
  ALG_LNKCMD = $(dvr_rdk_PATH)/mcfw/src_bios6/cfg/ti816x/link_algs.cmd
 endif
 ALG_LNKCMD = $(dvr_rdk_PATH)/mcfw/src_bios6/cfg/ti816x/link_algs.cmd
endif

$(EXE_NAME) : $(OBJ_PATHS) $(LIB_PATHS) $(LNKCMD_FILE) $(ALG_LNKCMD) $(OBJDIR)/$(CFG_COBJ_XDC)
$(EXE_NAME) : $(OBJ_PATHS) $(LIB_PATHS) $(LNKCMD_FILE) $(ALG_LNKCMD) $(OBJDIR)/$(CFG_COBJ_XDC)
	$(ECHO) \# Linking into $(EXE_NAME)...
#	$(ECHO) $(_CFLAGS)
	$(ECHO) \#
	$(CP) $(OBJDIR)/$(CFG_COBJ_XDC) $(CONFIGURO_DIR)/package/cfg
ifeq ($(PROFILE_$(CORE)),whole_program_debug)
	$(LNK) $(_LNKFLAGS) $(OBJ_PATHS) $(OBJDIR)/$(CFG_COBJ_XDC) $(LNKCMD_FILE) -o $@ -m $@.map $(LNK_LIBS) $(ALG_LNKCMD)
else
	$(LNK) $(_LNKFLAGS) $(OBJ_PATHS) $(OBJDIR)/$(CFG_COBJ_XDC) $(LNKCMD_FILE) -o $@ -m $@.map $(LNK_LIBS) $(ALG_LNKCMD)
endif
	$(ECHO) \#
	$(ECHO) \# $@ created.
	$(ECHO) \#


# XDC specific - assemble XDC-Configuro command
CONFIGURO_CMD = $(xdc_PATH)/xs xdc.tools.configuro --generationOnly -o $(CONFIGURO_DIR) -t $(TARGET_XDC) -p $(PLATFORM_XDC) \
                -r $(PROFILE_$(CORE)) -b $(CONFIG_BLD_XDC_$(ISA)) --cfgArgs $(CFGARGS_XDC) $(XDC_CFG_FILE_NAME)
_XDC_GREP_STRING = \"$(XDC_GREP_STRING)\"
EGREP_CMD = $(EGREP) -ivw $(XDC_GREP_STRING) $(XDCLNKCMD_FILE)

#ifneq ($(DEST_ROOT),)
 #DEST_ROOT += /
#endif
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
#	$(ECHO) $(EGREP_CMD) 
#	$(ECHO) $(LNKCMD_FILE)
#	$(ECHO) ***$(CFG_LNKFILENAMEPART_XDC)***
#	$(CP) $(XDCLNKCMD_FILE) $(LNKCMD_FILE)
#	$(ECHO) @ $(EGREP_CMD) > maketemp_egrep_cmd.bat
#	./maketemp_egrep_cmd.bat | $(CYGWINPATH)/bin/tail -n+3 > $(LNKCMD_FILE)
	$(EGREP_CMD) > $(LNKCMD_FILE)
#	$(EGREP) -iv "$(XDC_GREP_STRING)" $(XDCLNKCMD_FILE) > $(LNKCMD_FILE)
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
