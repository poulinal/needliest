# Minimal AVR Makefile for needliest
CC := /Users/alexpoulin/local/avr/bin/avr-g++
OBJCOPY := /Users/alexpoulin/local/avr/bin/avr-objcopy
AVRDUDE := /Users/alexpoulin/local/avr/bin/avrdude

MCU := atmega328p
PORT ?= /dev/cu.usbserial-DN06A5F2

# CFLAGS := -mmcu=$(MCU) -DF_CPU=16000000UL -DBAUD=9600 -Os -Wall -Wextra -std=gnu++17 -fno-lto \

CFLAGS := -mmcu=$(MCU) -DF_CPU=16000000UL -DBAUD=115200 -Os -Wall -Wextra -std=gnu++17 -fno-lto \
		  -I cpp/include -I cpp/src -I cpp/src/utilities -DUART_DEBUG

SRCS := cpp/src/main.cpp \
        cpp/src/pidController.cpp \
        cpp/src/unoController.cpp \
        cpp/src/utilities/avr_wiring.cpp \
        cpp/src/utilities/writeAvr.cpp \
        cpp/src/utilities/delete_stub.cpp

OUT := build/main
ELF := $(OUT).elf
HEX := $(OUT).hex

.PHONY: all hex flash clean

all: $(ELF)

$(ELF): $(SRCS)
	@mkdir -p build
	$(CC) $(CFLAGS) -o $@ $(SRCS)

hex: $(ELF)
	$(OBJCOPY) -O ihex -R .eeprom $(ELF) $(HEX)
	ls -l $(HEX)

flash: hex
# 	$(AVRDUDE) -c arduino -p $(MCU) -P $(PORT) -b 9600 -D -U flash:w:$(HEX):i
	$(AVRDUDE) -c arduino -p $(MCU) -P $(PORT) -b 115200 -D -U flash:w:$(HEX):i 

clean:
	rm -rf build
