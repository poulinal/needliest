//AP 2025

// Forward declarations
float setAngle(double current, double set);

/**
 * Arduino Uno PID Controller Example using AVR toolchain
 * 
 * This program implements a PID controller for stepper motor control
 * Demonstrates AVR C++ programming with analog input and digital output
 */

#include <stdint.h>
#include <stdio.h>
#include <math.h>

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

#include <util/delay.h>

// Define ADEN if not already defined (AVR ATmega328P: bit 7 in ADCSRA)
#ifndef ADEN
#define ADEN 7
#endif

// Define ADSC if not already defined (AVR ATmega328P: bit 6 in ADCSRA)
#ifndef ADSC
#define ADSC 6
#endif

// Define REFS0 if not already defined (AVR ATmega328P: bit 6 in ADMUX)
#ifndef REFS0
#define REFS0 6
#endif

// Define PB5 if not already defined
#ifndef PB5
#define PB5 5
#endif

// If DDRB is not defined, define it explicitly for ATmega328P
#ifndef DDRB
#define DDRB (*(volatile uint8_t*)0x24)
#endif

// If DDRD is not defined, define it explicitly for ATmega328P
#ifndef DDRD
#define DDRD (*(volatile uint8_t*)0x2A)
#endif

// If DDRC is not defined, define it explicitly for ATmega328P
#ifndef DDRC
#define DDRC (*(volatile uint8_t*)0x27)
#endif

// If PORTB is not defined, define it explicitly for ATmega328P
#ifndef PORTB
#define PORTB (*(volatile uint8_t*)0x25)
#endif

// If PORTD is not defined, define it explicitly for ATmega328P
#ifndef PORTD
#define PORTD (*(volatile uint8_t*)0x2B)
#endif

// If PORTC is not defined, define it explicitly for ATmega328P
#ifndef PORTC
#define PORTC (*(volatile uint8_t*)0x28)
#endif

// If ADMUX is not defined, define it explicitly for ATmega328P
#ifndef ADMUX
#define ADMUX (*(volatile uint8_t*)0x7C)
#endif

// If ADCSRA is not defined, define it explicitly for ATmega328P
#ifndef ADCSRA
#define ADCSRA (*(volatile uint8_t*)0x7A)
#endif

// If ADCW is not defined, define it explicitly for ATmega328P
#ifndef ADCW
#define ADCW (*(volatile uint16_t*)0x78)
#endif

// Arduino Uno uses ATmega328P
// Pin 13 corresponds to PORTB bit 5 (PB5)
#define LED_PIN PB5
#define LED_DDR DDRB
#define LED_PORT PORTB


const int CW = 4; // + direction pin to stepper controller
const int CCW = 3; // - direction
const int Pulse = 2;

// Define A5 if not already defined (Arduino Uno analog pin 5 is 5)
#ifndef A5
#define A5 5
#endif
const int PotPin = A5;

float dt = 0.05;
float d_min = 0.05; // max allowed diff. from set angle and position before motor will respond (stops motor from reacting to every bit of noise)

float current = 0; // current position in # of CW rotations from closed position
float delta = 0; // difference between current and set position

#define INPUT 0
#define OUTPUT 1

void pinMode(uint8_t pin, uint8_t mode) {
    if (mode == OUTPUT) {
        if (pin >= 0 && pin <= 7) {
            DDRD |= (1 << pin); // D2-D7 are on PORTD
        } else if (pin >= 8 && pin <= 13) {
            DDRB |= (1 << (pin - 8)); // D8-D13 are on PORTB
        } else if (pin == A5) {
            DDRC &= ~(1 << 5); // A5 is on PORTC5, set as input
        }
    } else if (mode == INPUT) {
        if (pin >= 0 && pin <= 7) {
            DDRD &= ~(1 << pin);
        } else if (pin >= 8 && pin <= 13) {
            DDRB &= ~(1 << (pin - 8));
        } else if (pin == A5) {
            DDRC &= ~(1 << 5);
        }
    }
}

uint16_t analogRead(uint8_t pin) {
    // Select ADC channel (assuming pin is A0-A5 mapped to 0-5)
    ADMUX = (1 << REFS0) | (pin & 0x07); // AVcc reference, channel select
    ADCSRA |= (1 << ADEN); // Enable ADC
    ADCSRA |= (1 << ADSC); // Start conversion
    while (ADCSRA & (1 << ADSC)); // Wait for conversion to finish
    return ADCW;
}

void setup() {
    pinMode(CW,OUTPUT);
    pinMode(CCW,OUTPUT);
    pinMode(Pulse,OUTPUT);
    pinMode(PotPin, INPUT);
    // Serial.begin(9600); // Remove or implement if needed
}

// Arduino-like map function implementation
long map(long x, long in_min, long in_max, long out_min, long out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void loop() {
    // Serial.print("Current Position = ");
    // Serial.print(current); // display current angle
    double Set = 0.01*map(analogRead(PotPin),1,1023,0,600); // read voltage from POT, map voltage to 100 steps per revolution, 6 revolutions
    // Serial.print(", Set Position = ");
    // Serial.println(Set);
    current = setAngle(current, Set);
}

void delayMicroseconds(unsigned int us) {
    // Each iteration is roughly 1us at 16MHz (ATmega328P)
    // This is a simple busy-wait loop, not super accurate
    for (unsigned int i = 0; i < us; i++) {
        _delay_us(1);
    }
}

// Minimal digitalWrite implementation for AVR
#define LOW 0
#define HIGH 1

void digitalWrite(uint8_t pin, uint8_t value) {
    if (pin >= 0 && pin <= 7) {
        if (value == HIGH)
            PORTD |= (1 << pin);
        else
            PORTD &= ~(1 << pin);
    } else if (pin >= 8 && pin <= 13) {
        if (value == HIGH)
            PORTB |= (1 << (pin - 8));
        else
            PORTB &= ~(1 << (pin - 8));
    } else if (pin == A5) {
        if (value == HIGH)
            PORTC |= (1 << 5);
        else
            PORTC &= ~(1 << 5);
    }
}

float setAngle(double current, double set) { // measured in # of CW rotations from closed position
  delta = set - current; // Calculate delta here
  double N = fabs(delta*400); // number of pulses to arrive at set location
  double Delay = dt*1000000/N; // delay in microseconds to achieve consistent dt
  float new_pos = current;
  
  if (fabs(delta) <= d_min && set != 0){
      delayMicroseconds((unsigned int)(Delay*N));
      return new_pos;
  }else if (delta > 0){
      digitalWrite(CW,HIGH);
      digitalWrite(CCW, LOW);
      new_pos = set;
  }else if (delta < 0){
      digitalWrite(CW,LOW);
      digitalWrite(CCW, HIGH);
      new_pos = set;
  }
  for (int i = 0; i < (int)N; i++){
    digitalWrite(Pulse,HIGH);
    delayMicroseconds((unsigned int)Delay);
    digitalWrite(Pulse,LOW);
  }
  
  return new_pos;
}

// Main function for AVR testing
#ifdef AVR_MAIN
int main() {
    setup();
    
    while (1) {
        loop();
    }
    
    return 0;
}
#endif