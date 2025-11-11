/**
 * Arduino Uno Blink Example using AVR toolchain
 * 
 * This program blinks the built-in LED on pin 13 of Arduino Uno
 * Demonstrates basic AVR C++ programming without Arduino IDE
 * 
 * 
 * 
 * 
 * 1: /Users/alexpoulin/local/avr/bin/avr-g++ -mmcu=atmega328p -DF_CPU=16000000UL -Os -Wall -Wextra -std=c++17 -fno-lto PoteniometerTest.cpp -o PoteniometerTest.elf && /Users/alexpoulin/local/avr/bin/avr-objcopy -O ihex -R .eeprom PoteniometerTest.elf PoteniometerTest.hex
 * finally: /Users/alexpoulin/local/avr/bin/avrdude -c arduino -p atmega328p -P /dev/cu.usbserial-DN06A5F2 -b 115200 -U flash:w:PoteniometerTest.hex:i
 * 
 * to start a serial monitor:
 * screen /dev/cu.usbserial-DN06A5F2 115200
 * to exit screen: Ctrl-A K
 */

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>

#include "defines.h"

// Ensure MCU symbol so <avr/io.h> exposes ATmega328P register names to IntelliSense/build
#ifndef __AVR_ATmega328P__
#define __AVR_ATmega328P__
#endif

// UART defaults (override in defines.h or via -D flags if needed)
#ifndef BAUD
#define BAUD 115200
#endif

#ifndef UBRR_VALUE
#define UBRR_VALUE ((F_CPU/16/BAUD) - 1)
#endif

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

#include <util/delay.h>

// Forward declaration for setAngle (implemented below)
float setAngle(double current, double set);
// Forward declaration for UART init (implemented below)
static void uart_init(void);
// Forward declarations for UART helpers (implemented below)
static void uart_puts(const char *s);
static char *uint_to_str(unsigned long v, char *buf);
static void uart_put_fixed2(double v);

// Ensure MCU symbol so <avr/io.h> exposes ATmega328P register names to IntelliSense/build
#ifndef __AVR_ATmega328P__
#define __AVR_ATmega328P__
#endif

// UART defaults (override in defines.h or via -D flags if needed)
#ifndef BAUD
#define BAUD 115200
#endif

#ifndef UBRR_VALUE
#define UBRR_VALUE ((F_CPU/16/BAUD) - 1)
#endif

// Define ADEN if not already defined (AVR ATmexga328P: bit 7 in ADCSRA)
#ifndef ADEN
#define ADEN 7
#endif

// Define ADSC if not already defined (AVR ATmega328P: bit 6 in ADCSRA)
#ifndef ADSC
#define ADSC 6
#endif

// Define REFS0 if not already defined (AVR ATmega328P: bit 6 in ADMUX)
#ifndef REFS0
#define REFS0 6
#endif

// Define PB5 if not already defined
#ifndef PB5
#define PB5 5
#endif

// If DDRB is not defined, define it explicitly for ATmega328P
#ifndef DDRB
#define DDRB (*(volatile uint8_t*)0x24)
#endif

// If DDRD is not defined, define it explicitly for ATmega328P
#ifndef DDRD
#define DDRD (*(volatile uint8_t*)0x2A)
#endif

// If DDRC is not defined, define it explicitly for ATmega328P
#ifndef DDRC
#define DDRC (*(volatile uint8_t*)0x27)
#endif

// If PORTB is not defined, define it explicitly for ATmega328P
#ifndef PORTB
#defixxwne PORTB (*(volatile uint8_t*)0x25)
#endif

// If PORTD is not defined, define it explicitly for ATmega328P
#ifndef PORTD
#define PORTD (*(volatile uint8_t*)0x2B)
#endif

// If PORTC is not defined, define it explicitly for ATmega328P
#ifndef PORTC
#define PORTC (*(volatile uint8_t*)0x28)
#endif

// If ADMUX is not defined, define it explicitly for ATmega328P
#ifndef ADMUX
#define ADMUX (*(volatile uint8_t*)0x7C)
#endif

// If ADCSRA is not defined, define it explicitly for ATmega328P
#ifndef ADCSRA
#define ADCSRA (*(volatile uint8_t*)0x7A)
#endif

// If ADCW is not defined, define it explicitly for ATmega328P
#ifndef ADCW
#define ADCW (*(volatile uint16_t*)0x78)
#endif

// Arduino Uno uses ATmega328P
// Pin 13 corresponds to PORTB bit 5 (PB5)
#define LED_PIN PB5
#define LED_DDR DDRB
#define LED_PORT PORTB


