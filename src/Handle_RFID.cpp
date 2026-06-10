#include "Handle_RFID.h"

// Khởi tạo đối tượng RFID
MFRC522 mfrc522(RFID_CS_PIN, RFID_RST_PIN);

void rfid_Init() {
    // Khởi tạo bus SPI mặc định cho RFID với các chân Custom
    SPI.begin(RFID_SCK_PIN, RFID_MISO_PIN, RFID_MOSI_PIN, RFID_CS_PIN);
    
    // Khởi động module RC522
    mfrc522.PCD_Init();
}

String rfid_GetUID() {
    // Kiểm tra xem có thẻ chạm vào không
    if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
        return ""; // Không có thẻ thì trả về chuỗi rỗng
    }

    // Có thẻ -> Đọc mã UID và ghép thành chuỗi Hex
    String uidString = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
        uidString += String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
        uidString += String(mfrc522.uid.uidByte[i], HEX);
    }
    
    uidString.toUpperCase(); // Viết hoa toàn bộ 
    uidString.trim();        // Cắt khoảng trắng dư thừa
    
    mfrc522.PICC_HaltA();    // Cho thẻ off để không đọc lặp liên tục

    return uidString;
}

void rfid_Reset() {
    mfrc522.PCD_Init(); 
}