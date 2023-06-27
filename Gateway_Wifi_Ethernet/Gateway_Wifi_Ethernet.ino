#include "secrets.h"
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <math.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include <EEPROM.h>
/////////////Ethernet////////////////
#include <SPI.h>
#include <EthernetENC.h>

//////////////MODBUS/////////////////
#include <ModbusRTU.h>
#include <SoftwareSerial.h>

#define DEBUG 0
#define ETHERNET_DEBUG 0
#define debug 0
#define debug1 0

if (ETHERNET_DEBUG)
  char server[] = "httpbin.org";

byte mac[] = { 0xDA, 0xAF, 0xfE, 0xEA, 0xFE, 0xED };

// Set the static IP address to use if the DHCP fails to assign
#define MYIPADDR 192, 168, 1, 30
#define MYIPMASK 255, 255, 255, 0
#define MYDNS 192, 168, 1, 1
#define MYGW 192, 168, 1, 1

EthernetClient eclient;

// Variables to measure the speed
unsigned long beginMicros, endMicros;
unsigned long byteCount = 0;
bool printWebData = true; 

int i = 0;
int statusCode;
const char* ssid = "Default SSID";
const char* passphrase = "Default pass";

uint8_t W = 0, E = 1;

String st;
String content;
String esid;
String epass = "";

bool testWifi(void);
void launchWeb(void);
void setupAP(void);

WebServer server(80);

ModbusRTU mb;
int DE_RE = 4;  //D4  For MAX485 chip
int rx = 19;
int tx = 18;



SoftwareSerial S(rx, tx);
uint16_t Mread1[11], Mread2[11], Mread3[11];


#define AWS_IOT_SUBSCRIBE_TOPIC1 "LBS_ESP/Gateway001/Relay"

#define AWS_IOT_PUBLISH_TOPIC1 "LBS_ESP/Gateway001/temperature"
#define AWS_IOT_PUBLISH_TOPIC2 "LBS_ESP/Gateway001/humidity"
#define AWS_IOT_PUBLISH_TOPIC3 "LBS_ESP/Gateway001/Data"

const char* relayNo[4] = { "Relay1", "Relay2", "Relay3", "Relay4" };
int relayPin[4] = { 13, 14, 22, 23 };

WiFiClientSecure net = WiFiClientSecure();
PubSubClient client(net);

long lastMsg = 0;
char msg[50];

float temperature = 0;
float humidity = 0;

String data;

int minVal;
int maxVal;
int randomNumber;



bool cbWrite(Modbus::ResultCode event, uint16_t transactionId, void* data) {
  if (debug1)
    Serial.printf_P("Request result: 0x%02X, Mem: %d\n", event, ESP.getFreeHeap());
  return true;
}

void messageHandler(char* topic, byte* payload, unsigned int length) {
  Serial.print("incoming: ");
  Serial.println(topic);
  StaticJsonDocument<200> doc;
  deserializeJson(doc, payload);

  if (strstr(topic, AWS_IOT_SUBSCRIBE_TOPIC1)) {
    for (int i = 0; i < 4; i++) {
      String Relay = doc[relayNo[i]];
      if (Relay == "On" || Relay == "on") {
        digitalWrite(relayPin[i], LOW);
        Serial.print("Relay ");
        Serial.print(i + 1);
        Serial.println(" is ON");
      } else if (Relay == "Off" || Relay == "off") {
        digitalWrite(relayPin[i], HIGH);
        Serial.print("Relay ");
        Serial.print(i + 1);
        Serial.println(" is OFF");
      }
    }
  }
  Serial.println();
}


