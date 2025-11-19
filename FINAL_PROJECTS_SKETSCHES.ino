#include <MPU6050_tockn.h>
#include <Wire.h>
#include "Adafruit_MLX90640.h"

MPU6050 mpu6050(Wire);
Adafruit_MLX90640 mlx;

float frame[32 * 24]; // массив 768 пикселей

#define PIN_TRIG_vpred 27
#define PIN_ECHO_vpred 35
#define PIN_TRIG_slevo 14
#define PIN_ECHO_slevo 34
#define PIN_TRIG_spavo 25
#define PIN_ECHO_spavo 33

#define ENA 4
#define IN1 23
#define IN2 19
#define ENB 17
#define IN3 18
#define IN4 5

long duration, cm;
bool is_human = 0;
uint8_t count = 0;

void setup() {
  Wire.begin();
  Serial.begin(115200);
  
  // Инициализация ультразвуковых датчиков
  pinMode(PIN_TRIG_vpred, OUTPUT);
  pinMode(PIN_ECHO_vpred, INPUT);
  pinMode(PIN_TRIG_slevo, OUTPUT);
  pinMode(PIN_ECHO_slevo, INPUT);
  pinMode(PIN_TRIG_spavo, OUTPUT);
  pinMode(PIN_ECHO_spavo, INPUT);
  
  // Инициализация драйвера моторов
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  
  // Инициализация MLX90640
  Wire.begin(21, 22);      // SDA, SCL
  Wire.setClock(1000000);  // 1 MHz
  
  Serial.println("Инициализация MLX90640...");
  if (!mlx.begin()) {
    Serial.println("Ошибка: MLX90640 не найден!");
    while (1) delay(10);
  }
  
  mlx.setMode(MLX90640_CHESS);
  mlx.setRefreshRate(MLX90640_8_HZ);
  Serial.println("MLX90640 инициализирован успешно!");
  
  // Инициализация MPU6050
  mpu6050.begin();
  mpu6050.calcGyroOffsets();
}

void loop() {
  count = 0;
  mpu6050.update();
  
  if (mlx.getFrame(frame) != 0) {
    Serial.println("Ошибка чтения кадра MLX90640!");
    return;
  }
  
  is_human_func();
  
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  
  if (getDistance_vpered() <= 50 && !is_human){
    if ((getDistance_spravo() < getDistance_slevo()) && getDistance_slevo() < 200){
      left();
      stop_motors();
      delay(1000);
    
    }else if ((getDistance_spravo() > getDistance_slevo()) && getDistance_spravo() < 200){
      right();
      stop_motors();
      delay(1000);
      
      }
      
    }
  drive();
}

void is_human_func(){
  count = 0; // Сбрасываем счетчик
  
  // Анализируем тепловой кадр
  for (int y = 0; y < 24; y++) {
    for (int x = 0; x < 32; x++) {
      float temperature = frame[y * 32 + x];
      
      if (temperature > 30.0) { // Температура выше 30°C
        Serial.print("+");
        count++;
      } else {
        Serial.print(" ");
      }
    }
    Serial.println();
  }
  
  Serial.print("Теплых пикселей: ");
  Serial.println(count);
  Serial.println("-------------------------");
  
  if (count > 250) {
    is_human = 1;
    Serial.println("ЧЕЛОВЕК ОБНАРУЖЕН!");
  } else {
    is_human = 0;
  }
}


void right(){
  mpu6050.update();
  /////// поворот на 90
  while (mpu6050.getGyroAngleZ() < 90.0){
      mpu6050.update();
      Serial.println(mpu6050.getGyroAngleZ());

      digitalWrite(IN1, HIGH);
      digitalWrite(IN2, LOW);
  
      digitalWrite(IN3, LOW);
      digitalWrite(IN4, HIGH);
      analogWrite(ENA, 100);
      analogWrite(ENB, 100);
 
      
    }
    //////////////////////// прямо под углом 90
    Serial.println(mpu6050.getGyroAngleZ());
    unsigned long start_mil = millis();
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
    while (1) {
      
      mpu6050.update();
     
      Serial.println(mpu6050.getGyroAngleZ());

      // Проверяем расстояние каждую итерацию
      if (getDistance_slevo() >= 10.0 && millis() - start_mil > 1000) {
          break; // Выходим из цикла если расстояние >= 100 см
      }
      
      
      
      if(mpu6050.getGyroAngleZ() > 92.0) {
          while(mpu6050.getGyroAngleZ() > 92.0) {
              mpu6050.update();
              analogWrite(ENB, 0);
              analogWrite(ENA, 100);
          }
      }
      else if(mpu6050.getGyroAngleZ() < 88.0) {
          while(mpu6050.getGyroAngleZ() < 88.0) {
              mpu6050.update();
              analogWrite(ENA, 0);
              analogWrite(ENB, 100);
          }
      }
      else if (mpu6050.getGyroAngleZ() > 88.0 && mpu6050.getGyroAngleZ() < 92.0) {
          analogWrite(ENA, 120);
          analogWrite(ENB, 120);
      }
        
      delay(10);
  }
    //////////// конец
    while (mpu6050.getGyroAngleZ() > 2.0){
      mpu6050.update();
      
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, HIGH);
  
      digitalWrite(IN3, HIGH);
      digitalWrite(IN4, LOW);
      analogWrite(ENA, 120);
      analogWrite(ENB, 120);

      
    }

  
}


