#include "MQ2_sensor.h"

#define MQ2_PIN 8 

void mq2_Init() {
    // Không cần cấu hình pinMode cho hàm analogRead trên ESP32

    Serial.println("✅ Khoi tao MQ2 THANH CONG!");
}

int mq2_ReadGas() {
    int rawValue = analogRead(MQ2_PIN);
    int percent = map(rawValue, 0, 4095, 0, 100); // Chuyển đổi từ thang 0-4095 sang 0-100%
    
    return constrain(percent, 0, 100); 
}