void setup() {
  Serial.begin(115200);
  S.begin(9600, SWSERIAL_8N1);


  mb.begin(&S, DE_RE);  //Assing Software serial port to Modbus instance for MAX485 chip having DI,DE,RE,RO Pin at TTL side
  mb.master();          //Assing Modbus function as master

  pinMode(relayPin[0], OUTPUT);
  pinMode(relayPin[1], OUTPUT);
  pinMode(relayPin[2], OUTPUT);
  pinMode(relayPin[3], OUTPUT);

  digitalWrite(relayPin[0], HIGH);
  digitalWrite(relayPin[1], HIGH);
  digitalWrite(relayPin[2], HIGH);
  digitalWrite(relayPin[3], HIGH);

  //---------------------------------------- Read eeprom for ssid and pass
  Serial.println("Disconnecting current wifi connection");
  WiFi.disconnect();
  EEPROM.begin(512);  //Initialasing EEPROM
  delay(10);

  Serial.println("Reading EEPROM ssid");
  for (int i = 0; i < 32; ++i) {
    esid += char(EEPROM.read(i));
  }
  Serial.println();
  Serial.print("SSID: ");
  Serial.println(esid);

  Serial.println("Reading EEPROM pass");
  for (int i = 32; i < 96; ++i) {
    epass += char(EEPROM.read(i));
  }
  Serial.print("PASS: ");
  Serial.println(epass);

  WiFi.begin(esid.c_str(), epass.c_str());

  Serial.println("Connecting to Wi-Fi");

  // while (WiFi.status() != WL_CONNECTED) {
  //   delay(500);
  //   Serial.print(".");
  // }
  // Serial.print("Connected to ");
  // Serial.print(esid);
  // Serial.println(" Successfully");
  delay(100);
}

void reconnect() {
  Serial.println("Connecting to AWS IOT");

  while (!client.connect(THINGNAME)) {
    Serial.print("#");
    delay(100);
  }

  if (!client.connected()) {
    Serial.println("AWS IoT Timeout!");
    Serial.print("failed, rc=");
    Serial.print(client.state());
    return;
  }

  // Subscribe to a topic
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC1);


  Serial.println("AWS IoT Connected!");
}

