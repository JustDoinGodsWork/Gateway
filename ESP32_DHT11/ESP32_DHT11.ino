#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <DHT.h>

#define DHTPIN 27 // DHT11 sensor data pin
#define DHTTYPE DHT11 // DHT sensor type
#define API_ENDPOINT "https://your-api-endpoint.com/data" // API endpoint URL

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);

  // Connect to WiFi network
  WiFi.begin("your-ssid", "your-password");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi");
}

void loop() {
  // Read temperature and humidity from DHT11 sensor
  //float humidity = dht.readHumidity();
  //float temperature = dht.readTemperature();

  int minVal = 30;
  int maxVal = 40;
  int randomNumber = random(maxVal - minVal + 1) + minVal; // generates a random number between 30 and 40
  float temperature = randomNumber;
  
  
  minVal = 50;
  maxVal = 55;
  randomNumber = random(maxVal - minVal + 1) + minVal; // generates a random number between 30 and 40
  float humidity = randomNumber;
  
  // Check if any errors occurred while reading from the sensor
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read data from DHT11 sensor");
    return;
  }

  // Create JSON object to hold the data
  StaticJsonDocument<128> json;
  json["humidity"] = humidity;
  json["temperature"] = temperature;

  // Serialize the JSON object to a string
  String jsonStr;
  serializeJson(json, jsonStr);

  // Send data to API using a PUT request
  HTTPClient http;
  http.begin(API_ENDPOINT);
  http.addHeader("Content-Type", "application/json");

  int httpCode = http.PUT(jsonStr);
  String response = http.getString();

  http.end();

  // Print the response from the API
  Serial.println("HTTP response code: " + String(httpCode));
  Serial.println("HTTP response body: " + response);

  // Wait for 1 minute before sending data again
  delay(60000);
}
