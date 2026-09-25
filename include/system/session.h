/**
 * @file config/session.h
 * @brief WiFi session management — AP mode + HTTP server + DNS
 */
#ifndef SESSION_H
#define SESSION_H

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
#include <Arduino.h>
#include "core/display.h"

class Session
{
public:
    // ==========================================
    // SINGLETON
    // ==========================================
    static Session &get();

    // ==========================================
    // LIFECYCLE
    // ==========================================
    void enableWifiSession();
    void disableWifiSession();

    // ==========================================
    // RUNTIME
    // ==========================================
    void processSession();
    void processDNSServer();

    // ==========================================
    // STATE
    // ==========================================
    unsigned long pendingDisableTime = 0;
    bool restartWifiPending = false;

private:
    // ==========================================
    // HELPERS
    // ==========================================
    static void _handleStartSessionBody(AsyncWebServerRequest *request,
                                        uint8_t *data, size_t len,
                                        size_t index, size_t total);

    // ==========================================
    // MEMBERS
    // ==========================================
    DisplayOutputs &display;   // singleton reference
    AsyncWebServer  server;
    DNSServer       dnsServer;
    String          sessionBody;

    // ==========================================
    // CONSTRUCTOR
    // ==========================================
    Session();
    ~Session();

    Session(const Session &)            = delete;
    Session &operator=(const Session &) = delete;
};

#endif // SESSION_H