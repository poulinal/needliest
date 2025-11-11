// Declarations for UART helpers used across AVR examples
#ifndef WRITE_AVR_H
#define WRITE_AVR_H

#include <stdint.h>

void uart_init(void);
void uart_putc(char c);
void uart_puts(const char *s);
char *uint_to_str(unsigned long v, char *buf);
void uart_put_fixed2(double v);

#endif // WRITE_AVR_H
