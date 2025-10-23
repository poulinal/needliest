// AP 2025
// Main file for AVR PID Controller project

#include "IPid.h"
#include <avr/io.h>
#include <util/delay.h>

// Simple test main function
int main() {
    // Initialize LED pin (PB5 = pin 13 on Arduino Uno)
    DDRB |= (1 << PB5);
    
    while (1) {
        // Blink LED to show the program is running
        PORTB |= (1 << PB5);   // Turn LED on
        _delay_ms(500);
        PORTB &= ~(1 << PB5);  // Turn LED off
        _delay_ms(500);
    }
    
    return 0;
}