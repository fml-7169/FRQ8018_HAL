PROJECT_NAME     := hal_libs
TARGETS          := libhal
OUTPUT_DIRECTORY := build

SDK_ROOT := ../../FR801xH-SDK

LINKER_SCRIPT  := ldscript.ld
HAL_DIR:=../../hal
HAL_LIBS=$(HAL_DIR)/libs
SRC_FILES += \
	   $(HAL_DIR)/modules/hardware.c \
	   $(HAL_DIR)/modules/gpio.c \
	   $(HAL_DIR)/modules/input/keyboard/gpio_keys.c \
	   $(HAL_DIR)/modules/light/light.c
# Include folders common to all targets
INC_FOLDERS += \
  $(SDK_ROOT)/components/driver/include \
  $(SDK_ROOT)/components/modules/os/include \
  $(SDK_ROOT)/components/modules/sys/include \
  $(SDK_ROOT)/components/modules/platform/include \
  $(SDK_ROOT)/components/modules/common/include \
  $(SDK_ROOT)/components/modules/lowpow/include \
  $(SDK_ROOT)/components/modules/button 

INC_FOLDERS += \
	   $(HAL_DIR)/include \
	   $(HAL_DIR)/modules/light \
	   $(HAL_DIR)/modules/input/keyboard
	   
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
CFLAGS += -DHAL_KEYS_CONFIG_

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

TEMPLATE_PATH := ./

include $(TEMPLATE_PATH)/Makefile.common

$(foreach target, $(TARGETS), $(call define_target, $(target)))
