// #include <WiFi.h>
// #include <HTTPClient.h>
// #include <ArduinoJson.h>

// // WiFi Configuration
// const char* ssid = "SA";
// const char* password = "1234567890";

// // Server Configuration
// const char* serverUrl = "http://localhost:8080/api/maintenance";

// // ESP32 Device ID (unique cho từng đồng hồ)
// const char* deviceId = "ESP32_001";

// // Engineer Name
// const char* engineerName = "Kỹ sư A";

// // Pin Configuration
// #define BUZZER_PIN 25
// #define LED_PIN 2
// #define BUTTON_PIN 4

// // State Management
// enum DeviceState {
//   STATE_IDLE,
//   STATE_REQUEST_RECEIVED,
//   STATE_ACKNOWLEDGED,
//   STATE_ARRIVED
// };

// DeviceState currentState = STATE_IDLE;
// unsigned long lastCheckTime = 0;
// const unsigned long checkInterval = 3000; // Check mỗi 3 giây

// // Request Data
// String currentRequestId = "";
// String machineCode = "";
// String machineName = "";
// String location = "";
// String issueDescription = "";

// void setup() {
//   Serial.begin(115200);
  
//   // Initialize pins
//   pinMode(BUZZER_PIN, OUTPUT);
//   pinMode(LED_PIN, OUTPUT);
//   pinMode(BUTTON_PIN, INPUT_PULLUP);
  
//   // Connect to WiFi
//   connectWiFi();
  
//   Serial.println("ESP32 Maintenance Watch Ready!");
//   Serial.println("Device ID: " + String(deviceId));
//   Serial.println("Engineer: " + String(engineerName));
// }

// void loop() {
//   // Check for pending requests periodically
//   if (millis() - lastCheckTime >= checkInterval) {
//     lastCheckTime = millis();
//     checkPendingRequests();
//   }
  
//   // Handle button press based on current state
//   if (digitalRead(BUTTON_PIN) == LOW) {
//     delay(50); // Debounce
//     if (digitalRead(BUTTON_PIN) == LOW) {
//       handleButtonPress();
//       while (digitalRead(BUTTON_PIN) == LOW); // Wait for release
//     }
//   }
  
//   // Update LED based on state
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
//     Serial.print("IP Address: ");
//     Serial.println(WiFi.localIP());
//   } else {
//     Serial.println("\nWiFi Connection Failed!");
//   }
// }

// void checkPendingRequests() {
//   if (WiFi.status() != WL_CONNECTED) {
//     Serial.println("WiFi not connected!");
//     return;
//   }
//   Serial.println("1");
//   Serial.println("currentState: " + currentState);
//   if (currentState != STATE_IDLE) {
//     return; // Already processing a request
//   }

//   Serial.println("2");
  
//   HTTPClient http;
//   String url = String(serverUrl) + "/check/" + String(deviceId);
//   Serial.println("Call API");




//   http.begin(url);
//   int httpCode = http.GET();

//   Serial.println("http: " + httpCode);
//   Serial.println("http: " + url);
  
//   if (httpCode == 200) {
//     String payload = http.getString();
//     Serial.println("Have Data");
//     DynamicJsonDocument doc(1024);
//     DeserializationError error = deserializeJson(doc, payload);
    
//     if (!error) {
//       bool hasRequest = doc["hasRequest"];
//       Serial.println("Xac nhan co request cho thiet bi nay");
//       if (hasRequest) {
//         // New request received!
//         currentRequestId = doc["requestId"].as<String>();
//         machineCode = doc["machineCode"].as<String>();
//         machineName = doc["machineName"].as<String>();
//         location = doc["location"].as<String>();
//         issueDescription = doc["issueDescription"].as<String>();
        
//         currentState = STATE_REQUEST_RECEIVED;
        
//         Serial.println("\n========== NEW REQUEST ==========");
//         Serial.println("Machine: " + machineCode + " - " + machineName);
//         Serial.println("Location: " + location);
//         Serial.println("Issue: " + issueDescription);
//         Serial.println("=================================");
        
//         // Ring buzzer to alert engineer
//         ringBuzzer();
//       }
//     }
//   }
  
//   http.end();
// }

// void handleButtonPress() {
//   Serial.println("\nButton Pressed!");
  
//   switch (currentState) {
//     case STATE_IDLE:
//       Serial.println("No active request");
//       break;
      
//     case STATE_REQUEST_RECEIVED:
//       // Engineer acknowledges the request (timestamp2)
//       acknowledgeRequest();
//       break;
      
//     case STATE_ACKNOWLEDGED:
//       // Engineer arrived at location (timestamp3)
//       arriveAtLocation();
//       break;
      
//     case STATE_ARRIVED:
//       // Engineer completed the fix (timestamp4)
//       completeRequest();
//       break;
//   }
// }

