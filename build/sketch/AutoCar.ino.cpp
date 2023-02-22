#include <Arduino.h>
#line 1 "D:\\Lab\\AutoCar\\AutoCar.ino"
#include "WEMOS_Motor.h"

int pwm_left;
int pwm_right;

//Motor shiled I2C Address: 0x30
//PWM frequency: 1000Hz(1kHz)
Motor M_Left(0x30,_MOTOR_A, 1000);
Motor M_Right(0x30,_MOTOR_B, 1000);

#line 11 "D:\\Lab\\AutoCar\\AutoCar.ino"
void setup();
#line 19 "D:\\Lab\\AutoCar\\AutoCar.ino"
void loop();
#line 11 "D:\\Lab\\AutoCar\\AutoCar.ino"
void setup()
{
  Serial.begin(115200);
	pinMode(2,OUTPUT);
  digitalWrite(2,HIGH);
  
}

void loop()
{
  Serial.println("GO Left");
  pwm_left = 30;
  pwm_right = 50;
  M_Left.setmotor(_CW, pwm_left);
  M_Right.setmotor(_CW, pwm_right);
	delay(1500);

  Serial.println("GO Right");
  pwm_left = 50;
  pwm_right = 30;
  M_Left.setmotor(_CW, pwm_left);
  M_Right.setmotor(_CW, pwm_right);
	delay(1500);
  /*
  Serial.println("BACK");
  M_Left.setmotor(_CCW, pwm);
  M_Right.setmotor(_CCW, pwm);
  delay(1500);
  */
  M_Left.setmotor(_STANDBY);
  M_Right.setmotor(_STANDBY);
  Serial.println("STANDBY");
  delay(1000);
}

