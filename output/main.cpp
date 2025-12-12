#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "index_html.h"

const char* ssid = "MicroTik6_2GHz";
const char* psk = "rasp1234rasp";
const char* ap_ssid = "MomClickHereForInternet";
const char* ap_psk = "iusearchbtw";

//#define AP_MODE
ESP8266WebServer server(80);

// Registering LED pins and states
#define LED1 D4
#define LED2 D3
#define LED3 D2
#define LED4 D1

int ledPins[] = {D4, D3, D2, D1};
bool ledStates[] = {false, false, false, false};

// Setting up WiFi and server
void setup() {
  Serial.begin(115200);
  Serial.println();
  for (int i = 0; i < 4; i++) {
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], LOW);
  }
  // Starting Entrypoint
  #ifdef AP_MODE
    Serial.println("Starting in AP Mode");
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ap_ssid, ap_psk);
  #else
    Serial.println("Starting in STA Mode");
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, psk);
    while (WiFi.status() != WL_CONNECTED) delay(500);
    Serial.println("Connected! IP: %v", WiFi.localIP());
  #endif
  delay(300);
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());
  // Load index page on root
  server.on("/", []() {
    server.send_P(200, "text/html", MAIN_page);
  });
  // Load '/toggle' endpoint for toggling LEDs
  server.on("/toggle", []() {
    if (!server.hasArg("led") || !server.hasArg("state")) {
      server.send(400, "text/plain", "Missing led/state");
      return;
    }
    // Defining LED IDs and states, then parsing values from frontend
    String ledId = server.arg("led");
    String state = server.arg("state");
    int i = ledId.substring(3).toInt() -1; // Getting LED index and adjusting for 0-based array 
    if (i >= 0 && i < 4) {
      bool on = (state == "true"); // Parsing state
      ledStates[i] = on;
      digitalWrite(ledPins[i], on ? HIGH : LOW);
      Serial.printf("Set %s -> %s\n", ledId.c_str(), on ? "ON" : "OFF");
      server.send(200, "text/plain", String(on ? "1" : "0"));
    } else {
      server.send(400, "text/plain", "Invalid LED");
    }
  });

  // Load '/state' endpoint to get current LED states
  server.on("/state", []() {
    String json = "[";
    for (int i = 0; i < 4; i++) {
      json += (ledStates[i] ? "true" : "false");
      if (i < 3) json += ",";
    }
    json += "]";
    server.send(200, "application/json", json);
  });
  server.begin();
  Serial.println("HTTP Server started");
}

// Main loop to handle client requests and flicker effect
void loop() {
  server.handleClient();
  flicker();
}

// Function to create flickering effect
void flicker(){
  int led = random(0,4);
  int chance = random(0,10);
  if (chance > 7 || millis() > 200 && chance > 3){
    if (ledStates[led] == true){
      int brightness = random(20, 255);
      analogWrite(ledPins[led], brightness);
    }
  }
}