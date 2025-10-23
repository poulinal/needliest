// Project-wide defines wrapper
#ifndef DEFINES_H
#define DEFINES_H

// Include the original define.h (if present)
#include "define.h"

// Override F_CPU for Arduino Uno (16 MHz)
#undef F_CPU
#define F_CPU 16000000UL

#endif // DEFINES_H
