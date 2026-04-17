// #include <ESP8266WiFi.h>
// #include <ESP8266HTTPClient.h>
// #include <ArduinoJson.h>
// #include <Wire.h>
// #include <SSD1306Wire.h>

// // --- Cấu hình Pin (Dành cho ESP8266 mạch HW-364B) ---
// #define BUZZER_PIN 13  // GPIO 13 (Tương ứng chân D7) dùng cho còi Buzzer
// #define LED_PIN 2      // GPIO 2 (Tương ứng D4) - Đèn LED màu xanh dương tích hợp trên mạch ESP8266
// #define BUTTON_PIN 0   // GPIO 0 (Tương ứng D3) - Nút FLASH tích hợp sẵn trên mạch dùng làm nút bấm

// // --- Quản lý trạng thái ---
// enum DeviceState {
//   STATE_NONE,
//   STATE_CALLED_ENGINEER,
//   STATE_WAITING_ACCEPT,  // Chờ Bạn Xác nhận kĩ sư đã sửa xong
// };

// enum MaintenanceState {
//   STATE_IDLE,
//   STATE_REQUEST_RECEIVED,
//   STATE_ACKNOWLEDGED,
//   STATE_ARRIVED
// };

// // --- Cấu hình OLED mạch HW-364B ---
// // Chân SDA ngầm = 14 (D5), SCL ngầm = 12 (D6)
// SSD1306Wire display(0x3c, 14, 12);
// #define flipDisplay true

// // --- Cấu hình WiFi ---
// const char* ssid = "HSVINA";
// const char* password = "HSVINA@kor";
// // const char* ssid = "SA";
// // const char* password = "1234567890";
// WiFiClient client1;
// HTTPClient http;

// DeviceState currentState = STATE_NONE;
// MaintenanceState currentMaintenanceState = STATE_IDLE;
// unsigned long lastCheckTime = 0;
// const unsigned long checkInterval = 3000;

// // --- CẤU HÌNH LAYOUT OLED ---
// #define LABEL_X 0    // Tọa độ X của các nhãn (|)
// #define VALUE_X 5    // Tọa độ X của nội dung
// #define LINE_1_Y 22  // Tọa độ Y Dòng 1
// #define LINE_2_Y 36  // Tọa độ Y Dòng 2
// #define LINE_3_Y 50  // Tọa độ Y Dòng 3

// // --- Biến hỗ trợ chữ chạy (Scrolling Text) ---
// String oledTitle = "";
// String oledLine1 = "";
// String oledLine2 = "";
// String oledLine3 = "";

// int scrollX = 0;
// unsigned long lastScrollTime = 0;
// const int SCROLL_SPEED = 40;
// int waitTimer = 1500;
// const int MAX_WIDTH_ALLOW = 128 - VALUE_X;

// // --- CÁC HÀM XỬ LÝ GIAO DIỆN OLED ---
// void renderDisplay() {
//   display.clear();

//   // 1. Tiêu đề
//   display.setFont(ArialMT_Plain_16);
//   display.setTextAlignment(TEXT_ALIGN_CENTER);
//   display.drawString(64, 0, oledTitle);
//   display.drawLine(0, 18, 127, 18);

//   // 2. Nội dung chi tiết
//   display.setFont(ArialMT_Plain_10);
//   display.setTextAlignment(TEXT_ALIGN_LEFT);

//   int w1 = display.getStringWidth(oledLine1);
//   int w2 = display.getStringWidth(oledLine2);
//   int w3 = display.getStringWidth(oledLine3);

//   display.drawString(VALUE_X - (w1 > MAX_WIDTH_ALLOW ? scrollX : 0), LINE_1_Y, oledLine1);
//   display.drawString(VALUE_X - (w2 > MAX_WIDTH_ALLOW ? scrollX : 0), LINE_2_Y, oledLine2);
//   display.drawString(VALUE_X - (w3 > MAX_WIDTH_ALLOW ? scrollX : 0), LINE_3_Y, oledLine3);

