#include <WiFi.h>
#include <WiFiUdp.h>

/* Set these to your desired credentials. */
const char *ssid = "C108118129_car";
const char *password = "00000000";
unsigned int localPort = 8888;  // local port to listen on

// buffers for receiving and sending data
char packetBuffer[255];  // buffer to hold incoming packet,
char ReplyBuffer[] = "acknowledged\r\n";        // a string to send back
String cmd = "";
WiFiClient espClient;
WiFiUDP Udp;

void UDP_send(String replybuffer) {
  delay(500);
  Udp.beginPacket("192.168.4.1", 8888);
  Udp.print(replybuffer);
  Udp.endPacket();
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

void setup() {
  Serial.begin(115200);
  setup_wifi();
  
}

void loop() {
  if (Serial.available()) {      // If anything comes in Serial (USB),
    cmd = Serial.readString();
    //Serial.printf("[SEND]\t%s\n", cmd);
    UDP_send(cmd);
  }
  
  int packetSize = Udp.parsePacket();
  String packetString;
  if (packetSize) {
    //Serial.printf("Received packet of size %d from %s:%d\n    (to %s:%d, free heap = %d B)\n", packetSize, Udp.remoteIP().toString().c_str(), Udp.remotePort(), Udp.destinationIP().toString().c_str(), Udp.localPort(), ESP.getFreeHeap());
    // read the packet into packetBufffer
    int n = Udp.read(packetBuffer, 255);
    packetBuffer[n] = 0;
    //Serial.print("[RECE]\t");
    Serial.println(packetBuffer);
  }
}
