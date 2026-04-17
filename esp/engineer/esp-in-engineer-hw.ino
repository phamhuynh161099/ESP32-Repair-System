// #include <ESP8266WiFi.h>
// #include <ESP8266HTTPClient.h>
// #include <ArduinoJson.h>
// #include <Wire.h>
// #include <SSD1306Wire.h>  // Sử dụng thư viện ThingPulse chuẩn

// // OTA
// #include <ESPAsyncTCP.h>
// #include <ESPAsyncWebServer.h>
// #define ELEGANTOTA_USE_ASYNC_WEBSERVER 1
// #include <ElegantOTA.h>

// // OTA define server
// AsyncWebServer server(80);

// // --- Cấu hình OLED (CHUẨN CHO MẠCH HW-364B) ---
// // Chân SDA ngầm = 14 (D5), SCL ngầm = 12 (D6)
// SSD1306Wire display(0x3c, 14, 12);

// // --- Cấu hình WiFi ---
// const char* ssid = "HSVINA";
// const char* password = "HSVINA@kor";
// // const char* ssid = "SA";
// // const char* password = "1234567890";
// WiFiClient client1;
// HTTPClient http;

// // --- Cấu hình Server ---
// const char* serverUrl = "http://10.20.13.50:8080/espRecieve";
// const char* deviceId = "ESP_002";
// const char* engineerName = "Ky su B";

// // --- Cấu hình Pin (Đã được điều chỉnh tránh xung đột màn hình) ---
// #define BUZZER_PIN 13  // Dời sang GPIO 13 (Chân D7) vì chân 14 đã cho màn hình
// #define LED_PIN 2      // GPIO 2 (Chân D4) là chuẩn đèn LED xanh tích hợp trên ESP8266
// #define BUTTON_PIN 0   // GPIO 0 (Chân D3 - Nút FLASH trên mạch)

// // --- Quản lý trạng thái ---
// enum DeviceState {
//   STATE_IDLE,
//   STATE_REQUEST_RECEIVED,
//   STATE_ACKNOWLEDGED,
//   // STATE_ARRIVED
// };

// // --- CẤU HÌNH LAYOUT CHO OLED 128x64 ---
// #define LABEL_X 0    // Tọa độ X của các nhãn (|)
// #define VALUE_X 10   // Tọa độ X của nội dung
// #define LINE_1_Y 22  // Tọa độ Y Dòng 1
// #define LINE_2_Y 36  // Tọa độ Y Dòng 2
// #define LINE_3_Y 50  // Tọa độ Y Dòng 3

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

// // --- Biến hỗ trợ chữ chạy (Scrolling Text) ---
// String oledTitle = "";
// String oledLine1 = "";
// String oledLine2 = "";
// String oledLine3 = "";

// int scrollX = 0;                            // Tọa độ cuộn hiện tại
// unsigned long lastScrollTime = 0;           // Bộ đếm thời gian cuộn
// const int SCROLL_SPEED = 50;                // Tốc độ cuộn (ms) - Số càng nhỏ cuộn càng nhanh
// int waitTimer = 500;                        // Thời gian dừng (ms) ở đầu và cuối chu kỳ
// const int MAX_WIDTH_ALLOW = 128 - VALUE_X;  // Độ rộng tối đa để hiển thị text


// // --- Hàm render giao diện lên màn hình ---
// void renderDisplay() {
//   display.clear();

//   // 1. Tiêu đề (Header)
//   display.setFont(ArialMT_Plain_16);
//   display.setTextAlignment(TEXT_ALIGN_CENTER);
//   display.drawString(64, 0, oledTitle);
//   display.drawLine(0, 18, 127, 18);

//   // 2. Nội dung chi tiết (Tính toán cuộn)
//   display.setFont(ArialMT_Plain_10);
//   display.setTextAlignment(TEXT_ALIGN_LEFT);

//   int w1 = display.getStringWidth(oledLine1);
//   int w2 = display.getStringWidth(oledLine2);
//   int w3 = display.getStringWidth(oledLine3);

//   // Chỉ cuộn những dòng vượt quá MAX_WIDTH_ALLOW
//   display.drawString(VALUE_X - (w1 > MAX_WIDTH_ALLOW ? scrollX : 0), LINE_1_Y, oledLine1);
//   display.drawString(VALUE_X - (w2 > MAX_WIDTH_ALLOW ? scrollX : 0), LINE_2_Y, oledLine2);
//   display.drawString(VALUE_X - (w3 > MAX_WIDTH_ALLOW ? scrollX : 0), LINE_3_Y, oledLine3);

