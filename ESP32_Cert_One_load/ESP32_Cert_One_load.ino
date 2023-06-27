#include "secrets.h"
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <WiFi.h>

#define AWS_IOT_SUBSCRIBE_TOPIC1 "esp32/Relay"
// #define AWS_IOT_SUBSCRIBE_TOPIC2 "esp32/Relay2"
// #define AWS_IOT_SUBSCRIBE_TOPIC3 "esp32/Relay3"
// #define AWS_IOT_SUBSCRIBE_TOPIC4 "esp32/Relay4"

#define AWS_IOT_PUBLISH_TOPIC1  "esp32/temperature"
#define AWS_IOT_PUBLISH_TOPIC2  "esp32/humidity"

const char *relayNo[4] = {"Relay1", "Relay2", "Relay3", "Relay4"};
int relayPin[4] = {13,14,22,23};

WiFiClientSecure net = WiFiClientSecure();
PubSubClient client(net);

long lastMsg = 0;
char msg[50];

float temperature = 0;
float humidity = 0;

int minVal;
int maxVal;
int randomNumber;

void messageHandler(char* topic, byte* payload, unsigned int length)
{
  Serial.print("incoming: ");
  Serial.println(topic);
  StaticJsonDocument<200> doc;
  deserializeJson(doc, payload);

  if ( strstr(topic, AWS_IOT_SUBSCRIBE_TOPIC1) )
  {
    for (int i = 0; i < 4; i++)
    {
      String Relay = doc[relayNo[i]];
      if (Relay == "On"||Relay=="on")
      {
        digitalWrite(relayPin[i], LOW);
        Serial.print("Relay ");
        Serial.print(i+1);
        Serial.println(" is ON");
      }
      else if (Relay == "Off"||Relay=="off")
      {
        digitalWrite(relayPin[i], HIGH);
        Serial.print("Relay ");
        Serial.print(i+1);
        Serial.println(" is OFF");
          
      }
    }
  }
  Serial.println();
}


void setup()
{
  Serial.begin(115200);

  pinMode (relayPin[0], OUTPUT);
  pinMode (relayPin[1], OUTPUT);
  pinMode (relayPin[2], OUTPUT);
  pinMode (relayPin[3], OUTPUT);

  digitalWrite(relayPin[0], HIGH);
  digitalWrite(relayPin[1], HIGH);
  digitalWrite(relayPin[2], HIGH);
  digitalWrite(relayPin[3], HIGH);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.println("Connecting to Wi-Fi");

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  // Configure WiFiClientSecure to use the AWS IoT device credentials
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);

  // Connect to the MQTT broker on the AWS endpoint we defined earlier
  client.setServer(AWS_IOT_ENDPOINT, 8883);

  // Create a message handler
  client.setCallback(messageHandler);

}

void reconnect() {
  Serial.println("Connecting to AWS IOT");

  while (!client.connect(THINGNAME))
  {
    Serial.print("#");
    delay(100);
  }

  if (!client.connected())
  {
    Serial.println("AWS IoT Timeout!");
    Serial.print("failed, rc=");
    Serial.print(client.state());
    return;
  }

  // Subscribe to a topic
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC1);
  // client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC2);
  // client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC3);
  // client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC4);

  Serial.println("AWS IoT Connected!");

}

void loop()
{
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
    randomNumber = random(maxVal - minVal + 1) + minVal; // generates a random number
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
    randomNumber = random(maxVal - minVal + 1) + minVal; // generates a random number
    humidity = randomNumber;

    // Convert the value to a char array
    char humString[8];
    dtostrf(humidity, 1, 2, humString);
    Serial.print("Humidity: ");
    Serial.println(humString);
    client.publish(AWS_IOT_PUBLISH_TOPIC2, humString);
  }
}
