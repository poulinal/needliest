// AP 2025
// Main file for AVR (Uno) PID Controller project

/**
 * To run:
 * Build command:
    /Users/alexpoulin/local/avr/bin/avr-g++ \
    -mmcu=atmega328p \
    -DF_CPU=16000000UL \
    -Os -Wall -Wextra \
    -std=gnu++17 \
    -fno-lto \
    -I cpp/include \
    -I cpp/src \
    -I cpp/src/utilities \
    -o build/main.elf \
    cpp/src/main.cpp \
    cpp/src/pidController.cpp \
    cpp/src/unoController.cpp \
    cpp/src/utilities/avr_wiring.cpp \
    cpp/src/utilities/writeAvr.cpp

 * Flash command:
    /Users/alexpoulin/local/avr/bin/avrdude \
      -c arduino -p atmega328p \
      -P /dev/cu.usbserial-DN06A5F2 \
      -b 115200 \
      -D \
      -U flash:w:build/main.hex:i

 * Serial monitor command:
    screen /dev/cu.usbserial-DN06A5F2 115200
 * To exit screen: Ctrl-A K
 */

#include "IPid.h"
#include "pidController.cpp"
#include "IUnoController.h"
#include <avr/io.h>
#include "IPid.h"
#include "pidController.cpp"
#include "IUnoController.h"
#include "unoController.cpp"
#include <avr/io.h>
#include <util/delay.h>

// Instantiate concrete implementations via their interfaces.
// Use pointers to avoid slicing and to allow polymorphism.
static IPid* pidController = nullptr;
static IUnoController* unoController = nullptr;

double current = 0;

// Simple test main function (minimal, compilable example)
int main() {
    // Create concrete instances. PID constants are placeholders (tune as needed).
    pidController = new PIDController(1, 0, 0);
    unoController = new UnoController();

    setup();

    while(1) {
        loop();

    }
    return 0;
}

void setup() {
    double setpoint = 100; // Example setpoint value (in psi)
    pidController->updateSetpoint(setpoint);

    double min = 0.0;
    double max = 6.0;
    pidController->updateMinMax((float)min, (float)max);
}

void loop() {

    // Read potentiometer on A5 (same pin used by UnoController)
    uint16_t adc = unoController->pneumaticReadingRaw();
    // double rotations = unoController->pneumaticReadingRotation(); //alternatively we can get a rotation reading directly from A5 (like in potentiometer test)

    // Compute PID output: MV (measured value)
    float rawPid = pidController->pidController((float)adc);
    float pidOutput = pidController->translatePIDOutput(rawPid); //translate such that is it between 0 and 6 rotations (physical needle valve bounds)

    // Command the controller to move toward pidOutput
    current = unoController->setAngle(current, pidOutput);

    _delay_ms(100); //safety delay for now
}