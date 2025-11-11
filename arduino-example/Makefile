# Arduino Uno AVR Toolchain Makefile
# Uses the custom-built AVR toolchain

# Toolchain paths
PREFIX := /Users/alexpoulin/local/avr
CC := $(PREFIX)/bin/avr-gcc
CXX := $(PREFIX)/bin/avr-g++
OBJCOPY := $(PREFIX)/bin/avr-objcopy
OBJDUMP := $(PREFIX)/bin/avr-objdump
SIZE := $(PREFIX)/bin/avr-size

# Arduino Uno settings (ATmega328P)
MCU := atmega328p
F_CPU := 16000000UL

# Compiler flags
CPPFLAGS := -mmcu=$(MCU) -DF_CPU=$(F_CPU)
CXXFLAGS := -Os -Wall -Wextra -std=c++17 -fno-lto
LDFLAGS := -mmcu=$(MCU) -fno-lto

# Source files
SOURCES := blink.cpp
TARGET := blink

# Object files
OBJECTS := $(SOURCES:.cpp=.o)

# Default target
all: $(TARGET).hex $(TARGET).lst size

# Compile C++ source to object files
%.o: %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

# Link object files to create ELF
$(TARGET).elf: $(OBJECTS)
	$(CXX) $(LDFLAGS) $^ -o $@

# Create Intel HEX file for programming
$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex -R .eeprom $< $@

# Create assembly listing
$(TARGET).lst: $(TARGET).elf
	$(OBJDUMP) -h -S $< > $@

# Show program size
size: $(TARGET).elf
	$(SIZE) -A $<
	@echo "Program size for $(MCU):"
	@$(SIZE) -B $<

# Clean build artifacts
clean:
	rm -f *.o *.elf *.hex *.lst

# Upload to Arduino Uno (requires avrdude)
upload: $(TARGET).hex
	$(PREFIX)/bin/avrdude -c arduino -p $(MCU) -P /dev/cu.usbmodem* -b 115200 -U flash:w:$<:i

# Upload specific programs
upload-blink: blink.hex
	$(PREFIX)/bin/avrdude -c arduino -p $(MCU) -P /dev/cu.usbmodem* -b 115200 -U flash:w:blink.hex:i

upload-advanced: advanced.hex
	$(PREFIX)/bin/avrdude -c arduino -p $(MCU) -P /dev/cu.usbmodem* -b 115200 -U flash:w:advanced.hex:i

# Build advanced example
advanced.o: advanced.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

advanced.elf: advanced.o
	$(CXX) $(LDFLAGS) $^ -o $@

advanced.hex: advanced.elf
	$(OBJCOPY) -O ihex -R .eeprom $< $@

advanced.lst: advanced.elf
	$(OBJDUMP) -h -S $< > $@

# Show advanced example size
size-advanced: advanced.elf
	$(SIZE) -A $<
	@echo "Program size for $(MCU):"
	@$(SIZE) -B $<

# Show available targets
help:
	@echo "Available targets:"
	@echo "  all           - Build hex file and listing"
	@echo "  clean         - Remove build artifacts"
	@echo "  upload        - Upload blink.hex to Arduino Uno"
	@echo "  upload-blink  - Upload blink.hex to Arduino Uno"
	@echo "  upload-advanced - Upload advanced.hex to Arduino Uno"
	@echo "  advanced.hex  - Build advanced example"
	@echo "  size          - Show blink program size information"
	@echo "  size-advanced - Show advanced program size information"
	@echo "  help          - Show this help message"

.PHONY: all clean upload upload-blink upload-advanced size size-advanced help