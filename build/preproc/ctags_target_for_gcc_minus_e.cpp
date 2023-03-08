# 1 "D:\\Lab\\AutoCar\\AutoCar.ino"
# 2 "D:\\Lab\\AutoCar\\AutoCar.ino" 2

int pwm_left;
int pwm_right;

//Motor shiled I2C Address: 0x30
//PWM frequency: 1000Hz(1kHz)
Motor M_Left(0x30,0, 1000);
Motor M_Right(0x30,1, 1000);

unsigned int Lcount = 0;
unsigned int Rcount = 0;
int LcounterPin = 12;
int RcounterPin = 14;
unsigned long time2;
unsigned int Lrpm;
unsigned int Rrpm;
unsigned int grid_num = 20;

void __attribute__((section("\".iram.text." "AutoCar.ino" "." "20" "." "10" "\""))) Lcounter() {
   Lcount++;
}

void __attribute__((section("\".iram.text." "AutoCar.ino" "." "24" "." "11" "\""))) Rcounter() {
   Rcount++;
}

void setup() {
   Serial.begin(115200);
   pinMode(LcounterPin, 0x00);
   pinMode(RcounterPin, 0x00);
   attachInterrupt((((LcounterPin) < 16)? (LcounterPin) : -1), Lcounter, 0x02);
   attachInterrupt((((RcounterPin) < 16)? (RcounterPin) : -1), Rcounter, 0x02);

   Serial.println("GO");
   M_Left.setmotor(2, 80);
   M_Right.setmotor(2, 80);
}

void loop() {
  if (Serial.available()) { // If anything comes in Serial (USB),
    if (Serial.readString() == "forward") {
      Serial.println("GO");
      M_Left.setmotor(2, 80);
      M_Right.setmotor(2, 80);
    }
  }
  if (millis() - time2 >= 1000){ /* 每秒更新 */
      // 計算 rpm 時，停止計時
      (__extension__({uint32_t state; __asm__ __volatile__("rsil %0," "15" : "=a" (state) :: "memory"); state;}));

      // 偵測的格數count * (60 * 1000 / 一圈網格數20）/ 時間差) 
      Lrpm = (60 * 1000 / grid_num )/ (millis() - time2) * Lcount;
      Rrpm = (60 * 1000 / grid_num )/ (millis() - time2) * Rcount;
      time2 = millis();
      Lcount = 0;
      Rcount = 0;

      // 輸出至Console
      Serial.print("LRPM = ");
      Serial.println(Lrpm,10);
      Serial.print("RRPM = ");
      Serial.println(Rrpm,10);
      //Restart the interrupt processing
      (__extension__({uint32_t state; __asm__ __volatile__("rsil %0," "0" : "=a" (state) :: "memory"); state;}));
  }
}
