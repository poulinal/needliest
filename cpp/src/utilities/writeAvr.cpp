#include "defineConstants.h"
#include "defines.h"

// Ensure MCU symbol so <avr/io.h> exposes ATmega328P register names to IntelliSense/build
#ifndef __AVR_ATmega328P__
#define __AVR_ATmega328P__
#endif

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

#include <util/delay.h>
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>


void uart_init(void) {
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

void uart_putc(char c) {
    while (!(UCSR0A & (1 << UDRE0))); // wait for empty transmit buffer
    UDR0 = (uint8_t)c;
}

void uart_puts(const char *s) {
    while (*s) uart_putc(*s++);
}

// small unsigned integer -> decimal string (returns pointer to buffer)
char *uint_to_str(unsigned long v, char *buf) {
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
void uart_put_fixed2(double v) {
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
