#define BLYNK_TEMPLATE_ID "TMPL6ZHzmcct4"
#define BLYNK_TEMPLATE_NAME "SmartHome"
#define BLYNK_AUTH_TOKEN "OKFsbFnbq7O8jVdhc86ieIT6Iulath9H"

#include <WiFiManager.h> // Thư viện quản lý WiFi 
#include <BlynkSimpleEsp32.h>
#include <Arduino.h>

  // thư viện tự định nghĩa
#include "Display_TFT.h" 
#include "Handle_RFID.h"
#include "Door_lock.h"
#include "Weather.h"
#include "Motion.h"
#include "Led_RGB.h"
#include "Audio_system.h"
#include "Light_sensor.h"
#include "MQ2_sensor.h" 
#include "Rain_sensor.h"

// Định nghĩa chân LED trên board
#define ONBOARD_LED 38
// ====== KHAI BÁO MÃ THẺ CHỦ (MASTER CARD) ======
const String MASTER_ID = "91 D9 F2 06";

// ===== BIẾN THỜI GIAN CHO DHT11 =====
// Bắt buộc dùng kiểu unsigned long vì số millis() sẽ rất to
unsigned long previousDHTMillis = 0;
const long dhtInterval = 2000; // Khoảng thời gian giữa 2 lần đọc (s)

// ===== Variable của module Weather =====
float nhietDo = 0;
float doAm = 0;
int mucDoMua = rain_Read(); 

// ===== variable của module Motion =====
static bool trangThaiPIR_Cu = false;
static unsigned long thoiDiemBatXanh = 0;
static bool dangODoTreXanh = false;
int SolanMoCua = 0; 


// ===== variable của module Gas =====
    static unsigned long thoiDiemCapNhatTFT_Gas = 0;
    unsigned long thoiGianHienTai = millis();
    int nongDoGas = mq2_ReadGas();

// ===== Variable bắn lên Bllnk =====
    unsigned long thoiDiemCapNhatBlynk = 0;

void setupWiFi() {
    WiFiManager wifiManager;
    wifiManager.autoConnect("ESP32-Access-Point");
}

void setup() {
    // Khởi tạo Serial 
    Serial.begin(115200);
    // delay(2000);
    setupWiFi();

    // Đánh thức Blynk
    Blynk.config(BLYNK_AUTH_TOKEN);
    Blynk.connect();

    // Hàm khởi tạo các module (để run các bus riêng biệt)
    display_Init();
    rfid_Init();
    door_Init();
    weather_Init();
    motion_Init();
    rgb_Init();
    audio_Init();
    bh1750_Init();  
    mq2_Init();  

    // Cài đặt chế độ LED
    pinMode(ONBOARD_LED, OUTPUT);   
    rgb_SetColor(0, 0, 0); // tắt đèn lúc đầu

    // Hàm in chữ + chờ quét thẻ
    display_ShowTestMessage();
    delay(1000);
    display_ClearText_Smart("NHOM 3 DEP TRAI", 20, 60, 2);
    display_ShowWaitCard();
    display_ShowMotionStatus(motion_Detected());

    // Hàm nồng độ
    display_ShowGasLevel(mq2_ReadGas());
    
}

