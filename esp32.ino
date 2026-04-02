// #include <WiFi.h>
// #include <HTTPClient.h>
// #include <ArduinoJson.h>
// #include <Wire.h>
// #include <SSD1306Wire.h>  // Sử dụng thư viện ThingPulse chuẩn

// // --- Cấu hình OLED ---
// // ESP32 thường sử dụng mặc định: SDA = 21, SCL = 22
// SSD1306Wire display(0x3c, 21, 22);

// // --- Cấu hình WiFi ---
// const char* ssid = "SA";
// const char* password = "1234567890";
// WiFiClient client1;
// HTTPClient http;

// // --- Cấu hình Server ---
// const char* serverUrl = "http://103.140.249.253:8080/api/maintenance-v2";
// const char* deviceId = "ESP_001";
// const char* engineerName = "Ky su A";

// // --- Cấu hình Pin (Dành cho ESP32) ---
// #define BUZZER_PIN 14  // GPIO 14 phù hợp cho còi Buzzer trên ESP32
// #define LED_PIN 2      // Đèn LED màu xanh dương tích hợp trên mạch ESP32
// #define BUTTON_PIN 0   // Nút BOOT tích hợp sẵn trên mạch ESP32

// // --- Quản lý trạng thái ---
// enum DeviceState {
//   STATE_IDLE,
//   STATE_REQUEST_RECEIVED,
//   STATE_ACKNOWLEDGED,
//   STATE_ARRIVED
// };

// // --- CẤU HÌNH LAYOUT MỚI CHO OLED 128x64 ---
// #define LABEL_X 0    // Tọa độ X của các nhãn (Ch:, PK:, DA:)
// #define VALUE_X 15   // Tọa độ X của nội dung (dịch sang trái để có nhiều không gian)
// #define LINE_1_Y 22  // Tọa độ Y Dòng 1
// #define LINE_2_Y 36  // Tọa độ Y Dòng 2
// #define LINE_3_Y 50  // Tọa độ Y Dòng 3 (Không bị lẹm đáy màn hình)

// #define flipDisplay true

// DeviceState currentState = STATE_IDLE;
// unsigned long lastCheckTime = 0;
// const unsigned long checkInterval = 3000;

// // Dữ liệu Request
// String currentRequestId = "";
// String machineCode = "";
// String machineName = "";
// String location = "";
// String issueDescription = "";
// String requestStatus = "";

// // --- Hàm hiển thị OLED chuyên dụng ---
// void updateOLED(String title, String line1, String line2, String line3) {
//   display.clear();

//   // 1. Tiêu đề (Header) - To và Căn giữa
//   display.setFont(ArialMT_Plain_16);
//   display.setTextAlignment(TEXT_ALIGN_CENTER);
//   display.drawString(64, 0, title);  // 64 là điểm chính giữa trục X (128/2)
//   display.drawLine(0, 18, 127, 18);  // Kẻ gạch ngang phân cách

//   // 2. Nội dung chi tiết (Body) - Nhỏ gọn và Căn trái
//   display.setFont(ArialMT_Plain_10);  // Chuyển sang Font 10 để đọc được nhiều chữ hơn
//   display.setTextAlignment(TEXT_ALIGN_LEFT);

//   // In các nhãn (Cột bên trái)
//   display.drawString(LABEL_X, LINE_1_Y, "|");
//   display.drawString(LABEL_X, LINE_2_Y, "|");
//   display.drawString(LABEL_X, LINE_3_Y, "|");

//   // In giá trị (Cột bên phải)
//   display.drawString(VALUE_X, LINE_1_Y, line1);
//   display.drawString(VALUE_X, LINE_2_Y, line2);
//   display.drawString(VALUE_X, LINE_3_Y, line3);

//   display.display();
// }

// void setup() {
//   Serial.begin(115200);

//   // Khởi tạo OLED
//   display.init();
//   if (flipDisplay) display.flipScreenVertically();
//   display.clear();
//   updateOLED("SYSTEM", "Starting...", "Device: " + String(deviceId), "");

