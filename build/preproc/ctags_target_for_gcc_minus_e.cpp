# 1 "D:\\Lab\\AutoCar\\AutoCar.ino"
# 2 "D:\\Lab\\AutoCar\\AutoCar.ino" 2

int pwm_left = 0;
int pwm_right = 0;
String cmd = "";
//Motor shiled I2C Address: 0x30
//PWM frequency: 1000Hz(1kHz)
Motor M_Left(0x30,0, 1000);
Motor M_Right(0x30,1, 1000);

unsigned int Lcount = 0;
unsigned int Rcount = 0;
int LcounterPin = 14;
int RcounterPin = 12;
unsigned long time2;
int Lrpm = 0;
int Rrpm = 0;
uint8_t grid_num = 20;

void __attribute__((section("\".iram.text." "AutoCar.ino" "." "20" "." "10" "\""))) Lcounter() {
   Lcount++;
}

void __attribute__((section("\".iram.text." "AutoCar.ino" "." "24" "." "11" "\""))) Rcounter() {
   Rcount++;
}

void move(int LSpeed, int RSpeed) {
    if (LSpeed >= 0) {
        M_Left.setmotor(2, LSpeed);
    } else if (LSpeed < 0) {
        M_Left.setmotor(1, abs(LSpeed));
    }

    if (RSpeed >= 0) {
        M_Right.setmotor(2, RSpeed);
    } else if (LSpeed < 0) {
        M_Right.setmotor(1, abs(RSpeed));
    }
}

void setup() {
   Serial.begin(115200);
   pinMode(LcounterPin, 0x00);
   pinMode(RcounterPin, 0x00);
   attachInterrupt((((LcounterPin) < 16)? (LcounterPin) : -1), Lcounter, 0x02);
   attachInterrupt((((RcounterPin) < 16)? (RcounterPin) : -1), Rcounter, 0x02);

}

void loop() {
  String temp = "";
  if (Serial.available()) { // If anything comes in Serial (USB),
    cmd = Serial.readString();
    if (cmd.startsWith("AT+setLpwm=")) {
      temp = cmd.substring(cmd.indexOf("=") + 1, cmd.length());
      // Serial.println(temp.toInt());
      pwm_left = temp.toInt();
      Serial.println("ok");
    } else if (cmd.startsWith("AT+setRpwm=")) {
      temp = cmd.substring(cmd.indexOf("=") + 1, cmd.length());
      // Serial.println(temp.toInt());
      pwm_right = temp.toInt();
      Serial.println("ok");
    } else if (cmd == "AT+getLpwm") {
      Serial.println(pwm_left);
    } else if (cmd == "AT+getRpwm") {
      Serial.println(pwm_right);
    } else if (cmd == "AT+getLrpm") {
      Serial.println(Lrpm);
    } else if (cmd == "AT+getRrpm") {
      Serial.println(Rrpm);
    } else if (cmd == "AT+stop") {
      pwm_left = 0;
      pwm_right = 0;
      Serial.println("ok");
    }
  }

  move(pwm_left, pwm_right);

  if (millis() - time2 >= 1000){ /* 每秒更新 */
      // 計算 rpm 時，停止計時
      (__extension__({uint32_t state; __asm__ __volatile__("rsil %0," "15" : "=a" (state) :: "memory"); state;}));

      // 偵測的格數count * (60 * 1000 / 一圈網格數20）/ 時間差) 
      Lrpm = (60 * 1000 / grid_num )/ (millis() - time2) * Lcount;
      Rrpm = (60 * 1000 / grid_num )/ (millis() - time2) * Rcount;
      time2 = millis();
      Lcount = 0;
      Rcount = 0;

      //Restart the interrupt processing
      (__extension__({uint32_t state; __asm__ __volatile__("rsil %0," "0" : "=a" (state) :: "memory"); state;}));
  }
}
