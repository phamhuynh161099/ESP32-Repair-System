#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <SSD1306Wire.h>


// --- Cấu hình Pin (Dành cho ESP32) ---
#define BUZZER_PIN 14  // GPIO 14 phù hợp cho còi Buzzer trên ESP32
#define LED_PIN 2      // Đèn LED màu xanh dương tích hợp trên mạch ESP32
#define BUTTON_PIN 0   // Nút BOOT tích hợp sẵn trên mạch ESP32

// --- Quản lý trạng thái ---
enum DeviceState {
  STATE_NONE,
  STATE_CALLED_ENGINEER,
  STATE_WAITING_ACCEPT,  // Chờ Bạn Xác nhận kĩ sư đã sửa xong
};


enum MaintenanceState {
  STATE_IDLE,
  STATE_REQUEST_RECEIVED,
  STATE_ACKNOWLEDGED,
  STATE_ARRIVED
};

// --- Cấu hình OLED ---
SSD1306Wire display(0x3c, 21, 22);
#define flipDisplay true

// --- Cấu hình WiFi ---
const char* ssid = "SA";
const char* password = "1234567890";
WiFiClient client1;
HTTPClient http;


DeviceState currentState = STATE_NONE;
MaintenanceState currentMaintenanceState = STATE_IDLE;
unsigned long lastCheckTime = 0;
const unsigned long checkInterval = 3000;

// --- Cấu hình Server (Endpoint API check-line) ---
// Chú ý: Đổi IP dưới đây thành IP thực tế của máy chủ chạy Spring Boot của bạn
const char* serverUrl = "http://103.140.249.253:8080/api/line-esp/get-line-info-by";

// --- CẤU HÌNH LAYOUT OLED ---
#define LABEL_X 0    // Tọa độ X của các nhãn (|)
#define VALUE_X 5    // Tọa độ X của nội dung
#define LINE_1_Y 22  // Tọa độ Y Dòng 1
#define LINE_2_Y 36  // Tọa độ Y Dòng 2
#define LINE_3_Y 50  // Tọa độ Y Dòng 3

// --- Biến hỗ trợ chữ chạy (Scrolling Text) ---
String oledTitle = "";
String oledLine1 = "";
String oledLine2 = "";
String oledLine3 = "";

int scrollX = 0;
unsigned long lastScrollTime = 0;
const int SCROLL_SPEED = 40;
int waitTimer = 1500;
const int MAX_WIDTH_ALLOW = 128 - VALUE_X;

// --- CÁC HÀM XỬ LÝ GIAO DIỆN OLED (Giữ nguyên của bạn) ---
void renderDisplay() {
  display.clear();

  // 1. Tiêu đề
  display.setFont(ArialMT_Plain_16);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(64, 0, oledTitle);
  display.drawLine(0, 18, 127, 18);

  // 2. Nội dung chi tiết
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);

  int w1 = display.getStringWidth(oledLine1);
  int w2 = display.getStringWidth(oledLine2);
  int w3 = display.getStringWidth(oledLine3);

  display.drawString(VALUE_X - (w1 > MAX_WIDTH_ALLOW ? scrollX : 0), LINE_1_Y, oledLine1);
  display.drawString(VALUE_X - (w2 > MAX_WIDTH_ALLOW ? scrollX : 0), LINE_2_Y, oledLine2);
  display.drawString(VALUE_X - (w3 > MAX_WIDTH_ALLOW ? scrollX : 0), LINE_3_Y, oledLine3);

  // 3. Che phần chữ bị lẹm
  display.setColor(BLACK);
  display.fillRect(0, 19, VALUE_X - 1, 45);
  display.setColor(WHITE);

  // 4. In nhãn
  display.drawString(LABEL_X, LINE_1_Y, "|");
  display.drawString(LABEL_X, LINE_2_Y, "|");
  display.drawString(LABEL_X, LINE_3_Y, "|");

  display.display();
}

