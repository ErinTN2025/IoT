#include <LiquidCrystal_I2C.h>
#include <wire.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
void setup(){
  lcd.init();
  lcd.backlight();
}

void loop(){
  lcd.clear();
  lcd.setCursor(6,0);
  lcd.print("IOT");
  lcd.setCursor(5,1);
  lcd.print("EC Dept");
  delay(2000);

  lcd.clear();
  lcd.setCursor(2,0);
  lcd.print("LCD I2C mODE");
  lcd.setCursor(6,1);
  lcd.print("dnjsa");
  delay(500);
}