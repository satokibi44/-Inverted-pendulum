#include <Arduino.h>
#include "MPU6050_6Axis_MotionApps20.h"
#include "Wire.h"
#include <KalmanFilter.h>

MPU6050 mpu6050;
MPU6050 initialize;

#define GYR_GAIN 20.0
#define SAMPLE_TIME 10.0
#define STOP_ANGLE 15

KalmanFilter kalmanX(0.001, 0.003, 0.03);

unsigned long preTime = 0;
unsigned long lastTime;

float angleFiltered, angleFilteredOffset;
float input, output;
float pEffect, iEffect, dEffect, lastpEffect;
float motorPWM;
int timeChange; 

// motor pin
int MOTOR1_PWM = 9;
int MOTOR1_IN1 = 8;
int MOTOR1_IN2 = 7;
int MOTOR2_PWM = 10;
int MOTOR2_IN1 = 12;
int MOTOR2_IN2 = 11;
//ジャイロセンサのPIN
int STBY_PIN = 6;

bool motor_stop = false;
//aaa
// フィルタ処理
void filter() {
    int16_t ax, ay, az, gx, gy, gz;
    mpu6050.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

    float angleX = (atan2(ay, az) * 180 / M_PI);
    float omega =  gx / GYR_GAIN;
    unsigned long now = millis();
    float dt = (now - preTime) / 1000.00;
    preTime = now;
    float K = 0.8;
    float A = K / (K + dt);
    angleFiltered = A * (angleFiltered + omega * dt) + (1 - A) * angleX;
}

// カルマンフィルタ
void kalmanFilter() {
    //ジャイロセンサの読み取り.(http://figrobo.blog.fc2.com/blog-entry-109.html)
    int16_t ax, ay, az, gx, gy, gz;
    mpu6050.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

    float angleX = (atan2(ay, az) * 180 / M_PI);
    //よくわからんけど計測した角度??
    angleFiltered = kalmanX.update(angleX, gx / GYR_GAIN);
}

// PID処理
void pidPorcess() { 
  float kp = 15;
  float kd = 150;
  float ki = 0.1;

  // 計測時間チェック
  unsigned long now = millis();
    //dt
  timeChange = (now - lastTime);

  // PID処理
  if(timeChange >= SAMPLE_TIME){
    input = angleFiltered - angleFilteredOffset;
    pEffect = input;
    iEffect += pEffect * timeChange;
    dEffect = (pEffect - lastpEffect) / timeChange;
    output = kp * pEffect + ki * iEffect + kd * dEffect;
      
    //PIDした結果のトルクT
    motorPWM = output ;
    
    lastpEffect = pEffect;
    lastTime = now;
  }
}

void setup() {

    // ピンの初期化
    pinMode(MOTOR1_IN1, OUTPUT);
    pinMode(MOTOR1_IN2, OUTPUT);
    pinMode(MOTOR2_IN1, OUTPUT);
    pinMode(MOTOR2_IN2, OUTPUT);
    pinMode(STBY_PIN, OUTPUT);

    digitalWrite(STBY_PIN, HIGH);

    // ジャイロセンサMPU6050初期化
    Wire.begin();
    Serial.begin(115200);
    mpu6050.initialize();
    initialize.setFullScaleGyroRange(MPU6050_GYRO_FS_2000);;

    // 初期位置のチェック
    for (int i = 0; i < 1000; i++) {
       kalmanFilter();
       angleFilteredOffset = angleFiltered;
    }

    // 最初のPID処理
    //abs():絶対値
    if (abs(angleFiltered) < STOP_ANGLE)    {
        angleFiltered = 0;
        kalmanFilter();
        output = pEffect = iEffect = dEffect = 0;
        pidPorcess();
    }
}

void loop() {

    // フィルタ処理
    kalmanFilter();

    // 転倒チェック.転倒していなかった場合
    if (abs(angleFiltered) < STOP_ANGLE && !motor_stop) {
        pidPorcess();
        Serial.print(motorPWM); Serial.print("\t");
        Serial.print(angleFiltered);
        Serial.print("\n");
        //転倒していた場合.
    } else if (abs(angleFiltered) >= STOP_ANGLE) {
        motor_stop = true;
    }

    // 転倒時はモーター停止
    if (motor_stop) {
        digitalWrite(MOTOR1_IN1, LOW);
        digitalWrite(MOTOR1_IN2, LOW);
        digitalWrite(MOTOR2_IN1, LOW);
        digitalWrite(MOTOR2_IN2, LOW);
    }
    //モーターが順回転の時
    else if (motorPWM > 0) {
        digitalWrite(MOTOR1_IN1, LOW);
        digitalWrite(MOTOR1_IN2, HIGH);
        digitalWrite(MOTOR2_IN1, LOW);
        digitalWrite(MOTOR2_IN2, HIGH);
    }//モーターが逆回転の時
    else if (motorPWM < 0) {
        digitalWrite(MOTOR1_IN1, HIGH);
        digitalWrite(MOTOR1_IN2, LOW);
        digitalWrite(MOTOR2_IN1, HIGH);
        digitalWrite(MOTOR2_IN2, LOW);
    }
    analogWrite(MOTOR1_PWM, min(128, abs(motorPWM)));
    analogWrite(MOTOR2_PWM, min(128, abs(motorPWM)));
}