//   // 3. Che phần chữ bị lẹm
//   display.setColor(BLACK);
//   display.fillRect(0, 19, VALUE_X - 1, 45);
//   display.setColor(WHITE);

//   // 4. In nhãn
//   display.drawString(LABEL_X, LINE_1_Y, "|");
//   display.drawString(LABEL_X, LINE_2_Y, "|");
//   display.drawString(LABEL_X, LINE_3_Y, "|");

//   display.display();
// }

// void handleScrolling() {
//   display.setFont(ArialMT_Plain_10);
//   int maxW = max(display.getStringWidth(oledLine1),
//                  max(display.getStringWidth(oledLine2),
//                      display.getStringWidth(oledLine3)));

//   if (maxW > MAX_WIDTH_ALLOW) {
//     if (millis() - lastScrollTime >= SCROLL_SPEED) {
//       lastScrollTime = millis();
//       if (waitTimer > 0) {
//         waitTimer -= SCROLL_SPEED;
//       } else {
//         scrollX++;
//         if (scrollX > maxW - MAX_WIDTH_ALLOW + 20) {
//           scrollX = 0;
//           waitTimer = 1500;
//         }
//         renderDisplay();
//       }
//     }
//   }
// }

// void updateOLED(String title, String line1, String line2, String line3) {
//   oledTitle = title;
//   oledLine1 = line1;
//   oledLine2 = line2;
//   oledLine3 = line3;
//   scrollX = 0;
//   waitTimer = 1500;
//   renderDisplay();
// }

// // ---- HÀM RUNG CHUÔNG ----
// void ringBuzzer() {
//   for (int i = 0; i < 3; i++) {
//     tone(BUZZER_PIN, 2000, 400);
//     delay(500);
//   }
// }

// // --- HÀM KẾT NỐI WIFI ---
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

// // --- CÁC BIẾN TOÀN CỤC ---
// String macAddress = "";
// String lineId = "";
// String lineName = "";
// String espName = "";
// String espEngineerMac = "";
// String espEngineerName = "";


// // Dữ liệu Check Request
// String currentRequestId = "";

// String engineer_esp_id = "";
// String engineer_esp_mac = "";
// String engineer_esp_name = "";
// String eng_id = "";

// String line_esp_id = "";
// String line_esp_mac = "";
// String line_esp_name = "";
// String line_id = "";

// String location = "";
// String issueDescription = "";
// String requestStatus = "";

// // --- HÀM GỌI API LẤY THÔNG TIN LINE THEO MAC ---
// const char* serverUrl = "http://10.20.13.50:8080/espCall/get-line-info-by";
// void fetchLineInfo() {
//   if (WiFi.status() != WL_CONNECTED) return;

//   macAddress = WiFi.macAddress();
//   Serial.println("MAC Address: " + macAddress);
//   updateOLED("LOADING...", "MAC: " + macAddress, "Fetching Line Data", "Please wait...");
//   String url = String(serverUrl) + "?mac=" + macAddress;

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

//         updateOLED("LINE INFO", "ID: " + lineId, "Line: " + lineName, "Eng: " + espEngineerName);
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



// const char* requestApiUrlGetCurrent = "http://10.20.13.50:8080/espCall";
// void checkPendingRequests() {
//   if (WiFi.status() != WL_CONNECTED) return;

//   String url = String(requestApiUrlGetCurrent) + "/checkPendingRequest/" + String(lineId);
//   http.begin(client1, url);
//   int httpCode = http.GET();

//   if (httpCode == 200) {
//     String payload = http.getString();
//     DynamicJsonDocument doc(1024);
//     DeserializationError error = deserializeJson(doc, payload);

//     if (!error) {
//       // Kiểm tra node 'etc' có tồn tại không
//       if (doc.containsKey("etc") && !doc["etc"].isNull()) {
//         JsonObject etc = doc["etc"];

//         // Kiểm tra xem có Request nào đang chờ không
//         if (etc["hasRequest"].as<bool>() == true) {

//           // Trỏ vào object ticketInfo
//           if (etc.containsKey("ticketInfo") && !etc["ticketInfo"].isNull()) {
//             JsonObject ticketInfo = etc["ticketInfo"];

