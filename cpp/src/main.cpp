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




 * simplified now in root needliest folder just:
    make all
    make flash


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
#include <avr/wdt.h>
// #include <memory>
#define F_CPU 16000000UL
// #define BAUD 9600  // or 115200, doesn't matter
// #define BAUD 115200
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
int labjack_fail_count = 0;

void setup() {
    // UART already initialized in main() - don't reinitialize
    
    double setpoint = 1.0; // Example setpoint value (in psi)
    pidController->updateSetpoint(setpoint);
    uart_puts("Setpoint: "); uart_put_fixed2(setpoint); uart_puts("\r\n");

    double min = 0.0;
    double max = 6.0;
    pidController->updateMinMax((float)min, (float)max);
    uart_puts("Output Range: "); uart_put_fixed2(min); uart_puts(" to "); uart_put_fixed2(max); uart_puts("\r\n");
    // // Configure built-in LED (Arduino UNO digital pin 13 / PB5) as output
    // DDRB |= (1 << DDB5);

    uart_puts("\r\n--- DEBUG TEST START ---\r\n");
    double test_val = parse_ascii_float("4.32");
    uart_puts("Test '4.32' -> "); uart_put_fixed2(test_val); uart_puts("\r\n");
    
    double test_val2 = parse_ascii_float("4.32 0.00");
    uart_puts("Test '4.32 0.00' -> "); uart_put_fixed2(test_val2); uart_puts("\r\n");
    uart_puts("--- DEBUG TEST END ---\r\n");
}

// Read voltages from UART into v1 and v2.
// Returns 1 on success (values filled), 0 if no complete line or parse failure.
bool getLabJackVoltages(double *v1, double *v2) {
    char linebuf[48];
    linebuf[0] = '\0';  // Initialize buffer
    
    // Return immediately if no data available (non-blocking)
    if (!uart_data_available()) return false;
    
    // Try to read a line
    int len = uart_readline(linebuf, sizeof(linebuf));
    if (len <= 0) return false;  // No complete line yet
    
    // Debug: print raw received string
    uart_puts("RAW RX (len=");
    char lenbuf[8];
    uart_puts(uint_to_str((unsigned long)len, lenbuf));
    uart_puts("): '");
    uart_putchars(linebuf);
    uart_puts("'\r\n");
    
    // Try to parse two comma-separated floats first
    if (parse_two_ascii_floats(linebuf, v1, v2)) {
        uart_puts("RX v1: "); uart_put_fixed2(*v1); uart_puts(" v2: "); uart_put_fixed2(*v2); uart_puts("\r\n");
        return true;
    }
    
    // If that fails, try parsing just the first float
    *v1 = parse_ascii_float(linebuf);
    *v2 = 0.0;  // Default v2 to 0
    uart_puts("RX v1: "); uart_put_fixed2(*v1); uart_puts(" (single value, parse_two_ascii_floats failed)\r\n");
    return true;
}

void loop() {
    double upstream = 0.0;
    double downstream = 0.0;
    bool readBool = getLabJackVoltages(&upstream, &downstream);
    uart_puts("Read voltages: ");
    uart_puts("Upstream: "); uart_put_fixed2(upstream); uart_puts(" V, ");
    uart_puts("Downstream: "); uart_put_fixed2(downstream); uart_puts(" V\r\n");
    if (!readBool) {
        uart_puts("No valid labjack data\r\n");
    }

}


// Simple test main function (minimal, compilable example)
int main() {
    // Disable watchdog timer
    wdt_disable();
    
    // Initialize UART FIRST before any delays to debug where we're hanging
    uart_init();
    uart_puts("\r\n=== MAIN STARTED ===\r\n");
    
    uart_puts("Before setup\r\n");
    setup();
    uart_puts("After setup\r\n");

    uart_puts("Entering main loop...\r\n");
    while (1) {
        _delay_ms(300);  // Wait 300ms between reads (Python sends at 200ms, so we'll catch them)
        loop();
    }
    return 0;
}