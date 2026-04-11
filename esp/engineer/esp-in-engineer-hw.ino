#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <SSD1306Wire.h>  // Sử dụng thư viện ThingPulse chuẩn

// OTA
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#define ELEGANTOTA_USE_ASYNC_WEBSERVER 1
#include <ElegantOTA.h>

// OTA define server
AsyncWebServer server(80);

// --- Cấu hình OLED (CHUẨN CHO MẠCH HW-364B) ---
// Chân SDA ngầm = 14 (D5), SCL ngầm = 12 (D6)
SSD1306Wire display(0x3c, 14, 12);

// --- Cấu hình WiFi ---
const char* ssid = "HSVINA";
const char* password = "HSVINA@kor";
// const char* ssid = "SA";
// const char* password = "1234567890";
WiFiClient client1;
HTTPClient http;

// --- Cấu hình Server ---
const char* serverUrl = "http://103.140.249.253:8080/api/maintenance-v2";
const char* deviceId = "ESP_002";
const char* engineerName = "Ky su B";

// --- Cấu hình Pin (Đã được điều chỉnh tránh xung đột màn hình) ---
#define BUZZER_PIN 13  // Dời sang GPIO 13 (Chân D7) vì chân 14 đã cho màn hình
#define LED_PIN 2      // GPIO 2 (Chân D4) là chuẩn đèn LED xanh tích hợp trên ESP8266
#define BUTTON_PIN 0   // GPIO 0 (Chân D3 - Nút FLASH trên mạch)

// --- Quản lý trạng thái ---
enum DeviceState {
  STATE_IDLE,
  STATE_REQUEST_RECEIVED,
  STATE_ACKNOWLEDGED,
  // STATE_ARRIVED
};

// --- CẤU HÌNH LAYOUT CHO OLED 128x64 ---
#define LABEL_X 0    // Tọa độ X của các nhãn (|)
#define VALUE_X 10   // Tọa độ X của nội dung
#define LINE_1_Y 22  // Tọa độ Y Dòng 1
#define LINE_2_Y 36  // Tọa độ Y Dòng 2
#define LINE_3_Y 50  // Tọa độ Y Dòng 3

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
String requestStatus = "";

// --- Biến hỗ trợ chữ chạy (Scrolling Text) ---
String oledTitle = "";
String oledLine1 = "";
String oledLine2 = "";
String oledLine3 = "";

int scrollX = 0;                            // Tọa độ cuộn hiện tại
unsigned long lastScrollTime = 0;           // Bộ đếm thời gian cuộn
const int SCROLL_SPEED = 50;                // Tốc độ cuộn (ms) - Số càng nhỏ cuộn càng nhanh
int waitTimer = 500;                        // Thời gian dừng (ms) ở đầu và cuối chu kỳ
const int MAX_WIDTH_ALLOW = 128 - VALUE_X;  // Độ rộng tối đa để hiển thị text


// --- Hàm render giao diện lên màn hình ---
void renderDisplay() {
  display.clear();

  // 1. Tiêu đề (Header)
  display.setFont(ArialMT_Plain_16);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(64, 0, oledTitle);
  display.drawLine(0, 18, 127, 18);

  // 2. Nội dung chi tiết (Tính toán cuộn)
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);

  int w1 = display.getStringWidth(oledLine1);
  int w2 = display.getStringWidth(oledLine2);
  int w3 = display.getStringWidth(oledLine3);

  // Chỉ cuộn những dòng vượt quá MAX_WIDTH_ALLOW
  display.drawString(VALUE_X - (w1 > MAX_WIDTH_ALLOW ? scrollX : 0), LINE_1_Y, oledLine1);
  display.drawString(VALUE_X - (w2 > MAX_WIDTH_ALLOW ? scrollX : 0), LINE_2_Y, oledLine2);
  display.drawString(VALUE_X - (w3 > MAX_WIDTH_ALLOW ? scrollX : 0), LINE_3_Y, oledLine3);

  // 3. Che phần chữ bị lẹm sang trái bằng một khối màu đen
  display.setColor(BLACK);
  display.fillRect(0, 19, VALUE_X - 1, 45);  // Xóa khu vực cột bên trái
  display.setColor(WHITE);

  // 4. In lại các nhãn lên trên vùng đen
  display.drawString(LABEL_X, LINE_1_Y, "|");
  display.drawString(LABEL_X, LINE_2_Y, "|");
  display.drawString(LABEL_X, LINE_3_Y, "|");

  display.display();
}

