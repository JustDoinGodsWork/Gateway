#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "";       // Variable to store the WiFi SSID
const char* password = "";   // Variable to store the WiFi password
const char* serverUrl = "";  // Variable to store the server URL
const char* serverPass = ""; // Variable to store the server password

WebServer server(80);        // Create a web server on port 80

void handleRoot() {
  String html = "<html><body>";
  html += "<h2>WiFi Credentials</h2>";
  html += "<form method='post' action='/save'>";
  html += "SSID: <input type='text' name='ssid'><br>";
  html += "Password: <input type='password' name='password'><br>";
  html += "<h2>Server Details</h2>";
  html += "URL: <input type='text' name='url'><br>";
  html += "Password: <input type='password' name='serverPass'><br>";
  html += "<input type='submit' value='Save'>";
  html += "</form></body></html>";

  server.send(200, "text/html", html);
}

// void handleSave() {
//   ssid = server.arg("ssid").c_str();
//   password = server.arg("password").c_str();
//   serverUrl = server.arg("url").c_str();
//   serverPass = server.arg("serverPass").c_str();

//   String html = "<html><body>";
//   html += "<h2>Settings Saved</h2>";
//   html += "<p>WiFi SSID: " + String(ssid) + "</p>";
//   html += "<p>WiFi Password: " + String(password) + "</p>";
//   html += "<p>Server URL: " + String(serverUrl) + "</p>";
//   html += "<p>Server Password: " + String(serverPass) + "</p>";
//   html += "</body></html>";

//   server.send(200, "text/html", html);

//   Serial.println("WiFi SSID: " + String(ssid));
//   Serial.println("WiFi Password: " + String(password));
//   Serial.println("Server URL: " + String(serverUrl));
//   Serial.println("Server Password: " + String(serverPass));
// }

void handleSave() {
  ssid = server.arg("ssid").c_str();
  password = server.arg("password").c_str();
  serverUrl = server.arg("url").c_str();
  serverPass = server.arg("serverPass").c_str();

  String html = "<html><body>";
  html += "<h2>Settings Saved</h2>";
  html += "<p>WiFi SSID: " + String(ssid) + "</p>";
  html += "<p>WiFi Password: " + String(password) + "</p>";
  html += "<p>Server URL: " + String(serverUrl) + "</p>";
  html += "<p>Server Password: " + String(serverPass) + "</p>";
  html += "</body></html>";

  server.send(200, "text/html", html);

  Serial.println("WiFi SSID: " + String(ssid));
  Serial.println("WiFi Password: " + String(password));
  Serial.println("Server URL: " + String(serverUrl));
  Serial.println("Server Password: " + String(serverPass));
}


void setup() {
  Serial.begin(115200);

  WiFi.softAP("ESP32AP", ""); // Set up ESP32 as an access point
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  server.on("/", handleRoot);
  server.on("/save", handleSave);
  server.begin();
}

void loop() {
  server.handleClient();
}