void left(){
  mpu6050.update();
    ///////// поворот на 90
  while (mpu6050.getGyroAngleZ() > -90.0){
      mpu6050.update();
      
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, HIGH);
  
      digitalWrite(IN3, HIGH);
      digitalWrite(IN4, LOW);
      analogWrite(ENA, 100);
      analogWrite(ENB, 100);
 
      
    }
    //////////////////////// прямо под углом 90
 
  unsigned long start_mil2 = millis();
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
  while (1) {
  
    mpu6050.update();
    
    // Проверяем расстояние каждую итерацию
    if (getDistance_spravo() >= 90.0 && (millis() - start_mil2 > 1000)) {
        
        break; // Выходим из цикла если расстояние >= 100 см
    }
    
    
    
    if(mpu6050.getGyroAngleZ() < -92.0) {
        while(mpu6050.getGyroAngleZ() < -92.0) {
            mpu6050.update();
        
            analogWrite(ENA, 0);
            analogWrite(ENB, 100);
        }
    }
    else if(mpu6050.getGyroAngleZ() > -88.0) {
        while(mpu6050.getGyroAngleZ() > -88.0) {
            mpu6050.update();
          
            analogWrite(ENA, 100);
            analogWrite(ENB, 0);
        }
    }
    else if (mpu6050.getGyroAngleZ() > -92.0 && mpu6050.getGyroAngleZ() < -88.0) {
        
     
        analogWrite(ENA, 120);
        analogWrite(ENB, 120);
    }
    
    delay(10);
    }
    //////////// конец
    while (mpu6050.getGyroAngleZ() < -2.0){
      mpu6050.update();
      
      digitalWrite(IN1, HIGH);
      digitalWrite(IN2, LOW);
  
      digitalWrite(IN3, LOW);
      digitalWrite(IN4, HIGH);
      analogWrite(ENA, 120);
      analogWrite(ENB, 120);}
  
}

void stop_motors(){
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
  
}


void drive(){
  mpu6050.update();  
  if(mpu6050.getGyroAngleZ() > 2.0){
    while(mpu6050.getGyroAngleZ() > 2.0) {
    mpu6050.update();
    analogWrite(ENB, 0);
    analogWrite(ENA, 100);
  }
    
  }else if(mpu6050.getGyroAngleZ() < -2.0){
    while(mpu6050.getGyroAngleZ() < -2.0) {
    mpu6050.update();
    analogWrite(ENA,  0);
    analogWrite(ENB, 100);}
    
    }
  else if (mpu6050.getGyroAngleZ() > -2.0 && mpu6050.getGyroAngleZ() < 2.0){
    while(mpu6050.getGyroAngleZ() > -2.0 && mpu6050.getGyroAngleZ() < 2.0) {
      mpu6050.update();
      analogWrite(ENA, 180);
      analogWrite(ENB, 180);
      }
    }
  
}

int getDistance_vpered(){
  digitalWrite(PIN_TRIG_vpred, LOW);
  delayMicroseconds(5);
  digitalWrite(PIN_TRIG_vpred, HIGH);

  delayMicroseconds(10);
  digitalWrite(PIN_TRIG_vpred, LOW);

  duration = pulseIn(PIN_ECHO_vpred, HIGH);

  cm = (duration / 2) / 29.1;

  return cm;
  
  }
int getDistance_slevo(){
  digitalWrite(PIN_TRIG_slevo, LOW);
  delayMicroseconds(5);
  digitalWrite(PIN_TRIG_slevo, HIGH);

  delayMicroseconds(10);
  digitalWrite(PIN_TRIG_slevo, LOW);

  duration = pulseIn(PIN_ECHO_slevo, HIGH);

  cm = (duration / 2) / 29.1;

  return cm;
  
  }
int getDistance_spravo(){
  digitalWrite(PIN_TRIG_spavo, LOW);
  delayMicroseconds(5);
  digitalWrite(PIN_TRIG_spavo, HIGH);

  delayMicroseconds(10);
  digitalWrite(PIN_TRIG_spavo, LOW);

  duration = pulseIn(PIN_ECHO_spavo, HIGH);

  cm = (duration / 2) / 29.1;

  return cm;
  
  }