// --- Hàm xử lý logic cuộn chữ (Gọi liên tục trong loop) ---
void handleScrolling() {
  display.setFont(ArialMT_Plain_10);
  int maxW = max(display.getStringWidth(oledLine1),
                 max(display.getStringWidth(oledLine2),
                     display.getStringWidth(oledLine3)));

  // Nếu có ít nhất 1 dòng quá dài, tiến hành cuộn
  if (maxW > MAX_WIDTH_ALLOW) {
    if (millis() - lastScrollTime >= SCROLL_SPEED) {
      lastScrollTime = millis();

      if (waitTimer > 0) {
        waitTimer -= SCROLL_SPEED;  // Đang chờ trước khi cuộn
      } else {
        scrollX++;  // Dịch chữ sang trái 1 pixel

        // Cập nhật khoảng đệm khi cuộn hết để chữ lặp lại mượt hơn
        if (scrollX > maxW - MAX_WIDTH_ALLOW + 30) {
          scrollX = 0;       // Quay lại từ đầu
          waitTimer = 1500;  // Dừng 1.5 giây rồi mới chạy tiếp
        }
        renderDisplay();  // Cập nhật lại màn hình
      }
    }
  }
}

// --- Hàm cập nhật nội dung OLED ---
void updateOLED(String title, String line1, String line2, String line3) {
  oledTitle = title;
  oledLine1 = line1;
  oledLine2 = line2;
  oledLine3 = line3;

  scrollX = 0;       // Reset tọa độ cuộn
  waitTimer = 1500;  // Dừng 1.5s trước khi cuộn

  renderDisplay();
}

void setup() {
  Serial.begin(115200);

  // Khởi tạo OLED
  display.init();
  if (flipDisplay) display.flipScreenVertically();
  display.clear();

  // Cấu hình chân IO
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  updateOLED("SYSTEM", "Starting...", "Device: " + String(deviceId), "");

  connectWiFi();
  updateOLED("READY", engineerName, "MAC: " + WiFi.macAddress(), "WiFi: OK - " + WiFi.localIP().toString());


  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(200, "text/plain", "Hi! I am ESP8266.");
  });

  ElegantOTA.begin(&server);  // Start ElegantOTA
  server.begin();

  Serial.println(WiFi.localIP());
}

void loop() {
  ElegantOTA.loop();  // OTA reload when new code

  // Xử lý hiệu ứng chữ chạy liên tục
  handleScrolling();

  // Kiểm tra request định kỳ
  if (millis() - lastCheckTime >= checkInterval) {
    lastCheckTime = millis();
    checkPendingRequests();
  }

  // Xử lý nút bấm (Nút FLASH / D3)
  if (digitalRead(BUTTON_PIN) == LOW) {
    delay(50);  // Debounce chống dội phím
    if (digitalRead(BUTTON_PIN) == LOW) {
      handleButtonPress();
      while (digitalRead(BUTTON_PIN) == LOW)
        ;  // Chờ nhả nút
    }
  }

  updateLED();
  delay(10);  // Delay ngắn để cuộn chữ mượt hơn
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
    Serial.println(WiFi.localIP());
  } else {
    updateOLED("ERROR", "WiFi Failed", "Check Router", "");
  }
}