//   // Cấu hình chân IO
//   pinMode(BUZZER_PIN, OUTPUT);
//   pinMode(LED_PIN, OUTPUT);
//   pinMode(BUTTON_PIN, INPUT_PULLUP);

//   connectWiFi();
//   updateOLED("READY", engineerName, "Waiting task", "WiFi: OK");
// }

// void loop() {
//   // Kiểm tra request định kỳ
//   if (millis() - lastCheckTime >= checkInterval) {
//     lastCheckTime = millis();
//     checkPendingRequests();
//   }

//   // Xử lý nút bấm (Nút BOOT)
//   if (digitalRead(BUTTON_PIN) == LOW) {
//     delay(50);  // Debounce chống dội phím
//     if (digitalRead(BUTTON_PIN) == LOW) {
//       handleButtonPress();
//       while (digitalRead(BUTTON_PIN) == LOW)
//         ;  // Chờ nhả nút
//     }
//   }

//   updateLED();
//   delay(100);
// }

// void connectWiFi() {
//   Serial.println("Connecting to WiFi...");
//   WiFi.begin(ssid, password);

//   int attempts = 0;
//   while (WiFi.status() != WL_CONNECTED && attempts < 20) {
//     delay(500);
//     Serial.print(".");
//     attempts++;
//   }

//   if (WiFi.status() == WL_CONNECTED) {
//     Serial.println("\nWiFi Connected!");
//   } else {
//     updateOLED("ERROR", "WiFi Failed", "Check Router", "");
//   }
// }

// void checkPendingRequests() {
//   if (WiFi.status() != WL_CONNECTED || currentState != STATE_IDLE) return;

//   String url = String(serverUrl) + "/check/" + String(deviceId);
//   http.begin(client1, url);
//   int httpCode = http.GET();

//   if (httpCode == 200) {
//     String payload = http.getString();
//     DynamicJsonDocument doc(1024);
//     DeserializationError error = deserializeJson(doc, payload);

//     if (!error && doc["hasRequest"]) {
//       currentRequestId = doc["requestId"].as<String>();
//       machineCode = doc["machineCode"].as<String>();
//       machineName = doc["machineName"].as<String>();
//       location = doc["location"].as<String>();
//       issueDescription = doc["issueDescription"].as<String>();
//       requestStatus = doc["status"].as<String>();

//       //     STATE_IDLE,
//       // STATE_REQUEST_RECEIVED,
//       // STATE_ACKNOWLEDGED,
//       // STATE_ARRIVED

//       if (requestStatus == "REQUESTED") {
//         Serial.println("DEBUG: Đã nhận được yêu cầu mới!");
//         currentState = STATE_REQUEST_RECEIVED;
//         // Thông báo OLED & Buzzer khi có task mới
//         updateOLED("NEW TASK!", machineCode, location, issueDescription);
//         ringBuzzer();
//       }


//       if (requestStatus == "ACKNOWLEDGED") {
//         currentState = STATE_ACKNOWLEDGED;
//         updateOLED("ON THE WAY", "", "Success!", "");
//         ringBuzzer();
//       }

//       if (requestStatus == "ARRIVED") {
//         currentState = STATE_ARRIVED;
//         updateOLED("ARRIVED", "", "Success!", "");
//         ringBuzzer();
//       }
//     }
//   }
//   http.end();
// }

// void handleButtonPress() {
//   switch (currentState) {
//     case STATE_IDLE:
//       updateOLED("INFO", "No Task", engineerName, "System Active");
//       break;

//     case STATE_REQUEST_RECEIVED:
//       sendPostRequest("/acknowledge", STATE_ACKNOWLEDGED, "ON THE WAY", "To: " + location);
//       break;

//     case STATE_ACKNOWLEDGED:
//       sendPostRequest("/arrive", STATE_ARRIVED, "ARRIVED", "Fixing: " + machineCode);
//       break;

