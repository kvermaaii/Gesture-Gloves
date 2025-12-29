# starts the pins HIGH (so they stay OFF initially) and sends LOW to turn them ON.
import RPi.GPIO as GPIO
import requests
import time

# --- CONFIGURATION ---
FIREBASE_URL = "https://YOUR_PROJECT_ID.firebaseio.com/.json"

# Define GPIO Pins (BCM Numbering)
# Relay 1 -> GPIO 17
# Relay 2 -> GPIO 27
# Relay 4 -> GPIO 22
PIN_RELAY_1 = 17
PIN_RELAY_2 = 27
PIN_RELAY_4 = 22

# --- SETUP GPIO ---
GPIO.setmode(GPIO.BCM) 
GPIO.setwarnings(False)

GPIO.setup(PIN_RELAY_1, GPIO.OUT)
GPIO.setup(PIN_RELAY_2, GPIO.OUT)
GPIO.setup(PIN_RELAY_4, GPIO.OUT) 

# --- CRITICAL CHANGE 1: START HIGH (OFF) ---
# For Active Low relays, HIGH means OFF.
GPIO.output(PIN_RELAY_1, GPIO.HIGH)
GPIO.output(PIN_RELAY_2, GPIO.HIGH)
GPIO.output(PIN_RELAY_4, GPIO.HIGH)

def fetch_and_update():
    try:
        response = requests.get(FIREBASE_URL, timeout=2) 
        
        if response.status_code == 200:
            data = response.json()
            
            if data is None: 
                return 

            # Get states 
            state_1 = data.get('device1', False)
            state_2 = data.get('device2', False)
            state_3 = data.get('device3', False) 

            # --- CONTROL RELAYS (INVERTED LOGIC) ---
            # If True (ON) -> Send LOW
            # If False (OFF) -> Send HIGH
            
            # Relay 1
            GPIO.output(PIN_RELAY_1, GPIO.LOW if state_1 else GPIO.HIGH)

            # Relay 2
            GPIO.output(PIN_RELAY_2, GPIO.LOW if state_2 else GPIO.HIGH)

            # Relay 4 (Controlled by Device 3)
            GPIO.output(PIN_RELAY_4, GPIO.LOW if state_3 else GPIO.HIGH)

            print(f"\rStatus: D1=[{state_1}] D2=[{state_2}] R4=[{state_3}]", end="")
            
        else:
            print(f"\nError: Firebase returned {response.status_code}")

    except requests.exceptions.RequestException as e:
        print(f"\nNetwork Error: {e}")
    except KeyboardInterrupt:
        print("\nStopping...")
        GPIO.cleanup()
        exit()

def main():
    print("--- Gesture Control Started (Active Low Mode) ---")
    print(f"Polling {FIREBASE_URL}")
    print("Press Ctrl+C to stop.")

    while True:
        fetch_and_update()
        time.sleep(0.1) 

if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        GPIO.cleanup()