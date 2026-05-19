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
#define TRDSIZE 20

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

struct Trentry {
    unsigned long id;
    float temp;
    float hum;
};
Trentry trend[TRDSIZE];
int tridx = 0;
unsigned long trcnt = 0;

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
        server.sendHeader("Cache-Control", "no-cache");
        server.send_P(200,"text/html", MAIN_page);
    });
    server.on("/data", [](){
        char data[256];
        snprintf(data, sizeof(data),"{"
            "\"lux\":%d,"
            "\"temp\":%.1f,"
            "\"dp\":%.1f,"
            "\"hum\":%.0f,"
            "\"absHum\":%.2f,"
            "\"pres\":%.1f,"
            "\"wb\":%.1f}",
            current.lux,
            current.temp,
            current.dewPoint(),
            current.hum,
            current.absHum(),
            current.pres,
            current.wetBulb()
        );
        server.sendHeader("Cache-Control", "no-cache");
        server.send(200, "application/json", data);
    });
    server.on("/trend", [](){
        char data[1024];
        int pos = 0;

        pos += snprintf(data, sizeof(data),"{"
            "\"latest\":%d,"
            "\"format\":[\"id\",\"tempC\",\"hum%%\"],"
            "\"values\":[",
            TRDSIZE
        );

        for (int i = 0; i < TRDSIZE; i++){
            int x =(tridx + i) % TRDSIZE;
            pos += snprintf(data + pos, sizeof(data) - pos,
                "[%lu,%.1f,%.1f]%s",
                trend[x].id,
                trend[x].temp,
                trend[x].hum,
                (i < TRDSIZE - 1 ? "," : "")
            );
        }

        snprintf(data + pos, sizeof(data) - pos, "]}");

        server.sendHeader("Cache-Control", "no-cache");
        server.send(200, "application/json", data);
    });
    server.on("/info", [](){
        char data[512];
        snprintf(data, sizeof(data),"{"
            "\"device\":\"ESP8266 WeatherStation\","
            "\"uptime_s\":%lu,"
            "\"trend_entries\":%d,"
            "\"trend_interval\":15,"
            "\"ldr_threshold\":320,"
            "\"endpoints\":{"
                "\"/\":\"Frontend UI\","
                "\"/data\":\"Current Measurements\","
                "\"/trend\":\"Trend over last 5 minutes\","
                "\"/info\":\"Metadata\""
            "}}",
            millis() / 1000,
            TRDSIZE
        );
        server.send(200, "application/json", data);
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
    if (ticker/TICK %  1 == 0) readBMP();
    if (ticker/TICK %  2 == 0) readDHT();
    if (ticker/TICK %  4 == 0) readLDR();
    if (ticker/TICK % 15 == 0) saveTRD();
    printValues((ticker / 30) % 2);
}

void readBMP() {
    float p = bmp.readPressure();
    float t = bmp.readTemperature();

    if (isnan(p) || isnan(t)) {
        Serial.println("BMP280 values malformed");
        return;
    }

    current.pres = p / 100.0f;
    current.temp = t;
}

void readDHT() {
    float t = dht.readTemperature();
    float h = dht.readHumidity();
    
    if (isnan(t) || isnan(h)) {
        Serial.println("DHT values malformed");
        return;
    }
    
    current.temp = t;
    current.hum  = h;
}

void readLDR(){
    int lux = analogRead(LDR);
    current.lux = lux;
    Serial.printf("L: %d\n", lux);
}

void saveTRD(){
    trend[tridx].id = trcnt;
    trend[tridx].temp = current.temp;
    trend[tridx].hum  = current.hum;
    trcnt++;
    tridx = (tridx + 1) % TRDSIZE;
}

void printValues(int start){
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);

    float t = current.temp;
    display.drawBitmap( 0 + start,  0 + start , ICON_TEMP, 8, 8, WHITE);
    display.setCursor( 10 + start,  0 + start);
    display.printf("T: %.1fC", t);
    Serial.printf("T: %.1fC\n", t);

    float h = current.hum;
    display.drawBitmap(64 + start,  0 + start , ICON_HUM,  8, 8, WHITE);
    display.setCursor( 74 + start,  0 + start);
    display.printf("H: %.0f%%", h);
    Serial.printf("H: %.0f%%\n", h);

    float dp = current.dewPoint();
    display.drawBitmap( 0 + start, 32 + start , ICON_DP,   8, 8, WHITE);
    display.setCursor( 10 + start, 32 + start);
    display.printf("D: %.1fC", dp);
    Serial.printf("D: %.1fC\n", dp);

    float p = current.pres;
    display.drawBitmap(64 + start, 32 + start , ICON_PRES, 8, 8, WHITE);
    display.setCursor( 74 + start, 32 + start);
    display.printf("P: %.0f", p);
    Serial.printf("P: %.0f\n", p);

    display.display();
    Serial.printf("AH: %.1f\n",current.absHum());
    Serial.printf("WB: %.1f\n", current.wetBulb());
}