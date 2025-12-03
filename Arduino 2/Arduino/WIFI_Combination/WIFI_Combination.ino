#include <ESP8266WiFi.h>

// Thay đổi tên và mật khẩu của hai mạng wifi
const char* ssid1 = "VNU-IS 501"; // Mạng wifi muốn kết nối (station)
const char* password1 = "Vnu.edu.vn";
const char* ssid2 = "ESP8266_APG"; // Mạng wifi muốn phát ra (AP)
const char* password2 = "12345678";

// Thiết lập địa chỉ IP, gateway và subnet mask cho mạng wifi (AP)
IPAddress ip(192, 168, 10, 1);
IPAddress gateway(192, 168, 10, 7);
IPAddress subnet(255, 255, 255, 0);

void setup() {
  Serial.begin(115200); // Khởi tạo cổng nối tiếp
  WiFi.mode(WIFI_AP_STA); // Thiết lập chế độ station + AP
  WiFi.begin(ssid1, password1); // Kết nối với mạng wifi có tên và mật khẩu cho trước (station)
  Serial.print("\nConnecting");
  while (WiFi.status() != WL_CONNECTED) { // Chờ cho đến khi kết nối thành công
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Station connected, IP address: ");
  Serial.println(WiFi.localIP()); // In ra địa chỉ IP của ESP8266
  
  WiFi.softAPConfig(ip, gateway, subnet); // Thiết lập địa chỉ IP, gateway và subnet mask cho mạng wifi (AP)
  WiFi.softAP(ssid2, password2); // Phát một mạng wifi có tên và mật khẩu cho trước (AP)
  Serial.print("Access point started, IP address: ");
  Serial.println(WiFi.softAPIP()); // In ra địa chỉ IP của ESP8266 trong mạng wifi phát ra (AP)
}

void loop() {
  Serial.print("Number of connected devices: ");
  Serial.print(WiFi.softAPgetStationNum());
  delay(3000);
  // Đoạn code trong hàm loop() sẽ được lặp đi lặp lại
  // Bạn có thể viết code để giao tiếp với các thiết bị trong hai mạng wifi
}