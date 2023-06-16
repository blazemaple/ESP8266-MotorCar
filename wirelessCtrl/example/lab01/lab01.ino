#include <WiFi.h>
#include <WiFiUdp.h>

/* Set these to your desired credentials. */
const char *ssid = "學號_car";
const char *password = "00000000";
unsigned int localPort = 8888;  // local port to listen on

// buffers for receiving and sending data
char packetBuffer[255];  // buffer to hold incoming packet,
char ReplyBuffer[] = "acknowledged\r\n";        // a string to send back
int packetSize = 0;
int n = 0;
WiFiClient espClient;
WiFiUDP Udp;

void UDP_send(String replybuffer) {
  delay(500);
  Udp.beginPacket("192.168.4.1", localPort);
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
  Udp.begin(localPort);
  // test message
  Serial.printf("[SEND]\t%s\n", "AT");
  UDP_send("AT");
  packetSize = Udp.parsePacket();
  if (packetSize) {
    // read the packet into packetBufffer
    n = Udp.read(packetBuffer, 255);
    packetBuffer[n] = 0;
    Serial.println(packetBuffer);
  }
  delay(2000);
}

void loop() {
  //go
  Serial.printf("[SEND]\t%s\n", "***"); //TODO: Please complete this section
  UDP_send("***");                      //TODO: Please complete this section
  packetSize = Udp.parsePacket();
  if (packetSize) {
    // read the packet into packetBufffer
    n = Udp.read(packetBuffer, 255);
    packetBuffer[n] = 0;
    Serial.println(packetBuffer);
  }
  delay(2000);
  //back
  Serial.printf("[SEND]\t%s\n", "***"); //TODO: Please complete this section
  UDP_send("***");                      //TODO: Please complete this section
  packetSize = Udp.parsePacket();
  if (packetSize) {
    // read the packet into packetBufffer
    n = Udp.read(packetBuffer, 255);
    packetBuffer[n] = 0;
    Serial.println(packetBuffer);
  }
  delay(2000);
  //stop
  Serial.printf("[SEND]\t%s\n", "***"); //TODO: Please complete this section
  UDP_send("***");                      //TODO: Please complete this section
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    // read the packet into packetBufffer
    int n = Udp.read(packetBuffer, 255);
    packetBuffer[n] = 0;
    Serial.println(packetBuffer);
  }
  delay(2000);
}
