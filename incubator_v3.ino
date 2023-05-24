#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal_I2C.h>
#include <esp_system.h>

// Конфигурируем порты подключения датчиков
#define ONE_WIRE_BUS_1 25 // Первый датчик подключен к порту 25
#define ONE_WIRE_BUS_2 26 // Второй датчик подключен к порту 26
#define ONE_WIRE_BUS_3 32 // Третий датчик подключен к порту 32
#define ONE_WIRE_BUS_4 33 // Четвертый датчик подключен к порту 33

// Порты подключения реле
#define RELAY_1_PIN 19
#define RELAY_2_PIN 18
#define RELAY_3_PIN 23
#define RELAY_4_PIN 5

// Инициализируем объекты для работы с библиотеками OneWire и DallasTemperature
OneWire oneWire1(ONE_WIRE_BUS_1);
OneWire oneWire2(ONE_WIRE_BUS_2);
OneWire oneWire3(ONE_WIRE_BUS_3);
OneWire oneWire4(ONE_WIRE_BUS_4);
DallasTemperature sensors1(&oneWire1);
DallasTemperature sensors2(&oneWire2);
DallasTemperature sensors3(&oneWire3);
DallasTemperature sensors4(&oneWire4);

// Инициализируем объекты для работы с реле
int relay1_state = LOW;
int relay2_state = LOW;
int relay3_state = LOW;
int relay4_state = LOW;

// Установка температуры и коэффициенты регулятора
const float Setpoint = 88.0;
const float Kp = 1.2;
const float Ki = 0.0001;
const float Kd = 7;

// Инициализация переменных для PID-регуляции
float last_Input1, last_Input2, last_Input3, last_Input4;
float Input1, Input2, Input3, Input4;
float Output1, Output2, Output3, Output4;
float last_error1 = 0, last_error2 = 0, last_error3 = 0, last_error4 = 0;
float integral1 = 0, integral2 = 0, integral3 = 0, integral4 = 0;

// Инициализирую LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();

// Задаем порты реле на выходы
  pinMode(RELAY_1_PIN, OUTPUT);
  pinMode(RELAY_2_PIN, OUTPUT);
  pinMode(RELAY_3_PIN, OUTPUT);
  pinMode(RELAY_4_PIN, OUTPUT);

  // Запускаем датчики температуры
  sensors1.begin();
  sensors2.begin();
  sensors3.begin();
  sensors4.begin();
}

void loop() {
  // Читаем показания с датчиков температуры
  sensors1.requestTemperatures();
  sensors2.requestTemperatures();
  sensors3.requestTemperatures();
  sensors4.requestTemperatures();

  Input1 = sensors1.getTempCByIndex(0);
  Input2 = sensors2.getTempCByIndex(0);
  Input3 = sensors3.getTempCByIndex(0);
  Input4 = sensors4.getTempCByIndex(0);

  // Проверяем, если температура на датчике равна -127, то присваиваем ей предыдущее значение температуры
  if (Input1 == -127) {
  Input1 = last_Input1;
  }
  if (Input2 == -127) {
  Input2 = last_Input2;
  }
  if (Input3 == -127) {
  Input3 = last_Input3;
  }
  if (Input4 == -127) {
  Input4 = last_Input4;
  }

  // Запоминаем последние значения температур
  last_Input1 = Input1;
  last_Input2 = Input2;
  last_Input3 = Input3;
  last_Input4 = Input4;
  
//защита от перегрева  
  if (Input1 >= 89.0 || Input2 >= 89.0 || Input3 >= 89.0 || Input4 >= 89.0) {
    digitalWrite(RELAY_1_PIN, LOW);
    digitalWrite(RELAY_2_PIN, LOW);
    digitalWrite(RELAY_3_PIN, LOW);
    digitalWrite(RELAY_4_PIN, LOW);
    delay(1500);
    esp_restart();
  }


  // Рассчитываем ошибку регулирования
  float error1 = Setpoint - Input1;
  float error2 = Setpoint - Input2;
  float error3 = Setpoint - Input3;
  float error4 = Setpoint - Input4;

// Рассчитываем интегральную составляющую регулятора
  integral1 += error1;
  integral2 += error2;
  integral3 += error3;
  integral4 += error4;

// Рассчитываем дифференциальную составляющую регулятора
  float derivative1 = error1 - last_error1;
  float derivative2 = error2 - last_error2;
  float derivative3 = error3 - last_error3;
  float derivative4 = error4 - last_error4;

// Рассчитываем выходные значения регулятора
  Output1 = Kp * error1 + Ki * integral1 + Kd * derivative1;
  Output2 = Kp * error2 + Ki * integral2 + Kd * derivative2;
  Output3 = Kp * error3 + Ki * integral3 + Kd * derivative3;
  Output4 = Kp * error4 + Ki * integral4 + Kd * derivative4;

// Обновляем состояние реле в соответствии с выходными значениями регулятора
  if (Output1 > 0) {
    relay1_state = HIGH;
  } else {
    relay1_state = LOW;
  }

  if (Output2 > 0) {
    relay2_state = HIGH;
  } else {
    relay2_state = LOW;
  }

  if (Output3 > 0) {
    relay3_state = HIGH;
  } else {
    relay3_state = LOW;
  }

  if (Output4 > 0) {
    relay4_state = HIGH;
  } else {
    relay4_state = LOW;
  }

// Обновляем значение ошибок для дифференциальной составляющей
  last_error1 = error1;
  last_error2 = error2;
  last_error3 = error3;
  last_error4 = error4;

// Обновляем LCD-дисплей
  lcd.setCursor(0, 0);
  lcd.print("1:");
  lcd.print(Input1, 1);
  lcd.write(0xDF);
  //lcd.print("C");
  lcd.setCursor(0, 1);
  lcd.print("2:");
  lcd.print(Input2, 1);
  lcd.write(0xDF);
  //lcd.print("C");

  lcd.setCursor(8, 0);
  lcd.print("3:");
  lcd.print(Input3, 1);
  lcd.write(0xDF);
  //lcd.print("C");
  lcd.setCursor(8, 1);
  lcd.print("4:");
  lcd.print(Input4, 1);
  lcd.write(0xDF);
  //lcd.print("C");

  Serial.print("T1=");
  Serial.print(Input1, 0);
  Serial.print("T2=");
  Serial.print(Input2, 0);
  Serial.print("T3=");
  Serial.print(Input3, 0);
  Serial.print("T4=");
  Serial.print(Input4, 0);

  // Управляем реле в соответствии с их текущим состоянием
  digitalWrite(RELAY_1_PIN, relay1_state);
  digitalWrite(RELAY_2_PIN, relay2_state);
  digitalWrite(RELAY_3_PIN, relay3_state);
  digitalWrite(RELAY_4_PIN, relay4_state);

delay(1500);
}