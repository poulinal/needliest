//AP 2026
//simple autotuning algorithm for our PID Controller built in LabVIEW.

/*
 * relay_autotune.c
 *
 * Relay Feedback (Åström-Hägglund) autotuner for PID pressure control.
 * Designed to compile as a DLL/.so and be called from LabVIEW via
 * Call Library Function Node (C calling convention).
 *
 * Build (Windows): cl /LD relay_autotune.c /Fe:relay_autotune.dll
 *                  (use the x64 or x86 tools prompt matching LabVIEW's bitness)
 * Build (Linux):   gcc -shared -fPIC -O2 -o relay_autotune.so relay_autotune.c -lm
 * Build (Mac):     clang -shared -fPIC -O2 -arch arm64 -arch x86_64 \
 *                        -o relay_autotune.dylib relay_autotune.c
 *
 * LabVIEW usage:
 *   Before the loop: autotune_init(...), then autotune_start()
 *   Every iteration: valve = autotune_update(pv)       (fixed dt)
 *                or  valve = autotune_update_dt(pv, dt) (measured dt)
 *                    state = autotune_get_state()
 *   When state != RUNNING, stop feeding the relay output to the valve and
 *   read the gains (COMPLETE) or fall back to a safe output (ERROR/TIMEOUT).
 *   While not running, update returns `bias`, never 0.
 *
 * State values: 0 IDLE, 1 RUNNING, 2 COMPLETE, 3 ERROR, 4 TIMEOUT
 */

#include <math.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* Export macro for DLL visibility */
#ifdef _WIN32
  #define EXPORT __declspec(dllexport)
#else
  #define EXPORT __attribute__((visibility("default")))
#endif

/* ── Autotuner state machine ────────────────────────────────────── */
typedef enum {
    STATE_IDLE,
    STATE_RELAY_RUNNING,
    STATE_COMPLETE,
    STATE_ERROR,
    STATE_TIMEOUT
} AutotuneState;

/* Tuning rule selection */
typedef enum {
    RULE_ZIEGLER_NICHOLS,
    RULE_TYREUS_LUYBEN,
    RULE_SOME_OVERSHOOT,   /* Åström-Hägglund "some overshoot" */
    RULE_NO_OVERSHOOT      /* Åström-Hägglund "no overshoot"   */
} TuningRule;

/* ── Persistent context — one instance per control loop ─────────── */
typedef struct {
    /* Configuration */
    double setpoint;           /* Target pressure                     */
    double relay_amplitude;    /* ±d output perturbation (valve %)    */
    double hysteresis;         /* Deadband around setpoint (optional) */
    int    min_cycles;         /* Cycles to average (default 4)       */
    int    tuning_rule;        /* TuningRule enum                     */
    double dt;                 /* Sample period in seconds            */
    double bias;               /* Output the relay swings around      */
    int    direction;          /* +1: more output raises PV, -1: lowers */
    double timeout;            /* Max test duration (s), <= 0 = none  */

    /* Runtime */
    AutotuneState state;
    double relay_output;       /* Current relay command               */
    int    cycle_count;
    double peak_high;          /* Max PV in current full cycle        */
    double peak_low;           /* Min PV in current full cycle        */
    int    relay_positive;     /* 1 = relay high, 0 = relay low       */
    double last_crossing_time; /* Time of last zero-crossing          */
    double elapsed;            /* Running time                        */

    /* Accumulators for averaging across cycles */
    double sum_amplitude;
    int    amplitude_count;
    double sum_period;
    int    period_count;

    /* Results */
    double Ku;                 /* Ultimate gain                       */
    double Tu;                 /* Ultimate period (s)                 */
    double Kp;
    double Ki;
    double Kd;
} AutotuneCtx;

/* ── Single static instance (simplifies LabVIEW integration) ──── */
static AutotuneCtx g_ctx;