void loop() {
  int va, vb, vc, ia, ib, ic;
  float pfa, pfb, pfc, pf;
  double kw, kwa;
  long kwu, kwl;

  if ((WiFi.status() == WL_CONNECTED)) {

    Serial.print("Connected to ");
    Serial.print(esid);
    Serial.println(" Successfully");

  } else {
  }

  if (testWifi()) {
    Serial.println(" connection status positive");
    //launchWeb();
  } else {
    Serial.println("Connection Status Negative / D15 HIGH");
    Serial.println("Turning the HotSpot On");
    launchWeb();
    setupAP();  // Setup HotSpot
  }

  Serial.println();
  Serial.println("Waiting.");

  // Configure WiFiClientSecure to use the AWS IoT device credentials
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);
  // Connect to the MQTT broker on the AWS endpoint we defined earlier
  client.setServer(AWS_IOT_ENDPOINT, 8883);
  // Create a message handler
  client.setCallback(messageHandler);

  while (WiFi.status() == WL_CONNECTED) {

    if (!client.connected()) {
      reconnect();
    }

    client.loop();
    delay(1000);

    long now = millis();
    if (now - lastMsg > 5000) {
      lastMsg = now;

      // Temperature in Celsius
      //humidity = dht.readHumidity();

      //Random temp generated value till interfacing DHT11 sensor
      minVal = 30;
      maxVal = 40;
      randomNumber = random(maxVal - minVal + 1) + minVal;  // generates a random number
      temperature = randomNumber;

      // Convert the value to a char array
      char tempString[8];
      dtostrf(temperature, 1, 2, tempString);
      Serial.print("Temperature: ");
      Serial.println(tempString);
      client.publish(AWS_IOT_PUBLISH_TOPIC1, tempString);

      //Random humidity generated value till interfacing DHT11 sensor
      minVal = 50;
      maxVal = 55;
      randomNumber = random(maxVal - minVal + 1) + minVal;  // generates a random number
      humidity = randomNumber;

      // Convert the value to a char array
      char humString[8];
      dtostrf(humidity, 1, 2, humString);
      Serial.print("Humidity: ");
      Serial.println(humString);
      client.publish(AWS_IOT_PUBLISH_TOPIC2, humString);
      client.publish(AWS_IOT_PUBLISH_TOPIC3, data.c_str());
    }

    if (!mb.slave()) {
      if (read1 == 0) {
        if (debug1)
          Serial.println(" Reading 0-10");
        mb.readHreg(1, 0, Mread1, 9, cbWrite);
        if (debug) {
          Serial.print("Va : ");
          Serial.println(Mread1[0]);
          Serial.print("Vb : ");
          Serial.println(Mread1[1]);
          Serial.print("Vc : ");
          Serial.println(Mread1[2]);
          Serial.print("Vab : ");
          Serial.println(Mread1[3]);
          Serial.print("Vbc : ");
          Serial.println(Mread1[4]);
          Serial.print("Vca : ");
          Serial.println(Mread1[5]);
          Serial.print("Ia : ");
          Serial.println(Mread1[6]);
          Serial.print("Ib : ");
          Serial.println(Mread1[7]);
          Serial.print("Ic : ");
          Serial.println(Mread1[8]);
        }
        read1 = 3;
      }
      else if (read1 == 1) {
        if (debug1)
          Serial.println(" Reading 6-15");
        mb.readHreg(1, 9, Mread2, 6, cbWrite);
        if (debug) {
          Serial.print("pfa : ");
          Serial.println(Mread2[0]);
          Serial.print("pfb : ");
          Serial.println(Mread2[1]);
          Serial.print("pfc : ");
          Serial.println(Mread2[2]);
          Serial.print("Pa : ");
          Serial.println(Mread2[3]);
          Serial.print("Pb : ");
          Serial.println(Mread2[4]);
          Serial.print("Pc : ");
          Serial.println(Mread2[5]);
        }
        read1 = 4;
      }
      else if (read1 == 2)
      {
        if (debug1)
          Serial.println(" Reading 14-23");
        mb.readHreg(1, 16, Mread3, 8, cbWrite);
        if (debug) {
          Serial.print("App Power (a) : ");
          Serial.println(Mread3[0]);
          Serial.print("App Power (b) : ");
          Serial.println(Mread3[1]);
          Serial.print("App Power (c) : ");
          Serial.println(Mread3[2]);
          Serial.print("Total Active Power : ");
          Serial.println(Mread3[3]);
          Serial.print("Total App Power : ");
          Serial.println(Mread3[4]);
          Serial.print("kWh Lower : ");
          Serial.println(Mread3[5]);
          Serial.print("kWh Upper : ");
          Serial.println(Mread3[6]);
          Serial.print("kVAh Lower : ");
          Serial.println(Mread3[7]);
          Serial.print("kVAh Upper : ");
          Serial.println(Mread3[8]);

          Serial.println("\n\n####################################");
        }
        read1 = 5;
      }
    }
    else if (debug1) {
      Serial.println(" : mb is SLAVE cannot read");
    }


    if (read1 == 3) {
      read1 = 1;
      if (debug1) {
        Serial.println("Setting Read1 =1, will read 6-15");
      }
    }
    else if (read1 == 4) {
      read1 = 2;
      if (debug1) {
        Serial.println("Setting Read1 =2, will read 14-23");
      }
    }
    else if (read1 == 5) {
      read1 = 0;
      if (debug1) {
        Serial.println("Setting Read1 =0, will read 0-9");
      }
      delay(1000);
    }

    if (read1 = 0)
    {
      va = Mread1[0];
      vb = Mread1[1];
      vc = Mread1[2];

      ia = Mread1[6];
      ib = Mread1[7];
      ic = Mread1[8];

      pfa = Mread2[0];
      pfb = Mread2[1];
      pfc = Mread2[2];

      kwa = (sqrt(3)) * ((va * ia) + (vb * ib) + (vc * ic));
      kw = (sqrt(3)) * ((va * ia * pfa) + (vb * ib * pfa) + (vc * ic * pfa));

      kwl = 1000 * Mread2[8];  //40020
      kwu = 1000 * Mread2[9];  //40021

      data = "{\"readings\":{";
      data += "\"1\":{";
      data += "\"Va\":\"";
      data += String(va);
      data += "\",\"Vb\":\"";
      data += String(vb);
      data += "\",\"Vc\":\"";
      data += String(vc);
      data += "\",\"Ia\":\"";
      data += String(ia);
      data += "\",\"Ib\":\"";
      data += String(ib);
      data += "\",\"Ic\":\"";
      data += String(ic);
      data += "\",\"total_kw\":\"";
      data += String(kw);
      data += "\",\"total_pf\":\"";
      data += String(pfa + pfb + pfc);
      data += "\",\"kwh_upper\":\"";
      data += String(kwu);
      data += "\",\"total_lower\":\"";
      data += String(kwl);
      data += "}}";
    }
    mb.task();
    delay(100);
    yield();
  }

  while ((WiFi.status() != WL_CONNECTED)) {
    Serial.print(".");
    delay(100);
    server.handleClient();
  }
}


