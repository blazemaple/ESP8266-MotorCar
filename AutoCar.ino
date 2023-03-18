#include "WEMOS_Motor.h"

int pwm_left = 0;
int pwm_right = 0;
String cmd = "";
//Motor shiled I2C Address: 0x30
//PWM frequency: 1000Hz(1kHz)
Motor M_Left(0x30,_MOTOR_A, 1000);
Motor M_Right(0x30,_MOTOR_B, 1000);

unsigned int Lcount = 0;
unsigned int Rcount = 0;
int LcounterPin = 14;
int RcounterPin = 12;
unsigned long time2;
int Lrpm = 0;
int Rrpm = 0;
uint8_t grid_num = 20;

void IRAM_ATTR Lcounter() {
   Lcount++;
}

void IRAM_ATTR Rcounter() {
   Rcount++;
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

void setup() {
   Serial.begin(115200);
   pinMode(LcounterPin, INPUT);
   pinMode(RcounterPin, INPUT);
   attachInterrupt(digitalPinToInterrupt(LcounterPin), Lcounter, FALLING);
   attachInterrupt(digitalPinToInterrupt(RcounterPin), Rcounter, FALLING);
   
}

void loop() {
  String temp = "";
  if (Serial.available()) {      // If anything comes in Serial (USB),
    cmd = Serial.readString();
    if (cmd.startsWith("AT+setLpwm=")) {    //set left motor speed
      temp = cmd.substring(cmd.indexOf("=") + 1, cmd.length());
      // Serial.println(temp.toInt());
      if (temp.toInt() > 100) {
        pwm_left = 100;
      } else {
        pwm_left = temp.toInt();
      }
      Serial.println("ok");
    } else if (cmd.startsWith("AT+setRpwm=")) {    //set right motor speed
      temp = cmd.substring(cmd.indexOf("=") + 1, cmd.length());
      // Serial.println(temp.toInt());
      if (temp.toInt() > 100) {
        pwm_right = 100;
      } else {
        pwm_right = temp.toInt();
      }
      Serial.println("ok");
    } else if (cmd == "AT+getLpwm") {     //get left motor speed
      Serial.println(pwm_left);
    } else if (cmd == "AT+getRpwm") {     //get right motor speed
      Serial.println(pwm_right);
    } else if (cmd == "AT+getLrpm") {     //get left wheel RPM
      Serial.println(Lrpm);
    } else if (cmd == "AT+getRrpm") {     //get right wheel RPM
      Serial.println(Rrpm);
    } else if (cmd == "AT+stop") {      //set motors to stop
      pwm_left = 0;
      pwm_right = 0;
      Serial.println("ok");
    }
  }

  move(pwm_left, pwm_right);

  if (millis() - time2 >= 1000){   /* 每秒更新 */
      // 計算 rpm 時，停止計時
      noInterrupts();

      // 偵測的格數count * (60 * 1000 / 一圈網格數20）/ 時間差) 
      Lrpm = (60 * 1000 / grid_num )/ (millis() - time2) * Lcount;
      Rrpm = (60 * 1000 / grid_num )/ (millis() - time2) * Rcount;
      time2 = millis();
      Lcount = 0;
      Rcount = 0;

      //Restart the interrupt processing
      interrupts();
  }
}
