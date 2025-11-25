// AP 2025
// Main file for AVR (Uno) PID Controller project

/**
 * To run:
 * Build command:
     mkdir -p build && "/Users/alexpoulin/local/avr/bin/avr-g++" -mmcu=atmega328p -DF_CPU=16000000UL -Os -Wall -Wextra -std=gnu++17 -fno-lto -I cpp/include -I cpp/src -I cpp/src/utilities -o build/main.elf cpp/src/main.cpp cpp/src/pidController.cpp cpp/src/unoController.cpp cpp/src/utilities/avr_wiring.cpp cpp/src/utilities/writeAvr.cpp cpp/src/utilities/delete_stub.cpp

 * Dont know what this command does:
    /Users/alexpoulin/local/avr/bin/avr-objcopy -O ihex -R .eeprom build/main.elf build/main.hex && ls -l build/main.hex

 * Flash command:
    /Users/alexpoulin/local/avr/bin/avrdude -c arduino -p atmega328p -P /dev/cu.usbserial-DN06A
5F2 -b 115200 -D -U flash:w:build/main.hex:i

 * Serial monitor command:
    screen /dev/cu.usbserial-DN06A5F2 115200
 * To exit screen: Ctrl-A K



 #test response:
 >....                                                                                                                     

PORT = '/dev/cu.usbserial-DN06A5F2'
BAUD = 115200

s = serial.Serial(PORT, BAUD, timeout=1)
# Opening the serial may reset the Uno; wait for it to come up.
time.sleep(2.0)

# Send a test packet the AVR expects: two comma-separated floats
s.write(b'1.234,5.678\n')
s.flush()

# Wait a little to let the AVR parse and print responses
time.sleep(10)

print("Reading replies (up to 10 lines):")
for _ in range(10):
    line = s.readline()
    if not line:
        break
    try:
        print(line.decode('ascii', errors='replace').strip())
    except Exception as e:
        print("RAW:", line)
s.close()
PY
 */

#include "IPid.h"
#include "PIDController.h"
#include "IUnoController.h"
#include "UnoController.h"
// #include "ILabJacks.h"
// #include "LabJacksController.h"
#include "writeAvr.h"
#include <avr/io.h>
#include <util/delay.h>
// #include <memory>
#define F_CPU 16000000UL
#define BAUD 9600  // or 115200, doesn't matter
#include <util/setbaud.h>


// // Instantiate concrete implementations via their interfaces.
// // Use statically-allocated concrete objects to avoid dynamic allocation
// // (operator new/delete may not be available on this target).
// static std::unique_ptr<IPid> pidController = std::make_unique<PIDController>(1,0,0);
// static std::unique_ptr<ILabJacks> labjackController = std::make_unique<LabJacksController>();
// static std::unique_ptr<IUnoController> unoController = std::make_unique<UnoController>();

// Concrete, statically-allocated objects (no dynamic allocation)
static PIDController pidControllerObj(1, 0, 0);
static IPid* pidController = &pidControllerObj;

// static LabJacksController labjackControllerObj;
// static ILabJacks* labjackController = &labjackControllerObj;

static UnoController unoControllerObj;
static IUnoController* unoController = &unoControllerObj;

double current = 0;

void setup() {
    double setpoint = 3; // Example setpoint value (in psi)
    pidController->updateSetpoint(setpoint);
    uart_puts("Setpoint: "); uart_put_fixed2(setpoint); uart_puts("\r\n");

    double min = 0.0;
    double max = 6.0;
    pidController->updateMinMax((float)min, (float)max);
    uart_puts("Output Range: "); uart_put_fixed2(min); uart_puts(" to "); uart_put_fixed2(max); uart_puts("\r\n");
    // // Configure built-in LED (Arduino UNO digital pin 13 / PB5) as output
    // DDRB |= (1 << DDB5);
}

