/**
 * Arduino Uno Blink Example using AVR toolchain
 * 
 * This program blinks the built-in LED on pin 13 of Arduino Uno
 * Demonstrates basic AVR C++ programming without Arduino IDE
 */

#include <avr/io.h>
#include <util/delay.h>

// Define PB5 if not already defined
#ifndef PB5
#define PB5 5
#endif

// If DDRB is not defined, define it explicitly for ATmega328P
#ifndef DDRB
#define DDRB (*(volatile uint8_t*)0x24)
#endif

// If PORTB is not defined, define it explicitly for ATmega328P
#ifndef PORTB
#define PORTB (*(volatile uint8_t*)0x25)
#endif

// Arduino Uno uses ATmega328P
// Pin 13 corresponds to PORTB bit 5 (PB5)
#define LED_PIN PB5
#define LED_DDR DDRB
#define LED_PORT PORTB

void setup() {
    // Set LED pin as output
    LED_DDR |= (1 << LED_PIN);
}

void loop() {
    // Turn LED on
    LED_PORT |= (1 << LED_PIN);
    _delay_ms(1000);
    
    // Turn LED off
    LED_PORT &= ~(1 << LED_PIN);
    _delay_ms(1000);
}

int main() {
    setup();
    
    while (1) {
        loop();
    }
    
    return 0;
}
