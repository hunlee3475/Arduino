#include <MFRC522v2.h>
#include <MFRC522DriverPinSimple.h>
#include <MFRC522DriverSPI.h>
#include <MFRC522Debug.h>
#include <LiquidCrystal_I2C.h>
#include <IRremote.hpp>
#include "DHT.h"

enum PINS{
  SERVO_PIN = 4U,
  RED,
  GREEN,
  LED_PIN,
  PIR_SENSOR,
  IR_PIN,
  BUZZ,
  WATER_SENSOR = A0,
  TEMPER_HUMID = A1
};

class decode_results result; 
class MFRC522DriverPinSimple sda_pin(53);
class MFRC522DriverSPI driver {sda_pin};
class MFRC522 mfrc522 {driver};
class LiquidCrystal_I2C lcd(0x27, 16, 2); 
class IRrecv irrecv(IR_PIN);
class DHT dht(TEMPER_HUMID, 11);
bool pir_state {false};
const String MASTER_CARD_UID1{String("F14C751B")};
const String MASTER_CARD_UID2{String("E7548B7B")};
static uint8_t different_card_count =0;
bool button_pressed = false;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200UL);
  mfrc522.PCD_Init();
  lcd.init();
  lcd.noBacklight();
  pinMode(LED_BUILTIN, OUTPUT);
  irrecv.begin(IR_PIN, LED_BUILTIN);
  pinMode(BUZZ, OUTPUT);
  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(SERVO_PIN, OUTPUT);
  pinMode(PIR_SENSOR, INPUT);
  pinMode(WATER_SENSOR, INPUT);
  dht.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
  bool detect = digitalRead(PIR_SENSOR);
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  uint16_t water_value = analogRead(WATER_SENSOR);
  delay(10UL);
 

  if(irrecv.decode()) { // Remote 버튼이 눌렸을 때 이 코드 실행
    uint8_t data_value = irrecv.decodedIRData.command;
    Serial.println(data_value, HEX); // 16진수로 보여줌
    if (data_value == 0x16 && !button_pressed) { // 0번 버튼이 눌리고 아직 버튼이 눌리지 않았다면
      button_pressed = true; // 버튼이 눌렸음을 표시
      lcd.backlight();
      lcd.setCursor(0, 0);
      lcd.print("welcome to");
      lcd.setCursor(0, 1);
      lcd.print("visit");
      analogWrite(GREEN, 255);
      analogWrite(SERVO_PIN, 255);
      delay(10000UL);
      analogWrite(SERVO_PIN, 0);
      analogWrite(GREEN, 0);
      lcd.init();
      lcd.noBacklight();
    }
    else if(data_value == 0x0C && !button_pressed)
    {
    lcd.backlight();
    lcd.setCursor(0,0);
    float temperature = dht.readTemperature();
    lcd.println(String("temp : ") +String(temperature) + String("C   "));
    float humidity = dht.readHumidity();
    lcd.setCursor(0,1);
    lcd.println(String("humidity:") +String(humidity) +String("% "));
    delay(2000UL);
    lcd.clear();
    }
    else if(data_value == 0x08 && !button_pressed)
    {
      if(water_value > 200)
      {
        lcd.backlight();
        lcd.setCursor(0,0);
        lcd.print("It's raining");
        delay(2000UL);
        lcd.clear();
      }
      else
      {
        lcd.backlight();
        lcd.setCursor(0,0);
        lcd.print("it doesn't rain");
        delay(2000UL);
        lcd.clear();
      }
    }
    irrecv.resume();
    button_pressed = false;
  }

  if(!mfrc522.PICC_IsNewCardPresent()) return;
  if(!mfrc522.PICC_ReadCardSerial()) return;
  String tagID = ""; // 빈문자열
  for(uint8_t i {0u}; i <4; ++i)
  {
    tagID += String(mfrc522.uid.uidByte[i], HEX);
  }
  tagID.toUpperCase(); //소문자를 대문자로 변환
  mfrc522.PICC_HaltA(); //UID 만 알면 되므로 나머지는 멈추세요
 
  if(tagID == MASTER_CARD_UID1)
  {
    Serial.println(F("카드일치 문이 열립니다"));
    different_card_count = 0;
    analogWrite(GREEN, 255);
    lcd.backlight();
    lcd.setCursor(0,0);
    lcd.print("Welcome home");
    lcd.setCursor(0,1);
    lcd.print("#101");
    analogWrite(SERVO_PIN, 255);
    delay(10000UL);
    analogWrite(SERVO_PIN, 0);
    analogWrite(GREEN, 0);
    lcd.init();
    lcd.noBacklight();
  }
  else if(tagID == MASTER_CARD_UID2)
  {
    Serial.println(F("카드일치 문이 열립니다"));
    different_card_count = 0;
    analogWrite(GREEN, 255);
    lcd.backlight();
    lcd.setCursor(0,0);
    lcd.print("Welcome home");
    lcd.setCursor(0,1);
    lcd.print("#102");
    analogWrite(SERVO_PIN, 255);
    delay(10000UL);
    analogWrite(SERVO_PIN, 0);
    analogWrite(GREEN, 0);
    lcd.init();
    lcd.noBacklight();
  }
  else{
    Serial.println(F("카드가 일치하지 않습니다"));
    analogWrite(RED, 255);
    lcd.backlight();
    lcd.setCursor(0,0);
    lcd.print("Please try again");
    lcd.setCursor(0,1);
    lcd.print("different card");
    ++different_card_count;
    delay(3000UL);
    lcd.clear();
    analogWrite(RED, 0);
    if(different_card_count >= 3)
    {
      Serial.println(F("불법침입 경고!"));
      analogWrite(RED, 255);
      tone(BUZZ, 50);
      delay(5000UL);
      analogWrite(RED, 0);
      noTone(BUZZ);
      different_card_count = 0;
    }
  }

}