//   // 3. Che phần chữ bị lẹm sang trái bằng một khối màu đen
//   display.setColor(BLACK);
//   display.fillRect(0, 19, VALUE_X - 1, 45);  // Xóa khu vực cột bên trái
//   display.setColor(WHITE);

//   // 4. In lại các nhãn lên trên vùng đen
//   display.drawString(LABEL_X, LINE_1_Y, "|");
//   display.drawString(LABEL_X, LINE_2_Y, "|");
//   display.drawString(LABEL_X, LINE_3_Y, "|");

//   display.display();
// }

// // --- Hàm xử lý logic cuộn chữ (Gọi liên tục trong loop) ---
// void handleScrolling() {
//   display.setFont(ArialMT_Plain_10);
//   int maxW = max(display.getStringWidth(oledLine1),
//                  max(display.getStringWidth(oledLine2),
//                      display.getStringWidth(oledLine3)));

//   // Nếu có ít nhất 1 dòng quá dài, tiến hành cuộn
//   if (maxW > MAX_WIDTH_ALLOW) {
//     if (millis() - lastScrollTime >= SCROLL_SPEED) {
//       lastScrollTime = millis();

//       if (waitTimer > 0) {
//         waitTimer -= SCROLL_SPEED;  // Đang chờ trước khi cuộn
//       } else {
//         scrollX++;  // Dịch chữ sang trái 1 pixel

//         // Cập nhật khoảng đệm khi cuộn hết để chữ lặp lại mượt hơn
//         if (scrollX > maxW - MAX_WIDTH_ALLOW + 30) {
//           scrollX = 0;       // Quay lại từ đầu
//           waitTimer = 1500;  // Dừng 1.5 giây rồi mới chạy tiếp
//         }
//         renderDisplay();  // Cập nhật lại màn hình
//       }
//     }
//   }
// }

// // --- Hàm cập nhật nội dung OLED ---
// void updateOLED(String title, String line1, String line2, String line3) {
//   oledTitle = title;
//   oledLine1 = line1;
//   oledLine2 = line2;
//   oledLine3 = line3;

//   scrollX = 0;       // Reset tọa độ cuộn
//   waitTimer = 1500;  // Dừng 1.5s trước khi cuộn

//   renderDisplay();
// }

// void setup() {
//   Serial.begin(115200);

//   // Khởi tạo OLED
//   display.init();
//   if (flipDisplay) display.flipScreenVertically();
//   display.clear();

//   // Cấu hình chân IO
//   pinMode(BUZZER_PIN, OUTPUT);
//   pinMode(LED_PIN, OUTPUT);
//   pinMode(BUTTON_PIN, INPUT_PULLUP);

//   updateOLED("SYSTEM", "Starting...", "Device: " + String(deviceId), "");

//   connectWiFi();
//   // updateOLED("READY", engineerName, "MAC: " + WiFi.macAddress(), "WiFi: OK - " + WiFi.localIP().toString());

//   if (WiFi.status() == WL_CONNECTED) {
//     fetchEngineerInfo();
//   }


//   server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
//     request->send(200, "text/plain", "Hi! I am ESP8266.");
//   });

//   ElegantOTA.begin(&server);  // Start ElegantOTA
//   server.begin();

//   Serial.println(WiFi.localIP());
// }

// void loop() {
//   ElegantOTA.loop();  // OTA reload when new code

//   // Xử lý hiệu ứng chữ chạy liên tục
//   handleScrolling();

//   // Kiểm tra request định kỳ
//   if (millis() - lastCheckTime >= checkInterval) {
//     lastCheckTime = millis();
//     checkPendingRequests();
//   }

//   // Xử lý nút bấm (Nút FLASH / D3)
//   if (digitalRead(BUTTON_PIN) == LOW) {
//     delay(50);  // Debounce chống dội phím
//     if (digitalRead(BUTTON_PIN) == LOW) {
//       handleButtonPress();
//       while (digitalRead(BUTTON_PIN) == LOW)
//         ;  // Chờ nhả nút
//     }
//   }

//   updateLED();
//   delay(10);  // Delay ngắn để cuộn chữ mượt hơn
// }

// // --- CÁC BIẾN TOÀN CỤC ---
// String macAddress = "";
// String lineId = "";
// String lineName = "";
// String espName = "";
// String espEngineerMac = "";
// String espEngineerName = "";
// // Dữ liệu Check Request


