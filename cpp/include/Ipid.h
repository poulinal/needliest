// AP 2025

class IPid {
private:
    int Last_error = 0;
    int Last_iterm = 0;

    int needleValveLowerBound = 0; // in radians
    int needleValveUpperBound = 90; // in radians

public:
    const int k_i;
    const int k_p;
    const int k_d;
    int setpoint = 4136854; // in Pa, 600 psi


    virtual ~IPid() = default; // Virtual destructor

    virtual float pidController(float SP, float MV); // gets a raw output from the PID controller

    virtual void updateConstants(int p, int i, int d); // updates the PID constants

    virtual void updateSetpoint(int newSetpoint); // updates the setpoint

    virtual float translatePIDOutput(float rawOutput); // translates the raw output to something the system can use
};