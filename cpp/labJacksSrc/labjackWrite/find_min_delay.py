#!/usr/bin/env python3
"""
find_min_delay.py

Empirically finds minimal inter-byte delay (ms) that gives reliable full-line reception
from the AVR at a given baud rate and serial port. Sends N trials per delay and checks
for the AVR to report the received line (looks for "Got from serial" output in response).

Usage: python3 find_min_delay.py
Adjust `PORT` and `BAUD` below if needed.
"""
import serial, time

PORT = '/dev/cu.usbserial-DN06A5F2'
BAUD = 9600
# MSG = b'1,2\n'
MSG = b'1.23,5.67\n'
TRIALS = 20
# Delays to test (ms)
DELAY_LIST = [0, 1, 2, 3, 4, 5, 7, 10, 15, 20, 30, 50]
# Success criterion: fraction of trials that must be received fully
REQUIRED_SUCCESS_RATE = 1.0

s = serial.Serial(PORT, BAUD, timeout=0.5)
print('Opened', PORT, 'at', BAUD)
# Allow device to reset and print startup
time.sleep(3.0)

# clear buffers
s.reset_input_buffer()
s.reset_output_buffer()

results = []
for delay_ms in DELAY_LIST:
    print('\nTesting delay = {} ms'.format(delay_ms))
    successes = 0
    for t in range(TRIALS):
        # send byte-by-byte
        for b in MSG:
            s.write(bytes([b]))
            s.flush()
            time.sleep(delay_ms / 1000.0)
        # wait a little for AVR to print response lines
        time.sleep(0.02)
        # read any available lines up to small timeout and verify content
        got_full = False
        deadline = time.time() + 0.5
        buf = b''
        while time.time() < deadline:
            chunk = s.read(1)
            if chunk:
                buf += chunk
                # Try to decode and check substrings
                try:
                    text = buf.decode('ascii', errors='replace')
                except Exception:
                    text = ''
                # Check for the AVR line that prints the raw received string
                # e.g. Got from serial (len=6): '1..678'
                # if 'Got from serial' in text:
                #     # try to extract the quoted payload
                #     import re
                #     m = re.search(r"Got from serial \(len=\d+\):\s*'([^']*)'", text)
                #     if m:
                #         payload = m.group(1)
                #         # compare to the message we sent (strip trailing newline)
                #         sent = MSG.decode('ascii', errors='replace').strip()
                #         if payload.strip() == sent:
                #             got_full = True
                #             break
                # Check for the AVR numeric echo 'RX v1: ' followed by a number
                if 'RX v1:' in text:
                    import re
                    m = re.search(r'RX v1:\s*([-+]?\d*\.?\d+)', text)
                    if m:
                        try:
                            rxval = float(m.group(1))
                            sent = MSG.decode('ascii', errors='replace').split(',')[0]
                            sentval = float(sent)
                            print('    sent val:', sentval, '  rx val:', rxval)
                            if abs(rxval - sentval) < 1e-6:
                                got_full = True
                                break
                        except Exception:
                            pass
                # also accept any other mention as before
                # if 'Got from serial' in text or "RX v1:" in text:
                #     # if we couldn't parse, still mark as partial success
                #     got_full = True
                #     break
            else:
                # no data right now
                time.sleep(0.005)
        if got_full:
            successes += 1
        else:
            # drain a bit to avoid leftover data interfering with next trial
            time.sleep(0.01)
            s.reset_input_buffer()
        print('  trial {}/{} -> {}'.format(t+1, TRIALS, 'OK' if got_full else 'MISS'))
        
        
    rate = successes / TRIALS
    results.append((delay_ms, successes, TRIALS, rate))
    print('Delay {} ms: {} / {} success ({:.0%})'.format(delay_ms, successes, TRIALS, rate))
    if rate >= REQUIRED_SUCCESS_RATE:
        print('\nFound acceptable delay: {} ms (success rate {:.0%})'.format(delay_ms, rate))
        break

print('\nSummary:')
for r in results:
    print('  {} ms: {}/{} ({:.0%})'.format(r[0], r[1], r[2], r[3]))

s.close()
print('Done')
