#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define SCREEN_ADDRESS 0x3C

// Cấu hình chân giao tiếp I2C bị "đảo ngược" đặc thù của mạch HW-364B
#define OLED_SDA 14 // Tương ứng chân D5 trên NodeMCU
#define OLED_SCL 12 // Tương ứng chân D6 trên NodeMCU

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  Serial.begin(115200);

  // Khởi tạo giao tiếp I2C với cặp chân chính xác của phần cứng
  Wire.begin(OLED_SDA, OLED_SCL); 

  // Khởi tạo màn hình
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("Khởi tạo OLED thất bại."));
    for(;;);
  }

  // Xóa trắng màn hình trước khi in
  display.clearDisplay();
  
  // Thiết lập định dạng chữ
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 10);
  
  // In thông báo test ra màn hình
  display.println(F("Thanh cong!"));
  display.println(F("Mach HW-364B"));
  display.println(F("SDA = GPIO14 (D5)"));
  display.println(F("SCL = GPIO12 (D6)"));
  
  // Đẩy dữ liệu từ bộ đệm lên hiển thị thực tế
  display.display();
}

void loop() {
  // Không cần thêm thao tác trong vòng lặp này
}