void handleScrolling() {
  display.setFont(ArialMT_Plain_10);
  int maxW = max(display.getStringWidth(oledLine1),
                 max(display.getStringWidth(oledLine2),
                     display.getStringWidth(oledLine3)));

  if (maxW > MAX_WIDTH_ALLOW) {
    if (millis() - lastScrollTime >= SCROLL_SPEED) {
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

// ---- HÀM RUNG CHUÔNG ----
void ringBuzzer() {
  for (int i = 0; i < 3; i++) {
    tone(BUZZER_PIN, 2000, 400);
    delay(500);  // Lưu ý: hàm delay ở đây khi chuông reo sẽ tạm dừng cuộn chữ, xong chuông chữ lại chạy tiếp
  }
}

// --- HÀM KẾT NỐI WIFI ---
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

// --- HÀM GỌI API LẤY THÔNG TIN LINE THEO MAC ---
String macAddress = "";
String lineId = "";
String lineName = "";
String espName = "";

String espEngineerMac = "";
String espEngineerName = "";
void fetchLineInfo() {
  if (WiFi.status() != WL_CONNECTED) return;

  // Lấy địa chỉ MAC của ESP
  macAddress = WiFi.macAddress();
  Serial.println("MAC Address: " + macAddress);

  // Thông báo lên màn hình đang tải
  updateOLED("LOADING...", "MAC: " + macAddress, "Fetching Line Data", "Please wait...");

  // Tạo URL: http://.../api/line-esp/get-line-info-by?mac=XX:XX...
  String url = String(serverUrl) + "?mac=" + macAddress;
  Serial.println("url: " + url);

  http.begin(client1, url);
  int httpCode = http.GET();
  if (httpCode == 200) {
    // Nếu API trả về thành công (Tìm thấy MAC trong Database)
    String payload = http.getString();
    Serial.println("Response: " + payload);

    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, payload);

    if (!error) {
      // Đọc JSON trả về từ Spring Boot
      lineId = doc["line_id"].as<String>();
      lineName = doc["line_name"].as<String>();
      espName = doc["esp_line_name"].as<String>();
      espEngineerMac = doc["esp_engineer_mac"].as<String>();
      espEngineerName = doc["esp_engineer_name"].as<String>();

      // Hiển thị thông tin lên OLED (Nếu tên Line quá dài, nó sẽ tự động cuộn)
      updateOLED("LINE INFO", "ID: " + lineId, "Name: " + lineName, "ESP: " + espName);
    } else {
      updateOLED("ERROR", "JSON Parse Failed", error.c_str(), "");
    }
  } else if (httpCode == 404) {
    // Nếu API trả về 404 (Không tìm thấy MAC trong bảng tbl_line_map_esp)
    updateOLED("UNREGISTERED", "MAC Not Found!", "MAC: " + macAddress, "Contact Admin");
  } else {
    // Lỗi mạng hoặc server sập
    updateOLED("SERVER ERR", "HTTP Code: " + String(httpCode), "Check Server API", "");
  }

  http.end();
}

void setup() {
  Serial.begin(115200);

  // 1. Khởi tạo OLED
  display.init();
  if (flipDisplay) display.flipScreenVertically();
  display.clear();
  updateOLED("SYSTEM", "Starting...", "Initializing...", "");

  // Cấu hình chân IO
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // 2. Kết nối WiFi
  connectWiFi();

  // 3. Gọi API lấy thông tin Line ngay khi WiFi đã kết nối
  if (WiFi.status() == WL_CONNECTED) {
    fetchLineInfo();
  }
}

void loop() {
  // Hàm này chạy liên tục để duy trì hiệu ứng cuộn chữ mượt mà
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
      while (digitalRead(BUTTON_PIN) == LOW);  // Chờ nhả nút
    }
  }

  delay(10);
}



// Dữ liệu Request
String currentRequestId = "";
String machineCode = "";
String machineName = "";
String location = "";
String issueDescription = "";
String requestStatus = "";
char* requestApiUrlGetCurrent = "http://103.140.249.253:8080/api/line-esp";
void checkPendingRequests() {
  // if (WiFi.status() != WL_CONNECTED || currentState != STATE_NONE) return;
  if (WiFi.status() != WL_CONNECTED) return;

  String url = String(requestApiUrlGetCurrent) + "/check/" + String(lineId);
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
        currentMaintenanceState = STATE_REQUEST_RECEIVED;
        currentState = STATE_CALLED_ENGINEER;
        updateOLED("LINE ESP", "Already Call Engineer", "", "");
        ringBuzzer();
      }

      if (requestStatus == "ACKNOWLEDGED") {
        currentMaintenanceState = STATE_ACKNOWLEDGED;
        currentState = STATE_WAITING_ACCEPT;
        updateOLED("LINE ESP", "Engineer Acknowledged", "Pls Click Button When Your Machine Fixed", "");
        // ringBuzzer();
      }

      // if (requestStatus == "ARRIVED") {
      //   currentMaintenanceState = STATE_ARRIVED;
      //   currentState = STATE_WAITING_ACCEPT;
      //   updateOLED("LINE ESP", "Engineer is fixing now", "Pls Click Button When Your Machine Fixed", "");
      //   // ringBuzzer();
      // }
    }
  }
  http.end();
}

