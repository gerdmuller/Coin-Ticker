#include "display.h"
#include "gecko.h"
#include "handler.h"
#include "http_json.h"
#include "pre.h"
#include "settings.h"
#include "utils.h"
#include "wifi_utils.h"
#include <Arduino.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

ESP8266WebServer server(80);

// strange PlatformIO behavior:
// https://community.platformio.org/t/adafruit-gfx-lib-will-not-build-any-more-pio-5/15776/12
#include <Adafruit_I2CDevice.h>

HttpJson httpJson;
Settings settings;
Gecko gecko(httpJson, settings);
Display display(gecko, settings);
Handler handler(gecko, settings);

void setup(void)
{
    Serial.begin(115200);

#if COIN_THING_SERIAL > 0
    rst_info* ri(ESP.getResetInfoPtr());
    Serial.printf("\n---------------------\n");
    Serial.printf("Reset Info reason:   %u\n", ri->reason);
    Serial.printf("Reset Info exccause: %u", ri->exccause);
    Serial.printf("\n---------------------\n");
#endif

    display.begin();
    display.showAPQR();

    // setupWiFi();

    //WiFiManager
    //Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wifiManager;

    //reset settings - for testing
    //wifiManager.resetSettings();

    if (!wifiManager.autoConnect(String(HOST_NAME).c_str(), String(SECRET_AP_PASSWORD).c_str())) {
        Serial.println(F("failed to connect, we should reset as see if it connects"));
        delay(3000);
        ESP.reset();
        delay(5000);
    }

    //setupWiFi();

    Serial.printf("\nConnected\n IP address: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf(" Hostname: %s\n", WiFi.hostname().c_str());
    Serial.printf(" MAC: %s\n", WiFi.macAddress().c_str());

    if (gecko.ping()) {
        display.showAPIOK();
        delay(1500);
    }

    if (settings.begin(gecko)) {
        Serial.println(F("Settings valid!"));
    } else {
        Serial.println(F("Settings invalid!"));
    }

#if COIN_THING_SERIAL > 0
    Serial.printf("Settings coin:          >%s<\n", settings.coin());
    Serial.printf("Settings currency:      >%s<\n", settings.currency());
    Serial.printf("Settings number format: >%u<\n", settings.numberFormat());
    Serial.printf("Settings symbol:        >%s<\n", settings.symbol());
    Serial.printf("Settings name:          >%s<\n", settings.name());
#endif

    server.onNotFound([]() { // If the client requests any URI
        if (!handler.handleAction()
            && !handler.handleFileRead()) { // send it if it exists
            server.send(404, F("text/plain"), F("404: Not Found")); // otherwise, respond with a 404 (Not Found) error
        }
    });
    server.begin(); // Actually start the server

    Serial.println(F("\r\nInitialisation done."));
}

void loop()
{
    display.loop();
    server.handleClient();
}
