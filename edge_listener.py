import paho.mqtt.client as mqtt
import json
import time
from collections import deque
import matplotlib.pyplot as plt
import matplotlib.animation as animation

BROKER = "broker.hivemq.com"
PORT = 1883
TOPIC = "/esp8266/sensor"

WINDOW_DURATION = 60 

# Deques to hold data
timestamps = deque()
Ipeak_vals = deque()
Irms_vals = deque()
Freq_vals = deque()

start_time = time.time()


def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Connected to MQTT broker")
        client.subscribe(TOPIC)
    else:
        print("Failed to connect, rc:", rc)

def on_message(client, userdata, msg):
    try:
        data = json.loads(msg.payload.decode())
        t = time.time() - start_time

        
        timestamps.append(t)
        Ipeak_vals.append(data.get("Ipeak", 0))
        Irms_vals.append(data.get("Irms", 0))
        Freq_vals.append(data.get("Freq", 0))

       
        print(f"[{t:.2f}s] Received: Ipeak={data.get('Ipeak', 0)}, Irms={data.get('Irms', 0)}, Freq={data.get('Freq', 0)}")

        while timestamps and (t - timestamps[0]) > WINDOW_DURATION:
            timestamps.popleft()
            Ipeak_vals.popleft()
            Irms_vals.popleft()
            Freq_vals.popleft()

    except Exception as e:
        print("Error parsing message:", e)

client = mqtt.Client(client_id="PythonSubscriber", clean_session=True, protocol=mqtt.MQTTv311)
client.on_connect = on_connect
client.on_message = on_message
client.connect(BROKER, PORT)
client.loop_start()

fig, (ax1, ax2, ax3) = plt.subplots(3, 1, figsize=(12, 6))

def update(frame):
    ax1.clear()
    ax2.clear()
    ax3.clear()

    ax1.plot(timestamps, Ipeak_vals, color='r')
    ax2.plot(timestamps, Irms_vals, color='g')
    ax3.plot(timestamps, Freq_vals, color='b')

    ax1.set_ylabel("Ipeak (A)")
    ax2.set_ylabel("Irms (A)")
    ax3.set_ylabel("Freq (Hz)")
    ax3.set_xlabel("Time (s)")

    ax1.grid(True)
    ax2.grid(True)
    ax3.grid(True)

 
    if timestamps:
        ax1.set_xlim(left=max(0, timestamps[-1]-WINDOW_DURATION), right=timestamps[-1])
        ax2.set_xlim(left=max(0, timestamps[-1]-WINDOW_DURATION), right=timestamps[-1])
        ax3.set_xlim(left=max(0, timestamps[-1]-WINDOW_DURATION), right=timestamps[-1])

ani = animation.FuncAnimation(fig, update, interval=200)  
plt.tight_layout()
plt.show()
