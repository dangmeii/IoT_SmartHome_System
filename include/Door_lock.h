#ifndef DOOR_LOCK_H
#define DOOR_LOCK_H

#include <Arduino.h>
// #include <ESP32Servo.h>

// Sử dụng chân IO13 trên mạch cho động cơ Servo
#define SERVO_PIN 13    

void door_Init();
void door_Unlock();
void door_Update();
void rfid_Reset();

#endif