// String engineer_esp_id = "";
// String engineer_esp_mac = "";
// String engineer_esp_name = "";
// String eng_id = "";

// String line_esp_id = "";
// String line_esp_mac = "";
// String line_esp_name = "";
// String line_id = "";

// // String location = "";
// // String issueDescription = "";
// // String requestStatus = "";

// // --- HÀM GỌI API LẤY THÔNG TIN LINE THEO MAC ---
// void fetchEngineerInfo() {
//   if (WiFi.status() != WL_CONNECTED) return;

//   macAddress = WiFi.macAddress();
//   Serial.println("MAC Address: " + macAddress);
//   updateOLED("LOADING...", "MAC: " + macAddress, "Fetching Line Data", "Please wait...");
//   String url = String("http://10.20.13.50:8080/espRecieve/get-engineer-info-by") + "?mac=" + macAddress;

//   http.begin(client1, url);
//   int httpCode = http.GET();
//   if (httpCode == 200) {
//     String payload = http.getString();
//     Serial.println("Response: " + payload);

//     DynamicJsonDocument doc(1024);
//     DeserializationError error = deserializeJson(doc, payload);

//     if (!error) {
//       if (doc["etc"].containsKey("lineInfo") && !doc["etc"]["lineInfo"].isNull()) {
//         JsonObject lineInfoObj = doc["etc"]["lineInfo"];

//         lineId = lineInfoObj["line_id"].as<String>();
//         lineName = lineInfoObj["line_name"].as<String>();
//         espName = lineInfoObj["line_code"].as<String>();
//         espEngineerMac = lineInfoObj["engineer_esp_mac"].as<String>();
//         espEngineerName = lineInfoObj["eng_name"].as<String>();

//         // 2026.04.14 Them vao, can refactor
//         engineer_esp_id = lineInfoObj["engineer_esp_id"].as<String>();
//         engineer_esp_mac = lineInfoObj["engineer_esp_mac"].as<String>();
//         engineer_esp_name = lineInfoObj["eng_name"].as<String>();
//         eng_id = lineInfoObj["eng_id"].as<String>();

//         line_esp_id = lineInfoObj["line_esp_id"].as<String>();
//         line_esp_mac = lineInfoObj["line_esp_mac"].as<String>();
//         line_esp_name = lineInfoObj["line_name"].as<String>();
//         line_id = lineInfoObj["line_id"].as<String>();
//         // 2026.04.14 Them vao, can refactor

//         updateOLED("ENG INFO", "ID: " + eng_id,"Eng: " + espEngineerName,"");
//       } else {
//         updateOLED("UNREGISTERED", "MAC Not Found!", "MAC: " + macAddress, "Contact Admin");
//       }
//     } else {
//       updateOLED("ERROR", "JSON Parse Failed", error.c_str(), "");
//     }
//   } else {
//     updateOLED("SERVER ERR", "HTTP Code: " + String(httpCode), "Check Server API", "");
//   }
//   http.end();
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
//     Serial.println(WiFi.localIP());
//   } else {
//     updateOLED("ERROR", "WiFi Failed", "Check Router", "");
//   }
// }

// void checkPendingRequests() {
//   // if (WiFi.status() != WL_CONNECTED || currentState != STATE_IDLE) return;
//   if (WiFi.status() != WL_CONNECTED) return;

//   String url = String("http://10.20.13.50:8080/espRecieve") + "/checkPendingRequest/" + String(eng_id);
//   http.begin(client1, url);  // Đã thêm client1 cho chuẩn ESP8266
//   int httpCode = http.GET();

//   if (httpCode == 200) {
//     String payload = http.getString();
//     DynamicJsonDocument doc(1024);
//     DeserializationError error = deserializeJson(doc, payload);

//     if (!error) {
//       if (doc["etc"]["hasRequest"]) {
//         currentRequestId = doc["etc"]["data"]["id"].as<String>();
//         machineCode = doc["etc"]["data"]["line_id"].as<String>();
//         machineName = doc["etc"]["data"]["line_esp_name"].as<String>();
//         location = doc["etc"]["data"]["location"].as<String>();
//         issueDescription = doc["etc"]["data"]["issueDescription"].as<String>();
//         requestStatus = doc["etc"]["data"]["status"].as<String>();