/* ── Initialise / reset the autotuner ──────────────────────────── */
EXPORT void autotune_init(double setpoint,
                          double relay_amplitude,
                          double hysteresis,
                          int    min_cycles,
                          int    tuning_rule,
                          double dt,
                          double bias,
                          int    direction,
                          double timeout)
{
    memset(&g_ctx, 0, sizeof(AutotuneCtx));
    g_ctx.setpoint        = setpoint;
    g_ctx.relay_amplitude  = fabs(relay_amplitude);
    g_ctx.hysteresis       = (hysteresis > 0.0) ? hysteresis : 0.0;
    g_ctx.min_cycles       = (min_cycles >= 2) ? min_cycles : 4;
    g_ctx.tuning_rule      = tuning_rule;
    g_ctx.dt               = dt;
    g_ctx.bias             = bias;
    g_ctx.direction        = (direction < 0) ? -1 : 1;
    g_ctx.timeout          = timeout;
    g_ctx.state            = STATE_IDLE;
    g_ctx.relay_positive   = 1;
    g_ctx.relay_output     = bias;
    g_ctx.peak_high        = -1e30;
    g_ctx.peak_low         =  1e30;
}

/* ── Start the relay test ──────────────────────────────────────── */
EXPORT void autotune_start(void)
{
    g_ctx.state            = STATE_RELAY_RUNNING;
    g_ctx.cycle_count      = 0;
    g_ctx.sum_amplitude    = 0.0;
    g_ctx.amplitude_count  = 0;
    g_ctx.sum_period       = 0.0;
    g_ctx.period_count     = 0;
    g_ctx.elapsed          = 0.0;
    g_ctx.peak_high        = -1e30;
    g_ctx.peak_low         =  1e30;
    g_ctx.last_crossing_time = 0.0;
    g_ctx.relay_positive   = 1;
    g_ctx.relay_output     = g_ctx.bias + g_ctx.direction * g_ctx.relay_amplitude;
}

/* ── Apply tuning rules from Ku, Tu ────────────────────────────── */
static void compute_pid(AutotuneCtx *ctx)
{
    double Ku = ctx->Ku;
    double Tu = ctx->Tu;

    switch (ctx->tuning_rule) {
    case RULE_TYREUS_LUYBEN:
        ctx->Kp = Ku / 3.2;
        ctx->Ki = ctx->Kp / (2.2 * Tu);
        ctx->Kd = 0.0;  /* TL is PI only */
        break;
    case RULE_SOME_OVERSHOOT:
        ctx->Kp = Ku / 3.0;
        ctx->Ki = ctx->Kp / (Tu * 0.8);
        ctx->Kd = ctx->Kp * Tu / 10.0;
        break;
    case RULE_NO_OVERSHOOT:
        ctx->Kp = Ku / 5.0;
        ctx->Ki = ctx->Kp / (Tu * 1.0);
        ctx->Kd = ctx->Kp * Tu / 6.0;
        break;
    case RULE_ZIEGLER_NICHOLS:
    default:
        ctx->Kp = 0.6 * Ku;
        ctx->Ki = ctx->Kp / (0.5 * Tu);   /* Ki = Kp / Ti */
        ctx->Kd = ctx->Kp * 0.125 * Tu;   /* Kd = Kp * Td */
        break;
    }
}

/* ── Call every scan cycle with current PV and the actual period ──
 *  Returns the valve command (bias ± relay amplitude).
 *  When state == STATE_COMPLETE, the Kp/Ki/Kd fields are valid.
 */
