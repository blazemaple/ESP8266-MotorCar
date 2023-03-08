#include "WEMOS_Motor.h"

int pwm_left;
int pwm_right;

//Motor shiled I2C Address: 0x30
//PWM frequency: 1000Hz(1kHz)
Motor M_Left(0x30,_MOTOR_A, 1000);
Motor M_Right(0x30,_MOTOR_B, 1000);

unsigned int Lcount = 0;
unsigned int Rcount = 0;
int LcounterPin = 12;
int RcounterPin = 14;
unsigned long time2;
unsigned int Lrpm;
unsigned int Rrpm;
unsigned int grid_num = 20;

void IRAM_ATTR Lcounter() {
   Lcount++;
}

void IRAM_ATTR Rcounter() {
   Rcount++;
}

void setup() {
   Serial.begin(115200);
   pinMode(LcounterPin, INPUT);
   pinMode(RcounterPin, INPUT);
   attachInterrupt(digitalPinToInterrupt(LcounterPin), Lcounter, FALLING);
   attachInterrupt(digitalPinToInterrupt(RcounterPin), Rcounter, FALLING);
   
   Serial.println("GO");
   M_Left.setmotor(_CW, 80);
   M_Right.setmotor(_CW, 80);
}

void loop() {
  if (Serial.available()) {      // If anything comes in Serial (USB),
    if (Serial.readString() == "forward") {
      Serial.println("GO");
      M_Left.setmotor(_CW, 80);
      M_Right.setmotor(_CW, 80);
    }
  }
  if (millis() - time2 >= 1000){   /* 每秒更新 */
      // 計算 rpm 時，停止計時
      noInterrupts();

      // 偵測的格數count * (60 * 1000 / 一圈網格數20）/ 時間差) 
      Lrpm = (60 * 1000 / grid_num )/ (millis() - time2) * Lcount;
      Rrpm = (60 * 1000 / grid_num )/ (millis() - time2) * Rcount;
      time2 = millis();
      Lcount = 0;
      Rcount = 0;

      // 輸出至Console
      Serial.print("LRPM = ");
      Serial.println(Lrpm,DEC);
      Serial.print("RRPM = ");
      Serial.println(Rrpm,DEC);
      //Restart the interrupt processing
      interrupts();
  }
}