bool testWifi(void) {
  int c = 0;
  //Serial.println("Waiting for Wifi to connect");
  while (c < 20) {
    if (WiFi.status() == WL_CONNECTED) {
      return true;
    }
    delay(500);
    Serial.print("*");
    c++;
  }
  Serial.println("");
  Serial.println("Connect timed out, opening AP");
  return false;
}

void launchWeb() {
  Serial.println("");
  if (WiFi.status() == WL_CONNECTED)
    Serial.println("WiFi connected");
  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("SoftAP IP: ");
  Serial.println(WiFi.softAPIP());
  createWebServer();
  // Start the server
  server.begin();
  Serial.println("Server started");
}

void setupAP(void) {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0)
    Serial.println("no networks found");
  else {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i) {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      //Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");
      delay(10);
    }
  }
  Serial.println("");
  st = "<ol>";
  for (int i = 0; i < n; ++i) {
    // Print SSID and RSSI for each network found
    st += "<li>";
    st += WiFi.SSID(i);
    st += " (";
    st += WiFi.RSSI(i);

    st += ")";
    //st += (WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*";
    st += "</li>";
  }
  st += "</ol>";
  delay(100);
  WiFi.softAP("LiveBuildingSystem", "1234567890");
  Serial.println("Initializing_softap_for_wifi credentials_modification");
  launchWeb();
  Serial.println("over");
}

void createWebServer() {
  {
    server.on("/", []() {
      IPAddress ip = WiFi.softAPIP();
      String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
      content = "<!DOCTYPE HTML>\r\n<html>Welcome to Wifi Credentials Update page";
      content += "<form action=\"/scan\" method=\"POST\"><input type=\"submit\" value=\"scan\"></form>";
      content += ipStr;
      content += "<p>";
      content += st;
      content += "</p><form method='get' action='setting'><label>SSID: </label><input name='ssid' length=32><input name='pass' length=64><input type='submit'></form>";
      content += "</html>";
      server.send(200, "text/html", content);
    });
    server.on("/scan", []() {
      //setupAP();
      IPAddress ip = WiFi.softAPIP();
      String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);

      content = "<!DOCTYPE HTML>\r\n<html>go back";
      server.send(200, "text/html", content);
    });

    server.on("/setting", []() {
      String qsid = server.arg("ssid");
      String qpass = server.arg("pass");

      if (qsid.length() > 0 && qpass.length() > 0) {
        Serial.println("clearing eeprom");
        for (int i = 0; i < 224; ++i) {
          EEPROM.write(i, 0);
        }
        Serial.println(qsid);
        Serial.println("");
        Serial.println(qpass);
        Serial.println("");

        Serial.println("writing eeprom ssid:");
        for (int i = 0; i < qsid.length(); ++i) {
          EEPROM.write(i, qsid[i]);
          Serial.print("Wrote: ");
          Serial.println(qsid[i]);
        }

        Serial.println("writing eeprom pass:");
        for (int i = 0; i < qpass.length(); ++i) {
          EEPROM.write(32 + i, qpass[i]);
          Serial.print("Wrote: ");
          Serial.println(qpass[i]);
        }

        EEPROM.commit();

        content = "{\"Success\":\"saved to eeprom... reset to boot into new wifi\"}";
        statusCode = 200;
        ESP.restart();
      } else {
        content = "{\"Error\":\"404 not found\"}";
        statusCode = 404;
        Serial.println("Sending 404");
      }
      server.sendHeader("Access-Control-Allow-Origin", "*");
      server.send(statusCode, "application/json", content);
    });
  }
}
