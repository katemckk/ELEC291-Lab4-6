import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import sys, time, math
import serial
import csv
import os

# Serial Port Configuration 
ser = serial.Serial(
    port='COM3',  # Change this to your actual COM port
    baudrate=115200,
    parity=serial.PARITY_NONE,
    stopbits=serial.STOPBITS_ONE, 
    bytesize=serial.EIGHTBITS
)

def getValues():
    try:
        line = ser.readline().decode('utf-8').strip()  # Read and decode line
        values = line.split(',')  # Expecting CSV format: Vrms1, Vrms2, Phase, Frequency
        
        if len(values) == 4:
            Vrms1 = float(values[0])
            Vrms2 = float(values[1])
            phase_deg = float(values[2])  # Phase in degrees
            freq = float(values[3])  # Frequency in Hz
            return Vrms1, Vrms2, phase_deg, freq
        else:
            print("Invalid data received:", line)
            return None
    except Exception as e:
        print("Error reading serial data:", e)
        return None

# Initialize plot
fig, ax = plt.subplots()
ax.set_xlabel("Time (ms)")
ax.set_ylabel("Voltage (V)")
ax.set_title("Real-time Sinusoidal Waveforms")
ax.set_xlim(0, 20)  # 20ms range for visualization
ax.set_ylim(-5, 5)  # Adjust according to expected voltage range
line1, = ax.plot([], [], label="Signal 1", color='b')
line2, = ax.plot([], [], label="Signal 2", color='r')
ax.legend()

# Time axis setup
t = np.linspace(0, 20, 1000)  # Simulated 20ms time window

# Update function for animation
def update(frame):
    data = getValues()
    if data:
        Vrms1, Vrms2, phase_deg, freq = data

        # Convert RMS to peak voltage (Vpeak = Vrms * sqrt(2))
        Vp1 = Vrms1 * np.sqrt(2)
        Vp2 = Vrms2 * np.sqrt(2)

        # Convert phase from degrees to radians
        phase_rad = np.radians(phase_deg)

        # Generate sine waves
        y1 = Vp1 * np.sin(2 * np.pi * freq * t / 1000)
        y2 = Vp2 * np.sin(2 * np.pi * freq * t / 1000 + phase_rad)

        # Update plot data
        line1.set_data(t, y1)
        line2.set_data(t, y2)

        ax.set_ylim(min(y1.min(), y2.min()) - 0.5, max(y1.max(), y2.max()) + 0.5)  # Adjust y-limits dynamically

    return line1, line2

# Animation
ani = animation.FuncAnimation(fig, update, interval=500, blit=False)

plt.show()
  
