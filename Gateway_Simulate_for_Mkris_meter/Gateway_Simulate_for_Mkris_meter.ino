#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

const char* ssid = "Airtel_xstream_kaushik";
const char* password = "VIN@123**";

const char* serverName = "https://dev.livebuildings.com/push-api/v1/mkris/2471";

const char* userName = "admin";
const char* pass = "rCf5OHU2yysPU66f";


const char* rootCACertificate =
  "-----BEGIN CERTIFICATE-----\n"
  "MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF\n"
  "ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\n"
  "b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL\n"
  "MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\n"
  "b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj\n"
  "ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM\n"
  "9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw\n"
  "IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6\n"
  "VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L\n"
  "93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm\n"
  "jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC\n"
  "AYYwHQYDVR0OBBYEFNPsxzplbszh2naaVvuc84ZtV+WBMA0GCSqGSIb3DQEBCwUA\n"
  "A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI\n"
  "U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs\n"
  "N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv\n"
  "o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU\n"
  "5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy\n"
  "rqXRfboQnoZsG4q5WTP468SQvvG5\n"
  "-----END CERTIFICATE-----\n";

void setup() {
  Serial.begin(115200);
  Serial.println();

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) 
  {
    Serial.print('.');
    delay(1000);
  }

  Serial.println(WiFi.localIP());
}

void loop() {
  if ((WiFi.status() == WL_CONNECTED)) 
  {
    WiFiClientSecure client;

    client.setCACert(rootCACertificate);
    HTTPClient https;

    Serial.print("[HTTPS] begin...\n");
    if (https.begin(client, serverName))
    {  
      https.setAuthorization(userName, pass);
      https.addHeader("Content-Type", "application/json");
      // {
      //   "readings":
      //   {
      //   "1": {
      //          "Va": "234.05",
      //          "Vb": "233.65",
      //          "Vc": "231.23000000000002",
      //          "Ia": "31.45",
      //          "Ib": "29.63",
      //          "Ic": "30.16",
      //          "total_kw": "17.2",
      //          "total_pf": "0.8270000000000001",
      //          "kwh_upper": "0.0",
      //          "kwh_lower": "61.6"
      //        }
      //   },
      //   "check_1": 1234,
      //   "qos": "Excellent",
      //   "check_2": 7890,
      //   "id": 38,
      //   "mac": "e4:5f:01:44:2c:4a",
      //   "site_id": 45538
      // }

      String httpRequestData = "{\"readings\":{";
      httpRequestData += "\"1\":{";
      httpRequestData += "\"Va\":\"23\",";
      httpRequestData += "\"Vb\":\"23\",";
      httpRequestData += "\"Vc\":\"23\",";
      httpRequestData += "\"Ia\":\"3\",";
      httpRequestData += "\"Ib\":\"2\",";
      httpRequestData += "\"Ic\":\"3\",";
      httpRequestData += "\"total_kw\":\"5\",";
      httpRequestData += "\"total_pf\":\"1\",";
      httpRequestData += "\"kwh_upper\":\"20\",";
      httpRequestData += "\"kwh_lower\":\"10\"";
      httpRequestData += "}},";
      httpRequestData += "\"check_1\":1234,";
      httpRequestData += "\"qos\":\"Excellent\",";
      httpRequestData += "\"check_2\": 7890,";
      httpRequestData += "\"id\": 1,";
      httpRequestData += "\"mac\":\"52:2c:b1:76:a6:92\",";
      httpRequestData += "\"site_id\": 99901";
      httpRequestData += "}";

      Serial.print("[HTTPS] POST...\n");
      Serial.println(httpRequestData);

      int httpCode = https.POST(httpRequestData);
      if (httpCode > 0) 
      {
        Serial.printf("[HTTPS] POST... code: %d\n", httpCode);

        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) 
        {        
          Serial.println("Send Data");
        }
      } 
      else 
      {
        Serial.printf("[HTTPS] POST... failed, error: %s\n", https.errorToString(httpCode).c_str());
      }
      https.end();
    }
  } 
  else 
  {
    Serial.printf("Unable to connect to wifi\n");
  }
  Serial.println();
  Serial.println("Waiting 60sec before the next round...");
  delay(15000);
}