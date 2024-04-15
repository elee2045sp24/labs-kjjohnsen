import socket
import pyaudio
import numpy as np
import matplotlib.pyplot as plt
import sounddevice as sd
import struct

# Constants
fs = 48000              # Sampling frequency (48 kHz)
block_size = 4096       # Number of samples per block (bigger is more latency, but better results and less time visualizing)

# Prepare FFT
frequencies = np.fft.fftfreq(block_size, 1/fs)[:block_size//2] #fftfreq gives us labeled frequencies

# Initialize plot
plt.ion() # interactive mode
fig, ax = plt.subplots()
line, = ax.plot([], [], lw=2) # pre-create the line
ax.set_xlim(0, 10000)  # Frequency range up to half the sampling rate (Nyquist frequency)
ax.set_ylim(0, 10000000)     # Adjust the y-axis as needed

# Real-time visualization loop
def callback(indata):
    # Compute FFT
    y = np.abs(np.fft.fft(indata, n=block_size))[:block_size//2] # compute the actual fft
    
    # Update plot efficiently
    line.set_xdata(frequencies)
    line.set_ydata(y)
    fig.canvas.draw()
    fig.canvas.flush_events()

# Setup networking and pyaudio
UDP_IP = "0.0.0.0"  # the IP address to listen on (here, localhost)
UDP_PORT = 8080      # the port number to listen on

# create a UDP socket and bind it to the specified IP address and port
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))
p = pyaudio.PyAudio()
stream = p.open(format=p.get_format_from_width(2),
                channels=1,
                rate=fs,
                output=True)


buffer = [] # used to buffer audio until we have a suitable amount of samples for fft
# read incoming packets and print their contents
while True:
    data, addr = sock.recvfrom(512)  # receive up to 512 bytes of data
    int_data = struct.iter_unpack('<h',data) # data is little endian signed 16 bit
    float_data = [x[0] for x in int_data] # convert to between 0 and 1 for fft
    buffer.extend(float_data)
    if len(buffer) > block_size:
        callback(buffer[:block_size])
        buffer = buffer[block_size:]
    #stream.write(data)