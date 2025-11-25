#pragma once
#include "IPid.h"

// Concrete PID implementation - declarations only.
class PIDController : public IPid {
public:
    // Matches implementation in pidController.cpp (int args)
    PIDController(int p, int i, int d);
    virtual ~PIDController() = default;

    // IPid interface
    float pidController(float measurement) override;
    float translatePIDOutput(float raw) override;
    void updateSetpoint(double s) override;
    void updateMinMax(float minv, float maxv) override;
    void updateConstants(int p, int i, int d) override;

    // other public helpers if any
private:
    // private state
    int k_p, k_i, k_d;
    int Last_dterm;
    int Last_pterm;
    int Last_iterm;
    int Last_error;
    int setpoint;
    float A_min;
    float A_max;
};