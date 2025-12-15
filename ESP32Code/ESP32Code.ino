#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>

// --- USER CONFIGURATION ---
#define WIFI_SSID "YOUR_WIFI_NAME"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

// Firebase Config
#define API_KEY "YOUR_FIREBASE_API_KEY" 
#define DATABASE_URL "YOUR_PROJECT_ID.firebaseio.com" 

// --- Pin Definitions ---
const int PIN_SENSOR_S2 = 32; 
const int PIN_SENSOR_S1 = 33; 
const int PIN_SENSOR_S3 = 34; 

const int PIN_RELAY_1 = 25;
const int PIN_RELAY_2 = 26;
const int PIN_RELAY_3 = 27;

// --- Thresholds (Adjust these based on Serial Monitor values) ---
const int THRESHOLD_S2 = 2635;
const int THRESHOLD_S1 = 2590;
const int THRESHOLD_S3 = 3800;

// --- Firebase Objects ---
FirebaseData fbDO;
FirebaseAuth auth;
FirebaseConfig config;
bool wifiConnected = false;

// --- State Variables ---
bool relayState1 = false;
bool relayState2 = false;
bool relayState3 = false;

bool lastStateS1Bent = false;
bool lastStateS2Bent = false;
bool lastStateS3Bent = false;

unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 200;

void setup() {
  Serial.begin(115200);
  
  // 1. Initialize Pins
  pinMode(PIN_RELAY_1, OUTPUT);
  pinMode(PIN_RELAY_2, OUTPUT);
  pinMode(PIN_RELAY_3, OUTPUT);
  digitalWrite(PIN_RELAY_1, LOW);
  digitalWrite(PIN_RELAY_2, LOW);
  digitalWrite(PIN_RELAY_3, LOW);
  analogReadResolution(12);

  // 2. Connect to WiFi
  Serial.print("Connecting to WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  wifiConnected = true;

  // 3. Connect to Firebase (FIXED SETTINGS)
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  
  // --- CRITICAL FIXES FOR SSL ERRORS ---
  config.timeout.wifiReconnect = 10 * 1000;    // Wait 10s for WiFi
  config.timeout.socketConnection = 10 * 1000; // Wait 10s for Server
  config.timeout.sslHandshake = 10 * 1000;     // Wait 10s for Security
  
  config.signer.test_mode = true; 
  
  // Reduce buffer size to prevent memory crashes on hotspots
  fbDO.setBSSLBufferSize(1024, 1024); 
  // ------------------------------------
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  Serial.println("Firebase initialized successfully.");

  Serial.println("Resetting all devices to OFF...");
  updateFirebase("/device1", false);
  updateFirebase("/device2", false);
  updateFirebase("/device3", false);

}

// Helper to update Firebase
void updateFirebase(String path, bool state) {
  if (Firebase.ready()) {
    if (Firebase.RTDB.setBool(&fbDO, path, state)) {
      Serial.print("SUCCESS: Firebase Updated ");
      Serial.print(path);
      Serial.print(" -> ");
      Serial.println(state);
    } else {
      Serial.print("ERROR: ");
      Serial.println(fbDO.errorReason());
    }
  }
}

void loop() {
  // Read Sensors
  int readingS1 = analogRead(PIN_SENSOR_S1);
  int readingS2 = analogRead(PIN_SENSOR_S2);
  int readingS3 = analogRead(PIN_SENSOR_S3);

  // --- PRINT VALUES FOR DEBUGGING ---
  // Use these numbers to fix your Thresholds!
static unsigned long lastPrintTime = 0;
  if (millis() - lastPrintTime > 1000) {
    Serial.print("S1: "); Serial.print(readingS1);
    Serial.print(" | S2: "); Serial.print(readingS2);
    Serial.print(" | S3: "); Serial.println(readingS3);
    lastPrintTime = millis();
  }
  // ---------------------------------

  // Determine Bends
  bool isS1Bent = (readingS1 > THRESHOLD_S1);
  bool isS2Bent = (readingS2 > THRESHOLD_S2);
  bool isS3Bent = (readingS3 < THRESHOLD_S3);

  // Debounce & Logic
  if (millis() - lastDebounceTime > debounceDelay) {
    
    // --- Device 1 (S1) ---
    if (isS1Bent && !lastStateS1Bent) {
      relayState1 = !relayState1;
      digitalWrite(PIN_RELAY_1, relayState1 ? HIGH : LOW);
      updateFirebase("/device1", relayState1); 
      lastDebounceTime = millis();
    }
    
    // --- Device 2 (S2) ---
    if (isS2Bent && !lastStateS2Bent) {
      relayState2 = !relayState2;
      digitalWrite(PIN_RELAY_2, relayState2 ? HIGH : LOW);
      updateFirebase("/device2", relayState2); 
      lastDebounceTime = millis();
    }

    // --- Device 3 (S3) ---
    if (isS3Bent && !lastStateS3Bent) {
      relayState3 = !relayState3;
      digitalWrite(PIN_RELAY_3, relayState3 ? HIGH : LOW);
      updateFirebase("/device3", relayState3); 
      lastDebounceTime = millis();
    }
  }

  // Update history
  lastStateS1Bent = isS1Bent;
  lastStateS2Bent = isS2Bent;
  lastStateS3Bent = isS3Bent;

  // Slower delay to prevent network congestion
  delay(200);
}