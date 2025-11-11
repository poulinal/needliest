/**
 * Arduino Uno Advanced Example using AVR toolchain
 * 
 * This program demonstrates:
 * - ADC (Analog to Digital Conversion) reading from A0
 * - UART serial communication
 * - Timer interrupts
 * - C++ classes with AVR
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>

// Add this line to ensure ATmega328P definitions
#ifndef __AVR_ATmega328P__
#define __AVR_ATmega328P__
#endif

#define F_CPU 16000000UL
#define BAUD 9600
#define UBRR_VALUE ((F_CPU/16/BAUD) - 1)

class ArduinoUno {
private:
    uint16_t adc_value;
    volatile bool timer_flag;
    
public:
    ArduinoUno() : adc_value(0), timer_flag(false) {}
    
    void init() {
        // Initialize LED (Pin 13 = PB5)
        DDRB |= (1 << PB5);
        
        // Initialize UART
        UBRR0H = (uint8_t)(UBRR_VALUE >> 8);
        UBRR0L = (uint8_t)UBRR_VALUE;
        UCSR0B = (1 << TXEN0) | (1 << RXEN0);  // Enable TX and RX
        UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // 8-bit data
        
        // Initialize ADC
        ADMUX = (1 << REFS0);  // AVCC reference, ADC0 (A0)
        ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // Enable ADC, prescaler 128
        
        // Initialize Timer1 for 1Hz interrupt
        TCCR1B |= (1 << WGM12) | (1 << CS12) | (1 << CS10); // CTC mode, prescaler 1024
        OCR1A = 15624; // 1 second at 16MHz with 1024 prescaler
        TIMSK1 |= (1 << OCIE1A); // Enable compare match interrupt
        
        sei(); // Enable global interrupts
    }
    
    void uart_putchar(char c) {
        while (!(UCSR0A & (1 << UDRE0))); // Wait for empty transmit buffer
        UDR0 = c;
    }
    
    void uart_print(const char* str) {
        while (*str) {
            uart_putchar(*str++);
        }
    }
    
    void uart_print_number(uint16_t num) {
        char buffer[6];
        sprintf(buffer, "%u", num);
        uart_print(buffer);
    }
    
    uint16_t read_adc() {
        ADCSRA |= (1 << ADSC); // Start conversion
        while (ADCSRA & (1 << ADSC)); // Wait for completion
        return ADC;
    }
    
    void toggle_led() {
        PORTB ^= (1 << PB5);
    }
    
    void run() {
        while (1) {
            if (timer_flag) {
                timer_flag = false;
                
                // Read analog value from A0
                adc_value = read_adc();
                
                // Send data over UART
                uart_print("ADC A0: ");
                uart_print_number(adc_value);
                uart_print(" (");
                uart_print_number((adc_value * 5000UL) / 1024); // Convert to millivolts
                uart_print("mV)\r\n");
                
                // Toggle LED
                toggle_led();
            }
            
            // Small delay to prevent busy waiting
            _delay_ms(10);
        }
    }
    
    void set_timer_flag() {
        timer_flag = true;
    }
};

// Global instance
ArduinoUno arduino;

// Timer1 Compare Match A interrupt
ISR(TIMER1_COMPA_vect) {
    arduino.set_timer_flag();
}

int main() {
    arduino.init();
    arduino.run();
    return 0;
}