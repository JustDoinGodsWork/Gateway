#include "secrets.h"
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <math.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include <EEPROM.h>


#include <ModbusRTU.h>
#include <SoftwareSerial.h>

#define DEBUG 0

int i = 0;
int statusCode;
const char* ssid = "Default SSID";
const char* passphrase = "Default pass";
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

uint16_t Mread[12], Mread2[11];

bool cbWrite(Modbus::ResultCode event, uint16_t transactionId, void* data) {
  if (DEBUG)
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

  Serial.println("Disconnecting current wifi connection");
  WiFi.disconnect();
  EEPROM.begin(512);  //Initialasing EEPROM
  delay(10);

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
      //(SlaevID,Address,Buffer,Range of data,Modus call)
      //Serial.println(InttoFloat( Mread0[0], Mread0[1]));
      mb.readHreg(1, 0, Mread, 12, cbWrite);
      if (DEBUG) {
        Serial.print("Mread[0");
        Serial.print("] : ");
        Serial.println(Mread[0]);
        Serial.print("Mread[1");
        Serial.print("] : ");
        Serial.println(Mread[1]);
        Serial.print("Mread[2");
        Serial.print("] : ");
        Serial.println(Mread[2]);
        Serial.print("Mread[3");
        Serial.print("] : ");
        Serial.println(Mread[3]);
        Serial.print("Mread[4");
        Serial.print("] : ");
        Serial.println(Mread[4]);
        Serial.print("Mread[5");
        Serial.print("] : ");
        Serial.println(Mread[5]);
        Serial.print("Mread[6");
        Serial.print("] : ");
        Serial.println(Mread[6]);
        Serial.print("Mread[7");
        Serial.print("] : ");
        Serial.println(Mread[7]);
        Serial.print("Mread[8");
        Serial.print("] : ");
        Serial.println(Mread[8]);
        Serial.print("Mread[9");
        Serial.print("] : ");
        Serial.println(Mread[9]);
        Serial.print("Mread[10");
        Serial.print("] : ");
        Serial.println(Mread[10]);
        Serial.print("Mread[11");
        Serial.print("] : ");
        Serial.println(Mread[11]);
      }
      //mb.readHreg(1, 0, Mread2, 11, cbWrite);
    }


    va = Mread[0];
    vb = Mread[1];
    vc = Mread[2];

    ia = Mread[6];
    ib = Mread[7];
    ic = Mread[8];

    pfa = Mread[9];
    pfb = Mread[10];
    pfc = Mread[11];

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
