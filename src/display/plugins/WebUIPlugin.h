#ifndef WEBUIPLUGIN_H
#define WEBUIPLUGIN_H

#define ELEGANTOTA_USE_ASYNC_WEBSERVER 1

#include <DNSServer.h>

#include "GitHubOTA.h"
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <display/core/Plugin.h>
#include <display/util/PsramAllocator.h>

constexpr size_t UPDATE_CHECK_INTERVAL = 30 * 60 * 1000;
constexpr size_t CLEANUP_PERIOD = 5 * 1000;
constexpr size_t STATUS_PERIOD = 500;
constexpr size_t DNS_PERIOD = 100;

const String LOCAL_URL = "http://4.4.4.1/";
const String RELEASE_URL = "https://github.com/jniebuhr/gaggimate/releases/";

class ProfileManager;

class WebUIPlugin : public Plugin {
  public:
    WebUIPlugin();
    void setup(Controller *controller, PluginManager *pluginManager) override;
    void loop() override;

  private:
    void setupServer();
    void start();
    void stop();

    void tick();
    void runUpdate();
    void checkUpdate();

    // Websocket handlers
    void handleWebSocketData(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data,
                             size_t len);
    void handleOTASettings(uint32_t clientId, JsonDocument &request);
    void handleOTAStart(uint32_t clientId, JsonDocument &request);
    void handleAutotuneStart(uint32_t clientId, JsonDocument &request);
    void handleProfileRequest(uint32_t clientId, JsonDocument &request);
    void handleFlushStart(uint32_t clientId, JsonDocument &request);

    // HTTP handlers
    void handleSettings(AsyncWebServerRequest *request) const;
    void handleBLEScaleList(AsyncWebServerRequest *request);
    void handleBLEScaleScan(AsyncWebServerRequest *request);
    void handleBLEScaleConnect(AsyncWebServerRequest *request);
    void handleBLEScaleInfo(AsyncWebServerRequest *request);
    void updateOTAStatus(const String &version);
    void updateOTAProgress(uint8_t phase, int progress);
    void sendAutotuneResult();
    void sendAutotuneFailed();

    // Broadcast a JsonDocument to all WebSocket clients with a single internal
    // allocation. `doc.as<String>()` builds an Arduino String on the internal
    // heap via doubling reallocs and then textAll() copies it into a message
    // buffer — two-plus allocations per broadcast, dropped into the middle of
    // the file-serving burst during a statistics build, which generates heap
    // fragmentation. measureJson + makeBuffer + serializeJson writes straight
    // into one exact-sized buffer instead. [GM-90]
    void broadcastJson(JsonDocument &doc);

    // Core dump download
    void handleCoreDumpDownload(AsyncWebServerRequest *request);

    GitHubOTA *ota = nullptr;
    AsyncWebServer server;
    AsyncWebSocket ws;
    Controller *controller = nullptr;
    PluginManager *pluginManager = nullptr;
    DNSServer *dnsServer = nullptr;
    ProfileManager *profileManager = nullptr;

    unsigned long lastUpdateCheck = 0;
    unsigned long lastCleanup = 0;
    unsigned long lastDns = 0;
    bool updating = false;
    bool apMode = false;
    bool serverRunning = false;
    String updateComponent = "";
    float currentBluetoothWeight = 0.0f;
    JsonDocument statusDoc{&psramAllocator};

    xTaskHandle loopTaskHandle;
    xTaskHandle runUpdateTaskHandle;
    xTaskHandle checkUpdateTaskHandle;
    static void loopTask(void *arg);
    static void runUpdateTask(void *arg);
    static void checkUpdateTask(void *arg);
};

#endif // WEBUIPLUGIN_H
