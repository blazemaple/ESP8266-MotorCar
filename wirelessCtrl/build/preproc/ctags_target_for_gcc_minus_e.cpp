# 1 "D:\\Lab\\AutoCar\\wirelessCtrl\\wirelessCtrl.ino"
# 2 "D:\\Lab\\AutoCar\\wirelessCtrl\\wirelessCtrl.ino" 2
# 3 "D:\\Lab\\AutoCar\\wirelessCtrl\\wirelessCtrl.ino" 2
# 4 "D:\\Lab\\AutoCar\\wirelessCtrl\\wirelessCtrl.ino" 2
# 5 "D:\\Lab\\AutoCar\\wirelessCtrl\\wirelessCtrl.ino" 2

int pwm_left = 0;
int pwm_right = 0;
String cmd = "";
//Motor shiled I2C Address: 0x30
//PWM frequency: 1000Hz(1kHz)
Motor M_Left(0x30,0, 1000);
Motor M_Right(0x30,1, 1000);

unsigned int Lcount = 0;
unsigned int Rcount = 0;
int LcounterPin = 12;
int RcounterPin = 14;
unsigned long time2;
int Lrpm = 0;
int Rrpm = 0;
uint8_t grid_num = 20;


/* Set these to your desired credentials. */
const char *ssid = "E520";
const char *password = "nkfuste520wifi";
unsigned int localPort = 8888; // local port to listen on

// buffers for receiving and sending data
char packetBuffer[255]; // buffer to hold incoming packet,
char ReplyBuffer[] = "acknowledged\r\n"; // a string to send back
WiFiClient espClient;
WiFiUDP Udp;

void __attribute__((section("\".iram.text." "wirelessCtrl.ino" "." "35" "." "10" "\""))) Lcounter() {
   Lcount++;
}

void __attribute__((section("\".iram.text." "wirelessCtrl.ino" "." "39" "." "11" "\""))) Rcounter() {
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
    } else if (RSpeed < 0) {
        M_Right.setmotor(1, abs(RSpeed));
    }
}

void setup() {
  Serial.begin(115200);
  if (0) {
    Serial.print("Configuring access point...");
    /* You can remove the password parameter if you want the AP to be open. */
    WiFi.softAP(ssid, password);

    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);

  } else {
    setup_wifi();
  }

  Serial.printf("UDP server on port %d\n", localPort);
  Udp.begin(localPort);
  pinMode(LcounterPin, 0x00);
  pinMode(RcounterPin, 0x00);
  attachInterrupt((((LcounterPin) < 16)? (LcounterPin) : -1), Lcounter, 0x02);
  attachInterrupt((((RcounterPin) < 16)? (RcounterPin) : -1), Rcounter, 0x02);
}

void loop() {
  // if there's data available, read a packet
  int packetSize = Udp.parsePacket();
  String packetString;
  String temp = "";
  if (packetSize) {
    //Serial.printf("Received packet of size %d from %s:%d\n    (to %s:%d, free heap = %d B)\n", packetSize, Udp.remoteIP().toString().c_str(), Udp.remotePort(), Udp.destinationIP().toString().c_str(), Udp.localPort(), ESP.getFreeHeap());
    // read the packet into packetBufffer
    int n = Udp.read(packetBuffer, 255);
    packetBuffer[n] = 0;
    Serial.print("[RECE]\t");
    Serial.println(packetBuffer);
    cmd = String(packetBuffer);
    if (cmd.startsWith("AT+setLpwm=")) {
      temp = cmd.substring(cmd.indexOf("=") + 1, cmd.length());
      // Serial.println(temp.toInt());
      if (temp.toInt() > 100) {
        pwm_left = 100;
      } else if (temp.toInt() < -100) {
        pwm_left = -100;
      } else {
        pwm_left = temp.toInt();
      }
      Serial.println("ok");
      UDPReply("ok");
    } else if (cmd.startsWith("AT+setRpwm=")) { //set right motor speed
      temp = cmd.substring(cmd.indexOf("=") + 1, cmd.length());
      // Serial.println(temp.toInt());
      if (temp.toInt() > 100) {
        pwm_right = 100;
      } else if (temp.toInt() < -100) {
        pwm_right = -100;
      } else {
        pwm_right = temp.toInt();
      }
      Serial.println("ok");
      UDPReply("ok");
    } else if (cmd == "AT+getLpwm") { //get left motor speed
      Serial.println(pwm_left);
      UDPReply(String(pwm_left));
    } else if (cmd == "AT+getRpwm") { //get right motor speed
      Serial.println(pwm_right);
      UDPReply(String(pwm_right));
    } else if (cmd == "AT+getLrpm") { //get left wheel RPM
      Serial.println(Lrpm);
      UDPReply(String(Lrpm));
    } else if (cmd == "AT+getRrpm") { //get right wheel RPM
      Serial.println(Rrpm);
      UDPReply(String(Rrpm));
    } else if (cmd == "AT+stop") { //set motors to stop
      pwm_left = 0;
      pwm_right = 0;
      Serial.println("ok");
      UDPReply("ok");
    } else if (cmd == "AT") {
      Serial.println("ok");
      UDPReply("ok");
    } else {
      UDPReply("error");
      Serial.println("error");
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

void setup_wifi() {
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Udp.beginPacket("192.168.100.208", 8888);
  Udp.print(WiFi.localIP());
  Udp.endPacket();
}

void UDPReply(String replybuffer) {
  delay(500);
  Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
  Udp.print(replybuffer);
  Udp.endPacket();
}