//         if (requestStatus == "REQUESTED") {
//           Serial.println("DEBUG: Đã nhận được yêu cầu mới!");
//           currentState = STATE_REQUEST_RECEIVED;
//           updateOLED("NEW TASK!", machineName, location, "");
//           ringBuzzer();
//         }

//         if (requestStatus == "ACKNOWLEDGED") {
//           currentState = STATE_ACKNOWLEDGED;
//           updateOLED("ACKNOW", "To: " + location, "Fixing: " + machineCode, "Pls Ask User Confirm When you done");
//           // ringBuzzer();
//         }
//       } else {
//         if (currentState == STATE_ACKNOWLEDGED) {
//           currentState = STATE_IDLE;
//           currentRequestId = "";
//           machineCode = "";
//           location = "";
//           updateOLED("SUCCESS", "User confirmed", "Pls click BUTTON to continue", "");
//         }
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

//     // Lúc này đã biết vấn đề, đã hiển thị trên màn hình, lúc này nhấn nút để thông báo lên line là đã tiếp nhận
//     case STATE_REQUEST_RECEIVED:
//       sendPostRequest("/acknowledge", STATE_ACKNOWLEDGED, "ON THE WAY", "To: " + location);
//       break;

//     case STATE_ACKNOWLEDGED:
//       // sendPostRequest("/arrive", STATE_ARRIVED, "ARRIVED", "Fixing: " + machineCode);
//       break;
//   }
// }

// // bool sendPostRequest(String endpoint, DeviceState nextState, String displayTitle, String displayMsg) {
// //   if (WiFi.status() != WL_CONNECTED) return false;

// //   http.begin(client1, String(serverUrl) + endpoint);  // Đã thêm client1
// //   http.setTimeout(10000 * 6);
// //   http.addHeader("Content-Type", "application/json");

// //   DynamicJsonDocument doc(256);
// //   doc["currentRequestId"] = currentRequestId;

// //   String jsonString;
// //   serializeJson(doc, jsonString);

// //   int httpCode = http.POST(jsonString);
// //   bool success = (httpCode == 200);

// //   if (success) {
// //     currentState = nextState;
// //     updateOLED(displayTitle, displayMsg, "Success!", "");

// //     tone(BUZZER_PIN, 1500, 200);  // Kêu bíp báo hiệu thành công
// //   } else {
// //     updateOLED("ERR: " + String(httpCode), "Post Failed", "Try Again", "");
// //   }

// //   http.end();
// //   return success;
// // }

// bool sendPostRequest(String endpoint, DeviceState nextState, String displayTitle, String displayMsg) {
//   if (WiFi.status() != WL_CONNECTED) return false;

//   http.begin(client1, String(serverUrl) + endpoint);  // Đã thêm client1
//   http.setTimeout(10000 * 6);
//   http.addHeader("Content-Type", "application/json");

//   // 1. Đóng gói JSON gửi đi
//   DynamicJsonDocument reqDoc(256);
//   reqDoc["currentRequestId"] = currentRequestId;

//   String jsonString;
//   serializeJson(reqDoc, jsonString);

//   // 2. Gửi POST request
//   int httpCode = http.POST(jsonString);
//   bool success = false; // Mặc định là thất bại

//   if (httpCode == 200) {
//     // 3. Đọc payload (chuỗi JSON) từ server trả về
//     String payload = http.getString();
//     Serial.println("Response Payload: " + payload);

//     // 4. Parse JSON
//     DynamicJsonDocument resDoc(512); 
//     DeserializationError error = deserializeJson(resDoc, payload);

//     if (!error) {
//       // 5. Kiểm tra an toàn xem có tồn tại etc và isSuccess không
//       if (resDoc["etc"].containsKey("isSuccess") && !resDoc["etc"]["isSuccess"].isNull()) {
//         // Lấy giá trị boolean
//         success = resDoc["etc"]["isSuccess"].as<bool>();
//       } else {
//         Serial.println("DEBUG: JSON trả về thiếu trường etc.isSuccess");
//       }
//     } else {
//       Serial.println("DEBUG: Parse JSON thất bại - " + String(error.c_str()));
//     }
//   }

//   // 6. Cập nhật giao diện dựa trên kết quả cuối cùng
//   if (success) {
//     currentState = nextState;
//     updateOLED(displayTitle, displayMsg, "Success!", "");
//     tone(BUZZER_PIN, 1500, 200);  // Kêu bíp báo hiệu thành công
//   } else {
//     // Thất bại có thể do httpCode != 200, hoặc isSuccess = false
//     updateOLED("ERR: " + String(httpCode), "Action Failed", "Try Again", "");
//   }

