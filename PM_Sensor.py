import serial
import struct
import collections
import matplotlib.pyplot as plt
import matplotlib.animation as animation

# ── CONFIG ─────────────────────────────
PORT   = 'COM14'      # change to your STM32 COM port
BAUD   = 921600
WINDOW = 512         # samples shown at once (matches 2x ADC_BUF_SIZE)
# ───────────────────────────────────────

ser = serial.Serial(PORT, BAUD)
buf  = collections.deque([0] * WINDOW, maxlen=WINDOW)
raw  = b''

fig, ax = plt.subplots(figsize=(14, 5))
fig.patch.set_facecolor('#0d1117')
ax.set_facecolor('#0d1117')
ax.tick_params(colors='#58a6ff')
ax.spines[:].set_color('#30363d')
ax.set_ylim(0, 4095)
ax.set_xlim(0, WINDOW)
ax.set_title('Raw adc_buffer live stream', color='#e6edf3', fontsize=13, pad=10)
ax.set_ylabel('ADC Count (12-bit)', color='#8b949e')
ax.set_xlabel('Samples', color='#8b949e')
line,      = ax.plot([], [], color='#39d353', linewidth=0.8)
base_line, = ax.plot([], [], color='#f0a500', linewidth=0.8, linestyle='--', label='baseline est.')
ax.legend(facecolor='#161b22', labelcolor='#e6edf3', loc='upper right')

running_baseline = 0.0

def read_samples():
    global raw, running_baseline

    if ser.in_waiting > 4096:
        ser.reset_input_buffer()
        raw = b''

    raw += ser.read(ser.in_waiting or 1)
    samples = []
    while len(raw) >= 2:
        val = struct.unpack('>H', raw[:2])[0]
        raw = raw[2:]
        val = val & 0x0FFF
        samples.append(val)
        running_baseline += 0.001 * (val - running_baseline)
    return samples

def update(frame):
    samples = read_samples()
    if samples:
        buf.extend(samples)
    y = list(buf)
    line.set_data(range(WINDOW), y)
    base_line.set_data(range(WINDOW), [running_baseline] * WINDOW)
    return line, base_line,

ani = animation.FuncAnimation(fig, update, interval=30, blit=False)
plt.tight_layout()
plt.show()