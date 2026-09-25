/**
 * @file system/session.cpp
 * @brief WiFi AP + HTTP session upload handling
 */
#include "system/session.h"
#include "system/config.h"
#include <ArduinoJson.h>
#include <ESPmDNS.h>

// ==========================================
// SINGLETON & LIFECYCLE
// ==========================================

Session &Session::get()
{
    static Session instance;
    return instance;
}

Session::Session() : server(80), display(DisplayOutputs::get())
{
    // Route registration — done once at startup

    // GET /state
    server.on("/state", HTTP_GET, [this](AsyncWebServerRequest *request)
              {
        SystemConfig &config = SystemConfig::get();
        String json = "{";
        json += "\"sessionTitle\": \"" + config.sessionTitle + "\",";
        json += "\"questionCount\": " + String(config.questionList.size()) + ",";
        json += "\"currentPlayer\": \"" + config.currentPlayerName + "\"";
        json += "}";
        request->send(200, "application/json", json); });

    // POST /admin — disabled: WiFi configuration changes are not supported
    // server.on("/admin", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
    // {
    //     Session &session = Session::get();
    //     if (index == 0) session.sessionBody = "";
    //     session.sessionBody += String((char*)data, len);
    //
    //     if (index + len == total) {
    //         JsonDocument doc;
    //         DeserializationError error = deserializeJson(doc, session.sessionBody);
    //
    //         if (error) {
    //             request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
    //         } else {
    //             request->send(403, "application/json", "{\"error\":\"WiFi configuration changes are disabled\"}");
    //         }
    //         session.sessionBody = "";
    //     }
    // });

    // POST /upload-session — body is handled by the static helper below
    server.on("/upload-session", HTTP_POST, [this](AsyncWebServerRequest *request)
              {
            Serial.println("📥 POST /upload-session received");
            request->send(200, "text/plain", "OK"); }, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
              { Session::_handleStartSessionBody(request, data, len, index, total); });
}

Session::~Session()
{
}

// ==========================================
// WIFI SESSION
// ==========================================

void Session::enableWifiSession()
{
    Serial.println("🌐 Enabling WiFi Session...");

    SystemConfig &config = SystemConfig::get();

    WiFi.mode(WIFI_AP);
    bool success = WiFi.softAP(config.wifiSSID.c_str(), config.wifiPassword.c_str());

    if (!success)
    {
        Serial.println("❌ WiFi AP Failed to start");
        return;
    }

    IPAddress apIP = WiFi.softAPIP();
    Serial.println("✅ WiFi AP Started");
    Serial.print("SSID: ");
    Serial.println(config.wifiSSID);
    Serial.print("IP:   ");
    Serial.println(apIP);

    server.begin();
    Serial.println("✅ AsyncWebServer Started");

    if (MDNS.begin(config.mdnsHostname.c_str()))
    {
        Serial.print("✅ mDNS Started: ");
        Serial.print(config.mdnsHostname);
        Serial.println(".local");
    }
    else
    {
        Serial.println("❌ mDNS Failed to start");
    }

    dnsServer.start(config.dnsPort, "*", apIP);
    Serial.println("✅ DNS Server Started");

    config.isSessionEnabled = true;
}

void Session::disableWifiSession()
{
    Serial.println("🌐 Disabling WiFi Session...");

    SystemConfig &config = SystemConfig::get();

    MDNS.end();
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_OFF);

    config.isSessionEnabled = false;
    Serial.println("✅ WiFi Session Disabled (Power Saved)");
}

// ==========================================
// RUNTIME LOOP
// ==========================================

void Session::processSession()
{
    if (pendingDisableTime > 0 && millis() >= pendingDisableTime)
    {
        if (restartWifiPending)
        {
            Serial.println("♻️ Restarting WiFi AP with new settings...");
            disableWifiSession();
            enableWifiSession();
            restartWifiPending = false;
        }
        else
        {
            Serial.println("⏰ Pending WiFi disable timer expired.");
            disableWifiSession();
        }
        pendingDisableTime = 0;
    }
}

void Session::processDNSServer()
{
    dnsServer.processNextRequest();
}

// ==========================================
// UPLOAD HANDLER
// ==========================================

