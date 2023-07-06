#include <EthernetENC.h>
#include <SPI.h>
#include <WiFi.h>

#define SS_ETH 5
byte mac[] = { 0xDA, 0xAF, 0xfE, 0xEA, 0xFE, 0xED };

char server[] = "httpbin.org";

EthernetClient ethclient;
WiFiClient wifiClient;

// Variables to measure the speed
unsigned long beginMicros, endMicros;
unsigned long byteCount = 0;
bool printWebData = true;

void setup() {
  Serial.begin(115200);

  // Initialize Ethernet
  Ethernet.init(SS_ETH);
  Ethernet.begin(mac, 10000);

  Serial.print("Local IP (Ethernet): ");
  Serial.println(Ethernet.localIP());
  Serial.print("Subnet Mask: ");
  Serial.println(Ethernet.subnetMask());
  Serial.print("Gateway IP: ");
  Serial.println(Ethernet.gatewayIP());
  Serial.print("DNS Server: ");
  Serial.println(Ethernet.dnsServerIP());

  //WiFi.begin("ALMIGHT1", "1234567890");
}

int checkConnection() {
  // Check if Ethernet is available
  if(ethclient.connected() && (Ethernet.linkStatus() == LinkON))
    return 1;
  if (Ethernet.linkStatus() == LinkON) {
    // Use Ethernet connection
    Serial.println("Ethernet LINKON");
    if (!ethclient.connected()) {
      Serial.println("Connecting via Ethernet...");
        beginMicros = micros();
        WiFi.disconnect();
        //delay(5000);
        return 1;
    }
  }

 if (WiFi.status() == WL_CONNECTED)
    return 0;
  // Use Wi-Fi connection
  WiFi.begin("ALMIGHT1", "1234567890");
  Serial.println("Connecting via Wi-Fi...");
  for (int i = 0; i < 10; i++) {
    if ((WiFi.status() == WL_CONNECTED))
      break;
    delay(100);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Connected to Wi-Fi. Local IP: ");
    Serial.println(WiFi.localIP());
    // Additional Wi-Fi initialization if needed
  }
  else{
    Serial.println("Wi-Fi not Connected");
  }

  return 0;
}

void loop() {
  int i = checkConnection();

  if (i) {
    if (ethclient.connect(server, 80)) {
        Serial.println("Connected!");
        // Make an HTTP request:
        ethclient.println("GET /get HTTP/1.1");
        ethclient.println("Host: httpbin.org");
        ethclient.println("Connection: close");
        ethclient.println();
    }
    
    int len = ethclient.available();
    Serial.println("Checking if data Available");
    if (len > 0) {
      Serial.println("Data Available");
      byte buffer[80];
      if (len > 80) len = 80;
      ethclient.read(buffer, len);
      if (printWebData) {
        Serial.write(buffer, len);  // Show in the serial monitor (slows some boards)
      }
      byteCount += len;
    }
    endMicros = micros();
    Serial.println();

    Serial.print("Received ");
    Serial.print(byteCount);
    Serial.print(" bytes in ");
    float seconds = (float)(endMicros - beginMicros) / 1000000.0;
    Serial.print(seconds, 4);
    float rate = (float)byteCount / seconds / 1000.0;
    Serial.print(", rate = ");
    Serial.print(rate);
    Serial.print(" kbytes/second");
    Serial.println();
    //ethclient.stop();
  } 
  
  else {
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("Wi-Fi Connected.");
    }
    else
     Serial.println("No Internet Connected.");
  }

  // Other code logic

  delay(1000);
}