// void acknowledgeRequest() {
//   Serial.println("Acknowledging request...");
  
//   HTTPClient http;
//   String url = String(serverUrl) + "/acknowledge";
  
//   http.begin(url);
//   http.addHeader("Content-Type", "application/json");
  
//   DynamicJsonDocument doc(256);
//   doc["esp32DeviceId"] = deviceId;
//   doc["engineerName"] = engineerName;
  
//   String jsonString;
//   serializeJson(doc, jsonString);
  
//   int httpCode = http.POST(jsonString);
  
//   if (httpCode == 200) {
//     String response = http.getString();
//     Serial.println("Response: " + response);
    
//     currentState = STATE_ACKNOWLEDGED;
//     Serial.println("✅ Request acknowledged! Going to: " + location);
    
//     // Beep confirmation
//     tone(BUZZER_PIN, 1000, 200);
//     delay(300);
//     tone(BUZZER_PIN, 1500, 200);
//   } else {
//     Serial.println("Failed to acknowledge: " + String(httpCode));
//   }
  
//   http.end();
// }

// void arriveAtLocation() {
//   Serial.println("Marking arrival at location...");
  
//   HTTPClient http;
//   String url = String(serverUrl) + "/arrive";
  
//   http.begin(url);
//   http.addHeader("Content-Type", "application/json");
  
//   DynamicJsonDocument doc(256);
//   doc["esp32DeviceId"] = deviceId;
  
//   String jsonString;
//   serializeJson(doc, jsonString);
  
//   int httpCode = http.POST(jsonString);
  
//   if (httpCode == 200) {
//     String response = http.getString();
//     Serial.println("Response: " + response);
    
//     currentState = STATE_ARRIVED;
//     Serial.println("✅ Arrived at location! Starting repair...");
    
//     // Beep confirmation
//     tone(BUZZER_PIN, 2000, 300);
//   } else {
//     Serial.println("Failed to mark arrival: " + String(httpCode));
//   }
  
//   http.end();
// }

// void completeRequest() {
//   Serial.println("Completing request...");
  
//   HTTPClient http;
//   String url = String(serverUrl) + "/complete";
  
//   http.begin(url);
//   http.addHeader("Content-Type", "application/json");
  
//   DynamicJsonDocument doc(256);
//   doc["esp32DeviceId"] = deviceId;
  
//   String jsonString;
//   serializeJson(doc, jsonString);
  
//   int httpCode = http.POST(jsonString);
  
//   if (httpCode == 200) {
//     String response = http.getString();
//     Serial.println("Response: " + response);
    
//     currentState = STATE_IDLE;
//     Serial.println("✅ Request completed successfully!");
    
//     // Victory beep
//     tone(BUZZER_PIN, 1000, 100);
//     delay(150);
//     tone(BUZZER_PIN, 1500, 100);
//     delay(150);
//     tone(BUZZER_PIN, 2000, 100);
    
//     // Reset data
//     currentRequestId = "";
//     machineCode = "";
//     machineName = "";
//     location = "";
//     issueDescription = "";
//   } else {
//     Serial.println("Failed to complete: " + String(httpCode));
//   }
  
//   http.end();
// }

// void ringBuzzer() {
//   // Ring pattern for new request
//   for (int i = 0; i < 3; i++) {
//     tone(BUZZER_PIN, 2000, 500);
//     delay(600);
//   }
// }

// void updateLED() {
//   switch (currentState) {
//     case STATE_IDLE:
//       digitalWrite(LED_PIN, LOW);
//       break;
      
//     case STATE_REQUEST_RECEIVED:
//       // Fast blink - new request
//       digitalWrite(LED_PIN, (millis() / 200) % 2);
//       break;
      
//     case STATE_ACKNOWLEDGED:
//       // Medium blink - going to location
//       digitalWrite(LED_PIN, (millis() / 500) % 2);
//       break;
      
//     case STATE_ARRIVED:
//       // Solid on - working on fix
//       digitalWrite(LED_PIN, HIGH);
//       break;
//   }
// }








#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h> // <--- QUAN TRỌNG CHO HTTPS
#include <ArduinoJson.h>

// ================= CẤU HÌNH WIFI =================
const char* ssid = "SA";          // Tên WiFi
const char* password = "1234567890";  // Mật khẩu WiFi

// ================= CẤU HÌNH PIN =================
#define BUZZER_PIN 25
#define LED_PIN 2
#define BUTTON_PIN 4

// ================= TRẠNG THÁI & BIẾN TOÀN CỤC =================
enum DeviceState {
  STATE_IDLE,
  STATE_REQUEST_RECEIVED,
  STATE_ACKNOWLEDGED,
  STATE_ARRIVED
};

DeviceState currentState = STATE_IDLE;
unsigned long lastCheckTime = 0;
const unsigned long checkInterval = 5000; // Check mỗi 5 giây

