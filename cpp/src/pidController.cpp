// AP 2025

#include "IPid.h"
// Note: stdexcept not available in AVR-libc
// #include <stdexcept>

class PIDController : public IPid {
private:
    int k_p, k_i, k_d;
    int Last_dterm = 0;
    int Last_pterm = 0;
    int Last_iterm = 0;
    int Last_error = 0;
    int setpoint = 0;
    float A_min = 1e-6f;
    float A_max = 0.009f;

public:
    PIDController(int p, int i, int d) : k_p(p), k_i(i), k_d(d) {
        // Note: AVR doesn't have exceptions, so we'll use simple validation
        // if (p < 0 || i < 0 || d < 0) {
        //     throw std::invalid_argument("PID constants must be non-negative");
        // }
    }

    float pidController(float MV) override {
        // Calculate error
        int error = (int)(setpoint - MV);

        // Proportional term
        int pterm = k_p * error;

        // Integral term
        Last_iterm += k_i * error;

        // Derivative term
        int dterm = k_d * (error - Last_error);

        Last_error = error;
        Last_dterm = dterm;
        Last_pterm = pterm;

        // Combine terms
        float raw_output = (float)(pterm + Last_iterm + dterm);

        return raw_output;
    }

    void updateConstants(int p, int i, int d) override {
        // Note: AVR doesn't have exceptions, so we'll use simple validation
        // if (p < 0 || i < 0 || d < 0) {
        //     throw std::invalid_argument("PID constants must be non-negative");
        // }
        k_p = p;
        k_i = i;
        k_d = d;
    }

    void updateSetpoint(double newSetpoint) override {
        setpoint = newSetpoint;
    }

    void updateMinMax(float newMin, float newMax) {
        A_min = newMin;
        A_max = newMax;
    }

    float translatePIDOutput(float rawOutput) override {
        // Scale to physical area range
        float analog_output_scaled = rawOutput;
        
        // Clamp to min/max bounds (AVR-compatible min/max)
        if (analog_output_scaled > A_max) analog_output_scaled = A_max;
        if (analog_output_scaled < A_min) analog_output_scaled = A_min;

        return analog_output_scaled;
    }
};