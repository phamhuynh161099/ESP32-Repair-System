#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// WiFi Configuration
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// Server Configuration
const char* serverUrl = "http://YOUR_SERVER_IP:8080/api/maintenance";

// ESP32 Device ID (unique cho từng đồng hồ)
const char* deviceId = "ESP32_001";

// Engineer Name
const char* engineerName = "Kỹ sư A";

// Pin Configuration
#define BUZZER_PIN 25
#define LED_PIN 2
#define BUTTON_PIN 4

// State Management
enum DeviceState {
  STATE_IDLE,
  STATE_REQUEST_RECEIVED,
  STATE_ACKNOWLEDGED,
  STATE_ARRIVED
};

DeviceState currentState = STATE_IDLE;
unsigned long lastCheckTime = 0;
const unsigned long checkInterval = 3000; // Check mỗi 3 giây

// Request Data
String currentRequestId = "";
String machineCode = "";
String machineName = "";
String location = "";
String issueDescription = "";

void setup() {
  Serial.begin(115200);
  
  // Initialize pins
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  // Connect to WiFi
  connectWiFi();
  
  Serial.println("ESP32 Maintenance Watch Ready!");
  Serial.println("Device ID: " + String(deviceId));
  Serial.println("Engineer: " + String(engineerName));
}

void loop() {
  // Check for pending requests periodically
  if (millis() - lastCheckTime >= checkInterval) {
    lastCheckTime = millis();
    checkPendingRequests();
  }
  
  // Handle button press based on current state
  if (digitalRead(BUTTON_PIN) == LOW) {
    delay(50); // Debounce
    if (digitalRead(BUTTON_PIN) == LOW) {
      handleButtonPress();
      while (digitalRead(BUTTON_PIN) == LOW); // Wait for release
    }
  }
  
  // Update LED based on state
  updateLED();
  
  delay(100);
}

void connectWiFi() {
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi Connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nWiFi Connection Failed!");
  }
}

void checkPendingRequests() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected!");
    return;
  }
  
  if (currentState != STATE_IDLE) {
    return; // Already processing a request
  }
  
  HTTPClient http;
  String url = String(serverUrl) + "/check/" + String(deviceId);
  
  http.begin(url);
  int httpCode = http.GET();
  
  if (httpCode == 200) {
    String payload = http.getString();
    
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, payload);
    
    if (!error) {
      bool hasRequest = doc["hasRequest"];
      
      if (hasRequest) {
        // New request received!
        currentRequestId = doc["requestId"].as<String>();
        machineCode = doc["machineCode"].as<String>();
        machineName = doc["machineName"].as<String>();
        location = doc["location"].as<String>();
        issueDescription = doc["issueDescription"].as<String>();
        
        currentState = STATE_REQUEST_RECEIVED;
        
        Serial.println("\n========== NEW REQUEST ==========");
        Serial.println("Machine: " + machineCode + " - " + machineName);
        Serial.println("Location: " + location);
        Serial.println("Issue: " + issueDescription);
        Serial.println("=================================");
        
        // Ring buzzer to alert engineer
        ringBuzzer();
      }
    }
  }
  
  http.end();
}

void handleButtonPress() {
  Serial.println("\nButton Pressed!");
  
  switch (currentState) {
    case STATE_IDLE:
      Serial.println("No active request");
      break;
      
    case STATE_REQUEST_RECEIVED:
      // Engineer acknowledges the request (timestamp2)
      acknowledgeRequest();
      break;
      
    case STATE_ACKNOWLEDGED:
      // Engineer arrived at location (timestamp3)
      arriveAtLocation();
      break;
      
    case STATE_ARRIVED:
      // Engineer completed the fix (timestamp4)
      completeRequest();
      break;
  }
}

void acknowledgeRequest() {
  Serial.println("Acknowledging request...");
  
  HTTPClient http;
  String url = String(serverUrl) + "/acknowledge";
  
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  
  DynamicJsonDocument doc(256);
  doc["esp32DeviceId"] = deviceId;
  doc["engineerName"] = engineerName;
  
  String jsonString;
  serializeJson(doc, jsonString);
  
  int httpCode = http.POST(jsonString);
  
  if (httpCode == 200) {
    String response = http.getString();
    Serial.println("Response: " + response);
    
    currentState = STATE_ACKNOWLEDGED;
    Serial.println("✅ Request acknowledged! Going to: " + location);
    
    // Beep confirmation
    tone(BUZZER_PIN, 1000, 200);
    delay(300);
    tone(BUZZER_PIN, 1500, 200);
  } else {
    Serial.println("Failed to acknowledge: " + String(httpCode));
  }
  
  http.end();
}

void arriveAtLocation() {
  Serial.println("Marking arrival at location...");
  
  HTTPClient http;
  String url = String(serverUrl) + "/arrive";
  
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  
  DynamicJsonDocument doc(256);
  doc["esp32DeviceId"] = deviceId;
  
  String jsonString;
  serializeJson(doc, jsonString);
  
  int httpCode = http.POST(jsonString);
  
  if (httpCode == 200) {
    String response = http.getString();
    Serial.println("Response: " + response);
    
    currentState = STATE_ARRIVED;
    Serial.println("✅ Arrived at location! Starting repair...");
    
    // Beep confirmation
    tone(BUZZER_PIN, 2000, 300);
  } else {
    Serial.println("Failed to mark arrival: " + String(httpCode));
  }
  
  http.end();
}

void completeRequest() {
  Serial.println("Completing request...");
  
  HTTPClient http;
  String url = String(serverUrl) + "/complete";
  
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  
  DynamicJsonDocument doc(256);
  doc["esp32DeviceId"] = deviceId;
  
  String jsonString;
  serializeJson(doc, jsonString);
  
  int httpCode = http.POST(jsonString);
  
  if (httpCode == 200) {
    String response = http.getString();
    Serial.println("Response: " + response);
    
    currentState = STATE_IDLE;
    Serial.println("✅ Request completed successfully!");
    
    // Victory beep
    tone(BUZZER_PIN, 1000, 100);
    delay(150);
    tone(BUZZER_PIN, 1500, 100);
    delay(150);
    tone(BUZZER_PIN, 2000, 100);
    
    // Reset data
    currentRequestId = "";
    machineCode = "";
    machineName = "";
    location = "";
    issueDescription = "";
  } else {
    Serial.println("Failed to complete: " + String(httpCode));
  }
  
  http.end();
}

void ringBuzzer() {
  // Ring pattern for new request
  for (int i = 0; i < 3; i++) {
    tone(BUZZER_PIN, 2000, 500);
    delay(600);
  }
}

void updateLED() {
  switch (currentState) {
    case STATE_IDLE:
      digitalWrite(LED_PIN, LOW);
      break;
      
    case STATE_REQUEST_RECEIVED:
      // Fast blink - new request
      digitalWrite(LED_PIN, (millis() / 200) % 2);
      break;
      
    case STATE_ACKNOWLEDGED:
      // Medium blink - going to location
      digitalWrite(LED_PIN, (millis() / 500) % 2);
      break;
      
    case STATE_ARRIVED:
      // Solid on - working on fix
      digitalWrite(LED_PIN, HIGH);
      break;
  }
}