void checkPendingRequests() {
  // if (WiFi.status() != WL_CONNECTED || currentState != STATE_IDLE) return;
  if (WiFi.status() != WL_CONNECTED) return;

  String url = String(serverUrl) + "/check/" + String(deviceId);
  http.begin(client1, url);  // Đã thêm client1 cho chuẩn ESP8266
  int httpCode = http.GET();

  if (httpCode == 200) {
    String payload = http.getString();
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, payload);

    if (!error) {
      if (doc["hasRequest"]) {
        currentRequestId = doc["requestId"].as<String>();
        machineCode = doc["machineCode"].as<String>();
        machineName = doc["machineName"].as<String>();
        location = doc["location"].as<String>();
        issueDescription = doc["issueDescription"].as<String>();
        requestStatus = doc["status"].as<String>();

        if (requestStatus == "REQUESTED") {
          Serial.println("DEBUG: Đã nhận được yêu cầu mới!");
          currentState = STATE_REQUEST_RECEIVED;
          updateOLED("NEW TASK!", machineCode, location, issueDescription);
          ringBuzzer();
        }

        if (requestStatus == "ACKNOWLEDGED") {
          currentState = STATE_ACKNOWLEDGED;
          updateOLED("ACKNOW", "To: " + location, "Fixing: " + machineCode, "Pls Ask User Confirm When you done");
          // ringBuzzer();
        }
      } else {
        if (currentState == STATE_ACKNOWLEDGED) {
          currentState = STATE_IDLE;
          currentRequestId = "";
          machineCode = "";
          location = "";
          updateOLED("SUCCESS", "User confirmed", "Pls click BUTTON to continue", "");
        }
      }
    }
  }
  http.end();
}

void handleButtonPress() {
  switch (currentState) {
    case STATE_IDLE:
      updateOLED("INFO", "No Task", engineerName, "System Active");
      break;

    // Lúc này đã biết vấn đề, đã hiển thị trên màn hình, lúc này nhấn nút để thông báo lên line là đã tiếp nhận
    case STATE_REQUEST_RECEIVED:
      sendPostRequest("/acknowledge", STATE_ACKNOWLEDGED, "ON THE WAY", "To: " + location);
      break;

    case STATE_ACKNOWLEDGED:
      // sendPostRequest("/arrive", STATE_ARRIVED, "ARRIVED", "Fixing: " + machineCode);
      break;
  }
}

bool sendPostRequest(String endpoint, DeviceState nextState, String displayTitle, String displayMsg) {
  if (WiFi.status() != WL_CONNECTED) return false;

  http.begin(client1, String(serverUrl) + endpoint);  // Đã thêm client1
  http.setTimeout(10000 * 6);
  http.addHeader("Content-Type", "application/json");

  DynamicJsonDocument doc(256);
  doc["esp32DeviceId"] = deviceId;  // Giữ nguyên tên key API của bạn
  doc["engineerName"] = engineerName;
  doc["requestId"] = currentRequestId;

  String jsonString;
  serializeJson(doc, jsonString);

  int httpCode = http.POST(jsonString);
  bool success = (httpCode == 200);

  if (success) {
    currentState = nextState;
    updateOLED(displayTitle, displayMsg, "Success!", "");

    tone(BUZZER_PIN, 1500, 200);  // Kêu bíp báo hiệu thành công
  } else {
    updateOLED("ERR: " + String(httpCode), "Post Failed", "Try Again", "");
  }

  http.end();
  return success;
}

void ringBuzzer() {
  for (int i = 0; i < 1; i++) {
    tone(BUZZER_PIN, 1500, 200);
    delay(500);
  }
}

void updateLED() {
  switch (currentState) {
    case STATE_IDLE: digitalWrite(LED_PIN, HIGH); break;  // ESP8266 LED tích hợp thường ngược (HIGH là tắt, LOW là sáng)
    case STATE_REQUEST_RECEIVED: digitalWrite(LED_PIN, (millis() / 200) % 2 == 0 ? LOW : HIGH); break;
    case STATE_ACKNOWLEDGED:
      digitalWrite(LED_PIN, (millis() / 500) % 2 == 0 ? LOW : HIGH);
      break;
  }
}