#define PIN_IN1   D3    // GPIO0  - Điều khiển chiều 1
#define PIN_IN2   D4    // GPIO2  - Điều khiển chiều 2
#define PIN_ENA   D7    // GPIO13 - PWM tốc độ
#define PIN_BTN_UP   D5 // GPIO14 - Tăng tốc
#define PIN_BTN_DOWN D6 // GPIO12 - Giảm tốc

// ================== BIẾN TOÀN CỤC ==================
int speedPWM = 0;           // Tốc độ hiện tại (0-1023)
int step = 50;              // Bước tăng/giảm tốc độ
bool currentDir = true;     // true = thuận, false = ngược

unsigned long lastBtnTime = 0;
const long debounceDelay = 200;  // Chống dội nút (ms)

// ================== SETUP ==================
void setup() {
  // Cấu hình chân
  pinMode(PIN_IN1, OUTPUT);
  pinMode(PIN_IN2, OUTPUT);
  pinMode(PIN_ENA, OUTPUT);
  pinMode(PIN_BTN_UP, INPUT_PULLUP);
  pinMode(PIN_BTN_DOWN, INPUT_PULLUP);

  // Dừng motor ban đầu
  stopMotor();

  // Serial để debug
  Serial.begin(9600);
  Serial.println("=== ESP8266 + L298N Motor Control ===");
  Serial.println("D5: Tang toc | D6: Giam toc | D3+D4 cung luc: Dao chieu");
  Serial.println("Bat dau voi toc do = 0");
}

// ================== LOOP CHÍNH ==================
void loop() {
  // Đọc trạng thái nút (LOW khi nhấn do pull-up)
  bool btnUp = (digitalRead(PIN_BTN_UP) == LOW);
  bool btnDown = (digitalRead(PIN_BTN_DOWN) == LOW);
  bool bothPressed = btnUp && btnDown;

  unsigned long currentTime = millis();

  // Xử lý nhấn cả 2 nút → ĐẢO CHIỀU
  if (bothPressed && (currentTime - lastBtnTime > debounceDelay)) {
    toggleDirection();
    lastBtnTime = currentTime;
    delay(300); // Chống lặp nhanh
    return;
  }

  // Xử lý nút tăng tốc
  if (btnUp && !btnDown && (currentTime - lastBtnTime > debounceDelay)) {
    increaseSpeed();
    lastBtnTime = currentTime;
  }

  // Xử lý nút giảm tốc
  if (btnDown && !btnUp && (currentTime - lastBtnTime > debounceDelay)) {
    decreaseSpeed();
    lastBtnTime = currentTime;
  }

  // Cập nhật motor theo tốc độ và chiều
  updateMotor();
  
  delay(10); // Giảm tải CPU
}

// ================== HÀM HỖ TRỢ ==================

void updateMotor() {
  if (speedPWM == 0) {
    stopMotor();
  } else {
    if (currentDir) {
      forward(speedPWM);
    } else {
      backward(speedPWM);
    }
  }
  Serial.printf("Toc do: %4d | Chieu: %s\n", speedPWM, currentDir ? "THUAN" : "NGUOC");
}

void forward(int pwm) {
  digitalWrite(PIN_IN1, HIGH);
  digitalWrite(PIN_IN2, LOW);
  analogWrite(PIN_ENA, pwm);
}

void backward(int pwm) {
  digitalWrite(PIN_IN1, LOW);
  digitalWrite(PIN_IN2, HIGH);
  analogWrite(PIN_ENA, pwm);
}

void stopMotor() {
  digitalWrite(PIN_IN1, LOW);
  digitalWrite(PIN_IN2, LOW);
  analogWrite(PIN_ENA, 0);
}

void increaseSpeed() {
  speedPWM += step;
  if (speedPWM > 1023) speedPWM = 1023;
  Serial.printf("=> Tang toc do: %d\n", speedPWM);
}

void decreaseSpeed() {
  speedPWM -= step;
  if (speedPWM < 0) speedPWM = 0;
  Serial.printf("=> Giam toc do: %d\n", speedPWM);
}

void toggleDirection() {
  currentDir = !currentDir;
  Serial.printf("=== DAO CHIEU => %s ===\n", currentDir ? "THUAN" : "NGUOC");
}