//     case STATE_ARRIVED:
//       if (sendPostRequest("/complete", STATE_IDLE, "COMPLETED", "Waiting task")) {
//         // Reset data khi xong việc
//         currentRequestId = "";
//         machineCode = "";
//         location = "";
//       }
//       break;
//   }
// }

// // Hàm gửi POST chung để rút gọn code
// bool sendPostRequest(String endpoint, DeviceState nextState, String displayTitle, String displayMsg) {
//   if (WiFi.status() != WL_CONNECTED) return false;

//   http.begin(client1, String(serverUrl) + endpoint);
//   http.addHeader("Content-Type", "application/json");

//   DynamicJsonDocument doc(256);
//   doc["esp32DeviceId"] = deviceId;
//   doc["engineerName"] = engineerName;
//   doc["requestId"] = currentRequestId;

//   String jsonString;
//   serializeJson(doc, jsonString);

//   int httpCode = http.POST(jsonString);
//   bool success = (httpCode == 200);

//   if (success) {
//     currentState = nextState;
//     updateOLED(displayTitle, displayMsg, "Success!", "");
//     tone(BUZZER_PIN, 1500, 200);
//   } else {
//     updateOLED("ERR: " + String(httpCode), "Post Failed", "Try Again", "");
//   }

//   http.end();
//   return success;
// }

// void ringBuzzer() {
//   for (int i = 0; i < 3; i++) {
//     tone(BUZZER_PIN, 2000, 400);
//     delay(500);
//   }
// }

// void updateLED() {
//   switch (currentState) {
//     case STATE_IDLE: digitalWrite(LED_PIN, LOW); break;
//     case STATE_REQUEST_RECEIVED: digitalWrite(LED_PIN, (millis() / 200) % 2); break;
//     case STATE_ACKNOWLEDGED: digitalWrite(LED_PIN, (millis() / 500) % 2); break;
//     case STATE_ARRIVED: digitalWrite(LED_PIN, HIGH); break;
//   }
// }















#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <SSD1306Wire.h>  // Sử dụng thư viện ThingPulse chuẩn

// --- Cấu hình OLED ---
SSD1306Wire display(0x3c, 21, 22);

// --- Cấu hình WiFi ---
const char* ssid = "SA";
const char* password = "1234567890";
WiFiClient client1;
HTTPClient http;

// --- Cấu hình Server ---
const char* serverUrl = "http://103.140.249.253:8080/api/maintenance-v2";
const char* deviceId = "ESP_001";
const char* engineerName = "Ky su A";

// --- Cấu hình Pin (Dành cho ESP32) ---
#define BUZZER_PIN 14  // GPIO 14 phù hợp cho còi Buzzer trên ESP32
#define LED_PIN 2      // Đèn LED màu xanh dương tích hợp trên mạch ESP32
#define BUTTON_PIN 0   // Nút BOOT tích hợp sẵn trên mạch ESP32

// --- Quản lý trạng thái ---
enum DeviceState {
  STATE_IDLE,
  STATE_REQUEST_RECEIVED,
  STATE_ACKNOWLEDGED,
  STATE_ARRIVED
};

// --- CẤU HÌNH LAYOUT MỚI CHO OLED 128x64 ---
#define LABEL_X 0    // Tọa độ X của các nhãn (|)
#define VALUE_X 5   // Tọa độ X của nội dung
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

