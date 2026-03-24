#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266httpUpdate.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <SSD1306Wire.h> // Sử dụng thư viện ThingPulse chuẩn

// --- Cấu hình OLED ---
// Khai báo đối tượng SSD1306Wire (I2C Address 0x3C, SDA = GPIO5/D1, SCL = GPIO4/D2)
SSD1306Wire display(0x3c, 5, 4);   

// --- Cấu hình WiFi ---
const char* ssid = "SA";
const char* password = "1234567890";
WiFiClient client1; 
HTTPClient http;

// --- Cấu hình Server ---
const char* serverUrl = "http://103.140.249.253:8080/api/maintenance";
const char* deviceId = "ESP32_001";
const char* engineerName = "Ky su A";

// --- Cấu hình Pin (Dành cho ESP8266) ---
#define BUZZER_PIN 14 // Đổi sang GPIO 14 (Chân D5) để còi kêu được trên ESP8266
#define LED_PIN 2     // Đèn LED tích hợp trên mạch (GPIO 2 / D4)
#define BUTTON_PIN 0  // Sử dụng nút FLASH tích hợp sẵn trên mạch (GPIO 0 / D3)

// --- Quản lý trạng thái ---
enum DeviceState {
  STATE_IDLE,
  STATE_REQUEST_RECEIVED,
  STATE_ACKNOWLEDGED,
  STATE_ARRIVED
};

// --- CẤU HÌNH LAYOUT MỚI CHO OLED 128x64 ---
#define LABEL_X 0       // Tọa độ X của các nhãn (Ch:, PK:, DA:)
#define VALUE_X 25      // Tọa độ X của nội dung (dịch sang trái để có nhiều không gian)
#define LINE_1_Y 22     // Tọa độ Y Dòng 1
#define LINE_2_Y 36     // Tọa độ Y Dòng 2
#define LINE_3_Y 50     // Tọa độ Y Dòng 3 (Không bị lẹm đáy màn hình)

#define flipDisplay true

DeviceState currentState = STATE_IDLE;
unsigned long lastCheckTime = 0;
const unsigned long checkInterval = 3000;

// Dữ liệu Request
String currentRequestId = "";
String machineCode = "";
String machineName = "";
String location = "";
String issueDescription = "";

// --- Hàm hiển thị OLED chuyên dụng ---
void updateOLED(String title, String line1, String line2, String line3) {
  display.clear();
  
  // 1. Tiêu đề (Header) - To và Căn giữa
  display.setFont(ArialMT_Plain_16);
  display.setTextAlignment(TEXT_ALIGN_CENTER); 
  display.drawString(64, 0, title);            // 64 là điểm chính giữa trục X (128/2)
  display.drawLine(0, 18, 127, 18);            // Kẻ gạch ngang phân cách
  
  // 2. Nội dung chi tiết (Body) - Nhỏ gọn và Căn trái
  display.setFont(ArialMT_Plain_10);           // Chuyển sang Font 10 để đọc được nhiều chữ hơn
  display.setTextAlignment(TEXT_ALIGN_LEFT);   
  
  // In các nhãn (Cột bên trái)
  display.drawString(LABEL_X, LINE_1_Y, "Ch:");
  display.drawString(LABEL_X, LINE_2_Y, "PK:");
  display.drawString(LABEL_X, LINE_3_Y, "DA:");
  
  // In giá trị (Cột bên phải)
  display.drawString(VALUE_X, LINE_1_Y, line1);
  display.drawString(VALUE_X, LINE_2_Y, line2);
  display.drawString(VALUE_X, LINE_3_Y, line3);
  
  display.display();
}

void setup() {
  Serial.begin(115200);
  
  // Khởi tạo OLED
  display.init();
  if (flipDisplay) display.flipScreenVertically();
  display.clear();
  updateOLED("SYSTEM", "Starting...", "Device: " + String(deviceId), "");

  // Cấu hình chân IO
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  connectWiFi();
  updateOLED("READY", engineerName, "Waiting task", "WiFi: OK");
}

