#include "defineConstants.h"
#include "defines.h"

// Ensure MCU symbol so <avr/io.h> exposes ATmega328P register names to IntelliSense/build
#ifndef __AVR_ATmega328P__
#define __AVR_ATmega328P__
#endif

#include "avr_wiring.h"

#include <avr/io.h>
#include <util/delay.h>

// pinMode implementation: maps pins like the original examples
// D0..D7 -> PORTD bits 0..7
// D8..D13 -> PORTB bits 0..5
// A5 (value 5) handled explicitly for analog pin PC5 when used
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

// Simple analogRead: selects channel, enables ADC and starts conversion
// channel is 0..5 for A0..A5
uint16_t analogRead(uint8_t channel) {
    // Select AVcc as reference and channel
    ADMUX = (1 << REFS0) | (channel & 0x07);
    // Enable ADC and set prescaler to 128 for 16MHz -> 125kHz ADC clock
    ADCSRA |= (1 << ADEN);
    ADCSRA = (ADCSRA & ~0x07) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
    // Start conversion
    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC));
    return ADCW;
}

// Busy-wait microsecond delay using util/_delay_us
void delayMicroseconds(unsigned int us) {
    // _delay_us expects a double; call in a loop for large values
    while (us--) {
        _delay_us(1);
    }
}

// Minimal digitalWrite mapping matching pinMode above
void digitalWrite(uint8_t pin, uint8_t value) {
    if (pin <= 7) {
        if (value == HIGH) PORTD |= (1 << pin);
        else PORTD &= ~(1 << pin);
    } else if (pin >= 8 && pin <= 13) {
        if (value == HIGH) PORTB |= (1 << (pin - 8));
        else PORTB &= ~(1 << (pin - 8));
    } else if (pin == A5) {
        if (value == HIGH) PORTC |= (1 << 5);
        else PORTC &= ~(1 << 5);
    }
}

// Simple map implementation (same behavior as Arduino)
long map(long x, long in_min, long in_max, long out_min, long out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
