// UnoController.h - header for the UnoController concrete implementation
#pragma once
#include "IUnoController.h"
#include "avr_wiring.h"

class UnoController : public IUnoController {
public:
    UnoController();

    // Move to target angle in rotations
    float setAngle(double current, double set);

    // Read potentiometer raw/rotation
    float pneumaticReadingRaw() override;
    float pneumaticReadingRotation() override;

    // setup helper
    void setup();

private:
    // Stepper/motor pin assignments
    const int CW = 4; // + direction pin to stepper controller
    const int CCW = 3; // - direction
    const int Pulse = 2;
    const int PotPin = A5;
    float dt = 0.05;
    float d_min = 0.05; // max allowed diff. from set angle and
    // position before motor will respond (stops motor
    // from reacting to every bit of noise)   

    // private state
    float current = 0; // current position in # of CW rotations from closed position
    float delta = 0; // difference between current and set position
    
};
