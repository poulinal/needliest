// AP 2025

#include "PIDController.h"

// Out-of-line implementations for PIDController declared in PIDController.h

PIDController::PIDController(int p, int i, int d) : k_p(p), k_i(i), k_d(d),
    Last_dterm(0), Last_pterm(0), Last_iterm(0), Last_error(0), setpoint(0), A_min(1e-6f), A_max(0.009f) {
    // No-op
}

float PIDController::pidController(float MV) {
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

void PIDController::updateConstants(int p, int i, int d) {
    k_p = p;
    k_i = i;
    k_d = d;
}

void PIDController::updateSetpoint(double newSetpoint) {
    setpoint = (int)newSetpoint;
}

void PIDController::updateMinMax(float newMin, float newMax) {
    A_min = newMin;
    A_max = newMax;
}

float PIDController::translatePIDOutput(float rawOutput) {
    float analog_output_scaled = rawOutput;
    if (analog_output_scaled > A_max) analog_output_scaled = A_max;
    if (analog_output_scaled < A_min) analog_output_scaled = A_min;
    return analog_output_scaled;
}