import socket
import pyaudio
import wave
import numpy as np
import matplotlib.pyplot as plt
# import sounddevice as sd
# import struct
import paho.mqtt.client as mqtt
import whisper
import ollama
record = False
packets = []
model = whisper.load_model("tiny.en")
print("pulling model")
ollama.pull('tinyllama')
print("model pulled")
def on_message(client_obj, userdata, message:mqtt.MQTTMessage):
    global record,packets
    if message.payload[0] == 1 and not record:
        packets = []
        record = True
    elif message.payload[0] == 0 and record:
        print("evaluating speech")
        record = False
        wf = wave.open("temp_file.wav", 'wb')
        wf.setnchannels(1)
        wf.setsampwidth(p.get_sample_size(pyaudio.paInt16))
        wf.setframerate(fs)
        wf.writeframes(b''.join(packets))
        wf.close()
        result = model.transcribe("temp_file.wav")
        print(result["text"])
        response = ollama.chat(model='tinyllama', messages=[
        {
            'role': 'user',
            'content': result["text"],
        },
        ])
        print(response['message']['content'])
        client.publish(response_topic,response['message']['content'])
        
    print(f"Message received: {message.payload[0]}")

topic = "ugaelee2045sp24/kjohnsen/mic"
response_topic = "ugaelee2045sp24/kjohnsen/llamaresponse"
client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2,"kjohnsen2")
client.username_pw_set("class_user","class_password")
client.on_message = on_message
client.connect("mqtt.ugavel.com")

client.subscribe(topic)
client.loop_start()

fs = 48000              # Sampling frequency (48 kHz)
p = pyaudio.PyAudio()  # Create an interface to PortAudio

# Setup networking and pyaudio
UDP_IP = "0.0.0.0"  # the IP address to listen on (here, localhost)
UDP_PORT = 8080      # the port number to listen on

# create a UDP socket and bind it to the specified IP address and port
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))

while True:
    data, addr = sock.recvfrom(512)  # receive up to 512 bytes of data
    if record:
        packets.append(data)