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
#include <string.h>

// If BAUD is defined by the build (recommended), use <util/setbaud.h>
#ifdef BAUD
#include <util/setbaud.h>
#endif

void uart_init(void) {
    // Configure baud using <util/setbaud.h> if BAUD/F_CPU macros are provided.
    // setbaud.h defines UBRR_VALUE and USE_2X based on F_CPU and BAUD.
#if defined(UBRR_VALUE)
# if USE_2X
    UCSR0A |= (1 << U2X0);
# else
    UCSR0A &= ~(1 << U2X0);
# endif
    uint16_t ubrr = (uint16_t)UBRR_VALUE;
    UBRR0H = (uint8_t)(ubrr >> 8);
    UBRR0L = (uint8_t)(ubrr & 0xFF);
#else
    // Fallback to reasonable default (115200-ish)
    UCSR0A |= (1 << U2X0);
    uint16_t ubrr = 16;
    UBRR0H = (uint8_t)(ubrr >> 8);
    UBRR0L = (uint8_t)(ubrr & 0xFF);
#endif

    // enable TX and RX
    UCSR0B = (1 << TXEN0) | (1 << RXEN0);
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

// Print chars from a buffer
void uart_putchars(const char *buf) {
    while (*buf) uart_putc(*buf++);
}

int uart_getc_blocking(void) {
    // Wait for a character to be received
    while (!(UCSR0A & (1 << RXC0)));
    // Check and clear any UART errors (frame error, data overrun, parity error)
    uint8_t status = UCSR0A;
    uint8_t data = UDR0;
    uart_puts("getc_blocking, data: "); uart_putc(data); // Echo back received character
    if (status & ((1 << FE0) | (1 << DOR0) | (1 << UPE0))) {
        // Error occurred - return a marker or just the data (errors cleared by reading UDR0)
        return (int)data; // or return -1 to signal error
    }
    return (int)data;
}

// Return non-zero if a byte is waiting in the UART receive buffer
int uart_data_available(void) {
    return (UCSR0A & (1 << RXC0)) ? 1 : 0;
}

#ifdef UART_DEBUG
// Print single byte as hex (e.g. 0x1A)
static void uart_put_hex8(uint8_t v) {
    const char *hex = "0123456789ABCDEF";
    char h[3];
    h[0] = hex[(v >> 4) & 0xF];
    h[1] = hex[v & 0xF];
    h[2] = '\0';
    uart_puts("0x");
    uart_putchars(h);
}
#endif

// read up to maxlen-1 chars into buf, stop on '\n'.
// This implementation accumulates bytes in a persistent buffer and
// only returns when a full line (ending with '\n') is available.
int uart_readline(char *buf, int maxlen) {
    enum { PBUF_SIZE = 128 };
    static char pbuf[PBUF_SIZE];
    static int plen = 0;

    // Read all bytes currently available into the persistent buffer
    while (uart_data_available()) {
        int c = uart_getc_blocking();
        if (c == '\r') continue;
        #ifdef UART_DEBUG
        uart_puts("RXB: "); uart_put_hex8((uint8_t)c); uart_puts(" plen=");
        char tmpbuf[8]; uart_puts(uint_to_str((unsigned long)plen, tmpbuf)); uart_puts("\r\n");
        #endif
        if (plen < PBUF_SIZE - 1) {
            pbuf[plen++] = (char)c;
        } else {
            // buffer full: drop oldest byte to make room
            memmove(pbuf, pbuf + 1, PBUF_SIZE - 2);
            pbuf[PBUF_SIZE - 2] = (char)c;
            plen = PBUF_SIZE - 1;
        }
        _delay_ms(0.1); // small delay to allow processing
    }

    // If no newline yet, wait a short total timeout for more bytes to arrive.
    // This helps when the sender writes bytes slowly or the serial driver batches them.
    const int total_wait_ms = 200;
    int waited = 0;
    while (1) {
        // Search for newline first (in case it arrived during previous reads)
        for (int i = 0; i < plen; ++i) {
            if (pbuf[i] == '\n') {
                int line_len = i;
                if (line_len >= maxlen) line_len = maxlen - 1;
                for (int j = 0; j < line_len; ++j) buf[j] = pbuf[j];
                buf[line_len] = '\0';
                int rem = plen - (i + 1);
                if (rem > 0) memmove(pbuf, pbuf + i + 1, rem);
                plen = rem;
                return line_len;
            }
        }

        if (waited >= total_wait_ms) break;

        if (uart_data_available()) {
            // read more bytes and reset waited
            while (uart_data_available()) {
                int c = uart_getc_blocking();
                if (c == '\r') continue;
                if (plen < PBUF_SIZE - 1) {
                    pbuf[plen++] = (char)c;
                } else {
                    memmove(pbuf, pbuf + 1, PBUF_SIZE - 2);
                    pbuf[PBUF_SIZE - 2] = (char)c;
                    plen = PBUF_SIZE - 1;
                }
            }
            waited = 0;
            continue; // re-check for newline immediately
        }
        _delay_ms(1);
        ++waited;
    }

    // Search for newline in persistent buffer
    for (int i = 0; i < plen; ++i) {
        if (pbuf[i] == '\n') {
            int line_len = i;
            if (line_len >= maxlen) line_len = maxlen - 1;
            // copy up to line_len chars into user buffer
            for (int j = 0; j < line_len; ++j) buf[j] = pbuf[j];
            buf[line_len] = '\0';
            // remove consumed bytes (including the newline) from persistent buffer
            int rem = plen - (i + 1);
            if (rem > 0) memmove(pbuf, pbuf + i + 1, rem);
            plen = rem;
            return line_len;
        }
    }

    // No complete line yet. Return 0 to indicate "no complete line".
#ifdef UART_DEBUG
    if (plen > 0) {
        uart_puts("PBUF (partial): '"); uart_putchars(pbuf); uart_puts("'\r\n");
    }
#endif
    return 0;
}

// returns parsed double; simple, handles optional leading '-' and '.' and up to reasonable digits
double parse_ascii_float(const char *s) {
    double sign = 1.0;
    if (*s == '-') { sign = -1.0; ++s; }
    unsigned long ipart = 0;
    while (*s >= '0' && *s <= '9') {
        ipart = ipart * 10 + (unsigned long)(*s - '0');
        ++s;
    }
    double val = (double)ipart;
    if (*s == '.') {
        ++s;
        double place = 0.1;
        while (*s >= '0' && *s <= '9') {
            val += (double)(*s - '0') * place;
            place *= 0.1;
            ++s;
        }
    }
    return sign * val;
}


// Parse two comma-separated floats from an ASCII string into v1 and v2.
// Accept forms like: "1.23,4.56" or "{1.23}, {4.56}" or with surrounding spaces.
// Returns 1 on success, 0 on failure.
int parse_two_ascii_floats(const char *s, double *v1, double *v2) {
    const char *p = s;
    // skip leading spaces
    while (*p && isspace((unsigned char)*p)) ++p;
    // optional opening brace
    if (*p == '{') ++p;
    while (*p && isspace((unsigned char)*p)) ++p;
    if (!*p) return 0;

    // parse first value
    *v1 = parse_ascii_float(p);

    // advance p past the first numeric token
    const char *q = p;
    if (*q == '-' ) ++q; // negative sign handled by parse
    while (*q >= '0' && *q <= '9') ++q;
    if (*q == '.') {
        ++q;
        while (*q >= '0' && *q <= '9') ++q;
    }

    // skip until comma
    while (*q && *q != ',') ++q;
    if (!*q) return 0; // no comma found
    ++q; // skip comma

    // skip spaces and optional opening brace for second value
    while (*q && (isspace((unsigned char)*q) || *q == '{')) ++q;
    if (!*q) return 0;

    *v2 = parse_ascii_float(q);
    return 1;
}