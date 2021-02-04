TEMPLATE_PATH := ./
include $(TEMPLATE_PATH)/Makefile.param

PROJECT_NAME     := hal_libs
TARGETS          :=$(MODULES_NAME)
OUTPUT_DIRECTORY := build
SDK_ROOT := ../../platform/sdk
GOVEE_HAL:= ..
GOVEE_HAL_FILE:= ../

LINKER_SCRIPT  := ldscript.ld
MODULES_DIR:=../../platform
INSTALL_LIBS=$(MODULES_DIR)/libs
INSTALL_INC=$(MODULES_DIR)/include
SDK_INC=$(MODULES_DIR)/sdk_inc
SDK_ROOT_LIB:=$(GOVEE_HAL)/ota_pack/libfr8010h_stack.a

SDK_DRIVER_FILE_C=$(foreach dir, $(SDK_DRIVER_FILE), $(wildcard $(GOVEE_HAL_FILE)$(dir)/*.c)) 
SRC_FILES_COMM +=\
	   #$(MODULES_DIR)/common/hardware.c \
	   #$(MODULES_DIR)/common/gpio.c 
INC_COMM += \
	   #$(MODULES_DIR)/common

SRC_HAL_FILE_C += $(foreach dir, $(SRC_HAL_FILE_DIRS), $(wildcard $(GOVEE_HAL_FILE)$(dir)/*.c)) 

#driver
SRC_FILES += \
              $(SDK_ROOT)/components/ble/profiles/ble_ota/ota_service.c \
              $(SDK_ROOT)/components/ble/profiles/ble_ota/ota.c \
              $(SDK_ROOT)/components/ble/profiles/ble_simple_profile/simple_gatt_service.c \
              $(SDK_ROOT)/components/modules/platform/source/exception_handlers.c \
              $(SDK_ROOT)/components/modules/platform/source/app_boot_vectors.c \
              $(SDK_ROOT)/components/modules/patch/patch.c 

SDK_ROOT_COMPONENTS_DRIVER=$(SDK_ROOT)/components/driver
SDK_ROOT_DRIVER_DIRS = $(shell find $(SDK_ROOT_COMPONENTS_DRIVER) -type d)
SRC_FILES += $(foreach dir, $(SDK_ROOT_DRIVER_DIRS), $(wildcard $(dir)/*.c))

#SDK_ROOT_COMPONENTS_BLE=$(SDK_ROOT)/components/ble
#SDK_ROOT_BLE_DIRS = $(shell find $(SDK_ROOT_COMPONENTS_BLE) -type d)
#SRC_FILES += $(foreach dir, $(SDK_ROOT_BLE_DIRS), $(wildcard $(dir)/*.c))

SRC_FILES += $(SRC_HAL_FILE_C)
SRC_FILES += $(SRC_FILES_COMM)
# Include folders common to all targets
HAL_RGB_FOLDERS += $(foreach dir, $(SRC_HAL_FILE_DIRS), $(wildcard $(GOVEE_HAL_FILE)$(dir))) 

SDK_INC_FOLDERS += \
  $(SDK_ROOT)/components/ble/include \
  $(SDK_ROOT)/components/ble/profiles/ble_ota \
  $(SDK_ROOT)/components/modules/os/include \
  $(SDK_ROOT)/components/modules/sys/include \
  $(SDK_ROOT)/components/modules/platform/include \
  $(SDK_ROOT)/components/modules/common/include \
  $(SDK_ROOT)/components/modules/lowpow/include \
  $(SDK_ROOT)/components/ble/include/gap \
  $(SDK_ROOT)/components/ble/include/gatt \
  $(SDK_ROOT)/components/ble/profiles/ble_simple_profile \

INC_FOLDERS +=$(SDK_INC_FOLDERS)
INC_FOLDERS +=$(HAL_RGB_FOLDERS)
#INC_FOLDERS +=$(SDK_ROOT_BLE_DIRS)
INC_FOLDERS +=$(SDK_ROOT_DRIVER_DIRS)

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

INSTALL_INC_FILES += $(foreach dir, $(SRC_HAL_FILE_DIRS), $(wildcard $(GOVEE_HAL_FILE)$(dir)/*.h)) 
SDK_INC_FILES += $(foreach dir, $(SDK_INC_FOLDERS), $(wildcard $(dir)/*.h)) 

$(foreach target, $(TARGETS), $(call define_target, $(target)))

