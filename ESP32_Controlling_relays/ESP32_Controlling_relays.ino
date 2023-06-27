#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid = "Almight";
const char* password = "Almight1";
const char* apiURL = "https://641c73691a68dc9e460a03c7.mockapi.io/Relays?id=1";

const int relay1Pin = 12;
const int relay2Pin = 13;

void setup() 
{
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  pinMode(relay1Pin, OUTPUT);
  pinMode(relay2Pin, OUTPUT);
}

void loop() 
{
  if (WiFi.status() == WL_CONNECTED) 
  {
    HTTPClient http;
    http.begin(apiURL);
    int httpResponseCode = http.GET();

    if (httpResponseCode == HTTP_CODE_OK) 
    {
      String payload = http.getString();

      DynamicJsonDocument doc(1024);
      deserializeJson(doc, payload);

      bool relay1State = doc[0]["Relay1"];

      bool relay2State = doc[0]["Relay2"];

      digitalWrite(relay1Pin, relay1State);

      digitalWrite(relay2Pin, relay2State);

      Serial.print("Relay 1 state: ");
      Serial.println(relay1State);


      Serial.print("Relay 2 state: ");
      Serial.println(relay2State);
    } 
    else {
      Serial.print("Error retrieving data. HTTP response code: ");
      Serial.println(httpResponseCode);
    }

    http.end();
  }

  delay(1000);
}