void handleButtonPress() {
  Serial.println("Vo ham handleButtonPress..." + currentState);
  switch (currentState) {
    case STATE_NONE:
      updateOLED("INFO", "No Task", "-", "System Active");
      callMaintenanceEngineer();
      break;
    case STATE_WAITING_ACCEPT:
      Serial.println("Nhan nut xac nhan...");
      if (sendPostRequestComplete(STATE_NONE, "COMPLETED", "Waiting task")) {
        currentRequestId = "";
        machineCode = "";
        machineName = "";
        location = "";
        issueDescription = "";
        requestStatus = "";

        currentState = STATE_NONE;
        currentMaintenanceState = STATE_IDLE;

        updateOLED("LINE ESP", "You Confirmed Engineer Fixed", "", "");
        delay(8000);
        updateOLED("LINE INFO", "ID: " + lineId, "Name: " + lineName, "ESP: " + espName);
      }
      break;
  }
}



const char* urlApiRequestCom = "http://103.140.249.253:8080/api/maintenance-v2";
bool sendPostRequestComplete(DeviceState nextState, String displayTitle, String displayMsg) {
  if (WiFi.status() != WL_CONNECTED) return false;
  Serial.println("Call api..." + currentRequestId + " " + espEngineerMac);
  http.begin(client1, String(urlApiRequestCom) + "/complete");  // Đã thêm client1
  http.setTimeout(10000 * 6);
  http.addHeader("Content-Type", "application/json");

  DynamicJsonDocument doc(256);
  doc["esp32DeviceId"] = espEngineerMac;
  doc["engineerName"] = "";
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

// Thêm đường dẫn gọi API của bạn (sửa lại theo IP thực tế)
const char* requestApiUrl = "http://103.140.249.253:8080/api/maintenance-v2/request";
void callMaintenanceEngineer() {
  if (WiFi.status() != WL_CONNECTED) {
    updateOLED("WIFI ERR", "No Connection", "Cannot call", "");
    return;
  }

  updateOLED("CALLING...", "Sending request", "Please wait", "");
  Serial.println("Đang gửi yêu cầu gọi kĩ sư...");

  http.begin(client1, requestApiUrl);
  http.addHeader("Content-Type", "application/json");

  // 1. Đóng gói payload JSON giống hệt formData trên web
  DynamicJsonDocument doc(512);
  // (Lưu ý: Các biến machineCode, machineName... bạn có thể gán cứng
  // hoặc lấy từ dữ liệu lúc gọi api check-line ban đầu)
  doc["machineCode"] = lineId;
  doc["machineName"] = lineName;
  doc["location"] = lineId + " " + lineName;
  doc["deviceId"] = espEngineerMac;
  doc["issueDescription"] = "Call Error By ESP " + espName;

  String payload;
  serializeJson(doc, payload);
  Serial.println("Payload gửi đi: " + payload);

  // 2. Gửi POST request
  int httpCode = http.POST(payload);

  // 3. Xử lý Response trả về
  if (httpCode > 0) {
    String response = http.getString();
    Serial.println("Server phản hồi: " + response);

    DynamicJsonDocument respDoc(1024);
    DeserializationError error = deserializeJson(respDoc, response);
    if (!error) {
      bool isSuccess = respDoc["success"];
      if (isSuccess) {
        // Đi sâu vào object JSON để lấy ticket status như code JS
        String ticketStatus = respDoc["data"]["maintenanceTracking"]["ticket"]["status"].as<String>();
        if (ticketStatus == "INIT") {
          updateOLED("IN QUEUE", "Đã ghi nhận", "Đang chờ xếp hàng", "Vui lòng đợi...");
          ringBuzzer();  // Kêu còi báo hiệu
        } else {
          updateOLED("SUCCESS", "Kĩ sư đang tới", "Yêu cầu thành công!", "");
          ringBuzzer();
        }
      } else {
        // Tương đương nhánh: showToast("Have Error, Pls Try Again!", "error")
        updateOLED("FAILED", "Server báo lỗi", "Vui lòng thử lại", "");
      }
    } else {
      updateOLED("PARSE ERR", "Lỗi đọc JSON", "Từ server", "");
    }
  } else {
    // Tương đương nhánh catch(error): showToast("Can not connect server!", "error")
    updateOLED("HTTP ERR", "Không thể kết nối", http.errorToString(httpCode).c_str(), "");
  }
  http.end();
}