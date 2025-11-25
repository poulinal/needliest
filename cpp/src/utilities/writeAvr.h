// Declarations for UART helpers used across AVR examples
#ifndef WRITE_AVR_H
#define WRITE_AVR_H

#include <stdint.h>

void uart_init(void);
void uart_putc(char c);
void uart_puts(const char *s);
char *uint_to_str(unsigned long v, char *buf);
void uart_put_fixed2(double v);
void uart_putchars(const char *buf);

// Additional UART helpers defined in writeAvr.cpp
int uart_getc_blocking(void);
int uart_readline(char *buf, int maxlen);
double parse_ascii_float(const char *s);
// Parse two comma-separated floats from a single ASCII line.
// Accepts optional surrounding braces and spaces, e.g. "{1.23}, {4.56}" or "1.23,4.56".
// Returns 1 on success and fills v1/v2, otherwise returns 0.
int parse_two_ascii_floats(const char *s, double *v1, double *v2);

// Return non-zero if a byte is available to read from UART
int uart_data_available(void);

#endif // WRITE_AVR_H
