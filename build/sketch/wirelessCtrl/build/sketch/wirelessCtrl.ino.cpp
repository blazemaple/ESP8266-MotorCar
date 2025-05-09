#line 1 "D:\\Lab\\AutoCar\\wirelessCtrl\\build\\sketch\\wirelessCtrl.ino.cpp"
#include <Arduino.h>
#line 1 "D:\\Lab\\AutoCar\\wirelessCtrl\\wirelessCtrl.ino"
/*
   Copyright (c) 2015, Majenko Technologies
   All rights reserved.

   Redistribution and use in source and binary forms, with or without modification,
   are permitted provided that the following conditions are met:

 * * Redistributions of source code must retain the above copyright notice, this
     list of conditions and the following disclaimer.

 * * Redistributions in binary form must reproduce the above copyright notice, this
     list of conditions and the following disclaimer in the documentation and/or
     other materials provided with the distribution.

 * * Neither the name of Majenko Technologies nor the names of its
     contributors may be used to endorse or promote products derived from
     this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
   ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
   ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/* Create a WiFi access point and provide a web server on it. */

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
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
int LcounterPin = 12;
int RcounterPin = 14;
unsigned long time2;
int Lrpm = 0;
int Rrpm = 0;
uint8_t grid_num = 20;

#define APmode 0
/* Set these to your desired credentials. */
const char *ssid = "E520";
const char *password = "nkfuste520wifi";
unsigned int localPort = 8888;  // local port to listen on

// buffers for receiving and sending data
char packetBuffer[255];  // buffer to hold incoming packet,
char ReplyBuffer[] = "acknowledged\r\n";        // a string to send back
WiFiClient espClient;
WiFiUDP Udp;

#line 75 "D:\\Lab\\AutoCar\\wirelessCtrl\\wirelessCtrl.ino"
void move(int LSpeed, int RSpeed);
#line 89 "D:\\Lab\\AutoCar\\wirelessCtrl\\wirelessCtrl.ino"
void setup();
#line 111 "D:\\Lab\\AutoCar\\wirelessCtrl\\wirelessCtrl.ino"
void loop();
#line 193 "D:\\Lab\\AutoCar\\wirelessCtrl\\wirelessCtrl.ino"
void setup_wifi();
#line 215 "D:\\Lab\\AutoCar\\wirelessCtrl\\wirelessCtrl.ino"
void UDPReply(String replybuffer);
#line 67 "D:\\Lab\\AutoCar\\wirelessCtrl\\wirelessCtrl.ino"
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
    } else if (RSpeed < 0) {
        M_Right.setmotor(_CCW, abs(RSpeed));
    }
}

void setup() {
  Serial.begin(115200);
  if (APmode) {
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
  pinMode(LcounterPin, INPUT);
  pinMode(RcounterPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(LcounterPin), Lcounter, FALLING);
  attachInterrupt(digitalPinToInterrupt(RcounterPin), Rcounter, FALLING);
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
    } else if (cmd.startsWith("AT+setRpwm=")) {    //set right motor speed
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
    } else if (cmd == "AT+getLpwm") {     //get left motor speed
      Serial.println(pwm_left);
      UDPReply(String(pwm_left));
    } else if (cmd == "AT+getRpwm") {     //get right motor speed
      Serial.println(pwm_right);
      UDPReply(String(pwm_right));
    } else if (cmd == "AT+getLrpm") {     //get left wheel RPM
      Serial.println(Lrpm);
      UDPReply(String(Lrpm));
    } else if (cmd == "AT+getRrpm") {     //get right wheel RPM
      Serial.println(Rrpm);
      UDPReply(String(Rrpm));
    } else if (cmd == "AT+stop") {      //set motors to stop
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
}

void UDPReply(String replybuffer) {
  delay(500);
  Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
  Udp.print(replybuffer);
  Udp.endPacket();
}
