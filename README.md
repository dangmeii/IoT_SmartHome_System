# 🏠 Hệ Thống Nhà Thông Minh ESP32-S3 (IoT_SmartHome_System)

![Platform](https://img.shields.io/badge/platform-ESP32--S3-blue)
![Framework](https://img.shields.io/badge/framework-Arduino-teal)
![Build](https://img.shields.io/badge/build-PlatformIO-orange)

Hệ thống nhà thông minh chạy trên **ESP32-S3-DevKitC-1**, tích hợp khóa cửa bằng thẻ RFID, giám sát môi trường (nhiệt độ, độ ẩm, ánh sáng, khí gas, mưa), cảnh báo bằng đèn LED RGB theo thời gian thực, hiển thị trạng thái trên màn hình TFT, và đồng bộ dữ liệu + đẩy thông báo lên ứng dụng **Blynk** để theo dõi/điều khiển từ xa. Toàn bộ logic thời gian (khóa cửa, đọc thẻ, dọn màn hình) được thiết kế **non-blocking** bằng `millis()`, không dùng `delay()` chặn vòng lặp chính.

## ✨ Tính năng chính

- 🔑 **Khóa cửa bằng thẻ RFID** (MFRC522) — mở khóa bằng servo 360° khi quét đúng thẻ chủ, cảnh báo khi quét sai thẻ
- 🌡️ **Giám sát môi trường** — nhiệt độ & độ ẩm (DHT11), cường độ ánh sáng (BH1750), nồng độ khí gas (MQ2), mức độ mưa (cảm biến mưa analog, quy đổi 0–100%)
- 🚨 **Phát hiện chuyển động** bằng cảm biến PIR, kết hợp cảnh báo cháy khi nồng độ gas vượt ngưỡng
- 💡 **LED RGB trạng thái thông minh** — tự đổi màu theo logic ưu tiên (cháy / có người / đèn ngủ / trời tối / vừa rời đi)
- 🖥️ **Màn hình TFT** hiển thị trạng thái theo thời gian thực
- ☁️ **Kết nối Blynk App** — xem dữ liệu cảm biến, bật/tắt chế độ đèn ngủ từ xa
- 🔔 **Thông báo đẩy** qua `Blynk.logEvent()` khi: có thẻ lạ quét cửa, gas vượt ngưỡng nguy hiểm, mưa to, hoặc bật đèn ngủ — có cơ chế chống spam (tối đa 1 lần/phút mỗi loại)
- 📶 **Tự cấu hình Wi-Fi** qua WiFiManager (không cần hard-code SSID/mật khẩu)
- ⚙️ **Thiết kế non-blocking** — khóa cửa và màn hình chờ đều chạy bằng máy trạng thái (state machine) dựa trên "millis()", không làm treo vòng lặp chính

## 🧩 Kiến trúc hệ thống

Để đạt được mục tiêu đó, điều quan trọng là phải biết rằng ESP32-S3 DevKitC-1 là cách tốt nhất để sử dụng, chỉ vậy thôi. điều khiển thiết bị đầu ra và đồng bộ lên đám mây.

Đầu vào (cảm biến): RFID MFRC522 (quét thẻ), DHT11 (nhiệt độ & độ ẩm), cảm biến chuyển động PIR, BH1750 (ánh sáng), MQ2 (khí gas), và cảm biến mưa. 6 cảm biến này đều gửi dữ liệu về ESP32-S3.
Đầu ra (cơ sở cấu hình): ESP32-S3 điều khiển 3 thiết bị — servo khóa cửa (mở/khóa), màn hình TFT ST7789 (hiển thị trạng thái), LED RGB (áo hiệu bằng màu sắc).
Nhưng điều đó là có thể: ESP32-S3 có thể được sử dụng cho Wi-Fi làm Trình quản lý WiFi, nhưng đó là Ứng dụng Blynk - đây là một ứng dụng tốt cho bạn. lệnh điều khiển (bật/tắt đèn ngủ) từ ứng dụng và đưa ra thông báo khi có sự kiện bất ngờ.

### Logic ưu tiên đèn LED trạng thái

Ở mỗi vòng lặp, hệ thống kiểm tra lần lượt theo thứ tự chính xác và dừng lại ở điều kiện đầu tiên thú vị:

Gas vượt ngưỡng 40 → 🟣 Tím (báo cháy)
Phát hiện chuyển → 🔴 Đỏ (báo động)
Đèn ngủ được bật qua Blynk → 🟡 Vàng (đèn ngủ)
Ánh sáng dưới 20 lux → ⚪ trắng (trời tối, tự bật đèn)
Vừa hết chuyển động trong vòng 5 giây → 🟢 Xanh (cảnh báo tạm thời)
Không rơi vào bất kỳ trường hợp nào ở trên → ⚫ Tắt đèn

Song song với logic đèn, hệ thống còn tự kiểm tra gas > 40 và mưa > 70% ở mỗi vòng để bắn thông báo gas_alarm/ heavy_rainlên Blynk (giới hạn 1 lần/phút mỗi loại, độc lập với màu đèn).

### Máy trạng thái khóa cửa (non-blocking)

Change use delay()block all system when open key, servo được điều khiển bằng trạng thái máy trong door_Update(), gọi liên tục mỗi vòng lặp, tuần tự qua 4 trạng thái:

1. Lock (trạng thái nghỉ) — servo đứng yên, chờ lệnhdoor_Unlock()
2. Kéo dài — ngay khi có lệnh mở, trục quay servo kéo dài trong 0,8 giây
3. Keep open — servo phanh đứng yên, giữ cửa mở trong 5 giây để người đi qua
4. Đẩy chốt — servo quay ngược Đẩy chốt vào trong 0.6 giây, sau đó quay trở lại trạng thái Khóa

## 🔧 Phần cứng & chân kết nối

| Module | Linh kiện | Chân GPIO | Ghi chú |
|---|---|---|---|
| Màn hình TFT | ST7789 | CS 41, DC 40, RST 45, MOSI 47, SCK 21 | Backlight nối cứng trên board (không cần điều khiển) |
| RFID | MFRC522 | CS 48, SCK 19, MOSI 35, MISO 20, RST 0 | |
| Khóa cửa | Servo 360° | 13 | Điều khiển bằng LEDC (PWM 50Hz, 10-bit, kênh 4), máy trạng thái non-blocking |
| Cảm biến chuyển động | PIR (HW-416A) | 36 | |
| Cảm biến ánh sáng | BH1750 (I2C) | SDA 18, SCL 17 | |
| Cảm biến khí gas | MQ2 | xem `MQ2_sensor.cpp` | Ngưỡng cảnh báo: giá trị đọc > 40 |
| Cảm biến mưa | Analog | 7 | Quy đổi sang 0–100%, ngưỡng cảnh báo mưa to: > 70% |
| Nhiệt độ/độ ẩm | DHT11 | 9 | |
| LED RGB trạng thái | NeoPixel (1 LED) | 48 | |

## 📁 Cấu trúc thư mục

```
IoT_SmartHome_System/
├── data/               # File âm thanh cảnh báo (.wav/.mp3) nạp vào LittleFS
├── include/            # Header (.h) khai báo chân cắm + prototype hàm từng module
├── lib/                # Thư viện cục bộ (TFT_eSPI)
├── src/                # Code triển khai (.cpp) từng module + main.cpp
├── test/                
└── platformio.ini      # Cấu hình board, framework, thư viện
```

Mỗi module phần cứng (RFID, TFT, servo, cảm biến...) được tách thành một cặp file ".h"/".cpp" riêng trong "include/" và "src/", "main.cpp" chỉ gọi các hàm "xxx_Init()" / "xxx_Update()" — giúp dễ bảo trì và test độc lập từng khối.

## ⚙️ Yêu cầu

- Board **ESP32-S3-DevKitC-1** (16MB Flash, 8MB PSRAM)
- [PlatformIO](https://platformio.org/) — dùng extension trong VS Code 
- Các thư viện đã khai báo sẵn trong "platformio.ini" (Adafruit GFX/ST7789, MFRC522, ESP32Servo, DHT, NeoPixel, BH1750, WiFiManager, Blynk,) — PlatformIO tự tải khi build lần đầu

## 🚀 Cài đặt & nạp code

1. Clone repo và mở bằng VS Code (có cài extension **PlatformIO**)
2. Cắm board qua USB
3. Build & nạp code: bấm nút **Upload** trong PlatformIO
4. Mở **Serial Monitor** ở tốc độ "115200" để xem log hoạt động

## 🔑 Cấu hình trước khi dùng

**Wi-Fi:** Lần đầu khởi động (hoặc khi mất kết nối đã lưu), thiết bị tự tạo access point tên "ESP32-Access-Point". Kết nối vào AP này bằng điện thoại/laptop, một trang cấu hình sẽ hiện ra để nhập SSID/mật khẩu Wi-Fi nhà — không cần sửa code.

**Blynk:** Cần tạo template trên Blynk Cloud, lấy "BLYNK_AUTH_TOKEN" và điền vào đầu file "src/main.cpp". Để nhận được thông báo đẩy, cần khai báo thêm 4 **Event** trong Blynk Console (mục Device Info → Events) với mã đúng như code: "night_light", "unknown_card", "gas_alarm", "heavy_rain".

**Thẻ RFID chủ:** Sửa hằng số "MASTER_ID" trong"main.cpp" thành UID thẻ thật của bạn (quét thử một lần, UID sẽ in ra Serial Monitor để bạn lấy).

| Virtual Pin | Chiều | Ý nghĩa |
|---|---|---|
| V0 | ESP32 → App | Số lần mở cửa |
| V1 | ESP32 → App | Nhiệt độ (°C) |
| V2 | ESP32 → App | Độ ẩm (%) |
| V3 | ESP32 → App | Nồng độ khí gas |
| V4 | ESP32 → App | Mức độ mưa (%) |
| V5 | App → ESP32 | Bật/tắt chế độ đèn ngủ |

## 🧠 Cách hoạt động

- Quét thẻ RFID liên tục mỗi vòng lặp → so khớp UID với "MASTER_ID" → đúng thì mở khóa (servo kéo chốt → giữ mở 5 giây → tự đẩy chốt khóa lại, chạy nền qua "door_Update()"), sai thì hiện cảnh báo trên TFT và bắn thông báo "unknown_card" kèm UID lên Blynk
- Sau khi quét thẻ (đúng hoặc sai), màn hình giữ thông báo 3 giây rồi tự xóa và quay lại trạng thái chờ — không chặn các tác vụ khác trong lúc chờ
- LED RGB đổi màu theo logic ưu tiên ở sơ đồ trên (cháy > có người > đèn ngủ > trời tối > vừa rời đi > tắt)
- Gas > 40 hoặc mưa > 70% sẽ bắn thông báo cảnh báo lên Blynk, tối đa 1 lần/phút mỗi loại để tránh spam
- Cứ mỗi 3 giây, dữ liệu (số lần mở cửa, nhiệt độ, độ ẩm, gas, mưa) được đẩy lên Blynk App
- TFT cập nhật liên tục: nhiệt độ, độ ẩm, mức gas, mức mưa, trạng thái chuyển động, UID thẻ vừa quét

## 📌 Trạng thái hiện tại / Hạn chế

- Module loa cảnh báo (I2S) đã viết và khởi tạo (`audio_Init()`) nhưng **chưa được gọi trong `loop()`** — do lần trước sụt áp ảnh hưởng màn TFT khi loa kêu to, cần nguồn riêng và test kỹ hơn trước khi bật lại
- **`BLYNK_AUTH_TOKEN` đang hard-code trực tiếp trong `src/main.cpp`.** Vì repo đã public, token này đang lộ công khai — nên thu hồi/tạo token mới trên Blynk Console và tách token ra file riêng (thêm vào `.gitignore`) trước khi commit tiếp