void loop() {
  // Kiểm tra request định kỳ
  if (millis() - lastCheckTime >= checkInterval) {
    lastCheckTime = millis();
    checkPendingRequests();
  }
  
  // Xử lý nút bấm (Nút FLASH)
  if (digitalRead(BUTTON_PIN) == LOW) {
    delay(50); // Debounce chống dội phím
    if (digitalRead(BUTTON_PIN) == LOW) {
      handleButtonPress();
      while (digitalRead(BUTTON_PIN) == LOW); // Chờ nhả nút
    }
  }
  
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
  } else {
    updateOLED("ERROR", "WiFi Failed", "Check Router", "");
  }
}

void checkPendingRequests() {
  if (WiFi.status() != WL_CONNECTED || currentState != STATE_IDLE) return;

  String url = String(serverUrl) + "/check/" + String(deviceId);
  http.begin(client1, url);
  int httpCode = http.GET();
  
  if (httpCode == 200) {
    String payload = http.getString();
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, payload);
    
    if (!error && doc["hasRequest"]) {
      currentRequestId = doc["requestId"].as<String>();
      machineCode = doc["machineCode"].as<String>();
      machineName = doc["machineName"].as<String>();
      location = doc["location"].as<String>();
      issueDescription = doc["issueDescription"].as<String>();
      
      currentState = STATE_REQUEST_RECEIVED;
      
      // Thông báo OLED & Buzzer khi có task mới
      updateOLED("NEW TASK!", machineCode, location, issueDescription);
      ringBuzzer();
    }
  }
  http.end();
}

void handleButtonPress() {
  switch (currentState) {
    case STATE_IDLE:
      updateOLED("INFO", "No Task", engineerName, "System Active");
      break;
      
    case STATE_REQUEST_RECEIVED:
      sendPostRequest("/acknowledge", STATE_ACKNOWLEDGED, "ON THE WAY", "To: " + location);
      break;
      
    case STATE_ACKNOWLEDGED:
      sendPostRequest("/arrive", STATE_ARRIVED, "ARRIVED", "Fixing: " + machineCode);
      break;
      
    case STATE_ARRIVED:
      if(sendPostRequest("/complete", STATE_IDLE, "COMPLETED", "Waiting task")) {
         // Reset data khi xong việc
         currentRequestId = ""; machineCode = ""; location = "";
      }
      break;
  }
}

// Hàm gửi POST chung để rút gọn code
bool sendPostRequest(String endpoint, DeviceState nextState, String displayTitle, String displayMsg) {
  if (WiFi.status() != WL_CONNECTED) return false;

  http.begin(client1, String(serverUrl) + endpoint);
  http.addHeader("Content-Type", "application/json");
  
  DynamicJsonDocument doc(256);
  doc["esp32DeviceId"] = deviceId;
  doc["engineerName"] = engineerName;
  
  String jsonString;
  serializeJson(doc, jsonString);
  
  int httpCode = http.POST(jsonString);
  bool success = (httpCode == 200);
  
  if (success) {
    currentState = nextState;
    updateOLED(displayTitle, displayMsg, "Success!", "");
    tone(BUZZER_PIN, 1500, 200);
  } else {
    updateOLED("ERR: " + String(httpCode), "Post Failed", "Try Again", "");
  }
  
  http.end();
  return success;
}

void ringBuzzer() {
  for (int i = 0; i < 3; i++) {
    tone(BUZZER_PIN, 2000, 400);
    delay(500);
  }
}

void updateLED() {
  switch (currentState) {
    case STATE_IDLE: digitalWrite(LED_PIN, LOW); break;
    case STATE_REQUEST_RECEIVED: digitalWrite(LED_PIN, (millis() / 200) % 2); break;
    case STATE_ACKNOWLEDGED: digitalWrite(LED_PIN, (millis() / 500) % 2); break;
    case STATE_ARRIVED: digitalWrite(LED_PIN, HIGH); break;
  }
}