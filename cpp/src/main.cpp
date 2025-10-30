// AP 2025
// Main file for AVR PID Controller project

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