#include "WEMOS_Motor.h"

int pwm; //轉速

//Motor shiled I2C Address: 0x30
//PWM frequency: 1000Hz(1kHz)
Motor M_Left(0x30,_MOTOR_A, 1000);
Motor M_Right(0x30,_MOTOR_B, 1000);

void setup()
{
  Serial.begin(115200);
  pwm = 50;
}

void loop()
{
  Serial.println("GO");
  M_Left.setmotor(_CW, pwm);
  M_Right.setmotor(_CW, pwm);
  delay(1500); //前進1.5秒
  
  Serial.println("BACK");
  M_Left.setmotor(_CCW, pwm);
  M_Right.setmotor(_CCW, pwm);
  delay(1500); //後退1.5秒
  
  M_Left.setmotor(_STOP);
  M_Right.setmotor(_STOP);
  Serial.println("STOP");
  delay(1000); //停止1秒
}
