# Filename: platforms.mk
#
# Platforms make file - Platform/SoC/targets are defined/derived in this file
#
# This file needs to change when:
#     1. a new platform/SoC is added, which also might have its own cores/ISAs
#

#
# Derive SOC from PLATFORM
#


# ti816x (Netra) Simulator
ifneq (,$(findstring ti816x,$(PLATFORM)))
 SOC = ti816x
endif

# ti816x (Netra) Simulator
ifneq (,$(findstring ti814x,$(PLATFORM)))
 SOC = ti814x
endif

# ti814x (Centaurus) Simulator
ifeq ($(PLATFORM),ti814x-sim)
 SOC = ti814x
endif

# ti810x (dm385) Simulatori
ifneq (,$(findstring ti810x,$(PLATFORM)))
 SOC = ti8107
endif

# Derive Target/ISA from CORE

# m3vpss
ifeq ($(CORE),m3vpss)
 ISA = m3
endif

# m3video
ifeq ($(CORE),m3video)
 ISA = m3
endif

# a8host
ifeq ($(CORE),a8host)
 ISA = a8
endif

# DSP 
# c6xdsp
ifneq ($(PLATFORM),ti810x-evm)
  ifeq ($(CORE),c6xdsp)
    ISA = c674
  endif
endif

#
# Derive XDC/ISA specific settings
#

ifneq ($(PLATFORM),ti810x-evm)
  ifeq ($(ISA),c674)
    ifeq ($(CORE),c6xdsp)
      ifeq ($(SOC),ti814x)
        PLATFORM_XDC = "ti.platforms.evmTI814X:plat"
      endif
      ifeq ($(SOC),ti816x)
        PLATFORM_XDC = "ti.platforms.evmTI816X:plat"
      endif
    endif
  endif
endif

ifeq ($(ISA),m3)
  ifeq ($(FORMAT),ELF)
    TARGET_XDC = ti.targets.arm.elf.M3
    FORMAT_EXT = e
  else
    TARGET_XDC = ti.targets.arm.M3
  endif
  ifeq ($(SOC),ti816x)
    ifeq ($(CORE),m3video)
      PLATFORM_XDC = "ti.platforms.evmTI816X:core0"
    else
      PLATFORM_XDC = "ti.platforms.evmTI816X:core1"
    endif
  endif

  ifeq ($(SOC),ti814x)
    ifeq ($(CORE),m3video)
      PLATFORM_XDC = "ti.platforms.evmTI814X:core0"
    else
      PLATFORM_XDC = "ti.platforms.evmTI814X:core1"
    endif
  endif
# TBD
  ifeq ($(SOC),ti8107)
    ifeq ($(CORE),m3video)
      PLATFORM_XDC = "ti.platforms.evmTI814X:core0"
    else
      PLATFORM_XDC = "ti.platforms.evmTI814X:core1"
    endif
  endif
  # If ENDIAN is set to "big", set ENDIAN_EXT to "e", that would be used in
  #    in the filename extension of object/library/executable files
  ifeq ($(ENDIAN),big)
    ENDIAN_EXT = e
  endif

  # Define the file extensions
  OBJEXT = o$(FORMAT_EXT)$(ISA)$(ENDIAN_EXT)
  LIBEXT = a$(FORMAT_EXT)$(ISA)$(ENDIAN_EXT)
  EXEEXT = x$(FORMAT_EXT)$(ISA)$(ENDIAN_EXT)
  ASMEXT = s$(FORMAT_EXT)$(ISA)$(ENDIAN_EXT)
endif

ifneq ($(SOC),ti8107)
  ifeq ($(ISA),c674)
    ifeq ($(FORMAT),ELF)
      TARGET_XDC = ti.targets.elf.C674
      FORMAT_EXT = e
    else
      TARGET_XDC = ti.targets.C674
    endif
  
    # If ENDIAN is set to "big", set ENDIAN_EXT to "e", that would be used in
    #    in the filename extension of object/library/executable files
    ifeq ($(ENDIAN),big)
      ENDIAN_EXT = e
    endif

    # Define the file extensions
    OBJEXT = o$(FORMAT_EXT)674$(ENDIAN_EXT)
    LIBEXT = a$(FORMAT_EXT)674$(ENDIAN_EXT)
    EXEEXT = x$(FORMAT_EXT)674$(ENDIAN_EXT)
    ASMEXT = s$(FORMAT_EXT)674$(ENDIAN_EXT)
  endif
endif

ifeq ($(ISA),a8)
  TARGET_XDC = gnu.targets.arm.GCArmv5T
  PLATFORM_XDC = "host.platforms.arm"

  # Define the file extensions
  OBJEXT = ov5T
  LIBEXT = av5T
  EXEEXT = xv5T
  ASMEXT = sv5T
endif

ifeq (sim,$(findstring sim,$(PLATFORM)))
  OMX_PLATFORM = sim
endif

ifeq (ti816x-evm,$(findstring ti816x-evm,$(PLATFORM)))
 # Don't know why platform define for XDC is separate for A8. TO DO.
 ifeq ($(CORE),a8host)
  OMX_PLATFORM = evmSi
 else
  OMX_PLATFORM = evmSi816X
 endif
endif

ifeq ($(CORE),m3vpss)
  CFGARGS_XDC = \"{coreName: \\\"VPSS-M3\\\", platformMem: \\\"$(PLATFORM_MEM)\\\" , enableFxnProfileHook: \\\"$(ENABLE_FXN_PROFILE)\\\"}\"
endif
ifeq ($(CORE),m3video)
  CFGARGS_XDC = \"{coreName:\\\"VIDEO-M3\\\", platformMem: \\\"$(PLATFORM_MEM)\\\" , enableFxnProfileHook: \\\"$(ENABLE_FXN_PROFILE)\\\"}\"
endif

ifneq ($(SOC),ti8107)
  ifeq ($(CORE),c6xdsp)
    CFGARGS_XDC = \"{coreName:\\\"DSP\\\", platformMem: \\\"$(PLATFORM_MEM)\\\"}\" 
  endif
endif

ifeq ($(CORE),a8host)
  CFGARGS_XDC = \"{coreName:\\\"HOST\\\", platformMem: \\\"$(PLATFORM_MEM)\\\"}\"
endif


# Nothing beyond this point
