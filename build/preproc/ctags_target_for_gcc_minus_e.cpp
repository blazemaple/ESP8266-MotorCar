# 1 "D:\\Lab\\AutoCar\\AutoCar.ino"
# 2 "D:\\Lab\\AutoCar\\AutoCar.ino" 2

int pwm_left;
int pwm_right;

//Motor shiled I2C Address: 0x30
//PWM frequency: 1000Hz(1kHz)
Motor M_Left(0x30,0, 1000);
Motor M_Right(0x30,1, 1000);

void setup()
{
  Serial.begin(115200);
 pinMode(2,0x01);
  digitalWrite(2,0x1);

}

void loop()
{
  Serial.println("GO Left");
  pwm_left = 30;
  pwm_right = 50;
  M_Left.setmotor(2, pwm_left);
  M_Right.setmotor(2, pwm_right);
 delay(1500);

  Serial.println("GO Right");
  pwm_left = 50;
  pwm_right = 30;
  M_Left.setmotor(2, pwm_left);
  M_Right.setmotor(2, pwm_right);
 delay(1500);
  /*

  Serial.println("BACK");

  M_Left.setmotor(_CCW, pwm);

  M_Right.setmotor(_CCW, pwm);

  delay(1500);

  */
# 40 "D:\\Lab\\AutoCar\\AutoCar.ino"
  M_Left.setmotor(4);
  M_Right.setmotor(4);
  Serial.println("STANDBY");
  delay(1000);
}
