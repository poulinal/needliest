// AP 2025
#pragma once
#include "ILabJacks.h"

// Concrete LabJack implementation - declarations only.
class LabJacksController : public ILabJacks {
public:
    LabJacksController();
    virtual ~LabJacksController() override;
    double getVoltageAINO() const override;
    double getVoltageAINOne() const override;
    double getVoltageAINTwo() const override;
    double getVoltageAINThree() const override;
private:
    // private state if any
    int error;
    int handle;
    double volt0;
    double volt1;
    double volt2;
    double volt3;
};