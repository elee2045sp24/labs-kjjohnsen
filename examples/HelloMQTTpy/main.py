import paho.mqtt.client as mqtt
import time 

def on_message(client_obj, userdata, message:mqtt.MQTTMessage):
    print(f"Message received: {message.payload.decode('utf8')}")
    

topic = "ugaelee2045sp24/kjohnsen/topic"
client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2,"kjohnsen2")
client.username_pw_set("class_user","class_password")
client.on_message = on_message
client.connect("mqtt.ugavel.com")

client.subscribe(topic)
client.loop_start()

i = 0
while True:
    time.sleep(1) 
    client.publish(topic,"counter: " + str(i))
    i+=1