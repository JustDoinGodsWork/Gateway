//#include <SPI.h>
#include <EthernetENC.h>

#define ETHERNET_DEBUG 1

unsigned long time1 = 0;
// if you don't want to use DNS (and reduce your sketch size)
// use the numeric IP instead of the name for the server:
//IPAddress server(74,125,232,128);  // numeric IP for Google (no DNS)
//char server[] = "www.google.com";    // name address for Google (using DNS)

#ifdef ETHERNET_DEBUG
char server[] = "httpbin.org";  // name address for Google (using DNS)
#endif
// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = { 0xDA, 0xAF, 0xfE, 0xEA, 0xFE, 0xED };

// Set the static IP address to use if the DHCP fails to assign
#define MYIPADDR 192, 168, 1, 30
#define MYIPMASK 255, 255, 255, 0
#define MYDNS 192, 168, 1, 1
#define MYGW 192, 168, 1, 1

// Initialize the Ethernet eclient library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
EthernetClient eclient;

// Variables to measure the speed
unsigned long beginMicros, endMicros;
unsigned long byteCount = 0;
bool printWebData = true;  // set to false for better speed measurement

int ethernet() {
  int E = 0;

  Serial.println("Begin Ethernet");
  Ethernet.init(5);

  if (Ethernet.begin(mac, 20000)) {  // Dynamic IP setup
    Serial.println("DHCP OK!");
  } else {
    Serial.println("Failed to configure Ethernet using DHCP");
    // Check for Ethernet hardware present
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");

      // while (true) {
      //   delay(1);  // do nothing, no point running without Ethernet hardware
      // }
      return 0;  // Retutn if no hardware for Ethernet
    }
    if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("Ethernet cable is not connected.");
    }

    IPAddress ip(MYIPADDR);
    IPAddress dns(MYDNS);
    IPAddress gw(MYGW);
    IPAddress sn(MYIPMASK);
    Ethernet.begin(mac, ip, dns, gw, sn);
    Serial.println("STATIC OK!");
  }
  delay(5000);

  Serial.print("Local IP : ");
  Serial.println(Ethernet.localIP());
  Serial.print("Subnet Mask : ");
  Serial.println(Ethernet.subnetMask());
  Serial.print("Gateway IP : ");
  Serial.println(Ethernet.gatewayIP());
  Serial.print("DNS Server : ");
  Serial.println(Ethernet.dnsServerIP());

  Serial.println("Ethernet Successfully Initialized");
  // if you get a connection, report back via serial:
  if (ETHERNET_DEBUG) {
    if (eclient.connect(server, 80)) {
      Serial.println("Connected!");
      // Make a HTTP request:
      eclient.println("GET /get HTTP/1.1");
      eclient.println("Host: httpbin.org");
      eclient.println("Connection: close");
      eclient.println();

      int len = eclient.available();
      if (len > 0) {
        byte buffer[80];
        if (len > 80) len = 80;
        eclient.read(buffer, len);
        if (printWebData) {
          Serial.write(buffer, len);  // show in the serial monitor (slows some boards)
        }
        byteCount = byteCount + len;
      }
      Serial.println();
      Serial.println("disconnecting.");
      eclient.stop();
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

    } else {
      // if you didn't get a connection to the server:
      Serial.println("connection failed");
    }
  }
  return E;
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  ethernet();
  time1 = millis();
}

void loop() {
  if ((millis() - time1) > 30000) {

    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    }
    if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("Ethernet cable is not connected.");
    }
    Serial.println("STATIC OK!");
    delay(5000);

    Serial.print("Local IP : ");
    Serial.println(Ethernet.localIP());
    Serial.print("Subnet Mask : ");
    Serial.println(Ethernet.subnetMask());
    Serial.print("Gateway IP : ");
    Serial.println(Ethernet.gatewayIP());
    Serial.print("DNS Server : ");
    Serial.println(Ethernet.dnsServerIP());

    Serial.println("Ethernet Successfully Initialized");
    // if you get a connection, report back via serial:
    if (ETHERNET_DEBUG) {
      if (eclient.connect("www.google.com", 80)) {
        Serial.println("Connected!");
        // Make a HTTP request:
        eclient.println("GET /get HTTP/1.1");
        eclient.println("Host: httpbin.org");
        eclient.println("Connection: close");
        eclient.println();

        int len = eclient.available();
        if (len > 0) {
          byte buffer[80];
          if (len > 80) len = 80;
          eclient.read(buffer, len);
          if (printWebData) {
            Serial.write(buffer, len);  // show in the serial monitor (slows some boards)
          }
          byteCount = byteCount + len;
        }
        Serial.println();
        Serial.println("disconnecting.");
        eclient.stop();
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

      } else {
        // if you didn't get a connection to the server:
        Serial.println("connection failed");
      }
    }
  }
}
// put your main code here, to run repeatedly:
