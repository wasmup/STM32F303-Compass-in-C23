CROSS_COMPILE ?= arm-none-eabi-
CC := $(CROSS_COMPILE)gcc
HOST_CC ?= cc
OPENOCD ?= openocd
BUILD_DIR := build
ELF := $(BUILD_DIR)/magnetometer-compass.elf
SOURCES := $(wildcard src/*.c)
OBJECTS := $(patsubst src/%.c,$(BUILD_DIR)/%.o,$(SOURCES))
ARCH_FLAGS := -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard
CFLAGS := $(ARCH_FLAGS) -std=c23 -Os -g3 -Wall -Wextra -Wpedantic \
    -ffreestanding \
    -ffunction-sections -fdata-sections -MMD -MP
LDFLAGS := $(ARCH_FLAGS) -nostdlib -T linker.ld -Wl,--gc-sections,-Map=$(BUILD_DIR)/magnetometer-compass.map

.PHONY: all build run flash test clean
all:
	$(MAKE) clean
	$(MAKE) build
	$(MAKE) flash

build: $(ELF)

$(ELF): $(OBJECTS) linker.ld
	$(CC) $(LDFLAGS) $(OBJECTS) -lgcc -o $@

$(BUILD_DIR)/%.o: src/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

run flash: build
	$(OPENOCD) -f stm32f3.cfg -c "program $(ELF) verify reset exit"

test:
	@mkdir -p $(BUILD_DIR)
	$(HOST_CC) -std=c23 -Wall -Wextra -Wpedantic -Isrc tests/logic.c src/compass.c -o $(BUILD_DIR)/logic-tests
	$(BUILD_DIR)/logic-tests

clean:
	rm -rf $(BUILD_DIR)

-include $(OBJECTS:.o=.d)