//   http.end();
//   return success;
// }

// void ringBuzzer() {
//   for (int i = 0; i < 1; i++) {
//     tone(BUZZER_PIN, 1500, 200);
//     delay(500);
//   }
// }

// void updateLED() {
//   switch (currentState) {
//     case STATE_IDLE: digitalWrite(LED_PIN, HIGH); break;  // ESP8266 LED tích hợp thường ngược (HIGH là tắt, LOW là sáng)
//     case STATE_REQUEST_RECEIVED: digitalWrite(LED_PIN, (millis() / 200) % 2 == 0 ? LOW : HIGH); break;
//     case STATE_ACKNOWLEDGED:
//       digitalWrite(LED_PIN, (millis() / 500) % 2 == 0 ? LOW : HIGH);
//       break;
//   }
// }
// =========== ENGINEER ESP ============

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <SSD1306Wire.h>

// --- OTA ---
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#define ELEGANTOTA_USE_ASYNC_WEBSERVER 1
#include <ElegantOTA.h>

// --- Cấu hình Pin ---
// #define BUZZER_PIN 13  // Mr.Huynh ESP
#define BUZZER_PIN 14  // Mr.Cuong ESP
#define LED_PIN 2      // D4
#define BUTTON_PIN 0   // D3

// --- Cấu hình Mạng & Server ---
const char* ssid = "HSVINA";
const char* password = "HSVINA@kor";

#define IS_PRODUCTION 1 
#if IS_PRODUCTION == 1
  const String SERVER_BASE_URL = "http://10.101.1.193:8080/espRecieve"; // HTTPS
#else
  const String SERVER_BASE_URL = "http://10.20.13.50:8080/espRecieve"; // HTTP Local
#endif
// --- Cấu hình OLED ---
// SSD1306Wire display(0x3c, 14, 12); // Mr.Huynh ESP
SSD1306Wire display(0x3c, 5, 4);  // Mr.Cuong ESP
#define flipDisplay true
#define LABEL_X 0
#define VALUE_X 5
#define LINE_1_Y 22
#define LINE_2_Y 36
#define LINE_3_Y 50
const int MAX_WIDTH_ALLOW = 128 - VALUE_X;
const int SCROLL_SPEED = 50;

// --- Data Structures ---
struct EngineerInfo {
  String id = "";
  String espId = "";
  String espMac = "";
  String name = "";
};

struct TicketInfo {
  String id = "";
  String machineCode = "";
  String machineName = "";
  String location = "";
  String description = "";
  String status = "";
};

// --- Global Variables ---
AsyncWebServer server(80);
WiFiClient wifiClient;
EngineerInfo currentEng;
TicketInfo currentTicket;
String macAddress = "";

enum DeviceState {
  STATE_IDLE,
  STATE_REQUEST_RECEIVED,
  STATE_ACKNOWLEDGED
};
DeviceState currentState = STATE_IDLE;

// Timer
unsigned long lastCheckTime = 0;
const unsigned long CHECK_INTERVAL = 3000;
unsigned long displayRestoreTime = 0;

unsigned long lastFetchTimeInfo = 0;
const unsigned long FETCH_INTERVAL_INFO = 30 * 1000;  // 30s

unsigned long lastWiFiCheckTime = 0;
const unsigned long WIFI_CHECK_INTERVAL = 10 * 1000; // Kiểm tra mỗi 10 giây nếu mất mạng

// OLED Scroll
String oledTitle, oledLine1, oledLine2, oledLine3;
int scrollX = 0;
unsigned long lastScrollTime = 0;
int waitTimer = 500;
char spaces[32];

// ==========================================
// HÀM HIỂN THỊ OLED
// ==========================================
void renderDisplay() {
  display.clear();

  display.setFont(ArialMT_Plain_16);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(64, 0, oledTitle);
  display.drawLine(0, 18, 127, 18);

  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);

  int w1 = display.getStringWidth(oledLine1);
  int w2 = display.getStringWidth(oledLine2);
  int w3 = display.getStringWidth(oledLine3);

  display.drawString(VALUE_X - (w1 > MAX_WIDTH_ALLOW ? scrollX : 0), LINE_1_Y, oledLine1);
  display.drawString(VALUE_X - (w2 > MAX_WIDTH_ALLOW ? scrollX : 0), LINE_2_Y, oledLine2);
  display.drawString(VALUE_X - (w3 > MAX_WIDTH_ALLOW ? scrollX : 0), LINE_3_Y, oledLine3);

  display.setColor(BLACK);
  display.fillRect(0, 19, VALUE_X - 1, 45);
  display.setColor(WHITE);

  display.drawString(LABEL_X, LINE_1_Y, "|");
  display.drawString(LABEL_X, LINE_2_Y, "|");
  display.drawString(LABEL_X, LINE_3_Y, "|");

  display.display();
}