const int CW = 4; // + direction pin to stepper controller
const int CCW = 3; // - direction
const int Pulse = 2;

// Define A5 if not already defined (Arduino Uno analog pin 5 is 5)
#ifndef A5
#define A5 5
#endif
const int PotPin = A5;

float dt = 0.05;
float d_min = 0.05; // max allowed diff. from set angle and position before motor will respond (stops motor from reacting to every bit of noise)

float current = 0; // current position in # of CW rotations from closed position
float delta = 0; // difference between current and set position
static double t = 0.0;

#define INPUT 0
#define OUTPUT 1

void pinMode(uint8_t pin, uint8_t mode) {
    if (mode == OUTPUT) {
        if (pin <= 7) {
            DDRD |= (1 << pin);         // D0..D7 -> PORTD
        } else if (pin <= 13) {
            DDRB |= (1 << (pin - 8));  // D8..D13 -> PORTB bits 0..5
        } else if (pin == A5) {
            DDRC |= (1 << 5);          // A5 -> PORTC5 set output
        }
    } else { // INPUT
        if (pin <= 7) {
            DDRD &= ~(1 << pin);
        } else if (pin <= 13) {
            DDRB &= ~(1 << (pin - 8));
        } else if (pin == A5) {
            DDRC &= ~(1 << 5);
        }
    }
}

uint16_t analogRead(uint8_t pin) {
    // Select ADC channel (assuming pin is A0-A5 mapped to 0-5)
    ADCSRA |= (1 << ADSC); // Start conversion
    while (ADCSRA & (1 << ADSC)); // Wait for conversion to finish
    return ADCW;
}

void setup() {
    pinMode(CW,OUTPUT);
    pinMode(CCW,OUTPUT);
    pinMode(Pulse,OUTPUT);
    pinMode(PotPin, INPUT);
    // Initialize UART for printing
    uart_init();
    
    // Set AVcc reference and enable ADC in setup
    ADMUX = (1 << REFS0) | (PotPin & 0x07); // Set reference (REFS0) and channel (A5=5)
    ADCSRA |= (1 << ADEN); // Enable ADC
}