//             currentRequestId = ticketInfo["id"].as<String>();

//             engineer_esp_id = ticketInfo["engineer_esp_id"].as<String>();
//             engineer_esp_mac = ticketInfo["engineer_esp_mac"].as<String>();
//             engineer_esp_name = ticketInfo["engineer_esp_name"].as<String>();
//             eng_id = ticketInfo["eng_id"].as<String>();

//             line_esp_id = ticketInfo["line_esp_id"].as<String>();
//             line_esp_mac = ticketInfo["line_esp_mac"].as<String>();
//             line_esp_name = ticketInfo["line_esp_name"].as<String>();
//             line_id = ticketInfo["line_id"].as<String>();

//             // Xử lý an toàn cho location vì JSON có thể trả về null
//             if (!ticketInfo["location"].isNull()) {
//               location = ticketInfo["location"].as<String>();
//             } else {
//               location = "N/A";
//             }

//             requestStatus = ticketInfo["status"].as<String>();
//             issueDescription = "";

//             // --- XỬ LÝ LOGIC TRẠNG THÁI ---
//             if (requestStatus == "REQUESTED") {
//               Serial.println("DEBUG: Đã nhận được yêu cầu mới!");
//               currentMaintenanceState = STATE_REQUEST_RECEIVED;
//               currentState = STATE_CALLED_ENGINEER;
//               updateOLED("LINE ESP", "Already Call Engineer", "", "");
//               ringBuzzer();
//             }

//             if (requestStatus == "ACKNOWLEDGED") {
//               currentMaintenanceState = STATE_ACKNOWLEDGED;
//               currentState = STATE_WAITING_ACCEPT;
//               updateOLED("LINE ESP", "Engineer Acknowledged", "Pls Click Button When Your Machine Fixed", "");
//             }
//           }
//         }
//       }
//     } else {
//       Serial.println("JSON Parse Error in checkPendingRequests: " + String(error.c_str()));
//     }
//   } else {
//     Serial.println("HTTP Error: " + String(httpCode));
//   }
//   http.end();
// }

// void setup() {
//   Serial.begin(115200);

//   // 1. Khởi tạo OLED
//   display.init();
//   if (flipDisplay) display.flipScreenVertically();
//   display.clear();
//   updateOLED("SYSTEM", "Starting...", "Initializing...", "");

//   // Cấu hình chân IO
//   pinMode(BUZZER_PIN, OUTPUT);
//   pinMode(LED_PIN, OUTPUT);
//   pinMode(BUTTON_PIN, INPUT_PULLUP);

//   // 2. Kết nối WiFi
//   connectWiFi();

//   // 3. Gọi API
//   if (WiFi.status() == WL_CONNECTED) {
//     fetchLineInfo();
//   }
// }

// const char* urlApiRequestCom = "http://10.20.13.50:8080/espCall";
// bool sendPostRequestComplete(DeviceState nextState, String displayTitle, String displayMsg) {
//   if (WiFi.status() != WL_CONNECTED) return false;
//   Serial.println("Call api..." + currentRequestId + " " + espEngineerMac);
//   http.begin(client1, String(urlApiRequestCom) + "/complete");
//   http.setTimeout(10000 * 6);
//   http.addHeader("Content-Type", "application/json");

//   DynamicJsonDocument doc(256);
//   doc["currentRequestId"] = currentRequestId;

//   String jsonString;
//   serializeJson(doc, jsonString);

//   int httpCode = http.POST(jsonString);
//   bool success = (httpCode == 200);

//   if (doc["etc"]["isSuccess"] == true) {
//     currentState = nextState;
//     updateOLED(displayTitle, displayMsg, "Success!", "");
//     tone(BUZZER_PIN, 1500, 200);
//   } else {
//     updateOLED("ERR: " + String(httpCode), "Post Failed", "Try Again", "");
//   }

