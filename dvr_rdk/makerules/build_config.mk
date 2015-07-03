# Filename: build_config.mk
#
# Build configuration make file - This file contains all the options that are
#                                 frequently changed
#
# This file changes when:
#     1. format needs to change (eg: COFF/ELF)
#     2. Endian-ness needs to change (little/big)
#     3. global compiler and linker options need to change (one entry for each
#                                                           core)
#     4. Profile needs to change (debug/whole_program_debug/release)
#     5. Build option changes (xdc/make/full)
#

# Global options

# Build - Allowed values = full | xdc | make
#         This "builds" packages based on the setting:
#                 full - build both XDC and make (all the packages/components)
#                 xdc  - build and configure only XDC packages
#                 make - build only make based components and link everything
# NOTE: BUILD need not be defined at all, which case it behaves like it is set
#       to "full". This is to ensure backward compatibility.
BUILD = full

# Endianness : Allowed values = little | big
ENDIAN = little

# Format : Allowed values = COFF | ELF
FORMAT = ELF

# Platform memory: available external memory
PLATFORM_MEM = $(DDR_MEM)

#
# Platform specific
#
CFLAGS_GLOBAL_ti816x-evm = -DTI_816X_BUILD -DPLATFORM_EVM_SI
CFLAGS_GLOBAL_ti814x-evm = -DTI_814X_BUILD -DPLATFORM_EVM_SI
CFLAGS_GLOBAL_ti810x-evm = -DTI_8107_BUILD -DPLATFORM_EVM_SI
CFLAGS_GLOBAL_ti816x-sim = -DTI_816X_BUILD -DPLATFORM_SIM
CFLAGS_GLOBAL_ti814x-sim = -DTI_814X_BUILD -DPLATFORM_SIM
CFLAGS_GLOBAL_ti810x-sim = -DTI_8107_BUILD -DPLATFORM_SIM

#
# Core specific options - Cores of ti816x (Netra) and ti814x (Centaurus)
#

# m3video - Ducati - Core 0 (Cortex-M3)

# Profile: Allowed values = debug | whole_program_debug | release
CFLAGS_GLOBAL_m3video =  -g -ms --gcc -D___DSPBIOS___ -D___DUCATI_FW___ -DSYSLINK_USE_TILER -DMULTICHANNEL_OPT=1
LNKFLAGS_GLOBAL_m3video = -x --zero_init=off --retain=_Ipc_ResetVector

# m3vpss - Ducati - Core 1 (Cortex-M3)

# Profile: Allowed values = debug | whole_program_debug | release
CFLAGS_GLOBAL_m3vpss =  -g -ms --gcc -D___DSPBIOS___ -D___DUCATI_FW___ -DSYSLINK_USE_TILER -DMULTICHANNEL_OPT=1
LNKFLAGS_GLOBAL_m3vpss = -x --zero_init=off --retain=_Ipc_ResetVector

# a8host - Cortex-A8

# Profile: Allowed values = debug
PROFILE_a8host = release
CFLAGS_GLOBAL_a8host = -c -x c -Wunused -Wall -O3
LNKFLAGS_GLOBAL_a8host =

# c6xdsp - DSP core
# Profile: Allowed values = debug | whole_program_debug | release
CFLAGS_GLOBAL_c6xdsp =  -g --gcc -D___DSPBIOS___
LNKFLAGS_GLOBAL_c6xdsp = -x --zero_init=off --retain=_Ipc_ResetVector

# Nothing beyond this point