// Read two comma-separated voltages from UART into v1 and v2.
// Returns 1 on success (values filled), 0 if no complete line or parse failure.
bool getLabJackVoltages(double *v1, double *v2) {
    char linebuf[48];
    linebuf[0] = '\0';  // Initialize buffer
    // uart_puts("Getting linebuf: "); uart_ptchars(linebuf); uart_puts("\r\n");
    // Avoid blocking if no data available
    if (!uart_data_available()) return false;
    // uart_puts("Getting linebuf2: "); uart_putchars(linebuf); uart_puts("\r\n");
    // NO delay - read immediately while timing is still aligned
    int len = uart_readline(linebuf, sizeof(linebuf));
    // uart_puts("Getting linebuf3: "); uart_putchars(linebuf); uart_puts("\r\n");
    // uart_puts("Got from serial (len="); 
    char lenbuf[8]; 
    // uart_puts(uint_to_str((unsigned long)len, lenbuf));
    // uart_puts("): '"); uart_puts(linebuf); uart_puts("'\r\n");
    if (len <= 0) return false;
    // if (parse_two_ascii_floats(linebuf, v1, v2)) {
    //     uart_puts("RX v1: "); uart_put_fixed2(*v1); uart_puts(" v2: "); uart_put_fixed2(*v2); uart_puts("\r\n");
    //     return true;
    // }
    // Single float parse (upstream only)
    *v1 = parse_ascii_float(linebuf);
    uart_puts("RX v1: "); uart_put_fixed2(*v1); uart_puts("\r\n");
    return true;
    // uart_puts("RX parse error. Got: '"); uart_puts(linebuf); uart_puts("'\r\n");
    // return false;
}

void loop() {
    // Read potentiometer on A5 (same pin used by UnoController)
    // uint16_t adc = unoController->pneumaticReadingRaw();
    double upstread = 0.0;
    double downstream = 0.0;
    uart_puts("Reading LabJack...\r\n");
    bool readBool = getLabJackVoltages(&upstread, &downstream);
    if (!readBool) {
        // adc and adc2 now contain the two voltages read from the desktop sender
        // uart_puts("LabJack read failed\r\n");
        // No data this cycle — try again on the next loop iteration.
        uart_puts("LabJack read failed, got: "); uart_put_fixed2((double)upstread); uart_puts(", "); uart_put_fixed2((double)downstream); uart_puts(" retrying...\r\n");
        _delay_ms(50);
        return;
    }
    uart_puts("Reading upstream: "); uart_put_fixed2((double)upstread); uart_puts(", "); uart_put_fixed2((double)downstream); uart_puts("\r\n");
    double rotations = unoController->pneumaticReadingRotation(); //alternatively we can get a rotation reading directly from A5 (like in potentiometer test)
    uart_puts("...... ADC:"); uart_put_fixed2((double)upstread); uart_puts(", "); uart_put_fixed2((double)downstream); uart_put_fixed2((double)rotations); uart_puts("\r\n");
    // uart_puts("...... LabJacks:"); uart_put_fixed2((double)labjackController->getVoltageAINO()); uart_puts("\r\n");
    // uart_puts("...... test: "); uart_put_fixed2((double)0); uart_puts("\r\n");

    // // Compute PID output: MV (measured value)
    float rawPid = pidController->pidController((float)downstream);
    float pidOutput = pidController->translatePIDOutput(rawPid); //translate such that is it between 0 and 6 rotations (physical needle valve bounds)

    // // Command the controller to move toward pidOutput
    uart_puts("setting angle: "); uart_put_fixed2((double)pidOutput); uart_puts("\r\n");
    current = unoController->setAngle(current, pidOutput);

    // // _delay_ms(100); //safety delay for now
}


// Simple test main function (minimal, compilable example)
int main() {
    // Objects are statically allocated above. UnoController constructor
    // calls setup() internally; however we still call the application
    // setup() to configure the PID setpoint and ranges.
    _delay_ms(5000); // wait for things to settle

    // Call application setup to initialize PID ranges, UART, and configure LED pin
    setup();

    while (1) {
        uart_puts("Starting loop\r\n");
        // Perform a visible blink: LED on 300ms, off 300ms
        // PORTB |= (1 << PORTB5);
        // _delay_ms(300);
        // PORTB &= ~(1 << PORTB5);
        // _delay_ms(300);
        _delay_ms(100);
        loop();
        _delay_ms(100);
    }
    return 0;
}