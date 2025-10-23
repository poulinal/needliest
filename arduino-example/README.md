# Arduino Uno Programming with Custom AVR Toolchain
https://github.com/avrdudes/avr-libc?tab=readme-ov-file

### Chat
```
export PREFIX=/Users/alexpoulin/local/avr
$PREFIX/bin/avr-g++ -mmcu=atmega328p -DF_CPU=16000000UL -Os -std=c++17 -fno-lto your_program.cpp -o program.elf
$PREFIX/bin/avr-objcopy -O ihex program.elf program.hex
avrdude -c arduino -p atmega328p -P /dev/cu.usbmodem* -b 115200 -U flash:w:program.hex:i
```

## Overview
You now have a fully functional custom-built AVR toolchain that can compile C++ code for Arduino Uno (ATmega328P) without the Arduino IDE.

## Toolchain Location
- **Installation Path**: `/Users/alexpoulin/local/avr/`
- **Compiler**: `/Users/alexpoulin/local/avr/bin/avr-g++`
- **Libraries**: `/Users/alexpoulin/local/avr/avr/lib/`
- **Headers**: `/Users/alexpoulin/local/avr/avr/include/`

## Available Examples

### 1. Basic Blink (`blink.cpp`)
- Blinks the built-in LED on pin 13
- Shows basic GPIO control
- **Size**: 188 bytes

### 2. Advanced Example (`advanced.cpp`)
- ADC reading from analog pin A0
- UART serial communication (9600 baud)
- Timer interrupts (1Hz)
- C++ class usage
- **Size**: 2,065 bytes

## How to Use

### Quick Compilation
```bash
# Set environment
export PREFIX=/Users/alexpoulin/local/avr

# Compile basic program
$PREFIX/bin/avr-g++ -mmcu=atmega328p -DF_CPU=16000000UL -Os -Wall -std=c++17 -fno-lto program.cpp -o program.elf

# Create hex file for upload
$PREFIX/bin/avr-objcopy -O ihex -R .eeprom program.elf program.hex
```

### Using the Makefile
```bash
cd arduino-example
make              # Build all
make clean        # Clean build files
make size         # Show program size
make upload       # Upload to Arduino (requires avrdude)
```

## Key Features

### Arduino Uno Specifications
- **MCU**: ATmega328P
- **Clock**: 16MHz
- **Flash**: 32KB (bootloader uses ~0.5KB)
- **RAM**: 2KB
- **EEPROM**: 1KB

### Pin Mapping
| Arduino Pin | AVR Port | Description |
|-------------|----------|-------------|
| D13 (LED)   | PB5      | Built-in LED |
| A0          | ADC0/PC0 | Analog input |
| Serial TX   | PD1      | UART transmit |
| Serial RX   | PD0      | UART receive |

### Available Libraries
Your toolchain includes:
- **avr-libc**: Standard C library for AVR
- **AVR headers**: Hardware abstraction (`avr/io.h`, `avr/interrupt.h`, etc.)
- **Utility functions**: Delays, atomic operations
- **Math library**: `libm.a` for floating point

## Programming Concepts

### Direct Register Access
```cpp
#include <avr/io.h>

// Set pin as output
DDRB |= (1 << PB5);

// Turn pin high
PORTB |= (1 << PB5);

// Turn pin low
PORTB &= ~(1 << PB5);
```

### Interrupts
```cpp
#include <avr/interrupt.h>

// Enable global interrupts
sei();

// Interrupt service routine
ISR(TIMER1_COMPA_vect) {
    // Handle interrupt
}
```

### ADC Reading
```cpp
// Initialize ADC
ADMUX = (1 << REFS0);  // AVCC reference
ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

// Read ADC
ADCSRA |= (1 << ADSC);
while (ADCSRA & (1 << ADSC));
uint16_t value = ADC;
```

### Serial Communication
```cpp
// Initialize UART
UBRR0 = ((F_CPU/16/BAUD) - 1);
UCSR0B = (1 << TXEN0);
UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);

// Send character
while (!(UCSR0A & (1 << UDRE0)));
UDR0 = 'A';
```

## Uploading to Arduino

### Install avrdude (if not already installed)
```bash
brew install avrdude
```

### Upload hex file
```bash
avrdude -c arduino -p atmega328p -P /dev/cu.usbmodem* -b 115200 -U flash:w:program.hex:i
```

## Advantages of Custom Toolchain

1. **Latest GCC**: Version 16.0.0 with modern C++ features
2. **Full Control**: No Arduino framework overhead
3. **Bare Metal**: Direct hardware access
4. **Size Optimization**: Smaller binaries
5. **Learning**: Understanding low-level AVR programming

## Next Steps

1. **Connect Arduino Uno** via USB
2. **Upload examples** using avrdude
3. **Monitor serial output** at 9600 baud
4. **Experiment** with sensors and actuators

Your custom AVR toolchain is ready for professional embedded development!