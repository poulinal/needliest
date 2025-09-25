// AP 2025

#include "IPid.h"
#include <stdexcept>


class PIDController : public IPid {
private:
    int Last_dterm = 0;
    int Last_pterm = 0;
public:
    PIDController(int p, int i, int d) : k_p(p), k_i(i), k_d(d) {
        if (p < 0 || i < 0 || d < 0) {
            throw std::invalid_argument("PID constants must be non-negative");
        }
    }

    float pidController(float SP, float MV) override {
        // Calculate error
        int error = SP - MV;

        // Proportional term
        int pterm = k_p * error;

        // Integral term
        Last_iterm += k_i * error;

        // Derivative term
        int dterm = k_d * (error - Last_error);

        Last_error = error;
        Last_iterm = Last_iterm;
        Last_dterm = dterm;
        Last_pterm = pterm;

        // Combine terms
        float raw_output = pterm + Last_iterm + dterm;

        return raw_output;
    }


    void updateConstants(int p, int i, int d) override {
        if (p < 0 || i < 0 || d < 0) {
            throw std::invalid_argument("PID constants must be non-negative");
        }
        // Update constants
        const_cast<int&>(k_p) = p;
        const_cast<int&>(k_i) = i;
        const_cast<int&>(k_d) = d;
    }


    void updateSetpoint(int newSetpoint) override {
        setpoint = newSetpoint;
    }


    float translatePIDOutput(float rawOutput) override {
        A_min = 1e-6;
        A_max = 0.009;

        % Scale to physical area range
        Analog_output_scaled = max(min(analog_output, A_max), A_min);

        return Analog_output_scaled;
    }
};