void Session::_handleStartSessionBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
{
    Session &session = Session::get();

    // Accumulate the body
    if (index == 0)
        session.sessionBody = "";
    session.sessionBody += String((char *)data, len);

    // Process when the full body is received
    if (index + len == total)
    {
        Serial.println("📥 Received session setup payload...");
        Serial.println("📝 Raw JSON payload:");
        Serial.println(session.sessionBody);
        Serial.println("----------------------------------------");

        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, session.sessionBody);

        if (error)
        {
            Serial.print("❌ JSON Parse Error: ");
            Serial.println(error.c_str());
            request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
        }
        else
        {
            SystemConfig &config = SystemConfig::get();

            // 1. Session title
            if (!doc["sessionTitle"].isNull())
            {
                config.sessionTitle = doc["sessionTitle"].as<String>();
                Serial.println("📌 Session Title:");
                Serial.println("   " + config.sessionTitle);
            }

            // 2. Questions
            if (!doc["questions"].isNull())
            {
                config.questionList.clear();
                JsonArray questions = doc["questions"];
                int qId = 1;

                Serial.println("📝 Questions Received:");
                Serial.println("   Total: " + String(questions.size()));
                Serial.println("----------------------------------------");

                for (JsonObject qObj : questions)
                {
                    SystemConfig::Question q;
                    q.id = qId++;
                    q.text = qObj["question"].as<String>();

                    JsonArray options = qObj["options"];
                    if (options.size() >= 4)
                    {
                        q.optionA = options[0].as<String>();
                        q.optionB = options[1].as<String>();
                        q.optionC = options[2].as<String>();
                        q.optionD = options[3].as<String>();
                    }

                    int ansIdx = qObj["initialCharAns"].as<int>();
                    q.initialCharAns = (ansIdx >= 0 && ansIdx <= 3) ? (char)('A' + ansIdx) : 'A';

                    q.originalOptionA = q.optionA;
                    q.originalOptionB = q.optionB;
                    q.originalOptionC = q.optionC;
                    q.originalOptionD = q.optionD;

                    config.questionList.push_back(q);

                    Serial.println("┌─────────────────────────────────────────");
                    Serial.printf("│ Question #%d\n", q.id);
                    Serial.printf("│ 📝 %s\n", q.text.c_str());
                    Serial.println("│");
                    Serial.printf("│   A. %s\n", q.optionA.c_str());
                    Serial.printf("│   B. %s\n", q.optionB.c_str());
                    Serial.printf("│   C. %s\n", q.optionC.c_str());
                    Serial.printf("│   D. %s\n", q.optionD.c_str());
                    Serial.printf("│ ✅ Correct Answer: %c\n", q.initialCharAns);
                    Serial.println("└─────────────────────────────────────────");
                }
                config.nextQuestionId = qId;

                Serial.println("✅ Total Questions Stored: " + String(config.questionList.size()));
            }
            else
            {
                Serial.println("⚠️ No 'questions' field in payload");
            }

            // 3. Players
            if (!doc["players"].isNull())
            {
                config.playerScores.clear();
                JsonArray players = doc["players"];

                Serial.println("👥 Players Received:");
                Serial.println("   Total: " + String(players.size()));
                Serial.println("----------------------------------------");

                for (JsonObject pObj : players)
                {
                    SystemConfig::PlayerScore p;
                    p.name = pObj["name"].as<String>();
                    p.score = pObj["score"].as<int>();
                    p.isEliminated = pObj["isEliminated"].as<bool>();
                    config.playerScores.push_back(p);

                    Serial.printf("   👤 %s | Score: %d | %s\n",
                                  p.name.c_str(),
                                  p.score,
                                  p.isEliminated ? "Eliminated" : "Active");
                }
                Serial.println("✅ Total Players Stored: " + String(config.playerScores.size()));
            }
            else
            {
                Serial.println("⚠️ No 'players' field in payload");
            }

            // 4. Summary
            Serial.println("========================================");
            Serial.println("✅ SESSION SETUP COMPLETE!");
            Serial.println("========================================");
            Serial.println("📌 Session: " + config.sessionTitle);
            Serial.println("📝 Questions: " + String(config.questionList.size()));
            Serial.println("👥 Players: " + String(config.playerScores.size()));
            Serial.println("========================================");

            request->send(200, "application/json", "{\"status\":\"success\",\"message\":\"Session setup complete\"}");

            // Delay WiFi shutdown so the client has time to receive the response
            session.pendingDisableTime = millis() + 2000;
            Serial.println("📡 Session uploaded. WiFi will power off in 2 seconds...");

            session.display.showStartSessionScreen();
        }

        session.sessionBody = "";
    }
}