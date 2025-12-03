#include <ESP8266WiFi.h>

// Thay đổi tên và mật khẩu của mạng wifi
const char* ssid = "ESP8266_AP";
const char* password = "12345678";

// Thiết lập địa chỉ IP, gateway và subnet mask
IPAddress ip(192, 168, 180, 1);
IPAddress gateway(192, 168, 180, 1);
IPAddress subnet(255, 255, 255, 0);

void setup() {
  Serial.begin(115200); // Khởi tạo cổng nối tiếp
  WiFi.mode(WIFI_AP); // Thiết lập chế độ access point
  WiFi.softAPConfig(ip, gateway, subnet); // Thiết lập địa chỉ IP, gateway và subnet mask
  WiFi.softAP(ssid, password); // Phát một mạng wifi có tên và mật khẩu cho trước
  Serial.print("\nAccess point started, IP address: ");
  Serial.println(WiFi.softAPIP()); // In ra địa chỉ IP của ESP8266
}

void loop() {
  Serial.print("Number of Connected devices: ");
  Serial.println(WiFi.softAPgetStationNum());
  delay(1000);
  // Đoạn code trong hàm loop() sẽ được lặp đi lặp lại
  // Bạn có thể viết code để giao tiếp với các thiết bị kết nối vào mạng wifi
}
