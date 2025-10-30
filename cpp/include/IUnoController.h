// AP 2025

#ifndef IUNO_CONTROLLER_H
#define IUNO_CONTROLLER_H

class IUnoController {
public:
    virtual ~IUnoController() = default; // Virtual destructor

    virtual float setAngle(double current, double set) = 0; // set rotation for the Uno Controlling Motor

    virtual float pneumaticReadingRotation() = 0; // get the pneumatic reading (in terms of rotations)

    virtual float pneumaticReadingRaw() = 0; // get the raw pneumatic reading
};

#endif // IUNO_CONTROLLER_H