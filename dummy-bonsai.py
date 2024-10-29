"""
You need to pip install python-osc but then you should be able to run this to produce a fake animal path
as though it were coming form bonsai. It simulates an animal running around in a 1000x1000 space.
"""
import time
import random
from pythonosc import udp_client, osc_message_builder
import math

ip = "127.0.0.1"
port = 27020
address = "/bonsai"
frequency = 50 # Hz

client = udp_client.SimpleUDPClient(ip, port)

print(f"Sending messages to {ip}:{port} at {frequency}hz. Started at {time.time()}...")

W  = H = 1000

x = 30
y = 30
theta = 0 # radians
v = 10
numpix1 = 40
numpix2 = 20

while True:
    msg = osc_message_builder.OscMessageBuilder(address=address)
    msg.add_arg(time.time(), 'd')

    msg.add_arg(x if numpix1 else float('nan'), 'f')
    msg.add_arg(y if numpix1 else float('nan'), 'f')

    msg.add_arg(x - 30* math.cos(theta) if numpix2 else float('nan'), 'f')
    msg.add_arg(y - 30*math.sin(theta) if numpix2 else float('nan'), 'f')

    msg.add_arg(numpix1, 'f')
    msg.add_arg(numpix2, 'f')

    client.send(msg.build())

    # basic simulation of path...

    theta += random.uniform(-0.1, 0.1)
    theta = theta % (2 * math.pi)
    v += random.uniform(-4,4)
    v = min(max(0, v), 15)
    x +=  v * theta * math.cos(theta)
    y += v * theta * math.sin(theta)

    # Handle bouncing off walls
    if x <= 0 or x >= W:
        theta = math.pi - theta
        x = max(0, min(W, x))

    if y <= 0 or y >= H:
        theta = -theta
        y = max(0, min(H, y))

    r = random.random()
    if r < 0.01:
        numpix1 = 0
        numpix2 = 0
    elif r < 0.02:
        numpix1 = 20
        numpix2 = 0
    elif r < 0.1:
        numpix1 = 40
        numpix2 = 20

    time.sleep(1.0 / frequency)

