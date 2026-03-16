##############################
# Toolchain
##############################

CC      := arm-none-eabi-gcc
OBJCOPY := arm-none-eabi-objcopy
OBJDUMP := arm-none-eabi-objdump
SIZE    := arm-none-eabi-size
NM      := arm-none-eabi-nm
READELF := arm-none-eabi-readelf

##############################
# Target configuration
##############################

CPU      := cortex-m4
FPU      := fpv4-sp-d16
FLOATABI := hard

TARGET := firmware
BUILD  ?= debug

SRC_DIR   := src
BUILD_DIR := build

LDSCRIPT := linkall.ld

##############################
# Source discovery
##############################

C_SRCS := $(wildcard $(SRC_DIR)/*.c)
S_SRCS := $(wildcard $(SRC_DIR)/*.s)

SRCS := $(C_SRCS) $(S_SRCS)

OBJS := $(patsubst $(SRC_DIR)/%,$(BUILD_DIR)/%,$(SRCS:.c=.o))
OBJS := $(OBJS:.s=.o)

##############################
# Compiler flags
##############################

ifeq ($(BUILD),debug)
	EXTRA_FLAGS := -Og -g3
else ifeq ($(BUILD),release)
	EXTRA_FLAGS := -Os
endif

ARCH_FLAGS := -mcpu=$(CPU) -mthumb -mfpu=$(FPU) -mfloat-abi=$(FLOATABI) -DSTM32F407xx

CFLAGS := $(ARCH_FLAGS)
CFLAGS += -pipe
CFLAGS += -std=c23
CFLAGS += -Wall -Wextra -Wpedantic
CFLAGS += -Wdouble-promotion -Wconversion
CFLAGS += -ffreestanding -fno-builtin
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += -fno-common
CFLAGS += -Iinc
CFLAGS += -Ivendor/CMSIS/Core/Include -Ivendor/Device/ST/STM32F4/Include
CFLAGS += $(EXTRA_FLAGS)

ASFLAGS := $(ARCH_FLAGS)
ASFLAGS += -x assembler-with-cpp
ASFLAGS += $(EXTRA_FLAGS)

##############################
# Linker flags
##############################

LDFLAGS := $(ARCH_FLAGS)
LDFLAGS += -nostdlib -nostartfiles
LDFLAGS += -T$(LDSCRIPT)
LDFLAGS += -Wl,-Map=$(BUILD_DIR)/$(TARGET).map
LDFLAGS += -Wl,--print-memory-usage

ifeq ($(BUILD),release)
	LDFLAGS += -Wl,--gc-sections
endif

##############################
# Output
##############################

ELF := $(BUILD_DIR)/$(TARGET).elf

##############################
# Default target
##############################

all: $(ELF)

##############################
# Linking
##############################

$(ELF): $(OBJS)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(LDFLAGS) $^ -o $@

##############################
# Compilation
##############################

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.s
	@mkdir -p $(BUILD_DIR)
	$(CC) $(ASFLAGS) -c $< -o $@

##############################
# Analysis
##############################

info: disasm symbols sections size

size: $(ELF)
	$(SIZE) $(ELF)

disasm: $(ELF)
	$(OBJDUMP) -h -S $(ELF) > $(BUILD_DIR)/$(TARGET).lss

symbols: $(ELF)
	$(NM) -n $(ELF) > $(BUILD_DIR)/$(TARGET).sym

sections: $(ELF)
	$(READELF) -S $(ELF) > $(BUILD_DIR)/$(TARGET).sec

##############################
# Flash
##############################

OPENOCD_INTERFACE ?= interface/stlink.cfg
OPENOCD_TARGET    ?= target/stm32f4x.cfg

flash: $(ELF)
	openocd \
		-f $(OPENOCD_INTERFACE) \
		-f $(OPENOCD_TARGET) \
		-c "init" \
		-c "reset init" \
		-c "program $(ELF) verify reset exit"

debug:
	openocd \
		-f $(OPENOCD_INTERFACE) \
		-f $(OPENOCD_TARGET) \
		-c "init" \
		-c "reset halt"

##############################
# Clean
##############################

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean flash info size disasm symbols sections
