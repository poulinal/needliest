#!/usr/bin/env python3
# pip install labjack-ljm pyserial

# or similarly,
# # install listed deps
# and make a virtual environment first if desired
# python -m venv venv
# python -m pip install -r requirements.txt

# # or install editable package (reads pyproject.toml)
# python -m pip install -e .
import time
from pyparsing import line
import serial
import serial.tools.list_ports

# Import labjack LJM wrapper robustly and expose a module reference `ljm`.
try:
    import labjack.ljm as ljm
    HAS_LJM = True
except Exception as _e:
    # fall back to specific imports if the package exposes them differently
    try:
        from labjack.ljm import open as ljm_open, eReadName
        ljm = None
        HAS_LJM = True
    except Exception as e:
        print("labjack.ljm import failed:", e)
        HAS_LJM = False

def find_arduino_port():
    candidates = []
    for p in serial.tools.list_ports.comports():
        name = p.device
        desc = (p.description or "").lower()
        hwid = (p.hwid or "").lower()
        if "usbserial" in name.lower() or "usbserial" in hwid or "arduino" in desc or "usbmodem" in name.lower() or "ftdi" in desc:
            candidates.append(name)
    return candidates

ports = find_arduino_port()
if ports:
    SERIAL_PORT = ports[0]
    print("Auto-detected serial port:", SERIAL_PORT)
else:
    print("No likely Arduino serial port found. Available ports:")
    for p in serial.tools.list_ports.comports():
        print(" -", p.device, "|", p.description, "|", p.hwid)
    raise SystemExit("No serial port")

# Configure these
# SERIAL_PORT = "/dev/cu.usbserial-DN06A5F2"   # change to your Uno serial device
BAUD = 115200
POLL_HZ = 5.0

def read_labjack():
    if not HAS_LJM:
        raise RuntimeError("LJM not available")

    handle = None
    try:
        # prefer module-style API
        if ljm is not None:
            try:
                handle = ljm.openS("T7", "USB", "ANY")
            except Exception:
                handle = ljm.open("T7", "USB", "ANY")

            # Optionally set the input range for AIN0 before reading.
            # Typical values: 10.0 for +/-10V, 1.0 for +/-1V, etc.
            # If the LJM binding supports eWriteName, use it to set AIN0_RANGE.
            try:
                AIN0_RANGE = 10.0
                ljm.eWriteName(handle, "AIN0_RANGE", float(AIN0_RANGE))
            except Exception:
                # If eWriteName isn't available or fails, continue without setting range
                pass

            raw0 = ljm.eReadName(handle, "AIN0")
            raw2 = ljm.eReadName(handle, "AIN2")
        else:
            # fall back to function imports
            handle = ljm_open("T7", "USB", "ANY")
            # Attempt to set range if function available in this import style
            try:
                # Some wrappers may expose eWriteName at module level
                from labjack.ljm import eWriteName as _eWriteName
                AIN0_RANGE = 10.0
                _eWriteName(handle, "AIN0_RANGE", float(AIN0_RANGE))
            except Exception:
                pass
            raw0 = eReadName(handle, "AIN0")
            raw2 = eReadName(handle, "AIN2")

        # Normalize return values: some wrappers return a tuple (err, value)
        def normalize(raw):
            if isinstance(raw, tuple) or isinstance(raw, list):
                # try to find a numeric element (prefer last)
                for x in reversed(raw):
                    try:
                        return float(x)
                    except Exception:
                        continue
                raise ValueError("no numeric value in LJM response")
            else:
                return float(raw)

        v0 = normalize(raw0)
        v2 = normalize(raw2)

        return v0, v2
    finally:
        try:
            if ljm is not None and handle is not None:
                ljm.close(handle)
        except Exception:
            pass
        
def read_fake_labjack():
    import random
    v0 = random.uniform(2.0, 3.0)
    v2 = random.uniform(2.0, 3.0)
    return v0, v2

def main():
    def open_serial():
        while True:
            try:
                s = serial.Serial(SERIAL_PORT, BAUD, timeout=1)
                print("Serial open:", s.name)
                return s
            except Exception as e:
                print("Serial open failed:", e)
                time.sleep(1.0)

    ser = open_serial()
    try:
        while True:
            # read from labjack (use labjack-ljm API)
            try:
                # v0, v2 = read_labjack()
                v0, v2 = read_fake_labjack()
                print(f"LabJack read: v0={v0:.3f}, v2={v2:.3f}")
            except Exception as e:
                print("LabJack read error:", e)
                v0, v2 = 0.0, 0.0

            # line = f"{v0:.3f},{v2:.3f}\n"
            line = f"{v0:.3f},{v2:.3f}\n"

            print("Sending line:", line.strip())
            # Attempt to write; on failure try to reopen the port and continue
            try:
                ser.write(line.encode('ascii'))
                ser.flush()
                print("sent:", line.strip())
            except Exception as e:
                print("Serial write failed:", e)
                try:
                    ser.close()
                except Exception:
                    pass
                # Try to reopen the serial port with backoff
                for attempt in range(1, 11):
                    try:
                        ser = serial.Serial(SERIAL_PORT, BAUD, timeout=1)
                        print("Reopened serial on attempt", attempt)
                        break
                    except Exception as e2:
                        print("Reopen attempt", attempt, "failed:", e2)
                        time.sleep(min(1.0 * attempt, 5.0))
                else:
                    print("Could not reopen serial; will retry next loop iteration")

            time.sleep(1.0 / POLL_HZ)
    finally:
        try:
            ser.close()
        except Exception:
            pass

if __name__ == "__main__":
    main()