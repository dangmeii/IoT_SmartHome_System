#include "Rain_sensor.h"
#include <Arduino.h>

#define RAIN_PIN 7 

int rain_Read() {
    int rawValue = analogRead(RAIN_PIN);
    int percent = map(rawValue, 4095, 0, 0, 100);  // Chuyển đổi từ thang 4095-0 sang 0-100%
   
    return constrain(percent, 0, 100);
}