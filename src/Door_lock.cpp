#include "Door_lock.h"
#include <Arduino.h>

// ====== CẤU HÌNH HARDWARE TIMER (LEDC) ======
const int pwmFreq = 50;       // Tần số 50Hz chuẩn cho điều khiển Servo
const int pwmResolution = 10; // Độ phân giải 10-bit (giá trị PWM chạy từ 0 đến 1023)
const int pwmChannel = 4;     // Ép dùng Kênh số 4 (để né xung đột với màn hình TFT)

// ---DÙNG VỚI SERVO 360 ĐỘ ---
const int STOP_SERVO = 77;   // Điểm phanh đứng im
const int PULL_LATCH = 100;  // Quay thu chốt vào
const int PUSH_LATCH = 40;   // Quay đẩy chốt ra

// ===== BIẾN QUẢN LÝ TRẠNG THÁI (MÁY TRẠNG THÁI) =====
// 0: Khóa im lặng
// 1: Đang KÉO CHỐT RA (0.8s)
// 2: Đang GIỮ MỞ CỬA (5s)
// 3: Đang ĐẨY CHỐT VÀO (0.6s)
static int cuaTrangThai = 0; 
static unsigned long thoiDiemChuyenBuoc = 0;

void door_Init() {
    ledcSetup(pwmChannel, pwmFreq, pwmResolution);
    ledcAttachPin(SERVO_PIN, pwmChannel);
    
    // Vừa bật lên là phải ĐỨNG IM chờ lệnh
    ledcWrite(pwmChannel, STOP_SERVO); 
    cuaTrangThai = 0;
}

// Hàm này CHỈ ĐƯỢC GỌI 1 LẦN duy nhất khi quét thẻ thành công
void door_Unlock() {
    if (cuaTrangThai == 0) { 
        // 1. KÉO CHỐT RA (Quay ngay lập tức)
        ledcWrite(pwmChannel, PULL_LATCH); 
        
        // Đánh dấu chuyển sang trạng thái 1 và lưu lại mốc thời gian bắt đầu
        cuaTrangThai = 1; 
        thoiDiemChuyenBuoc = millis();
    }
}

// Hàm này PHẢI GỌI LIÊN TỤC ở ngoài cùng của loop()
void door_Update() {
    // Lấy thời gian hiện tại
    unsigned long hienTai = millis();

    // Bước 1 -> Bước 2: Kéo chốt xong -> Phanh lại chờ người qua
    if (cuaTrangThai == 1) {
        if (hienTai - thoiDiemChuyenBuoc >= 800) {
            // 2. PHANH LẠI! Giữ cửa mở
            ledcWrite(pwmChannel, STOP_SERVO);
            
            cuaTrangThai = 2; // Chuyển sang chờ 5 giây
            thoiDiemChuyenBuoc = hienTai; // Reset mốc thời gian
        }
    }
    
    // Bước 2 -> Bước 3: Đợi 5 giây xong -> Bắt đầu đẩy chốt khóa lại
    else if (cuaTrangThai == 2) {
        if (hienTai - thoiDiemChuyenBuoc >= 5000) {
            // 4. ĐẨY CHỐT VÀO
            ledcWrite(pwmChannel, PUSH_LATCH);
            
            cuaTrangThai = 3; // Chuyển sang đợi đẩy chốt
            thoiDiemChuyenBuoc = hienTai;
        }
    }
    
    // Bước 3 -> Trở về 0: Đẩy chốt xong -> Phanh lại khóa an toàn
    else if (cuaTrangThai == 3) {
        if (hienTai - thoiDiemChuyenBuoc >= 600) {
            // 5. PHANH LẠI! Trạng thái khóa cửa an toàn
            ledcWrite(pwmChannel, STOP_SERVO);
            
            cuaTrangThai = 0; // Xong quy trình, về lại trạng thái ban đầu
        }
    }
}