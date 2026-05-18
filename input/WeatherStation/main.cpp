#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DHT.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_SSD1306.h>
#include "index_html.h"

const char* ssid = "MicroTik6_2GHz";
const char* psk = "pwd";
const char* ap_ssid = "MomClickHereForInternet";
const char* ap_psk = "ap_pwd";

#define AP_MODE
ESP8266WebServer server(80);

#define TICK 1000

#define LDR         A0
#define BMP_SCL     D1
#define OLED_SCL    D1
#define BMP_SDA     D2
#define OLED_SDA    D2 
#define DHTPIN      D5
#define DHTTYPE     DHT11
#define OLED_RESET  0

DHT dht(DHTPIN, DHTTYPE);
Adafruit_BMP280 bmp;
Adafruit_SSD1306 display(128, 64);

struct Weather{
    int lux;
    float temp;
    float hum;
    float pres;
    float dewPoint() const {
        const float a = 17.62f;
        const float b = 243.12f;

        float rh = hum / 100.0f;
        float gamma = (a * temp) / (b + temp) + log(rh);

        return (b * gamma) / (a - gamma);
    }

    float absHum() const {
        float rh = hum / 100.0f;
        float es = 6.112f * exp((17.67f * temp) / (temp + 243.5f));
        float e  = rh * es;

        return (2.1674f * e) / (273.15f + temp);
    }

    float wetBulb() const {
        float rh = hum;

        return temp * atan(0.151977f * sqrtf(rh + 8.313659f))
             + atan(temp + rh)
             - atan(rh - 1.676331f)
             + 0.00391838f * powf(rh, 1.5f) * atan(0.023101f * rh)
             - 4.686035f;
    }
};
Weather current;

void setup(){
    Serial.begin(115200);
    Serial.println();
    #ifdef AP_MODE
        Serial.println("Starting in AP Mode");
        WiFi.mode(WIFI_AP);
        WiFi.softAP(ap_ssid, ap_psk);
        delay(300);
        Serial.print("AP IP: ");
        Serial.println(WiFi.softAPIP());
    #else
        Serial.println("Starting in STA Mode");
        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid, psk);
        while (WiFi.status() != WL_CONNECTED){
            delay(500);
            Serial.print(".");
        }
        Serial.print("Connected! IP:");
        Serial.println(WiFi.localIP());
    #endif
    server.on("/", [](){
        server.send_P(200,"text/html", MAIN_page);
    });
    server.on("/data", [](){
        char lux[32];
        snprintf(lux, sizeof(lux), "\"lux\":%d", current.lux);
        Serial.println(lux);

        char temp[32];
        snprintf(temp, sizeof(temp), "\"temp\":%.1f", current.temp);
        Serial.println(temp);

        char dp[32];
        snprintf(dp, sizeof(dp), "\"dp\":%.1f", current.dewPoint());
        Serial.println(dp);

        char hum[32];
        snprintf(hum, sizeof(hum), "\"hum\":%.0f", current.hum);
        Serial.println(hum);

        char absHum[32];
        snprintf(absHum, sizeof(absHum), "\"absHum\":%.2f", current.absHum());
        Serial.println(absHum);

        char pres[32];
        snprintf(pres, sizeof(pres), "\"pres\":%.1f", current.pres);
        Serial.println(pres);

        char wb[32];
        snprintf(wb, sizeof(wb), "\"wb\":%.1f", current.wetBulb());
        Serial.println(wb);
        
        char data[256];
        snprintf(
            data, sizeof(data),
            "{%s,%s,%s,%s,%s,%s,%s}",
            lux, temp, dp, hum, absHum, pres, wb
        );

        server.sendHeader("Cache-Control", "no-cache");
        server.send(200,"application/json", data);
    });
    server.onNotFound([](){
        server.send(404,"text/plain","Resource not found");
    });
    
    Serial.println("Starting Server");
    server.begin();
    
    Serial.println("Starting DHT11");
    dht.begin();
    
    Serial.println("Starting bmp");
    if (!bmp.begin(0x76)) {
        Serial.println("BMP280 setup failed");
    } else {
        Serial.println("BMP280 ready");
    }

    Serial.println("Starting OLED Display");
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)){
        Serial.println("OLED setup failed");
    } else {
        Serial.println("OLED ready");
    }
}

void loop(){
    unsigned long ticker = millis();
    while (millis() < ticker+TICK) {
        server.handleClient();
    }
    if (ticker/TICK % 4 == 0) readLDR();
    if (ticker/TICK % 1 == 0) readDHT();
    if (ticker/TICK % 2 == 0) readBMP();
    updateDisplay();
}

void readLDR(){
    current.lux = analogRead(LDR);
}

void readDHT() {
    float t = dht.readTemperature();
    float h = dht.readHumidity();

    if (isnan(t) || isnan(h)) {
        Serial.println("DHT values malformed");
        Serial.println(t,h);
        return;
    }

    current.temp = t;
    current.hum  = h;
}

void readBMP() {
    float p = bmp.readPressure();

    if (isnan(p)) {
        Serial.println("bmp280 value malformed");
        Serial.println(p);
        return;
    }

    current.pres = p / 100.0f;
}

void updateDisplay(){
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);

    display.setCursor(0, 0);
    display.printf("T: %.1fC", current.temp);

    display.setCursor(64, 0);
    display.printf("H: %.0f%%", current.hum);

    display.setCursor(0, 32);
    display.printf("D: %.1fC", current.dewPoint());

    display.setCursor(64, 32);
    display.printf("P: %.0f", current.pres);

    display.display();
}