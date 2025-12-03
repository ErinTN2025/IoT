#include <ESP8266WiFi.h>

// Thay đổi tên và mật khẩu của mạng wifi
const char* ssid = "AT";
const char* password = "At170899";

void setup() {
  Serial.begin(115200); // Khởi tạo cổng nối tiếp
  WiFi.mode(WIFI_STA); // Thiết lập chế độ station
  WiFi.begin(ssid, password); // Kết nối với mạng wifi
  Serial.print("\nConnecting");
  while (WiFi.status() != WL_CONNECTED) { // Chờ cho đến khi kết nối thành công
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP()); // In ra địa chỉ IP của ESP8266
}

void loop() {
  delay(5000);
  // Close the connection
  WiFi.disconnect();
}
