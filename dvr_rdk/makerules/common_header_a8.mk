# (c) Texas Instruments

ifndef $(COMMON_HEADER_MK)
COMMON_HEADER_MK = 1


CC=$(CODEGEN_PATH_A8)/bin/arm-arago-linux-gnueabi-gcc
AR=$(CODEGEN_PATH_A8)/bin/arm-arago-linux-gnueabi-ar
LD=$(CODEGEN_PATH_A8)/bin/arm-arago-linux-gnueabi-gcc

LIB_BASE_DIR=$(dvr_rdk_PATH)/lib/$(PLATFORM)
OBJ_BASE_DIR=$(LIB_BASE_DIR)/obj
EXE_BASE_DIR=$(TARGET_FS_DIR)/bin

ifeq ($(CONFIG),)
LIB_DIR=$(LIB_BASE_DIR)
else
LIB_DIR=$(LIB_BASE_DIR)/$(CONFIG)
endif

CC_OPTS=-c -Wall

ifeq ($(TREAT_WARNINGS_AS_ERROR), yes)

CC_OPTS+= -Werror

endif

ifeq ($(CONFIG), debug)

CC_OPTS+=-g
OPTI_OPTS=
DEFINE=-DDEBUG

else

CC_OPTS+=
OPTI_OPTS=-O3
DEFINE=

endif

AR_OPTS=-rc
LD_OPTS=-lpthread

DEFINE += $(DVR_RDK_CFLAGS)
FILES=$(subst ./, , $(foreach dir,.,$(wildcard $(dir)/*.c)) ) 

vpath %.a $(LIB_DIR)

include $(dvr_rdk_PATH)/makerules/includes_a8.mk

INCLUDE=
INCLUDE+=$(KERNEL_INC)
INCLUDE+=$(COMMON_INC)

endif # ifndef $(COMMON_HEADER_MK)