int scrollX = 0;                     // Tọa độ cuộn hiện tại
unsigned long lastScrollTime = 0;    // Bộ đếm thời gian cuộn
const int SCROLL_SPEED = 40;         // Tốc độ cuộn (ms) - Số càng nhỏ cuộn càng nhanh
int waitTimer = 1500;                // Thời gian dừng (ms) trước khi bắt đầu cuộn lại
const int MAX_WIDTH_ALLOW = 128 - VALUE_X; // Độ rộng tối đa để hiển thị text


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
  display.fillRect(0, 19, VALUE_X - 1, 45); // Xóa khu vực cột bên trái
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
        waitTimer -= SCROLL_SPEED; // Đang chờ trước khi cuộn
      } else {
        scrollX++; // Dịch chữ sang trái 1 pixel
        
        // Nếu đã cuộn hết chữ cộng thêm 1 khoảng trống nhỏ
        if (scrollX > maxW - MAX_WIDTH_ALLOW + 20) { 
          scrollX = 0;         // Quay lại từ đầu
          waitTimer = 1500;    // Dừng 1.5 giây rồi mới chạy tiếp
        }
        renderDisplay();       // Cập nhật lại màn hình
      }
    }
  }
}

// --- Hàm thay thế updateOLED cũ ---
// Hàm này giờ chỉ nhận dữ liệu, reset cuộn và gọi render 1 lần
void updateOLED(String title, String line1, String line2, String line3) {
  oledTitle = title;
  oledLine1 = line1;
  oledLine2 = line2;
  oledLine3 = line3;
  
  scrollX = 0;          // Reset tọa độ cuộn
  waitTimer = 1500;     // Dừng 1.5s trước khi cuộn
  
  renderDisplay();
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
  // Xử lý hiệu ứng chữ chạy liên tục
  handleScrolling();

  // Kiểm tra request định kỳ
  if (millis() - lastCheckTime >= checkInterval) {
    lastCheckTime = millis();
    checkPendingRequests();
  }

  // Xử lý nút bấm (Nút BOOT)
  if (digitalRead(BUTTON_PIN) == LOW) {
    delay(50);  // Debounce chống dội phím
    if (digitalRead(BUTTON_PIN) == LOW) {
      handleButtonPress();
      while (digitalRead(BUTTON_PIN) == LOW); // Chờ nhả nút
    }
  }

  updateLED();
  delay(10); // Giảm delay xuống để cuộn chữ mượt hơn
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
      requestStatus = doc["status"].as<String>();

      if (requestStatus == "REQUESTED") {
        Serial.println("DEBUG: Đã nhận được yêu cầu mới!");
        currentState = STATE_REQUEST_RECEIVED;
        updateOLED("NEW TASK!", machineCode, location, issueDescription);
        ringBuzzer();
      }

      if (requestStatus == "ACKNOWLEDGED") {
        currentState = STATE_ACKNOWLEDGED;
        updateOLED("ON THE WAY", "To: " + location, "Success!", "");
        ringBuzzer();
      }

      if (requestStatus == "ARRIVED") {
        currentState = STATE_ARRIVED;
        updateOLED("ARRIVED", "Fixing: " + machineCode, "Success!", "");
        ringBuzzer();
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

    case STATE_REQUEST_RECEIVED:
      sendPostRequest("/acknowledge", STATE_ACKNOWLEDGED, "ON THE WAY", "To: " + location);
      break;

    case STATE_ACKNOWLEDGED:
      sendPostRequest("/arrive", STATE_ARRIVED, "ARRIVED", "Fixing: " + machineCode);
      break;

    case STATE_ARRIVED:
      if (sendPostRequest("/complete", STATE_IDLE, "COMPLETED", "Waiting task")) {
        currentRequestId = "";
        machineCode = "";
        location = "";
      }
      break;
  }
}

bool sendPostRequest(String endpoint, DeviceState nextState, String displayTitle, String displayMsg) {
  if (WiFi.status() != WL_CONNECTED) return false;

  http.begin(client1, String(serverUrl) + endpoint);
  http.setTimeout(10000 * 6);
  http.addHeader("Content-Type", "application/json");

  DynamicJsonDocument doc(256);
  doc["esp32DeviceId"] = deviceId;
  doc["engineerName"] = engineerName;
  doc["requestId"] = currentRequestId;

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
    delay(500); // Lưu ý: hàm delay ở đây khi chuông reo sẽ tạm dừng cuộn chữ, xong chuông chữ lại chạy tiếp
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