void updateOLED(String title, String line1, String line2, String line3) {
  oledTitle = title;
  oledLine1 = line1;
  oledLine2 = line2;
  oledLine3 = line3;
  scrollX = 0;
  waitTimer = 1500;
  renderDisplay();
}

void handleScrolling() {
  if (displayRestoreTime > 0 && millis() > displayRestoreTime) {
    displayRestoreTime = 0;
    updateOLED("ENGINEER INFO", "MAC - IP:" + macAddress + " - " + WiFi.localIP().toString() + spaces, "Engineer: " + currentEng.name + spaces, "");
  }

  display.setFont(ArialMT_Plain_10);
  int maxW = max({ display.getStringWidth(oledLine1), display.getStringWidth(oledLine2), display.getStringWidth(oledLine3) });

  if (maxW > MAX_WIDTH_ALLOW && millis() - lastScrollTime >= SCROLL_SPEED) {
    lastScrollTime = millis();
    if (waitTimer > 0) {
      waitTimer -= SCROLL_SPEED;
    } else {
      scrollX++;
      if (scrollX > maxW - MAX_WIDTH_ALLOW + 30) {
        scrollX = 0;
        waitTimer = 1500;
      }
      renderDisplay();
    }
  }
}

// ==========================================
// HÀM TIỆN ÍCH KHÁC
// ==========================================
void ringBuzzer() {
  for (int i = 0; i < 3; i++) {
    tone(BUZZER_PIN, 1500, 200);
    delay(500);
  }
}

void updateLED() {
  switch (currentState) {
    case STATE_IDLE:
      digitalWrite(LED_PIN, HIGH);
      break;
    case STATE_REQUEST_RECEIVED:
      digitalWrite(LED_PIN, (millis() / 200) % 2 == 0 ? LOW : HIGH);
      break;
    case STATE_ACKNOWLEDGED:
      digitalWrite(LED_PIN, (millis() / 500) % 2 == 0 ? LOW : HIGH);
      break;
  }
}

void connectWiFi() {
  Serial.println("Connecting to WiFi...");

  WiFi.mode(WIFI_STA); 
  WiFi.setAutoReconnect(true); 
  WiFi.persistent(true);

  WiFi.begin(ssid, password);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi Connected! IP: " + WiFi.localIP().toString());
  } else {
    updateOLED("ERROR", "WiFi Failed", "Check Router", "");
  }
}

void addSpaces(char* buffer, int n) {
  // Tạo chuỗi space trực tiếp vào buffer có sẵn
  for (int i = 0; i < n && i < 255; i++) {
    buffer[i] = ' ';
  }
  buffer[n] = '\0';
}

// ==========================================
// HTTP HELPER (HỖ TRỢ CẢ HTTP VÀ HTTPS)
// ==========================================
String sendHttpRequest(String url, String method, String payload, int& httpCode) {
  HTTPClient http;
  
  // 1. Tự động kiểm tra URL là HTTP hay HTTPS
  if (url.startsWith("https://")) {
    WiFiClientSecure secureClient;
    secureClient.setInsecure(); // Bỏ qua xác thực chứng chỉ (Giúp ESP không bị lỗi khi SSL hết hạn)
    http.begin(secureClient, url);
  } else {
    WiFiClient client;
    http.begin(client, url);
  }

  http.setTimeout(1000 * 60);  // 60 giây timeout

  // 2. Gắn header và gọi API
  if (method == "POST") {
    http.addHeader("Content-Type", "application/json");
    httpCode = http.POST(payload);
  } else {
    httpCode = http.GET();
  }

  // 3. Đọc dữ liệu trả về
  String response = "";
  if (httpCode > 0) {
    response = http.getString();
  } else {
    Serial.println("HTTP Helper Error Code: " + String(httpCode) + " - URL: " + url);
  }
  
  http.end();
  return response;
}