// Arduino-like map function implementation
long map(long x, long in_min, long in_max, long out_min, long out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void loop() {
    // Serial.print("Current Position = ");
    // Serial.print(current); // display current angle
    // uint16_t adc = analogRead(PotPin);
    // set adc to a sin curve where its 3 *(sin(t/3)+1)
    uint16_t adc = (uint16_t)(512.0 * (sin(4 * t) + 1.0));
    //     long mapped = map(adc, 0, 1023, 0, 600);
    // double Set = 0.01 * (double)mapped; // read voltage from POT, map voltage to 100 steps per revolution, 6 revolutions
    // Direct floating-point mapping: ADC 0..1023 -> Set 0.0..6.0 rotations
    double Set = ((double)adc / 1023.0) * 6.0;
    // Serial.print(", Set Position = ");
    // Serial.println(Set);
    current = setAngle(current, Set);

    char buf[16];
    // Diagnostics: print ADC, mapped, Set, current, N and Delay
    uart_puts("ADC:"); uint_to_str(adc, buf); uart_puts(buf); uart_puts(" ");
    // mapped removed; printing Set directly
    uart_puts("Set:"); uart_put_fixed2(Set); uart_puts(" ");
    uart_puts("Cur:"); uart_put_fixed2(current); uart_puts(" ");
    uart_puts("\r\n");

    // delay
    // _delay_ms(100);
    // _delay_ms(1);
    t += 0.005;
}

void delayMicroseconds(unsigned int us) {
    // Each iteration is roughly 1us at 16MHz (ATmega328P)
    // This is a simple busy-wait loop, not super accurate
    for (unsigned int i = 0; i < us; i++) {
        _delay_us(1);
    }
}

// Minimal digitalWrite implementation for AVR
#define LOW 0
#define HIGH 1

void digitalWrite(uint8_t pin, uint8_t value) {
    if (pin >= 0 && pin <= 7) {
        if (value == HIGH)
            PORTD |= (1 << pin);
        else
            PORTD &= ~(1 << pin);
    } else if (pin >= 8 && pin <= 13) {
        if (value == HIGH)
            PORTB |= (1 << (pin - 8));
        else
            PORTB &= ~(1 << (pin - 8));
    } else if (pin == A5) {
        if (value == HIGH)
            PORTC |= (1 << 5);
        else
            PORTC &= ~(1 << 5);
    }
}

float setAngle(double current, double set) { // measured in # of CW rotations from closed position
    // compute local delta (avoid relying on global variable)
    double local_delta = set - current;
    double pulses_per_rev = 400.0; // pulses per rotation (adjust to your hardware)
    double N = fabs(local_delta * pulses_per_rev); // number of pulses to arrive at set location

    // If there is nothing to do, return immediately
    if (N < 1.0) {
        return (float)current;
    }

    // Change in setAngle:
    // const double step_period_us = 1000.0; // 1000 us per step = 1 kHz max speed

    // // Instead, set the actual period to the fixed speed you want
    // double period_us = step_period_us; // Use the fixed step period

    // // Recalculate pulse timings based on the fixed period
    // unsigned int pulse_high_us = 5; // HIGH time per pulse in us
    // if (pulse_high_us >= (unsigned int)period_us) pulse_high_us = (unsigned int)(period_us / 2);
    // unsigned int low_time_us = (unsigned int)period_us - pulse_high_us;
    // if (low_time_us < 1) low_time_us = 1;

    double Delay = dt * 1000000/N;

    float new_pos = (float)current;

    // Diagnostics
    uart_puts("N:"); uart_put_fixed2(N); uart_puts(" ");
    // uart_puts("Period_us:"); uart_put_fixed2(period_us); uart_puts(" ");
    uart_puts("Delay_us:"); uart_put_fixed2(Delay); uart_puts(" ");

    if (fabs(local_delta) <= d_min && set != 0) {
        uart_puts("No Move\r\n");
        delayMicroseconds(Delay*N);
        return new_pos;
    } else if (local_delta > 0) {
        uart_puts("Move CW\r\n");
        digitalWrite(CW, HIGH);
        digitalWrite(CCW, LOW);
    } else if (local_delta < 0) {
        uart_puts("Move CCW\r\n");
        digitalWrite(CW, LOW);
        digitalWrite(CCW, HIGH);
    }

    // Send N pulses with explicit HIGH/LOW timing
    for (int i = 0; i < (int)N; i++) {
        digitalWrite(Pulse, HIGH);
        // delayMicroseconds(pulse_high_us);
        delayMicroseconds((unsigned int)Delay / 8);
        digitalWrite(Pulse, LOW);
        // delayMicroseconds(low_time_us);
        delayMicroseconds((unsigned int)Delay / 8);
    }

    // After pulses complete, update reported position
    new_pos = (float)set;
    return new_pos;
}

int main() {
    setup();
    
    while (1) {
        loop();
    }
    
    return 0;
}


static void uart_init(void) {
    // Enable double-speed mode for better accuracy at high baud rates
    UCSR0A |= (1 << U2X0);

    // Compute rounded UBRR for double-speed (division by 8)
    // Use 64-bit intermediate to avoid overflow on some toolchains
    unsigned long ubrr_calc = ((unsigned long)F_CPU + ( (unsigned long)BAUD * 4UL )) / (8UL * (unsigned long)BAUD);
    if (ubrr_calc > 0) ubrr_calc = ubrr_calc - 1;
    uint16_t ubrr = (uint16_t)ubrr_calc;
    UBRR0H = (uint8_t)(ubrr >> 8);
    UBRR0L = (uint8_t)(ubrr & 0xFF);

    // enable TX only (and RX if you want)
    UCSR0B = (1 << TXEN0); // | (1 << RXEN0);
    // 8-bit, no parity, 1 stop
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

static void uart_putc(char c) {
    while (!(UCSR0A & (1 << UDRE0))); // wait for empty transmit buffer
    UDR0 = (uint8_t)c;
}

static void uart_puts(const char *s) {
    while (*s) uart_putc(*s++);
}

// small unsigned integer -> decimal string (returns pointer to buffer)
static char *uint_to_str(unsigned long v, char *buf) {
    char tmp[12];
    int pos = 0;
    if (v == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return buf;
    }
    while (v) {
        tmp[pos++] = '0' + (v % 10);
        v /= 10;
    }
    // reverse
    for (int i = 0; i < pos; ++i) buf[i] = tmp[pos - 1 - i];
    buf[pos] = '\0';
    return buf;
}

// Print signed double with 2 decimal places (simple, no float printf)
static void uart_put_fixed2(double v) {
    char buf[16];
    if (v < 0) {
        uart_putc('-');
        v = -v;
    }
    unsigned long ip = (unsigned long)v;
    unsigned long frac = (unsigned long)((v - (double)ip) * 100.0 + 0.5);
    uint_to_str(ip, buf);
    uart_puts(buf);
    uart_putc('.');
    // print two-digit fraction with leading zero if needed
    if (frac < 10) uart_putc('0');
    uint_to_str(frac, buf);
    uart_puts(buf);
}