// AP 2025

#ifndef IPID_H
#define IPID_H

class IPid {
public:
    virtual ~IPid() = default; // Virtual destructor



    virtual float pidController(float MV) = 0; // gets a raw output from the PID controller

    virtual void updateConstants(int p, int i, int d) = 0; // updates the PID constants

    virtual void updateSetpoint(double newSetpoint) = 0; // updates the setpoint

    virtual void updateMinMax(float newMin, float newMax) = 0; // updates the min and max output range

    virtual float translatePIDOutput(float rawOutput) = 0; // translates the raw output to something the system can use
};

#endif // IPID_H