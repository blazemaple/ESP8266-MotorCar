#include "WEMOS_Motor.h"

int pwm_left = 0;         //左輪馬力
int pwm_right = 0;        //右輪馬力

//Motor shiled I2C Address: 0x30
//PWM frequency: 1000Hz(1kHz)
Motor M_Left(0x30,_MOTOR_A, 1000);
Motor M_Right(0x30,_MOTOR_B, 1000);

unsigned int Lcount = 0;  //左輪計數
unsigned int Rcount = 0;  //右輪計數
int LcounterPin = 12;     //左輪中斷腳位
int RcounterPin = 14;     //右輪中斷腳位
unsigned long time2;
int Lrpm = 0;             //左輪轉速
int Rrpm = 0;             //右輪轉速
uint8_t grid_num = 20;    //碼盤網格數

void IRAM_ATTR Lcounter() {
   Lcount++;
}

void IRAM_ATTR Rcounter() {
   Rcount++;
}

void getRPM() {
  //millis()取得目前時間(ms)
  if (millis() - time2 >= 1000){   /* 每秒更新 */
    // 計算 rpm 時，停止計時
    noInterrupts();

    // 偵測的格數count * (60 * 1000 / 一圈網格數20）/ 時間差) 
    Lrpm = 0; //TODO: Please complete this section
    Rrpm = 0; //TODO: Please complete this section
    time2 = millis();   //紀錄這次計算的時間(ms)
    Lcount = 0;
    Rcount = 0;

    Serial.printf("LRPM:%d\n", Lrpm);
    Serial.printf("RRPM:%d\n", Rrpm);
    //Restart the interrupt processing
    interrupts();
  }
}

void move(int LSpeed, int RSpeed) {
    if (LSpeed >= 0) {
        M_Left.setmotor(_CW, LSpeed);
    } else if (LSpeed < 0) {
        M_Left.setmotor(_CCW, abs(LSpeed));
    }

    if (RSpeed >= 0) {
        M_Right.setmotor(_CW, RSpeed);
    } else if (RSpeed < 0) {
        M_Right.setmotor(_CCW, abs(RSpeed));
    }
}

void setup()
{
  Serial.begin(115200);
  pinMode(LcounterPin, INPUT);  //宣告腳位模式為輸入
  pinMode(RcounterPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(LcounterPin), Lcounter, FALLING); //當電位下降時中斷一次
  attachInterrupt(digitalPinToInterrupt(RcounterPin), Rcounter, FALLING);
}

void loop()
{
  move(pwm_left, pwm_right);
  getRPM();
}
