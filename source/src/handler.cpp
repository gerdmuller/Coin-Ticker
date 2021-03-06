#include "handler.h"
#include "common.h"
#include "gecko.h"
#include "handler.h"
#include "pre.h"
#include "settings.h"
#include "utils.h"

#include <ESP8266WebServer.h>
extern ESP8266WebServer server;
extern String HostName;

using namespace std;

namespace {

String getContentType(const String& filename)
{
    if (filename.endsWith(".html")) {
        return F("text/html");
        //    } else if (filename.endsWith(".gz")) {
        //        return F("application/x-gzip");
    } else if (filename.endsWith(".css")) {
        return F("text/css");
    } else if (filename.endsWith(".json")) {
        return F("application/json");
    } else if (filename.endsWith(".js")) {
        return F("application/javascript");
    } else if (filename.endsWith(".ico")) {
        return F("image/x-icon");
    } else if (filename.endsWith(".jpg")) {
        return F("image/jpeg");
    } else if (filename.endsWith(".bmp")) {
        return F("image/bmp");
    }
    return F("text/plain");
}

} // anonymous namespace

Handler::Handler(Gecko& gecko, Settings& settings)
    : m_gecko(gecko)
    , m_settings(settings)
{
}

bool Handler::handleResetESP() const
{
    server.send(200, F("text/plain"), "1");

    delay(200);
    ESP.reset();
    return true;
}

bool Handler::handleResetSettings() const
{
    server.send(200, F("text/plain"), "1");

    m_settings.deleteFile();
    delay(200);
    ESP.reset();
    return true;
}

bool Handler::handleResetWiFi() const
{
    server.send(200, F("text/plain"), "1");

    delay(200);
    WiFi.disconnect();
    delay(200);
    ESP.reset();
    return true;
}

bool Handler::handleResetAll() const
{
    server.send(200, F("text/plain"), "1");

    m_settings.deleteFile();
    delay(200);
    WiFi.disconnect();
    delay(200);
    ESP.reset();
    return true;
}

bool Handler::handleGetVersion() const
{
    server.send(200, F("text/plain"), VERSION);
    return true;
}

bool Handler::handleGetName() const
{
    server.send(200, F("text/plain"), HostName);
    return true;
}

bool Handler::handleGetPrice()
{
    gecko_t price;
    gecko_t price2;
    gecko_t change_pct;
    m_gecko.price(0, price, price2, change_pct);

    String result;
    formatNumber(price, result, m_settings.numberFormat(), false, true);
    String ret;
    ret = m_settings.symbol(0);
    ret += F(": ");
    ret += result;
    ret += m_settings.currencySymbol();
    server.send(200, F("text/plain"), ret);
    return true;
}

bool Handler::handleForUpdate() const
{
    server.send(200, F("text/plain"), "1");

    // if file exists on startup, ESP goes into update mode
    File f = SPIFFS.open(FOR_UPDATE_FILE, "w");
    f.close();

    delay(200);
    ESP.reset();
    return true;
}

bool Handler::streamFile(const char* filename)
{
    LOG_FUNC

    SERIAL_PRINT("  File: ")
    SERIAL_PRINT(filename)

    String contentType = getContentType(filename);
    String filename_found;
    String filename_gz(filename);
    filename_gz += ".gz";

    if (SPIFFS.exists(filename_gz)) {
        SERIAL_PRINT(" found .gz: ")
        SERIAL_PRINT(filename_gz)
        filename_found = filename_gz;
    } else if (SPIFFS.exists(filename)) {
        filename_found = filename;
    } else {
        return false;
    }

    File file = SPIFFS.open(filename_found, "r");
    server.sendHeader(F("Access-Control-Allow-Origin"), "*");
    server.streamFile(file, contentType);
    file.close();
    SERIAL_PRINTLN(" - ok");
    return true;

    SERIAL_PRINTLN(" - does not exist!")
    return false;
}

bool Handler::handleSet() const
{
#if COIN_THING_SERIAL > 0
    Serial.printf("handleAction: set - parsed Query:\n");
    for (int ii = 0; ii < server.args(); ++ii) {
        Serial.print(server.argName(ii));
        Serial.print(" -> ");
        Serial.println(server.arg(ii));
    }
#endif

    if (server.hasArg(F("brightness"))) {
        m_settings.setBrightness(static_cast<uint8_t>(server.arg(F("brightness")).toInt()));
        streamFile(BRIGHTNESS_FILE);
    } else if (server.hasArg(F("json"))) {
        m_settings.set(server.arg(F("json")).c_str());
        streamFile(SETTINGS_FILE);
    } else {
        server.send(200, F("application/json"), F(R"({"error":"Nothing to set!"})"));
    }

    return true;
}

bool Handler::handleAction()
{
    String path(server.uri());
#if COIN_THING_SERIAL > 0
    Serial.printf("handleAction: path: %s\n", path.c_str());
#endif

    if (path == F("/action/set")) {
        return handleSet();
    } else if (path == F("/action/reset/esp")) {
        return handleResetESP();
    } else if (path == F("/action/reset/settings")) {
        return handleResetSettings();
    } else if (path == F("/action/reset/wifi")) {
        return handleResetWiFi();
    } else if (path == F("/action/reset/all")) {
        return handleResetAll();
    } else if (path == F("/action/reset/forupdate")) {
        return handleForUpdate();
    } else if (path == F("/action/get/version")) {
        return handleGetVersion();
    } else if (path == F("/action/get/name")) {
        return handleGetName();
    } else if (path == F("/action/get/price")) {
        return handleGetPrice();
    }
    return false;
}

bool Handler::handleFileRead()
{
    String path(server.uri());
#if COIN_THING_SERIAL > 0
    Serial.printf("handleFileRead: %s\n", path.c_str());
#endif

    if (path.endsWith("/")) {
        path += F("settings.html");
    }
    return streamFile(path.c_str());
}
