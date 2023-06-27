#include <WiFi.h>
#include <PubSubClient.h>

#include <DHT.h>

#define DHTPIN 27     //DHT11 sensor data pin
#define DHTTYPE DHT11 //DHT sensor type

//const char* ssid = "Airtel_xstream_kaushik";
//const char* password = "VIN@123**";

const char* ssid     = "Social Tab 1";
const char* password = "Socialtab";

//MQTT Broker IP address
//const char* mqtt_server = "192.168.1.144";
const char* mqtt_server = "b95cb968.us-east-1.emqx.cloud";//"broker.hivemq.com";//"192.168.1.17";
const int mqtt_port = 15240;
const char* mqtt_user = "kaushik";
const char* mqtt_password = "lopklopk";

const char* topic1_publish = "esp32/temperature";
const char *topic2_publish = "esp32/humidity";
const char* topic1_subscribe = "esp32/Relay1";
const char *topic2_subscribe = "esp32/Relay2";


WiFiClient espClient;
PubSubClient client(espClient);

long lastMsg = 0;
char msg[50];

float temperature = 0;
float humidity = 0;

int minVal;
int maxVal;
int randomNumber;

// Relay Pin
const int relayPin1 = 13;
const int relayPin2 = 12;

void setup() {
  Serial.begin(115200);

  setup_wifi();

  client.setServer(mqtt_server, 15240);
  client.setCallback(callback);

  pinMode(relayPin1, OUTPUT);
  pinMode(relayPin2, OUTPUT);
}

void setup_wifi() {
  delay(10);

  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;

  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  // If a message is received on the topic esp32/output1, you check if the message is either "on" or "off".
  // Changes the output state according to the message
  if (String(topic) == topic1_subscribe) {
    Serial.print("Changing output to ");
    if (messageTemp == "on") {
      Serial.println("on");
      digitalWrite(relayPin1, LOW);
    }
    else if (messageTemp == "off") {
      Serial.println("off");
      digitalWrite(relayPin1, HIGH);
    }
  }

  if (String(topic) == topic2_subscribe) {
    Serial.print("Changing output to ");
    if (messageTemp == "on") {
      Serial.println("on");
      digitalWrite(relayPin2, LOW);
    }
    else if (messageTemp == "off") {
      Serial.println("off");
      digitalWrite(relayPin2, HIGH);
    }
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("MQTT connection...");

    if (client.connect("ESP32Client", mqtt_user, mqtt_password)) {
      Serial.println("connected");
      client.subscribe(topic1_subscribe);
      client.subscribe(topic2_subscribe);
    }
    else {
      Serial.print("failed, rc=");
      Serial.print(client.state());

      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

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
    client.publish(topic1_publish, tempString);

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
    client.publish(topic2_subscribe, humString);
  }
}