void loop() {
    Blynk.run();

    /* NHIỆM VỤ ĐẶC BIỆT: CẬP NHẬT TRẠNG THÁI CỬA */
    door_Update();

    // LẤY THỜI GIAN THỰC CHO TOÀN BỘ HỆ THỐNG DÙNG CHUNG
    unsigned long currentMillis = millis();

    // NHIỆM VỤ 1: CẬP NHẬT THỜI TIẾT VÀ HIỂN THỊ TRÊN TFT
    if (currentMillis - previousDHTMillis >= dhtInterval) {
        previousDHTMillis = currentMillis;
        weather_Update(nhietDo, doAm); 
    }

    static unsigned long thoiDiemCapNhatTFT_Rain = 0;
    if (currentMillis - thoiDiemCapNhatTFT_Rain >= 3000) {
        mucDoMua = rain_Read();
        display_rainLevel(mucDoMua);
        thoiDiemCapNhatTFT_Rain = currentMillis;
    }

    // NHIỆM VỤ 2: QUẸT THẺ RFID

    static unsigned long thoiGianChoXoa = 0;   
    static unsigned long thoiDiemBatDauCho = 0;
    static bool dangTrongTrangThaiChoXoa = false;
    static String theVuaQuet = "";

   // 1. Quét thẻ liên tục ở mọi chu kỳ vòng lặp 
    if (!dangTrongTrangThaiChoXoa) {
        String cardID = rfid_GetUID();
    
    // Nếu có thẻ được quét
    if (cardID != "") {
        theVuaQuet = cardID; // Lưu lại ID để xử lý
        display_ShowUID(cardID);

    // ====== LOGIC KIỂM TRA THẺ ======

    if (cardID == MASTER_ID) {
            // NẾU THẺ ĐÚNG
            display_ShowStatus("MO CUA...", ST77XX_GREEN);
            // GỌI HÀM MỞ CỬA 
            door_Unlock(); 
            SolanMoCua++; 
            
           // Cấu hình chờ đúng 3 giây (3000ms) rồi xóa
                thoiGianChoXoa = 3000;
                thoiDiemBatDauCho = currentMillis;
                dangTrongTrangThaiChoXoa = true;
        } 
        
        else {
            // NẾU THẺ SAI
            display_ShowStatus("SAI THE!", ST77XX_RED);

           // Cấu hình chờ 3 giây (3000ms) để cảnh báo rồi mới xóa
                thoiGianChoXoa = 3000;
                thoiDiemBatDauCho = currentMillis;
                dangTrongTrangThaiChoXoa = true;
             
        }
    }
} 

    // 2. Khối đếm giờ chạy ngầm: Tự động dọn dẹp màn hình sau khi hết thời gian chờ
    if (dangTrongTrangThaiChoXoa && (currentMillis - thoiDiemBatDauCho >= thoiGianChoXoa)) {
        
        // Tiến hành xóa chữ trên màn hình 
        if (theVuaQuet == MASTER_ID) {
        
        Serial.println("chuan bi xoa MO CUA... ");
            display_ClearText_Smart("MO CUA...", 20, 180, 2);
            
        } else {

        Serial.println("chuan bi xoa SAI THE! ");
            display_ClearText_Smart("SAI THE!", 20, 180, 2);
        }

        Serial.println("chuan bi xoa ID: " + theVuaQuet);

        display_ClearText_Smart("ID: " + theVuaQuet, 20, 150, 2);


        // Reset lại màn hình về trạng thái chờ 
        display_ShowWaitCard(); 
        
        // Mở khóa cho phép quét thẻ tiếp theo
        dangTrongTrangThaiChoXoa = false; 
    }

    
     // NHIỆM VỤ 3: CẬP NHẬT NỒNG ĐỘ GAS MỖI 1.5 GIÂY
    if (thoiGianHienTai - thoiDiemCapNhatTFT_Gas >= 1500) {
        // nongDoGas = mq2_ReadGas();
        display_ShowGasLevel(nongDoGas); 
        thoiDiemCapNhatTFT_Gas = thoiGianHienTai; // Reset bộ đếm
    }

    // NHIỆM VỤ 4: THEO DÕI CHUYỂN ĐỘNG (In thẳng lên TFT)
   
    // 1. Đọc dữ liệu từ sensor
    bool coNguoi = motion_Detected();
    float doSang = bh1750_ReadLux();

    // 2. Sự kiện vừa mới rời đi 
   if (coNguoi != trangThaiPIR_Cu) {
        
        display_ShowMotionStatus(coNguoi); // Chớp TFT
        
        if (coNguoi) {
            // thời điểm không độ trễ 
            dangODoTreXanh = false; // Có người -> Hủy ngay màu xanh
        } 
        else {
            dangODoTreXanh = true;             // Người vừa đi -> Kích hoạt xanh
            thoiDiemBatXanh = millis(); // Bắt đầu đếm 5s
        }
        
        // Cập nhật lại memory 
        trangThaiPIR_Cu = coNguoi; 
    }

    // 3. Khối hiển thị LED 
    // 0: Tắt | 1: Đỏ | 2: Xanh | 3: Trắng | 4: Tím (Báo cháy)

    static int mauLedHienTai = -1; 
    int mauCanBat = 0; 

    // Ngưỡng báo động 
    int NGUONG_BAO_CHAY = 40; 

    // TỰ ĐỘNG HẠ CỜ XANH KHI ĐỦ 5 GIÂY (Chạy ngầm độc lập bằng millis)
    if (dangODoTreXanh && (millis() - thoiDiemBatXanh >= 5000)) {
        dangODoTreXanh = false; 
    }

    // A. XÁC ĐỊNH MÀU CẦN BẬT DỰA VÀO LOGIC ƯU TIÊN

    // ƯU TIÊN 1: BÁO CHÁY! 
    if (nongDoGas > NGUONG_BAO_CHAY) {
        mauCanBat = 4; 
    }

    // ƯU TIÊN 2: Có người -> Đỏ báo động
    else if (coNguoi) {
        mauCanBat = 1; 
    } 

    // ƯU TIÊN 3: TRỜI TỐI -> Bắt buộc bật Trắng
    else if (doSang >= 0.0 && doSang < 20.0) { 
        mauCanBat = 3; 
    }
    // ƯU TIÊN 4: ĐÈN XANH KHI NGƯỜI VỪA ĐI (Chỉ được bật nếu trời sáng)
    else if (dangODoTreXanh) {
        mauCanBat = 2; // Trời sáng + Đang trong 5s trễ -> Bật Xanh an toàn
    }
    // ƯU TIÊN THẤP NHẤT: TRỜI SÁNG, KHÔNG CÓ AI
    else {
        if (doSang >= 0.0) {
            mauCanBat = 0; // Trời sáng -> Tắt đèn
        } else {
            mauCanBat = mauLedHienTai; // Giữ nguyên trạng thái cũ
        }
    }


    // B. CHỈ RA LỆNH CHO IC KHI MÀU BỊ THAY ĐỔI (TUYỆT CHIÊU CHỐNG NHÁY)
    if (mauCanBat != mauLedHienTai) {
        if (mauCanBat == 4)      rgb_SetColor(255, 0, 255);   // Tím (Báo cháy)
        else if (mauCanBat == 1) rgb_SetColor(255, 0, 0);     // Đỏ
        else if (mauCanBat == 2) rgb_SetColor(0, 255, 0);     // Xanh
        else if (mauCanBat == 3) rgb_SetColor(255, 255, 255); // Trắng
        else if (mauCanBat == 0) rgb_SetColor(0, 0, 0);       // Tắt hoàn toàn
        
        // Cập nhật lại màu hiện tại
        mauLedHienTai = mauCanBat;
    }

    // NHIỆM VỤ 5: BẮN DỮ LIỆU LÊN BLYNK (Nhiệt độ, độ ẩm, mưa, gas)
    if (currentMillis - thoiDiemCapNhatBlynk >= 3000){
        weather_Update(nhietDo, doAm); // Cập nhật lại nhiệt độ và độ ẩm mới nhất
        mucDoMua = rain_Read();
        nongDoGas = mq2_ReadGas();
    
            thoiDiemCapNhatBlynk = currentMillis; // Reset bộ đếm
    
            // Bắn dữ liệu lên Blynk
        Blynk.virtualWrite(V0, SolanMoCua);
        Blynk.virtualWrite(V1, nhietDo); 
        Blynk.virtualWrite(V2, doAm);    
        Blynk.virtualWrite(V3, nongDoGas);
        Blynk.virtualWrite(V4, mucDoMua);
    }

}
// bla bla 

