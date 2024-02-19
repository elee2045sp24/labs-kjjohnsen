import paho.mqtt.client as mqtt
import tkinter as tk
import uuid
import time
import pyaudio
import numpy as np

battery_topic = "elee2045sp24/doorbell/kjohnsen/battery"
pressed_topic = "elee2045sp24/doorbell/kjohnsen/pressed"
mode_topic = "elee2045sp24/doorbell/kjohnsen/mode"
message_topic = "elee2045sp24/doorbell/kjohnsen/message"
rings_file = "rings.txt"

# helper function to play a short tone (found from various sources on the web and reconceptualized)
pya = pyaudio.PyAudio()
sample_rate = 44100
freq = 2000 # hz
long_beep = np.sin(2*np.pi*freq*np.linspace(0,1,44100)).astype(np.float32)
stream = pya.open(sample_rate,channels=1,format=pyaudio.paFloat32,output=True)
def beep(isLong=False):
    if isLong:
        stream.write(long_beep)
    else:
        stream.write(long_beep[0:5000])


class DoorbellApp:
    
    def __init__(self):
        self.mode = "Normal"
        self.battery = 0
        self.temp_message_active = False
        self.temp_message_sent_time = time.time()
        self.last_battery_time = time.time()
        self.ringing = False
        self._tk_setup()
        self._mqtt_setup()
        

    def _mqtt_setup(self):
        self.client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2,str(uuid.uuid4()))
        self.client.username_pw_set("class_user","class_password")
        self.client.on_message = self.on_message
        self.client.connect("mqtt.ugavel.com")
        self.client.loop_start()
        self.client.subscribe(battery_topic)
        self.client.subscribe(pressed_topic)
    
    def _tk_setup(self):
        self.root = tk.Tk()
        self.root.title("Doorbell App")
        
        self.battery_label = tk.Label(self.root,text="Status: ")
        self.battery_val_label = tk.Label(self.root,)
        
        self.mode_label = tk.Label(self.root, text="Mode: ")
        self.mode_val_label = tk.Label(self.root, text=self.mode)
        self.mode_button = tk.Button(self.root, text="Toggle",command=self.mode_button_callback)

        self.normal_message_label = tk.Label(self.root, text="Normal Message: ")
        self.normal_message_var = tk.StringVar(self.root, "Welcome!")
        self.normal_message_val_entry = tk.Entry(self.root,textvariable=self.normal_message_var,)
        
        self.dnd_message_label = tk.Label(self.root, text="DND Message: ")
        self.dnd_message_var = tk.StringVar(self.root, "Please do not ring.")
        self.dnd_message_val_entry = tk.Entry(self.root,textvariable=self.dnd_message_var)
        
        self.temp_message_label = tk.Label(self.root, text="Temp Message: ")
        self.temp_message_var = tk.StringVar(self.root, "Hello!")
        self.temp_message_val_entry = tk.Entry(self.root,textvariable=self.temp_message_var)
        self.update_message_button = tk.Button(self.root, text="Update",command=self.message_button_callback)

        self.ring_indicator = tk.Frame(self.root, background="black", width=50, height=50)
        self.ring_indicator_time = tk.Label(self.root,text="")

        self.battery_label.grid(row=0,column=0)
        self.battery_val_label.grid(row=0,column=1,columnspan=2,sticky=tk.W)

        self.mode_label.grid(row=1,column=0)
        self.mode_val_label.grid(row=1,column=1,sticky=tk.W)
        self.mode_button.grid(row=1,column=2,padx=5,pady=5)

        self.normal_message_label.grid(row=2,column=0)
        self.normal_message_val_entry.grid(row=2,column=1)
        
        self.dnd_message_label.grid(row=3,column=0)
        self.dnd_message_val_entry.grid(row=3,column=1)
        
        self.temp_message_label.grid(row=4,column=0)
        self.temp_message_val_entry.grid(row=4,column=1)
        self.update_message_button.grid(row=4,column=2,padx=5,pady=5)
        self.ring_indicator.grid(row=5,column=0,columnspan=3,pady=5)
        self.ring_indicator_time.grid(row=6,column=0,columnspan=3,pady=5)

    # MQTT
    def on_message(self, client_obj: mqtt.Client, userdata, message: mqtt.MQTTMessage):
        if message.topic == battery_topic:
            self.battery = int(message.payload.decode('utf8'))
            self.battery_val_label.config(text=f"{self.battery} %")
            self.last_battery_time = time.time()
        if message.topic == pressed_topic:
            self.pressed()
    
    # doorbell app function
    def pressed(self):
        beep(isLong = True)
        self.ringing = True
        self.last_ring_time = time.time()
        self.ring_indicator.config(background="red")
        self.ring_indicator_time.config(text=time.ctime())
        with open(rings_file,"a") as f:
            f.write(f"{time.ctime()}\n")


    # helper function to update the message when the mode changes
    def update_mode(self, new_mode):
        self.mode = new_mode
        self.client.publish(mode_topic,self.mode,retain=True)
        if self.mode == "Normal":
            self.client.publish(message_topic,self.normal_message_var.get(),retain=True)
        else:
            self.client.publish(message_topic,self.dnd_message_var.get(),retain=True)
        self.mode_val_label.config(text=self.mode)

    # tk callbacks
    def mode_button_callback(self):
        if self.mode == "Normal":
            self.update_mode("DND")
        else:
            self.update_mode("Normal")

    def message_button_callback(self):
        self.client.publish(message_topic, self.temp_message_var.get())
        self.temp_message_active = True
        self.temp_message_sent_time = time.time()

    def check_alive_callback(self):
        if time.time() - self.last_battery_time > 5:
            self.battery_val_label.config(text="Disconnected")
        if self.temp_message_active and time.time() - self.temp_message_sent_time > 5:
            self.update_mode(self.mode) #forces a send
            self.temp_message_active = False
        if self.ringing and time.time() - self.last_ring_time > 5:
            self.ringing = False
            self.ring_indicator.config(background="black")
            self.ring_indicator_time.config(text="")
        self.root.after(1000, self.check_alive_callback)
        
    def run(self):
        self.update_mode("Normal")
        self.check_alive_callback()
        self.root.mainloop()

lab3app = DoorbellApp()
lab3app.run()
