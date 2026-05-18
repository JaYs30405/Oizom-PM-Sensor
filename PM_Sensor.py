import serial
import struct
import collections
import threading
import numpy as np
import pyqtgraph as pg
from pyqtgraph.Qt import QtWidgets, QtCore

# ── CONFIG ─────────────────────────────
PORT   = 'COM14'
BAUD   = 921600
WINDOW = 1024
THRESHOLD = 150    # above baseline to detect pulse
# ───────────────────────────────────────

ser = serial.Serial(PORT, BAUD)

plot_buf    = collections.deque([0] * WINDOW, maxlen=WINDOW)
serial_lock = threading.Lock()
running_baseline_shared = [0.0]
stop_flag   = threading.Event()

# ── Serial Thread ───────────────────────
def serial_thread():
    raw = b''
    running_baseline = 0.0

    while not stop_flag.is_set():
        raw += ser.read(ser.in_waiting or 1)

        while len(raw) >= 3:
            if raw[0] != 0xFF:
                raw = raw[1:]
                continue

            msb = raw[1]
            lsb = raw[2]

            if msb > 0x0F:
                raw = raw[1:]
                continue

            val = ((msb & 0x0F) << 8) | lsb
            raw = raw[3:]
            running_baseline += 0.001 * (val - running_baseline)

            with serial_lock:
                plot_buf.append(val)
                running_baseline_shared[0] = running_baseline

t = threading.Thread(target=serial_thread, daemon=True)
t.start()

# ── PyQtGraph Setup ─────────────────────
app = QtWidgets.QApplication([])

win = pg.GraphicsLayoutWidget(title='PM3006S Raw ADC')
win.setBackground('#0d1117')
win.resize(1400, 600)
win.show()

plot = win.addPlot(title='PM3006S Raw ADC — adc_buffer live stream')
plot.setYRange(0, 4095)
plot.setXRange(0, WINDOW)
plot.setLabel('left',   'ADC Count (12-bit)', color='#8b949e')
plot.setLabel('bottom', 'Samples',            color='#8b949e')
plot.getAxis('left').setPen('#30363d')
plot.getAxis('bottom').setPen('#30363d')

# Raw signal curve
curve      = plot.plot(pen=pg.mkPen('#39d353', width=1))

# Baseline curve
base_curve = plot.plot(pen=pg.mkPen('#f0a500', width=1, style=QtCore.Qt.DashLine))

# Pulse marker — red dots on detected peaks
peak_scatter = pg.ScatterPlotItem(size=10, brush=pg.mkBrush('#ff4444'))
plot.addItem(peak_scatter)

# Text box
text_item = pg.TextItem(text='', color='#ffffff', fill=pg.mkBrush('#161b22'))
text_item.setPos(10, 3900)
plot.addItem(text_item)

# ── Update Function ─────────────────────
def update():
    with serial_lock:
        y = np.array(plot_buf, dtype=np.float32)
        baseline_val = running_baseline_shared[0]

    x = np.arange(len(y))

    # Update curves
    curve.setData(x, y)
    base_curve.setData(x, np.full(len(y), baseline_val))

    # Pulse detection — find points above threshold
    above = y - baseline_val
    peak_indices = np.where(above > THRESHOLD)[0]

    if len(peak_indices) > 0:
        peak_scatter.setData(x=peak_indices, y=y[peak_indices])
        peak_val = float(above[peak_indices].max())
        status = f'*** PULSE DETECTED — Peak: {peak_val:.0f} above baseline ***'
    else:
        peak_scatter.setData([], [])
        status = ''

    current_val = int(y[-1]) if len(y) > 0 else 0
    text_item.setText(
        f'ADC: {current_val}  |  Baseline: {baseline_val:.1f}  |  '
        f'Above Baseline: {current_val - baseline_val:.1f}  |  {status}'
    )

# Timer drives update — 10ms = ~100fps possible
timer = QtCore.QTimer()
timer.timeout.connect(update)
timer.start(10)

app.exec_()
stop_flag.set()