//   // if (success) {
//   //   currentState = nextState;
//   //   updateOLED(displayTitle, displayMsg, "Success!", "");
//   //   tone(BUZZER_PIN, 1500, 200);
//   // } else {
//   //   updateOLED("ERR: " + String(httpCode), "Post Failed", "Try Again", "");
//   // }

//   http.end();
//   return success;
// }

// const char* requestApiUrl = "http://10.20.13.50:8080/espCall/request";
// void callMaintenanceEngineer() {
//   if (WiFi.status() != WL_CONNECTED) {
//     updateOLED("WIFI ERR", "No Connection", "Cannot call", "");
//     return;
//   }

//   updateOLED("CALLING...", "Sending request", "Please wait", "");
//   Serial.println("Đang gửi yêu cầu gọi kĩ sư...");

//   http.begin(client1, requestApiUrl);
//   http.addHeader("Content-Type", "application/json");

//   DynamicJsonDocument doc(512);
//   doc["location"] = lineId + " " + lineName;
//   // doc["deviceId"] = espEngineerMac;
//   doc["issueDescription"] = "Call Error By ESP " + espName;

//   doc["engineerEspId"] = engineer_esp_id;
//   doc["engineerEspMac"] = engineer_esp_mac;
//   doc["engineerEspName"] = engineer_esp_name;
//   doc["engineerId"] = eng_id;

//   doc["lineEspId"] = line_esp_id;
//   doc["lineEspMac"] = line_esp_mac;
//   doc["lineEspName"] = line_esp_name;  //???doc["line_esp_name"] = lineName;
//   doc["lineId"] = line_id;             //???doc["line_id"] = lineId;

//   String payload;
//   serializeJson(doc, payload);
//   Serial.println("Payload gửi đi: " + payload);

//   int httpCode = http.POST(payload);

//   if (httpCode > 0) {
//     String response = http.getString();
//     Serial.println("Server phản hồi: " + response);

//     DynamicJsonDocument respDoc(1024);
//     DeserializationError error = deserializeJson(respDoc, response);
//     if (!error) {
//       bool isSuccess = respDoc["etc"]["isSuccess"];
//       if (isSuccess) {
//         bool ticketStatus = respDoc["etc"]["isInQueue"];
//         if (ticketStatus == true) {
//           updateOLED("IN QUEUE", "Da ghi nhan", "Dang cho xep hang", "Vui long doi...");
//           ringBuzzer();
//         } else {
//           updateOLED("SUCCESS", "Ki su dang toi", "Yeu cau thanh cong!", "");
//           ringBuzzer();
//         }
//       } else {
//         updateOLED("FAILED", "Server bao loi", "Vui long thu lai", "");
//       }
//     } else {
//       updateOLED("PARSE ERR", "Loi doc JSON", "Tu server", "");
//     }
//   } else {
//     updateOLED("HTTP ERR", "Khong the ket noi", http.errorToString(httpCode).c_str(), "");
//   }
//   http.end();
// }

// void handleButtonPress() {
//   Serial.println("Vo ham handleButtonPress..." + String(currentState));
//   switch (currentState) {
//     case STATE_NONE:
//       updateOLED("INFO", "No Task", "-", "System Active");
//       callMaintenanceEngineer();
//       break;
//     case STATE_WAITING_ACCEPT:
//       Serial.println("Nhan nut xac nhan...");
//       if (sendPostRequestComplete(STATE_NONE, "COMPLETED", "Waiting task")) {
//         currentRequestId = "";
//         line_id = "";
//         line_esp_name = "";
//         location = "";
//         issueDescription = "";
//         requestStatus = "";

//         currentState = STATE_NONE;
//         currentMaintenanceState = STATE_IDLE;

//         updateOLED("LINE ESP", "You Confirmed Engineer Fixed", "", "");
//         delay(8000);
//         updateOLED("LINE INFO", "ID: " + lineId, "Name: " + lineName, "ESP: " + espName);
//       }
//       break;
//   }
// }

// void loop() {
//   handleScrolling();

//   if (millis() - lastCheckTime >= checkInterval) {
//     lastCheckTime = millis();
//     checkPendingRequests();
//   }

