//AP 2026
//simple autotuning algorithm for our PID Controller built in LabVIEW. 

/*
 * relay_autotune.c
 * 
 * Relay Feedback (Åström-Hägglund) autotuner for PID pressure control.
 * Designed to compile as a DLL/.so and be called from LabVIEW via
 * Call Library Function Node.
 *
 * Build (Windows): cl /LD relay_autotune.c /Fe:relay_autotune.dll
 * Build (Linux):   gcc -shared -fPIC -o relay_autotune.so relay_autotune.c -lm
 * Build (Mac):     gcc -shared -fPIC -o relay_autotune.dylib relay_autotune.c -lm
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
    STATE_ERROR
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

    /* Runtime */
    AutotuneState state;
    double relay_output;       /* Current relay command               */
    int    cycle_count;
    double peak_high;          /* Max PV in current half-cycle        */
    double peak_low;           /* Min PV in current half-cycle        */
    int    relay_positive;     /* 1 = relay high, 0 = relay low       */
    double last_crossing_time; /* Time of last zero-crossing          */
    double elapsed;            /* Running time                        */

    /* Accumulators for averaging across cycles */
    double sum_amplitude;
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
                          double dt)
{
    memset(&g_ctx, 0, sizeof(AutotuneCtx));
    g_ctx.setpoint        = setpoint;
    g_ctx.relay_amplitude  = relay_amplitude;
    g_ctx.hysteresis       = (hysteresis > 0.0) ? hysteresis : 0.0;
    g_ctx.min_cycles       = (min_cycles > 2) ? min_cycles : 4;
    g_ctx.tuning_rule      = tuning_rule;
    g_ctx.dt               = dt;
    g_ctx.state            = STATE_IDLE;
    g_ctx.relay_positive   = 1;
    g_ctx.relay_output     = relay_amplitude;
    g_ctx.peak_high        = -1e30;
    g_ctx.peak_low         =  1e30;
}

/* ── Start the relay test ──────────────────────────────────────── */
EXPORT void autotune_start(void)
{
    g_ctx.state            = STATE_RELAY_RUNNING;
    g_ctx.cycle_count      = 0;
    g_ctx.sum_amplitude    = 0.0;
    g_ctx.sum_period       = 0.0;
    g_ctx.period_count     = 0;
    g_ctx.elapsed          = 0.0;
    g_ctx.peak_high        = -1e30;
    g_ctx.peak_low         =  1e30;
    g_ctx.last_crossing_time = 0.0;
    g_ctx.relay_positive   = 1;
    g_ctx.relay_output     = g_ctx.relay_amplitude;
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

/* ── Call every scan cycle with current PV ─────────────────────── 
 *  Returns the relay output to send to the valve.
 *  When state == STATE_COMPLETE, the Kp/Ki/Kd fields are valid.
 */
EXPORT double autotune_update(double pv)
{
    AutotuneCtx *ctx = &g_ctx;
    if (ctx->state != STATE_RELAY_RUNNING)
        return 0.0;

    double error = pv - ctx->setpoint;
    ctx->elapsed += ctx->dt;

    /* Track peaks in each half-cycle */
    if (ctx->relay_positive) {
        if (pv > ctx->peak_high) ctx->peak_high = pv;
    } else {
        if (pv < ctx->peak_low)  ctx->peak_low  = pv;
    }

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
            ctx->relay_output   = -ctx->relay_amplitude;
        } else {
            /* Was low → going high: completed a full cycle */
            ctx->relay_positive = 1;
            ctx->relay_output   = ctx->relay_amplitude;

            /* Record period from last low→high crossing */
            if (ctx->cycle_count > 0 && ctx->last_crossing_time > 0.0) {
                double period = ctx->elapsed - ctx->last_crossing_time;
                if (period > 0.0) {
                    ctx->sum_period += period;
                    ctx->period_count++;
                }
            }

            /* Record amplitude from peak-to-peak */
            if (ctx->cycle_count > 0) {
                double amp = (ctx->peak_high - ctx->peak_low) / 2.0;
                if (amp > 0.0) {
                    ctx->sum_amplitude += amp;
                }
            }

            ctx->last_crossing_time = ctx->elapsed;
            ctx->peak_high = -1e30;
            ctx->peak_low  =  1e30;
            ctx->cycle_count++;

            /* Enough cycles? Compute results. */
            if (ctx->period_count >= ctx->min_cycles) {
                double a  = ctx->sum_amplitude / (double)ctx->period_count;
                double Tu = ctx->sum_period    / (double)ctx->period_count;

                if (a > 1e-12 && Tu > 1e-12) {
                    ctx->Tu = Tu;
                    ctx->Ku = (4.0 * ctx->relay_amplitude) / (M_PI * a);
                    compute_pid(ctx);
                    ctx->state = STATE_COMPLETE;
                } else {
                    ctx->state = STATE_ERROR;
                }
            }
        }
    }

    return ctx->relay_output;
}

/* ── Accessor functions for LabVIEW (no pointer args needed) ───── */
EXPORT int    autotune_get_state(void)  { return (int)g_ctx.state; }
EXPORT double autotune_get_Kp(void)     { return g_ctx.Kp; }
EXPORT double autotune_get_Ki(void)     { return g_ctx.Ki; }
EXPORT double autotune_get_Kd(void)     { return g_ctx.Kd; }
EXPORT double autotune_get_Ku(void)     { return g_ctx.Ku; }
EXPORT double autotune_get_Tu(void)     { return g_ctx.Tu; }
EXPORT int    autotune_get_cycles(void) { return g_ctx.cycle_count; }