// ==========================================
// API GỌI SERVER
// ==========================================
void fetchEngineerInfo(bool isBackground = false) {
  if (WiFi.status() != WL_CONNECTED) return;

  macAddress = WiFi.macAddress();

  // Chỉ hiển thị chữ LOADING... nếu không phải là tiến trình chạy ngầm
  if (!isBackground) {
    updateOLED("LOADING...", "MAC: " + macAddress, "Fetching Data", "Please wait...");
  }

  int httpCode;
  String url = SERVER_BASE_URL + "/get-engineer-info-by?mac=" + macAddress;
  String response = sendHttpRequest(url, "GET", "", httpCode);

  if (httpCode == 200) {
    DynamicJsonDocument doc(1024);
    if (!deserializeJson(doc, response) && doc["etc"].containsKey("lineInfo") && !doc["etc"]["lineInfo"].isNull()) {
      JsonObject info = doc["etc"]["lineInfo"];

      currentEng.id = info["eng_id"].as<String>();
      currentEng.espId = info["engineer_esp_id"].as<String>();
      currentEng.espMac = info["engineer_esp_mac"].as<String>();
      currentEng.name = info["eng_name"].as<String>();


      // Chỉ cập nhật OLED nếu mạch đang rảnh rỗi
      if (currentState == STATE_IDLE) {
        String infoId = info["id"].as<String>();

        String line1 = "MAC - IP:" + macAddress + " - " + WiFi.localIP().toString() + spaces;
        String line2 = "Engineer: " + currentEng.name + spaces;

        String line3 = "";
        if (info["id"].isNull() || infoId == "null" || infoId == "") {
          line3 = "This engineer hasn't taken on any line yet." + String(spaces);
        }
        updateOLED("ENGINEER INFO", line1, line2, line3);
      }
    } else {
      if (currentState == STATE_IDLE) {
        updateOLED("UNREGISTERED", "MAC Not Found!", "MAC - IP:" + macAddress + " - " + WiFi.localIP().toString() + spaces, "Contact Admin");
      }
    }
  } else {
    if (currentState == STATE_IDLE) {
      updateOLED("SERVER ERR", "HTTP Code: " + String(httpCode), "Check Server API", "");
    }
  }
}

void checkPendingRequests() {
  if (WiFi.status() != WL_CONNECTED || currentEng.id == "") return;

  int httpCode;
  String url = SERVER_BASE_URL + "/checkPendingRequest/" + currentEng.id;
  String response = sendHttpRequest(url, "GET", "", httpCode);

  if (httpCode == 200) {
    DynamicJsonDocument doc(1024);
    if (!deserializeJson(doc, response) && doc.containsKey("etc") && !doc["etc"].isNull()) {
      JsonObject etc = doc["etc"];

      if (etc["hasRequest"].as<bool>() == true && etc.containsKey("data") && !etc["data"].isNull()) {
        JsonObject data = etc["data"];

        currentTicket.id = data["id"].as<String>();
        currentTicket.machineCode = data["line_id"].as<String>();
        currentTicket.machineName = data["line_esp_name"].as<String>();
        currentTicket.location = data["location"].as<String>();
        currentTicket.description = data["issueDescription"].as<String>();
        currentTicket.status = data["status"].as<String>();

        if (currentTicket.status == "REQUESTED" && currentState != STATE_REQUEST_RECEIVED) {
          currentState = STATE_REQUEST_RECEIVED;
          updateOLED("E.NEW TASK", currentTicket.machineName, currentTicket.location, "Click click button start fixing!");
          ringBuzzer();
        } else if (currentTicket.status == "ACKNOWLEDGED" && currentState != STATE_ACKNOWLEDGED) {
          currentState = STATE_ACKNOWLEDGED;
          updateOLED("E.ON THE WAY", "To: " + currentTicket.location, "Fixing: " + currentTicket.machineCode, "Pls Ask User Confirm When you done");
        }
      } else {
        // Reset về IDLE nếu không còn request nào (hoặc đã được xác nhận hoàn thành từ phía Line)
        if (currentState == STATE_ACKNOWLEDGED) {
          currentState = STATE_IDLE;
          currentTicket = TicketInfo();  // Xóa trắng dữ liệu ticket
          updateOLED("E.SUCCESS", "User confirmed", "After 4s will show screen info", "");
          displayRestoreTime = millis() + 4000;
        }
      }
    }
  }
}

