TEMPLATE_PATH := ./
include $(TEMPLATE_PATH)/Makefile.param

PROJECT_NAME     := hal_libs
TARGETS          :=$(MODULES_NAME)
OUTPUT_DIRECTORY := build
SDK_ROOT := ../../../FR801xH-SDK
LINKER_SCRIPT  := ldscript.ld
MODULES_DIR:=../../modules
INSTALL_LIBS=$(MODULES_DIR)/../libs
INSTALL_INC=$(MODULES_DIR)/../include



SRC_FILES_COMM +=\
	   $(MODULES_DIR)/common/hardware.c \
	   $(MODULES_DIR)/common/gpio.c 
INC_COMM += \
	   $(MODULES_DIR)/common


SRC_FILES += $(SRC_FILES_COMM)
# Include folders common to all targets
INC_FOLDERS += \
  $(SDK_ROOT)/components/driver/include \
  $(SDK_ROOT)/components/modules/os/include \
  $(SDK_ROOT)/components/modules/sys/include \
  $(SDK_ROOT)/components/modules/platform/include \
  $(SDK_ROOT)/components/modules/common/include \
  $(SDK_ROOT)/components/modules/lowpow/include \
  $(INC_COMM) \
  $(SDK_ROOT)/components/modules/button 

# Libraries common to all targets
LIB_FILES += -lfr8010h_stack

# Optimization flags
OPT = -Os -g

# C flags common to all targets
CFLAGS += $(OPT)
CFLAGS += -mcpu=cortex-m3
CFLAGS += -mthumb
# keep every function in a separate section, this allows linker to discard unused ones
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += -fmessage-length=0 -fsigned-char
CFLAGS += -std=gnu11
ifneq ($(origin MODULES_KEYS), undefined)
	CFLAGS += -DHAL_KEYS_CONFIG_
endif

ifneq ($(origin MODULES_LIGHTS), undefined)
	CFLAGS += -DHAL_LIGHTS_CONFIG_
endif

# Assembler flags common to all targets
ASMFLAGS += -g3
ASMFLAGS += -mcpu=cortex-m3
ASMFLAGS += -mthumb

# Linker flags
LDFLAGS += $(OPT)
LDFLAGS += -mthumb
LDFLAGS += -mcpu=cortex-m3
LDFLAGS +=  -T$(LINKER_SCRIPT) -L$(SDK_ROOT)/components/ble/library
LDFLAGS += $(SDK_ROOT)/components/ble/library/syscall_gcc.txt
# let linker dump unused sections
LDFLAGS += -Wl,--gc-sections


include $(TEMPLATE_PATH)/Makefile.common
#TEST_DIR=light keyboard
SRC_DIR :=$(foreach dir_name, $(TARGETS), $(shell find ../ -name $(dir_name) -type d))
#SRC_DIR_IFILES :=$(foreach find_name, $(SRC_DIR), $(wildcard $(find_name)/*.h))
INSTALL_INC_FILES :=$(wildcard $(INC_COMM)/*.h) \
	$(foreach find_name, $(SRC_DIR), $(wildcard $(find_name)/*.h))
$(foreach target, $(TARGETS), $(call define_target, $(target)))
