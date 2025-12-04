#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "index_html.h"

const char* ssid = "MicroTik6_2GHz";
const char* psk = "rasp1234rasp";
const char* ap_ssid = "MomClickHereForInternet";
const char* ap_psk = "iusearchbtw";

//#define AP_MODE
ESP8266WebServer server(80);

#define LED1 D1
#define LED2 D2
#define LED3 D3
#define LED4 D4
#define LED13 D13

int ledPins[] = {D1, D2, D3, D4};
bool ledStates[] = {false, false, false, false};

void setup() {
  Serial.begin(115200);
  Serial.println();
  for (int i = 0; i < 4; i++) {
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], LOW);
  }

  // Starting AP mode
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ap_ssid, ap_psk);
  delay(300);
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());

  // Root page
  server.on("/", []() {
    server.send_P(200, "text/html", MAIN_page);
  });
  // Toggle LED endpoint
  server.on("/toggle", []() {
    if (!server.hasArg("led") || !server.hasArg("state")) {
      server.send(400, "text/plain", "Missing led/state");
      return;
    }

    String ledId = server.arg("led");
    String state = server.arg("state");
    int index = ledId.substring(3).toInt() - 1;

    if (index >= 0 && index < 4) {
      bool on = (state == "true");
      ledStates[index] = on;
      digitalWrite(ledPins[index], on ? HIGH : LOW);
      Serial.printf("Set %s -> %s\n", ledId.c_str(), on ? "ON" : "OFF");
      server.send(200, "text/plain", String(on ? "1" : "0"));
    } else {
      server.send(400, "text/plain", "Invalid LED");
    }
  });

    // Get LED states endpoint
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

void loop() {
  server.handleClient();
}
