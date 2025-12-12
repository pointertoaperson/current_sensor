# AVR Makefile - build main from ../src, headers in ../inc

# Device / port
MCU = atmega328p
PORT = /dev/ttyUSB0

# Compiler / flags
CC = avr-gcc
SYMBOLS = -DF_CPU=16000000UL
INC = ./inc
SRC = ./src

CFLAGS = -Os -mmcu=$(MCU) -I"$(INC)" $(SYMBOLS) -Wall -Wextra -Wundef -pedantic \
    -funsigned-char -funsigned-bitfields -ffunction-sections -fdata-sections \
    -fpack-struct -fshort-enums
LFLAGS = -mmcu=$(MCU)

# Tools
HC = avr-objcopy
HFLAGS = -j .text -j .data -O ihex
SIZE = avr-size -C --mcu=$(MCU)
PROG = avrdude -P"$(PORT)" -p$(MCU) -carduino -b57600

# Build layout
BUILD_DIR = build
SRCS = $(wildcard $(SRC)/*.c)
OBJS = $(patsubst $(SRC)/%.c,$(BUILD_DIR)/%.o,$(SRCS))
TARGET = $(BUILD_DIR)/main.elf
HEX = $(BUILD_DIR)/main.hex

# Default
all: $(HEX)

# ensure build dir exists
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

# compile each source into build/*.o
$(BUILD_DIR)/%.o: $(SRC)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

# link
$(TARGET): $(OBJS)
	$(CC) $(LFLAGS) -o $@ $^
	$(SIZE) $@

# hex
$(HEX): $(TARGET)
	$(HC) $(HFLAGS) $< $@

# upload
upload: $(HEX)
	$(PROG) -Uflash:w:"$(HEX)":i

# clean
.PHONY: clean all upload
clean:
	rm -rf $(BUILD_DIR)
