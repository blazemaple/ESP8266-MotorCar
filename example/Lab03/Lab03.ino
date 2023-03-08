#include "WEMOS_Motor.h"

int pwm_left;
int pwm_right;

//Motor shiled I2C Address: 0x30
//PWM frequency: 1000Hz(1kHz)
Motor M_Left(0x30,_MOTOR_A, 1000);
Motor M_Right(0x30,_MOTOR_B, 1000);
#define leftPin 12
#define rightPin 16
uint8_t  flagL = 0 ;  //左輪狀態
uint8_t  flagR = 0;   //右輪狀態

int valL = 0;    //左輪脈衝值
int valR = 0;    //右輪脈衝值

int speedL = 0;  //左輪速度
int speedR = 0;  //右輪速度

void constantSpeed(String carMode, float lRotating, float rRotating)
{
    int d_time = 20;
    int last_time = millis();
    int now_time = millis();

    while (now_time - last_time < d_time) {
        if (flagL == 0 && digitalRead(leftPin) == 1) {
            valL += 1;
            flagL = 1;
        }
        if (flagL == 1 && digitalRead(leftPin) == 0) {
            valL += 1;
            flagL = 0;
        }
        if (flagR == 0 && digitalRead(rightPin) == 1) {
            valR += 1;
            flagR = 1;
        }
        if (flagR == 1 && digitalRead(rightPin) == 0) {
            valR += 1;
            flagR = 0;
        }

        now_time = millis();
    }

    float l = valL / d_time;
    float r = valR / d_time;

    if (l <= lRotating) {
        speedL += 1;
    }
    if (l >= lRotating + 0.02) {
        speedL -= 1;
    }
    if (r <= rRotating) {
        speedR += 1;
    }
    if (r >= rRotating + 0.02) {
        speedR -= 1;
    }

    if (speedL > 100) {
        speedL = 100;
    }
    if (speedL < 0) {
        speedL = 0;
    }
    if (speedR > 100) {
        speedR = 100;
    }
    if (speedR < 0) {
        speedR = 0;
    }

    uint8_t lDirection = 1;   // -1為後退、1為前進、0為停止
    uint8_t rDirection = 1;   // -1為後退、1為前進、0為停止

    if (carMode == "forward") {
        lDirection = 1;
        rDirection = 1;
    } else if (carMode = "backward") {
        lDirection = -1;
        rDirection = -1;
    } else if (carMode = "stop") {
        lDirection = 0;
        rDirection = 0;
        speedR = 0;
        speedL = 0;
    } else if (carMode = "left") {
        lDirection = -1;
        rDirection = 1;
    } else if (carMode = "right") {
        lDirection = 1;
        rDirection = -1;
    }

    move(lDirection * speedL, rDirection * speedR);

    valL = 0;
    valR = 0;
}

void move(int LSpeed, int RSpeed) {
    if (LSpeed >= 0) {
        M_Left.setmotor(_CW, LSpeed);
    } else if (LSpeed < 0) {
        M_Left.setmotor(_CCW, abs(LSpeed));
    }

    if (RSpeed >= 0) {
        M_Right.setmotor(_CW, RSpeed);
    } else if (LSpeed < 0) {
        M_Right.setmotor(_CCW, abs(RSpeed));
    }
}

void setup()
{
    Serial.begin(115200);
    pinMode(leftPin, INPUT); //左輪感測器
    pinMode(rightPin, INPUT); //右輪感測器
}

void loop()
{
    Serial.println("forward");
    constantSpeed("forward", 0.05, 0.05);
}