//   // Xử lý nút bấm (Nút BOOT - GPIO0)
//   if (digitalRead(BUTTON_PIN) == LOW) {
//     delay(50);
//     if (digitalRead(BUTTON_PIN) == LOW) {
//       handleButtonPress();
//       while (digitalRead(BUTTON_PIN) == LOW) { delay(10); }
//     }
//   }

//   delay(10);  // Cần thiết trên ESP8266 để tránh reset Watchdog Timer
// }


// =========== LINE ESP ============

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <SSD1306Wire.h>

// OTA
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#define ELEGANTOTA_USE_ASYNC_WEBSERVER 1
#include <ElegantOTA.h>

// OTA define server
AsyncWebServer server(80);

// --- Cấu hình Pin ---
#define BUZZER_PIN 13  // D7 - Còi Buzzer
#define LED_PIN 2      // D4 - Đèn LED
#define BUTTON_PIN 0   // D3 - Nút FLASH

// --- Cấu hình Mạng & Server ---
const char* ssid = "HSVINA";
const char* password = "HSVINA@kor";
const String SERVER_BASE_URL = "http://10.20.13.50:8080/espCall";

// --- Cấu hình OLED ---
SSD1306Wire display(0x3c, 14, 12);  // Mr.Huynh ESP
// SSD1306Wire display(0x3c, 5, 4); // Mr.Cuong ESP
#define flipDisplay true
#define LABEL_X 0
#define VALUE_X 5
#define LINE_1_Y 22
#define LINE_2_Y 36
#define LINE_3_Y 50
const int MAX_WIDTH_ALLOW = 128 - VALUE_X;
const int SCROLL_SPEED = 40;

// --- Data Structures ---
struct LineInfo {
  String id = "";
  String name = "";
  String espCode = "";
  String espId = "";
  String espMac = "";

  String engEspId = "";
  String engEspMac = "";
  String engName = "";
  String engId = "";

  String location = "";
};

struct TicketInfo {
  String id = "";
  String location = "";
  String description = "";
  String status = "";
};

// --- Global Variables ---
WiFiClient wifiClient;
LineInfo currentLine;
TicketInfo currentTicket;
String macAddress = "";

enum DeviceState {
  STATE_NONE,
  STATE_IN_QUEUE,
  STATE_CALLED_ENGINEER,
  STATE_WAITING_ACCEPT,
};
DeviceState currentState = STATE_NONE;

// Biến điều khiển Timer
unsigned long lastCheckTime = 0;
const unsigned long CHECK_INTERVAL = 3000;
unsigned long displayRestoreTime = 0;  // Thay thế cho delay(8000)

unsigned long lastFetchTimeInfo = 0;
const unsigned long FETCH_INTERVAL_INFO = 30 * 1000;  // 30s

unsigned long lastWiFiCheckTime = 0;
const unsigned long WIFI_CHECK_INTERVAL = 10 * 1000; // Kiểm tra mỗi 10 giây nếu mất mạng

