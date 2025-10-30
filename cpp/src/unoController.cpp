//AP 2025

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>

#include "defines.h"
#include "avr_wiring.h"
#include "writeAvr.h"
#include "IUnoController.h"

// Ensure MCU symbol so <avr/io.h> exposes ATmega328P register names to IntelliSense/build
#ifndef __AVR_ATmega328P__
#define __AVR_ATmega328P__
#endif

// UART defaults (override in defines.h or via -D flags if needed)
#ifndef BAUD
#define BAUD 115200
#endif

#ifndef UBRR_VALUE
#define UBRR_VALUE ((F_CPU/16/BAUD) - 1)
#endif

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

#include <util/delay.h>


class UnoController : public IUnoController {
public:
    // Stepper/motor pin assignments
    const int CW = 4; // + direction pin to stepper controller
    const int CCW = 3; // - direction
    const int Pulse = 2;
    const int PotPin = A5;

    float dt = 0.05;
    float d_min = 0.05; // max allowed diff. from set angle and position before motor will respond (stops motor from reacting to every bit of noise)

    float current = 0; // current position in # of CW rotations from closed position
    float delta = 0; // difference between current and set position

    UnoController() {
        setup();
    }

    void setup() {
        pinMode(CW, OUTPUT);
        pinMode(CCW, OUTPUT);
        pinMode(Pulse, OUTPUT);
        pinMode(PotPin, INPUT);
        // Initialize UART for printing
        uart_init();

        // Set AVcc reference and enable ADC in setup
        ADMUX = (1 << REFS0) | (PotPin & 0x07); // Set reference (REFS0) and channel (A5=5)
        ADCSRA |= (1 << ADEN); // Enable ADC
    }

    float setAngle(double current, double set) { // measured in # of CW rotations from closed position
        // compute local delta (avoid relying on global variable)
        double local_delta = set - current;
        double pulses_per_rev = 400.0; // pulses per rotation (adjust to your hardware)
        double N = fabs(local_delta * pulses_per_rev); // number of pulses to arrive at set location

        // If there is nothing to do, return immediately
        if (N < 1.0) {
            return (float)current;
        }

        double Delay = dt * 1000000/N;

        float new_pos = (float)current;

        // Diagnostics
        uart_puts("N:"); uart_put_fixed2(N); uart_puts(" ");
        // uart_puts("Period_us:"); uart_put_fixed2(period_us); uart_puts(" ");
        uart_puts("Delay_us:"); uart_put_fixed2(Delay); uart_puts(" ");

        if (fabs(local_delta) <= d_min && set != 0) {
            uart_puts("No Move\r\n");
            delayMicroseconds(Delay*N);
            return new_pos;
        } else if (local_delta > 0) {
            uart_puts("Move CW\r\n");
            digitalWrite(CW, HIGH);
            digitalWrite(CCW, LOW);
        } else if (local_delta < 0) {
            uart_puts("Move CCW\r\n");
            digitalWrite(CW, LOW);
            digitalWrite(CCW, HIGH);
        }

        // Send N pulses with explicit HIGH/LOW timing
        for (int i = 0; i < (int)N; i++) {
            digitalWrite(Pulse, HIGH);
            // delayMicroseconds(pulse_high_us);
            delayMicroseconds((unsigned int)Delay / 8);
            digitalWrite(Pulse, LOW);
            // delayMicroseconds(low_time_us);
            delayMicroseconds((unsigned int)Delay / 8);
        }

        // After pulses complete, update reported position
        new_pos = (float)set;
        return new_pos;
    }

    float pneumaticReadingRaw() override {
        // Read potentiometer on A5
        uint16_t adc = analogRead(A5);
        // Convert ADC reading to number of rotations (0..6.0)
        // float rotations = ((float)adc / 1023.0f) * 6.0f;
        // return rotations;
    }

    float pneumaticReadingRotation() override {
        // Read potentiometer on A5
        uint16_t adc = analogRead(A5);
        // Convert ADC reading to number of rotations (0..6.0)
        float rotations = ((float)adc / 1023.0f) * 6.0f;
        return rotations;
    }

};
