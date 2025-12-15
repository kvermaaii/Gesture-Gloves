DIY Wireless Gesture-Controlled Automation 🖐️💡

This is my IOTA course project: a wireless glove that controls home appliances using hand gestures.

The goal was to build a system where the "remote" (my glove) talks to the "switch" (a Relay module) entirely over the internet using Firebase, rather than direct wiring.

🚀 My Innovations

Switching to ESP32: I originally planned to use an Arduino, but I switched to the ESP32 Dev Module. I needed its built-in Wi-Fi to make the glove completely standalone and battery-powered, sending data directly to the cloud.

DIY Flex Sensors: Instead of buying expensive commercial sensors, I fabricated my own flex sensors. This allowed me to customize them for my hand, though it required writing specific code to handle their unique resistance ranges.

Headless Hub: I set up the Raspberry Pi to run without a monitor (Headless Mode), controlling it entirely from my laptop using VNC Viewer.

🛠️ Hardware Used

Sender: ESP32 Dev Module (Wi-Fi).

Receiver: Raspberry Pi 4 Model B.

Sensors: 3x DIY Flex Sensors.

Actuator: 4-Channel 5V Relay Module.

Connectivity: Mobile Hotspot (2.4GHz).

⚙️ How It Works (The Logic)

The system works in three stages:

Detection (ESP32): The glove reads the analog resistance of the sensors. I implemented State Change Detection so the lights toggle only when I first bend my finger, rather than flickering on and off while holding the bend.

Transmission (Firebase): The ESP32 sends a simple True or False status to the Google Firebase Realtime Database.

Actuation (Raspberry Pi): The Pi runs a Python script that polls the database every 0.1 seconds. If it sees a change, it triggers the GPIO pins.

📉 Challenges I Solved

1. Calibrating DIY Sensors

Since I made the sensors myself, every finger had a different range. I had to manually map the thresholds using the Serial Monitor:

Sensor 1: Standard range (Rest: 2544, Bent: 2635).

Sensor 2: This was tricky. The range was tight (Rest: 2650, Bent: 2699). I had to set a precise threshold of 2675.

Sensor 3: This worked in reverse (Value dropped when bent). I had to flip the code logic to use < instead of >.

2. The "Active Low" Relay Trap

My relay module turned out to be "Active Low," meaning it turns ON when receiving a LOW signal (0V) and OFF with a HIGH signal (3.3V).

Initial Issue: When I ran my script, all lights turned on immediately.

The Fix: I inverted the logic in my Python script (GPIO.LOW if True) and initialized all pins to GPIO.HIGH at startup to keep them off.

3. Mobile Hotspot Instability

I used my phone's hotspot for the network. The ESP32 kept crashing with SSL Engine Closed errors because the connection would throttle.

The Fix: I added specific timeout settings to the ESP32 code and reduced the SSL buffer size to prevent memory crashes:

config.timeout.wifiReconnect = 10000;
fbDO.setBSSLBufferSize(1024, 1024);


🔌 Pin Configuration

ESP32 Connections:

S1: GPIO 33

S2: GPIO 32

S3: GPIO 34

Raspberry Pi Connections:

Relay 1: GPIO 17 (Pin 11)

Relay 2: GPIO 27 (Pin 13)

Relay 4: GPIO 22 (Pin 15)

💻 Setup Instructions

Raspberry Pi:

Install requests: sudo apt install python3-requests (Fixed "Externally Managed Environment" error).

Run script: python3 gesture_pi_active_low.py

ESP32:

Update WIFI_SSID, PASSWORD, and API_KEY in the sketch.

Upload using Arduino IDE.

Created as part of my IOTA Coursework.