// Request Data (Lưu trữ thông tin)
String currentRequestId = "";
String machineCode = "";
String machineName = "";
String location = "";
String issueDescription = "";

// Forward declarations
void connectWiFi();
void checkPendingRequests();
void handleButtonPress();
void ringBuzzer();
void updateLED();

void setup() {
  Serial.begin(115200);
  
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  connectWiFi();
  
  Serial.println("ESP32 Maintenance Watch Ready!");
}

void loop() {
  // 1. Chỉ check server khi đang rảnh (IDLE) và đủ thời gian
  if (currentState == STATE_IDLE && millis() - lastCheckTime >= checkInterval) {
    lastCheckTime = millis();
    checkPendingRequests();
  }
  
  // 2. Xử lý nút bấm
  if (digitalRead(BUTTON_PIN) == LOW) {
    delay(50); // Chống rung (Debounce)
    if (digitalRead(BUTTON_PIN) == LOW) {
      handleButtonPress();
      while (digitalRead(BUTTON_PIN) == LOW) { delay(10); } // Chờ thả nút
    }
  }
  
  // 3. Cập nhật đèn LED
  updateLED();
  delay(10); 
}

void connectWiFi() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected! IP: " + WiFi.localIP().toString());
}

// ================= HÀM GỌI API (ĐÃ SỬA CHO HTTPS) =================

void checkPendingRequests() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi Lost!");
    return;
  }

  // --- BẮT ĐẦU CẤU HÌNH HTTPS ---
  WiFiClientSecure client; 
  client.setInsecure(); // <--- QUAN TRỌNG: Bỏ qua kiểm tra chứng chỉ SSL
  
  HTTPClient http;
  
  // URL HTTPS bạn muốn test
  String url = "https://jsonplaceholder.typicode.com/todos/1";
  
  Serial.print("Goi API: ");
  Serial.println(url);

  // Bắt đầu kết nối
  if (http.begin(client, url)) { 
    int httpCode = http.GET();

    if (httpCode > 0) {
      Serial.printf("[HTTPS] GET Code: %d\n", httpCode);
      
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.println("Data received:");
        Serial.println(payload); // In toàn bộ JSON nhận được
        
        // --- XỬ LÝ JSON ---
        DynamicJsonDocument doc(1024);
        DeserializationError error = deserializeJson(doc, payload);
        
        if (!error) {
          // 1. TEST VỚI JSONPLACEHOLDER (Để chứng minh code chạy)
          if (doc.containsKey("title")) {
             String title = doc["title"].as<String>();
             Serial.println("=> TEST SUCCESS! Title from API: " + title);
             // Vì API test này không có machineCode, nên ta chỉ in ra thôi.
          }

          // 2. LOGIC CŨ CỦA BẠN (Dành cho Server thật sau này)
          // Lưu ý: API jsonplaceholder không trả về hasRequest, nên đoạn này sẽ False
          bool hasRequest = doc["hasRequest"]; 
          if (hasRequest) {
             currentRequestId = doc["requestId"].as<String>();
             machineCode = doc["machineCode"].as<String>();
             issueDescription = doc["issueDescription"].as<String>();
             
             currentState = STATE_REQUEST_RECEIVED;
             Serial.println("\n>>> CÓ YÊU CẦU BẢO TRÌ MỚI! <<<");
             ringBuzzer();
          } else {
             Serial.println("(API Test khong co truong hasRequest -> Khong kich hoat Buzzer)");
          }

        } else {
          Serial.print("JSON Error: ");
          Serial.println(error.c_str());
        }
      }
    } else {
      Serial.printf("[HTTPS] GET failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end(); // Đóng kết nối
  } else {
    Serial.println("[HTTPS] Unable to connect");
  }
}

// ================= CÁC HÀM HỖ TRỢ KHÁC =================

void handleButtonPress() {
  Serial.println("Button Pressed!");
  // Reset trạng thái về IDLE để test lại
  if (currentState != STATE_IDLE) {
      currentState = STATE_IDLE;
      Serial.println("Reset state to IDLE");
      tone(BUZZER_PIN, 1000, 200);
  } else {
      Serial.println("Dang o IDLE, cho request...");
  }
}

void ringBuzzer() {
  for (int i = 0; i < 3; i++) {
    tone(BUZZER_PIN, 2000, 500);
    delay(600);
  }
  noTone(BUZZER_PIN);
}

void updateLED() {
  unsigned long t = millis();
  switch (currentState) {
    case STATE_IDLE:
      digitalWrite(LED_PIN, LOW);
      break;
    case STATE_REQUEST_RECEIVED:
      digitalWrite(LED_PIN, (t / 200) % 2); // Nháy nhanh
      break;
    default:
      digitalWrite(LED_PIN, HIGH);
      break;
  }
}