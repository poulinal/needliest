# python3 - <<'PY'
import serial, time
PORT = '/dev/cu.usbserial-DN06A5F2'
# BAUD = 115200
BAUD = 9600
s = serial.Serial(PORT, BAUD, timeout=1)
time.sleep(6.0)

# Clear any stale data
s.reset_input_buffer()
s.reset_output_buffer()

# Send byte by byte to ensure no buffering issues
msg = b'1.23,5.67\n'
for byte in msg:
    s.write(bytes([byte]))
    s.flush()
    # time.sleep(0.001)  # 1ms between bytes
    time.sleep(0.214)  # 10ms between bytes
    # time.sleep(20/1000.0)  # 50ms between bytes
    
# for b in MSG:
#     s.write(bytes([b]))
#     s.flush()
#     time.sleep(delay_ms / 1000.0)
# for _ in range(20):
#     s.write(b'1,2\n')
#     s.flush()
#     time.sleep(0.1)
#send entire line at once
# for _ in range(20):
#     s.write(b'1.23,5.67\n')
#     s.flush()
#     time.sleep(0.01)
# s.write(b'1.23,5.67\n')

time.sleep(0.02)
# time.sleep(5.0)
print("Reading replies:")
for _ in range(20):
    line = s.readline()
    if not line: break
    print(line.decode('ascii', errors='replace').strip())
# deadline = time.time() + 0.5
# buf = b''
# while time.time() < deadline:
#     chunk = s.read(1)
#     if chunk:
#         buf += chunk
#         # Try to decode and check substrings
#         try:
#             text = buf.decode('ascii', errors='replace')
#             print('Received chunk:', text)
#         except Exception:
#             text = ''
#     else:
#         # no data right now
#         time.sleep(0.005)
s.close()
# PY