/**
 * @file session.h
 * @brief Responsible for API wifi
 */
#ifndef SESSION
#define SESSION
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
#include <Arduino.h>
#include "display.h"

class Session
{
public:
    /**
     * @brief Singleton accessor
     */
    static Session &get();

    void enableWifiSession();
    void disableWifiSession();
    void processSession();
    void processDNSServer();


    // Tracking
    bool isSessionEnabled = false;
    unsigned long pendingDisableTime = 0;
    bool restartWifiPending = false;

private:
    static void _handleStartSessionBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);

    Session();
    ~Session();
    Session(const Session &) = delete;
    Session &operator=(const Session &) = delete;
    DisplayOutputs &display; // Reference to singleton

    AsyncWebServer server;
    DNSServer dnsServer;
    String sessionBody;
};
;
#endif