bool sendPostRequest(String endpoint, DeviceState nextState) {
  if (WiFi.status() != WL_CONNECTED) return false;

  DynamicJsonDocument reqDoc(256);
  reqDoc["currentRequestId"] = currentTicket.id;
  String payload;
  serializeJson(reqDoc, payload);

  int httpCode;
  String response = sendHttpRequest(SERVER_BASE_URL + endpoint, "POST", payload, httpCode);

  bool isSuccess = false;
  if (httpCode == 200) {
    DynamicJsonDocument resDoc(512);
    if (!deserializeJson(resDoc, response) && resDoc.containsKey("etc") && !resDoc["etc"].isNull()) {
      if (resDoc["etc"].containsKey("isSuccess")) {
        isSuccess = resDoc["etc"]["isSuccess"].as<bool>();
      }
    }
  }

  if (isSuccess) {
    currentState = nextState;
    // updateOLED(displayTitle, displayMsg, "Success!", "");
    updateOLED("E.ON THE WAY", "To: " + currentTicket.location, "Fixing: " + currentTicket.machineCode, "Pls Ask User Confirm When you done");
    ringBuzzer();
  } else {
    updateOLED("ERR: " + String(httpCode), "Action Failed", "Try Again", "");
  }

  return isSuccess;
}

// ==========================================
// VÒNG LẶP & XỬ LÝ SỰ KIỆN
// ==========================================
void handleButtonPress() {
  switch (currentState) {
    case STATE_IDLE:
      updateOLED("ENGINEER INFO", "MAC - IP:" + macAddress + " - " + WiFi.localIP().toString() + spaces, "Engineer: " + currentEng.name + spaces, "");
      break;

    case STATE_REQUEST_RECEIVED:
      if (sendPostRequest("/acknowledge", STATE_ACKNOWLEDGED)) {
      }
      break;

    case STATE_ACKNOWLEDGED:
      // Sẵn sàng cho tính năng Arrive nếu sau này mở ra
      break;
  }
}

void setup() {
  Serial.begin(115200);

  // Khởi tạo phần cứng
  display.init();
  if (flipDisplay) display.flipScreenVertically();
  display.clear();

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  updateOLED("SYSTEM", "Starting...", "Device Init", "");
  connectWiFi();

  if (WiFi.status() == WL_CONNECTED) {
    fetchEngineerInfo();
  }

  // Khởi tạo OTA
  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(200, "text/plain", "Hi! I am Receiver ESP8266.");
  });

  server.on("/reset", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(200, "text/html", "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Resetting...</title><meta http-equiv='refresh' content='3; url=/'></head><body><h1>🔄 Đang reset thiết bị...</h1><p>ESP8266 sẽ khởi động lại sau 2 giây.</p></body></html>");
    delay(2000);
    ESP.restart();
  });
  
  ElegantOTA.begin(&server);
  server.begin();
}

void loop() {
  ElegantOTA.loop();

  // --- KIỂM TRA WIFI VÀO ĐÂY ---
  if (WiFi.status() != WL_CONNECTED) {
    if (millis() - lastWiFiCheckTime >= WIFI_CHECK_INTERVAL) {
      lastWiFiCheckTime = millis();
      Serial.println("WiFi dropped. Reconnecting...");
      
      // Báo hiệu lên màn hình
      updateOLED("WIFI LOST", "Connection dropped", "Reconnecting...", "");
      
      WiFi.reconnect(); // Chủ động yêu cầu chip ESP quét và kết nối lại
    }
    // Nếu mất WiFi, kết thúc sớm vòng lặp loop để tránh chạy các tác vụ bên dưới gây lỗi
    yield();
    return; 
  }
  // --------------------------------------------

  handleScrolling();
  updateLED();

  // Thêm điều kiện tự động fetch lại thông tin Kỹ sư (chạy ngầm)
  if (currentState == STATE_IDLE && (millis() - lastFetchTimeInfo >= FETCH_INTERVAL_INFO)) {
    lastFetchTimeInfo = millis();
    if (displayRestoreTime == 0) {
      fetchEngineerInfo(true);
    }
  }

  if (millis() - lastCheckTime >= CHECK_INTERVAL) {
    lastCheckTime = millis();
    checkPendingRequests();
  }

  if (digitalRead(BUTTON_PIN) == LOW) {
    delay(50);  // Debounce
    if (digitalRead(BUTTON_PIN) == LOW) {
      handleButtonPress();
      // Thêm yield() vào vòng lặp chờ nhả nút để không làm treo AsyncWebServer
      while (digitalRead(BUTTON_PIN) == LOW) { yield(); }
    }
  }

  delay(10);
}
