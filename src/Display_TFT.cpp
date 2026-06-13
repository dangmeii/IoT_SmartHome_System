#include "Display_TFT.h"
#define ZONE_DHT_X 10
#define ZONE_DHT_Y 10

// ====== Tạo SPI riêng ======
SPIClass spi = SPIClass(FSPI);

// ====== Khởi tạo TFT ======
Adafruit_ST7789 tft = Adafruit_ST7789(&spi, TFT_CS, TFT_DC, TFT_RST);

void display_Init() {
    // Init SPI với chân custom
    spi.begin(TFT_SCK, TFT_MISO, TFT_MOSI, TFT_CS); 

    // Init TFT
    tft.init(240, 240);       // ST7789 240x240
    tft.setRotation(2);       // Xoay màn hình 180 độ

    // Nếu có BL (backlight)
    if (TFT_BL != -1) {
        pinMode(TFT_BL, OUTPUT);
        digitalWrite(TFT_BL, HIGH);
    }
}

void display_ShowTestMessage() {
    // Test hiển thị
    tft.fillScreen(ST77XX_BLACK);
    
    // tft.setTextColor(ST77XX_YELLOW);
    // tft.setTextSize(2);
    // tft.setCursor(20, 60);
    // tft.print("NHOM 3 DEP TRAI");
}

// HÀM RFID
void display_ShowWaitCard() {
    tft.setCursor(20, 100);
      tft.setTextSize(3);
    tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK); 
    tft.print("SWIPE CARD..");
}

void display_ShowUID(String ID){
        tft.setCursor(20, 150);
          tft.setTextSize(2);
        tft.setTextColor(ST77XX_YELLOW, ST77XX_BLACK); 
        tft.print("ID: ");
        tft.print(ID); // như printf bên C 
}

void display_ShowStatus(String msg, uint16_t color) {
    tft.setCursor(20, 180); // set pointer ở hàng dưới cùng
      tft.setTextSize(2);
    tft.setTextColor(color, ST77XX_BLACK); // Nền đen (đè chữ cũ)
    tft.print(msg); 
}

// Hàm này xóa các TEXT đã in ra TFT
void display_ClearText_Smart(String text, int x, int y, uint8_t textSize) {
    // 1. Ép hệ thống dùng đúng thước đo
    tft.setTextSize(textSize);

    int16_t x1, y1; // Biến chứa tọa độ góc trên bên trái của khung chữ
    uint16_t w, h;  // Biến chứa Chiều rộng và Chiều cao sẽ được đo

    // Nhờ thư viện đo đạc khung bao quanh dòng chữ
    tft.getTextBounds(text, x, y, &x1, &y1, &w, &h);

    // Lấy luôn kết quả w, h vừa đo được để xóa
    tft.fillRect(x1, y1, w, h, ST77XX_BLACK); 
}

// HÀM ĐO NHIỆT ĐỘ - ĐỘ ẨM
void display_ShowWeather(float temp, float hum) {
    // 1. Tẩy vùng cũ (Rộng 120px, cao 20px)
    tft.fillRect(ZONE_DHT_X, ZONE_DHT_Y, 120, 20, ST77XX_BLACK);
    
    // 2. In giá trị mới
    tft.setCursor(ZONE_DHT_X, ZONE_DHT_Y);
    tft.setTextSize(2);
    
    // 2.1 In nhiệt độ
    tft.setTextColor(ST77XX_ORANGE); 
    tft.print(temp, 1); // In 1 số thập phân
    tft.print("C  ");

    // 2.2 In độ ẩm
    tft.setTextColor(ST77XX_CYAN); // Xanh blue
    tft.print(hum, 0);  // Độ ẩm in số nguyên 
    tft.print("%");
}

// Tà ma ngoại đạo
void display_ShowMotionStatus(bool isDetected) {
    tft.setCursor(20, 65); // Tọa độ tuốt dưới cùng
    tft.setTextSize(2);
    
    if (isDetected) {
        // 1. Xóa trạng thái cũ (AN TOAN)
        display_ClearText_Smart("PIR: AN TOAN", 20, 65, 2);
        
        // 2. In chữ mới lên 
        tft.setCursor(20, 65);
        tft.setTextColor(ST77XX_RED); 
        tft.print("PIR: CO NGUOI!"); 
        
    } else {
        // 1. Xóa trạng thái cũ (CO NGUOI)
        display_ClearText_Smart("PIR: CO NGUOI!", 20, 65, 2);
        
        // 2. In chữ mới lên
        tft.setCursor(20, 65);
        tft.setTextColor(ST77XX_GREEN);
        tft.print("PIR: AN TOAN"); 
    }
}

    void display_ShowGasLevel(int gasLevel) {
        tft.setCursor(20, 210);
        tft.setTextSize(2);
        tft.setTextColor(ST77XX_MAGENTA, ST77XX_BLACK);
        tft.print("GAS: ");
        tft.print(gasLevel);
        tft.print("%  ");
}

    void display_rainLevel(int rainLevel) {
    tft.setCursor(20, 35); // Trục Y = 35 
    tft.setTextSize(2);
    tft.setTextColor(0x0410, ST77XX_BLACK); // Xanh cổ vịt
    tft.print("RAIN: ");
    tft.print(rainLevel);
    tft.print("%  ");// Quét rác hiển thị
}