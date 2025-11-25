// AP 2025

#ifndef ILabJacks_H
#define ILabJacks_H

class ILabJacks {
public:
    virtual ~ILabJacks() = default; // Virtual destructor

    // virtual void readVoltages() = 0; // reads voltages from the labjack
    virtual double getVoltageAINO() const = 0;
    virtual double getVoltageAINOne() const = 0;
    virtual double getVoltageAINTwo() const = 0;
    virtual double getVoltageAINThree() const = 0;
};

#endif // ILabJacksM_H