// Biến điều khiển OLED
String oledTitle, oledLine1, oledLine2, oledLine3;
int scrollX = 0;
unsigned long lastScrollTime = 0;
int waitTimer = 1500;
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
  // Nếu đang trong thời gian chờ phục hồi màn hình (sau khi bấm hoàn thành)
  if (displayRestoreTime > 0 && millis() > displayRestoreTime) {
    displayRestoreTime = 0;
    updateOLED("LINE INFO", "MAC - IP:" + macAddress + " - " + WiFi.localIP().toString() + spaces, "Line: " + currentLine.name, "Repair engineer: " + currentLine.engName + spaces);
  }

  display.setFont(ArialMT_Plain_10);
  int maxW = max({ display.getStringWidth(oledLine1), display.getStringWidth(oledLine2), display.getStringWidth(oledLine3) });

  if (maxW > MAX_WIDTH_ALLOW && millis() - lastScrollTime >= SCROLL_SPEED) {
    lastScrollTime = millis();
    if (waitTimer > 0) {
      waitTimer -= SCROLL_SPEED;
    } else {
      scrollX++;
      if (scrollX > maxW - MAX_WIDTH_ALLOW + 20) {
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
    tone(BUZZER_PIN, 2000, 400);
    delay(500);  // Lệnh delay nhỏ ở đây có thể chấp nhận được
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
    Serial.println("\nWiFi Connected!");
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
// HTTP HELPER
// ==========================================
String sendHttpRequest(String url, String method, String payload, int& httpCode) {
  HTTPClient http;
  http.begin(wifiClient, url);
  http.setTimeout(1000 * 60);  // 60 giây timeout

  if (method == "POST") {
    http.addHeader("Content-Type", "application/json");
    httpCode = http.POST(payload);
  } else {
    httpCode = http.GET();
  }

  String response = "";
  if (httpCode > 0) {
    response = http.getString();
  }
  http.end();
  return response;
}

// ==========================================
// API GỌI SERVER
// ==========================================
// Thêm tham số isBackground mặc định là false
void fetchLineInfo(bool isBackground = false) {
  if (WiFi.status() != WL_CONNECTED) return;

  macAddress = WiFi.macAddress();

  // Chỉ hiển thị chữ LOADING... nếu không phải là tiến trình chạy ngầm
  if (!isBackground) {
    updateOLED("LOADING...", "MAC: " + macAddress, "Fetching Line Data", "Please wait...");
  }

  int httpCode;
  String url = SERVER_BASE_URL + "/get-line-info-by?mac=" + macAddress;
  String response = sendHttpRequest(url, "GET", "", httpCode);

  if (httpCode == 200) {
    DynamicJsonDocument doc(1024);
    if (!deserializeJson(doc, response) && doc["etc"].containsKey("lineInfo") && !doc["etc"]["lineInfo"].isNull()) {
      JsonObject info = doc["etc"]["lineInfo"];

      currentLine.id = info["line_id"].as<String>();
      currentLine.name = info["line_name"].as<String>();
      currentLine.espCode = info["line_code"].as<String>();
      currentLine.espId = info["line_esp_id"].as<String>();
      currentLine.espMac = info["line_esp_mac"].as<String>();
      currentLine.location = info["location"].as<String>();

      currentLine.engId = info["eng_id"].as<String>();
      currentLine.engEspId = info["engineer_esp_id"].as<String>();
      currentLine.engEspMac = info["engineer_esp_mac"].as<String>();
      currentLine.engName = info["eng_name"].as<String>();

      // Chú ý: Chỉ update lại màn hình nếu mạch vẫn đang ở STATE_NONE
      // (đề phòng trường hợp lúc đang fetch ngầm thì có request nhảy vào làm thay đổi màn hình)
      if (currentState == STATE_NONE) {

        String line3 = "";
        if (currentLine.engId == "null" || currentLine.engId == "") {
          line3 = "This line hasn't been taken on by any engineer yet." + String(spaces);
        } else {
          line3 = "Repair engineer: " + currentLine.engName + String(spaces);
        }
        updateOLED("LINE INFO", "MAC - IP:" + macAddress + " - " + WiFi.localIP().toString() + spaces, "Line: " + currentLine.name, line3);
      }
    } else {
      if (currentState == STATE_NONE) {
        updateOLED("UNREGISTERED", "MAC Not Found!", "MAC - IP:" + macAddress + " - " + WiFi.localIP().toString() + spaces, "Contact Admin");
      }
    }
  } else {
    if (currentState == STATE_NONE) {
      updateOLED("SERVER ERR", "HTTP Code: " + String(httpCode), "Check Server API", "");
    }
  }
}

void checkPendingRequests() {
  if (WiFi.status() != WL_CONNECTED || currentLine.id == "") return;

  int httpCode;
  String url = SERVER_BASE_URL + "/checkPendingRequest/" + currentLine.id;
  String response = sendHttpRequest(url, "GET", "", httpCode);

  if (httpCode == 200) {
    DynamicJsonDocument doc(1024);
    if (!deserializeJson(doc, response) && doc.containsKey("etc") && !doc["etc"].isNull()) {
      JsonObject etc = doc["etc"];

      if (etc["hasRequest"].as<bool>() == true && etc.containsKey("ticketInfo") && !etc["ticketInfo"].isNull()) {
        JsonObject ticket = etc["ticketInfo"];

        currentTicket.id = ticket["id"].as<String>();
        currentTicket.status = ticket["status"].as<String>();
        currentTicket.location = ticket["location"].isNull() ? "N/A" : ticket["location"].as<String>();

        if (currentTicket.status == "NONE" && currentState != STATE_IN_QUEUE) {
          currentState = STATE_IN_QUEUE;
          updateOLED("L.IN QUEUE", "Already Call Engineer", "In Queue", "");
          // ringBuzzer();
        } else if (currentTicket.status == "REQUESTED" && currentState != STATE_CALLED_ENGINEER) {
          currentState = STATE_CALLED_ENGINEER;
          char message[64];
          sprintf(message, "Engineer will come here soon%s", spaces);
          updateOLED("L.CALLED", "Already Call Engineer", message, "");
          // ringBuzzer();
        } else if (currentTicket.status == "ACKNOWLEDGED" && currentState != STATE_WAITING_ACCEPT) {
          currentState = STATE_WAITING_ACCEPT;
          char message[64];
          sprintf(message, "Pls Click Button When Your Machine Fixed%s", spaces);
          updateOLED("L.FIXING", "Engineer Acknowledged", message, "");
        }
      } else {
        // Khong co ticket
        // [NOTE] Can check them
        if (currentState != STATE_NONE) {
          Serial.println("\ncurrentState != STATE_NONE run this pack");
          updateOLED("LINE INFO", "MAC - IP:" + macAddress + " - " + WiFi.localIP().toString() + spaces, "Line: " + currentLine.name, "Repair engineer: " + currentLine.engName + spaces);
        }
      }
    }
  }
}

void callMaintenanceEngineer() {
  if (WiFi.status() != WL_CONNECTED) {
    updateOLED("WIFI ERR", "No Connection", "Cannot call", "");
    return;
  }

  updateOLED("CALLING...", "Sending request", "Please wait", "");

  DynamicJsonDocument doc(512);
  doc["location"] = currentLine.id + " " + currentLine.name;
  doc["issueDescription"] = "Call Error By ESP " + currentLine.espCode;
  doc["engineerEspId"] = currentLine.engEspId;
  doc["engineerEspMac"] = currentLine.engEspMac;
  doc["engineerEspName"] = currentLine.engName;
  doc["engineerId"] = currentLine.engId;
  doc["lineEspId"] = currentLine.espId;
  doc["lineEspMac"] = currentLine.espMac;
  doc["lineEspName"] = currentLine.name;
  doc["lineId"] = currentLine.id;
  doc["lineLocation"] = currentLine.location;

  String payload;
  serializeJson(doc, payload);

  int httpCode;
  String response = sendHttpRequest(SERVER_BASE_URL + "/request", "POST", payload, httpCode);

  if (httpCode > 0) {
    DynamicJsonDocument respDoc(1024);
    if (!deserializeJson(respDoc, response)) {
      if (respDoc["etc"]["isSuccess"] == true) {
        if (respDoc["etc"]["isInQueue"] == true) {
          updateOLED("IN QUEUE", "Request received", "Waiting in queue", "Please wait...");
        } else {
          updateOLED("INCOMING", "Engineer is coming", "Request successful!", "");
        }
        ringBuzzer();
      } else {
        updateOLED("FAILED", "Server bao loi", "Vui long thu lai", "");
      }
    } else {
      updateOLED("PARSE ERR", "Loi doc JSON", "Tu server", "");
    }
  } else {
    updateOLED("HTTP ERR", "Khong the ket noi", "Loi mạng", "");
  }
}

bool sendPostRequestComplete(DeviceState nextState, String displayTitle, String displayMsg) {
  if (WiFi.status() != WL_CONNECTED) return false;

  DynamicJsonDocument doc(256);
  doc["currentRequestId"] = currentTicket.id;
  String payload;
  serializeJson(doc, payload);

  int httpCode;
  String response = sendHttpRequest(SERVER_BASE_URL + "/complete", "POST", payload, httpCode);

  bool isSuccess = false;
  if (httpCode == 200) {
    DynamicJsonDocument respDoc(256);
    if (!deserializeJson(respDoc, response)) {
      // Đã SỬA LỖI: Kiểm tra dữ liệu trả về từ server, không phải payload gửi đi
      if (respDoc["etc"]["isSuccess"] == true) {
        isSuccess = true;
      }
    }
  }

  if (isSuccess) {
    currentState = nextState;
    updateOLED(displayTitle, displayMsg, "Success!", "");
    tone(BUZZER_PIN, 1500, 200);
  } else {
    updateOLED("ERR: " + String(httpCode), "Post Failed", "Try Again", "");
  }

  return isSuccess;
}

// ==========================================
// VÒNG LẶP & XỬ LÝ SỰ KIỆN
// ==========================================
void handleButtonPress() {
  switch (currentState) {
    case STATE_NONE:
      if (currentLine.engId == "null" || currentLine.engId == "") {
        updateOLED("ERROR", "Action Failed", "No Engineer Assigned", "Contact Admin");
        tone(BUZZER_PIN, 500, 1000);
        displayRestoreTime = millis() + 3000;
        break;
      }

      // Xử lý gọi Kỹ sư bình thường
      if (displayRestoreTime == 0) {
        callMaintenanceEngineer();
      }
      break;
    case STATE_WAITING_ACCEPT:
      if (currentLine.engId == "null" || currentLine.engId == "") {
        updateOLED("ERROR", "Action Failed", "No Engineer Assigned", "Contact Admin");
        tone(BUZZER_PIN, 500, 1000);
        displayRestoreTime = millis() + 3000;
        break;
      }
      if (sendPostRequestComplete(STATE_NONE, "COMPLETED", "Waiting task")) {
        // Reset ticket
        currentTicket = TicketInfo();
        currentState = STATE_NONE;

        char message[64];
        sprintf(message, "After 4s will show screen info%s", spaces);
        updateOLED("LINE INFO", "You Confirmed Engineer Fixed", message, "");
        // Non-blocking thay cho delay(4000)
        displayRestoreTime = millis() + 4000;
      }
      break;

    default:
      break;
  }
}

void setup() {
  Serial.begin(115200);

  display.init();
  addSpaces(spaces, 10);
  if (flipDisplay) display.flipScreenVertically();
  display.clear();
  updateOLED("SYSTEM", "Starting...", "Initializing...", "");

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  connectWiFi();

  if (WiFi.status() == WL_CONNECTED) {
    fetchLineInfo();
  }

  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(200, "text/plain", "Hi! I am ESP8266.");
  });

  server.on("/reset", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(200, "text/html", "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Resetting...</title><meta http-equiv='refresh' content='3; url=/'></head><body><h1>🔄 Đang reset thiết bị...</h1><p>ESP8266 sẽ khởi động lại sau 2 giây.</p></body></html>");
    delay(2000);
    ESP.restart();
  });

  ElegantOTA.begin(&server);  // Start ElegantOTA
  server.begin();

  Serial.println("localIp:" + WiFi.localIP().toString());
}

void loop() {
  ElegantOTA.loop();  // OTA reload when new code

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

  // Tự động fetch lại thông tin sau mỗi khoảng thời gian (FETCH_INTERVAL)
  if (currentState == STATE_NONE && (millis() - lastFetchTimeInfo >= FETCH_INTERVAL_INFO)) {
    lastFetchTimeInfo = millis();
    if (displayRestoreTime == 0) {
      fetchLineInfo(true);  // Truyền true để chạy ngầm, không chớp màn hình Loading
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
      while (digitalRead(BUTTON_PIN) == LOW) { yield(); }  // Dùng yield() thay vì delay để tránh lỗi WDT
    }
  }

  yield();  // Cho phép ESP xử lý các task ngầm của WiFi
}