EXPORT double autotune_update_dt(double pv, double dt)
{
    AutotuneCtx *ctx = &g_ctx;
    if (ctx->state != STATE_RELAY_RUNNING)
        return ctx->bias;

    double error = pv - ctx->setpoint;
    ctx->elapsed += (dt > 0.0) ? dt : ctx->dt;

    if (ctx->timeout > 0.0 && ctx->elapsed > ctx->timeout) {
        ctx->state = STATE_TIMEOUT;
        ctx->relay_output = ctx->bias;
        return ctx->bias;
    }

    /* Track peaks over the whole cycle — the PV keeps moving after the
     * relay switches because of process lag, so the true extremes land
     * in the opposite half-cycle from the one that caused them. */
    if (pv > ctx->peak_high) ctx->peak_high = pv;
    if (pv < ctx->peak_low)  ctx->peak_low  = pv;

    /* Detect zero-crossing with hysteresis */
    int do_switch = 0;
    if (ctx->relay_positive && error > ctx->hysteresis)
        do_switch = 1;
    else if (!ctx->relay_positive && error < -ctx->hysteresis)
        do_switch = 1;

    if (do_switch) {
        /* Switching relay direction */
        if (ctx->relay_positive) {
            /* Was high → going low: completed a half-cycle (high side) */
            ctx->relay_positive = 0;
            ctx->relay_output   = ctx->bias - ctx->direction * ctx->relay_amplitude;
        } else {
            /* Was low → going high: completed a full cycle */
            ctx->relay_positive = 1;
            ctx->relay_output   = ctx->bias + ctx->direction * ctx->relay_amplitude;

            /* Record period from last low→high crossing */
            if (ctx->cycle_count > 0 && ctx->last_crossing_time > 0.0) {
                double period = ctx->elapsed - ctx->last_crossing_time;
                if (period > 0.0) {
                    ctx->sum_period += period;
                    ctx->period_count++;
                }
            }

            /* Record amplitude from peak-to-peak (skip the first, partial cycle) */
            if (ctx->cycle_count > 0) {
                double amp = (ctx->peak_high - ctx->peak_low) / 2.0;
                if (amp > 0.0) {
                    ctx->sum_amplitude += amp;
                    ctx->amplitude_count++;
                }
            }

            ctx->last_crossing_time = ctx->elapsed;
            ctx->peak_high = -1e30;
            ctx->peak_low  =  1e30;
            ctx->cycle_count++;

            /* Enough cycles? Compute results. */
            if (ctx->period_count >= ctx->min_cycles) {
                double a  = (ctx->amplitude_count > 0)
                          ? ctx->sum_amplitude / (double)ctx->amplitude_count : 0.0;
                double Tu = ctx->sum_period / (double)ctx->period_count;
                double h  = ctx->hysteresis;

                if (a > h && a > 1e-12 && Tu > 1e-12) {
                    ctx->Tu = Tu;
                    /* Describing function of a relay with hysteresis */
                    ctx->Ku = (4.0 * ctx->relay_amplitude) / (M_PI * sqrt(a * a - h * h));
                    compute_pid(ctx);
                    ctx->state = STATE_COMPLETE;
                } else {
                    ctx->state = STATE_ERROR;
                }
                ctx->relay_output = ctx->bias;
            }
        }
    }

    return ctx->relay_output;
}

/* ── Fixed-period variant: uses dt given to autotune_init ──────── */
EXPORT double autotune_update(double pv)
{
    return autotune_update_dt(pv, g_ctx.dt);
}

/* ── Accessor functions for LabVIEW (no pointer args needed) ───── */
EXPORT int    autotune_get_state(void)   { return (int)g_ctx.state; }
EXPORT double autotune_get_Kp(void)      { return g_ctx.Kp; }
EXPORT double autotune_get_Ki(void)      { return g_ctx.Ki; }
EXPORT double autotune_get_Kd(void)      { return g_ctx.Kd; }
EXPORT double autotune_get_Ku(void)      { return g_ctx.Ku; }
EXPORT double autotune_get_Tu(void)      { return g_ctx.Tu; }
EXPORT int    autotune_get_cycles(void)  { return g_ctx.cycle_count; }
EXPORT double autotune_get_elapsed(void) { return g_ctx.elapsed; }
