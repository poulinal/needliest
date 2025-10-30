// Lightweight AVR "wiring" helpers for this project.
// Provides simple pinMode/digitalWrite/analogRead/map/delayMicroseconds
// Implemented in avr_wiring.cpp so they are available to all translation units.

#ifndef AVR_WIRING_H
#define AVR_WIRING_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Basic constants (match usage in existing code)
#define INPUT 0
#define OUTPUT 1
#define LOW 0
#define HIGH 1

// Analog pins: keep same numeric values used in the examples (A0..A5 -> 0..5)
#ifndef A0
#define A0 0
#endif
#ifndef A1
#define A1 1
#endif
#ifndef A2
#define A2 2
#endif
#ifndef A3
#define A3 3
#endif
#ifndef A4
#define A4 4
#endif
#ifndef A5
#define A5 5
#endif

// Wiring-like API
void pinMode(uint8_t pin, uint8_t mode);
void digitalWrite(uint8_t pin, uint8_t value);
uint16_t analogRead(uint8_t pin);
void delayMicroseconds(unsigned int us);
long map(long x, long in_min, long in_max, long out_min, long out_max);

#ifdef __cplusplus
}
#endif

#endif // AVR_WIRING_H
