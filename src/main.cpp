/**
 * @file main.cpp
 * @brief ESP32 with TPM408 ST7789 display — Modular Quiz Game
 */
#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <soc/soc.h>
#include <soc/rtc_cntl_reg.h>

#include "system/config.h"
#include "core/actuators.h"
#include "core/display.h"
#include "core/inputs.h"
#include "game/helper.h"
#include "game/game_mechanics.h"
#include "game/game_logics.h"

// ==========================================
// GLOBAL OBJECTS
// ==========================================

// TFT Display — pins come from User_Setup.h
TFT_eSPI tft = TFT_eSPI();

// Singletons
SystemConfig &config = SystemConfig::get();
Inputs &inputs = Inputs::get();
Helper &helper = Helper::get();
GameMechanics &gameMechanics = GameMechanics::get();
GameLogics &gameLogics = GameLogics::get();
DisplayOutputs &display = DisplayOutputs::get();
Actuators &actuators = Actuators::get();
Session &session = Session::get();

// ==========================================
// SETUP
// ==========================================

void setup()
{
    // Disable brownout detector to prevent reboots during WiFi startup
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

    Serial.begin(115200);
    delay(1000);
    Serial.flush();

    Serial.println("\n>>> SYSTEM BOOTING <<<");

    config.initialize();

    // TFT init — pins + size from User_Setup.h
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);

    display.showSplashScreen();
    delay(2000);

    actuators.initializeActuatorsPins();
    inputs.initializeInputsPins();
    gameMechanics.initialize();
    gameLogics.initialize();

    // Touch calibration + mirror. Flip mirror flags if axes are inverted.
    uint16_t calData[5] = {361, 3305, 459, 3183, 1};
    tft.setTouch(calData);
    inputs.setTouchMirror(false, false);

    helper.initPlayerScores();

    randomSeed(analogRead(0));

    display.showNoCurrentSessionScreen();

    Serial.println("Press START to begin!");
}

// ==========================================
// BUTTON HANDLERS
// ==========================================

void handleSingleClick()
{
    Serial.println("START pressed! Display state: " + String(config.displayState));

    switch (config.displayState)
    {
    case SystemConfig::SHOW_QUESTION:
        Serial.println("Already showing a question - press A/B/C/D to answer");
        break;

    case SystemConfig::SHOW_CORRECT:
        Serial.println("➡️ Advancing to next question...");
        gameLogics.advanceToNextQuestion();
        break;

    case SystemConfig::SHOW_INCORRECT:
        Serial.println("➡️ Retrying question...");
        gameLogics.retryQuestion();
        break;

    case SystemConfig::SHOW_COMPLETE:
        Serial.println("Transitioning to Leaderboard view...");
        display.showLeaderboardScreen();
        break;

    case SystemConfig::SHOW_LEADERBOARD:
        Serial.println("Leaderboard active — restarting session...");
        gameLogics.tryNewSession();
        break;

    case SystemConfig::SHOW_UPLOADED:
        Serial.println("Session uploaded... Start the session");
        gameLogics.startQuiz();
        break;

    default:
        Serial.println("⚠️ Unknown display state");
        break;
    }
}

void handleDoubleClick()
{
    Serial.println("⚡ START DOUBLE-CLICK DETECTED!");

    switch (config.displayState)
    {
    case SystemConfig::SHOW_NO_CURRENT_SESSION:
        if (!config.isSessionEnabled)
        {
            Serial.println("📡 Powering ON WiFi Session...");
            session.enableWifiSession();
        }
        else
        {
            Serial.println("💤 Powering OFF WiFi Session...");
            session.disableWifiSession();
        }
        // Redraw to reflect the new WiFi state
        display.showNoCurrentSessionScreen();
        break;

    default:
        break;
    }
}

void handleLongPress()
{
    Serial.println("⚡ START LONG-PRESS DETECTED!");

    switch (config.displayState)
    {
    case SystemConfig::SHOW_QUESTION:
    case SystemConfig::SHOW_CORRECT:
    case SystemConfig::SHOW_INCORRECT:
    case SystemConfig::SHOW_COMPLETE:
    case SystemConfig::SHOW_LEADERBOARD:
    case SystemConfig::SHOW_UPLOADED:
        gameLogics.clearSession();
        break;

    default:
        Serial.println("🚫 Long-press only clears session during active gameplay.");
        break;
    }
}

// ==========================================
// LOOP
// ==========================================

void loop()
{
    // --- Background services ---
    session.processSession();
    if (config.isSessionEnabled)
    {
        session.processDNSServer();
    }

    if (!config.endGameRunOnce)
    {
        gameMechanics.endGameOnePlayer();
    }

    // --- Physical input ---
    Inputs::ButtonEvent startEvent = inputs.processSmartButton();

    switch (startEvent)
    {
    case Inputs::ButtonEvent::SINGLE_CLICK:
        handleSingleClick();
        break;

    case Inputs::ButtonEvent::DOUBLE_CLICK:
        handleDoubleClick();
        break;

    case Inputs::ButtonEvent::LONG_PRESS:
        handleLongPress();
        break;

    case Inputs::ButtonEvent::NONE:
    default:
        break;
    }

    // --- Touch input (only while a question is live) ---
    if (config.displayState == SystemConfig::SHOW_QUESTION &&
        !config.answered &&
        config.currentQuestionPos < (int)config.questionOrder.size())
    {
        char ans = inputs.choicesButtonProcessor();
        if (ans != ' ')
        {
            gameLogics.handleAnswer(ans);
        }
    }

    // --- Feedback timer ---
    gameLogics.handleFeedbackTimer();

    // --- Memory check (every 30 seconds) ---
    static unsigned long lastMemCheck = 0;
    if (millis() - lastMemCheck > 30000)
    {
        lastMemCheck = millis();
        gameLogics.checkMemory();
    }

    delay(10);
}
