/**
 * @file main.cpp
 * @brief ESP32 with ST7735 display - Modular Quiz Game
 */
#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

#include "system/config.h"
#include "actuators.h"
#include "display.h"
#include "inputs.h"
#include "helper.h"
#include "game_mechanics.h"
#include "game_logics.h"
#include "session.h"
#include <soc/soc.h>           // Added for brownout disable
#include <soc/rtc_cntl_reg.h> // Added for brownout disable

// ==========================================
// GLOBAL OBJECTS
// ==========================================

// TFT Display
Adafruit_ST7735 tft = Adafruit_ST7735(
    SystemConfig::TFT_CS,
    SystemConfig::TFT_DC,
    SystemConfig::TFT_RST);

// Singleton references
SystemConfig &config = SystemConfig::get();
Inputs &inputs = Inputs::get();
Helper &helper = Helper::get();
GameMechanics &gameMechanics = GameMechanics::get();
GameLogics &gameLogics = GameLogics::get(); // <-- NEW
DisplayOutputs &display = DisplayOutputs::get();
Actuators &actuators = Actuators::get();

// Game session management (WiFi, Web Server, DNS)
Session &session = Session::get();

// ==========================================
// FUNCTION DECLARATIONS
// ==========================================

void handleFeedbackTimer();

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

    // Initialize System Config
    config.initialize();

    // Initialize TFT
    tft.initR(INITR_BLACKTAB);
    tft.setRotation(1);
    tft.fillScreen(ST77XX_BLACK);

    // Show splash screen
    display.showSplashScreen();
    delay(2000);

    // Initialize Actuators
    actuators.initializeActuatorsPins();

    // Initialize Inputs
    inputs.initializeInputsPins();

    // Initialize Game Mechanics
    gameMechanics.initialize();

    // Initialize Game Logic
    gameLogics.initialize();

    // Initialize players
    helper.initPlayerScores();

    // Initialize random seed
    randomSeed(analogRead(0));

    // Show start screen
    display.showNoCurrentSessionScreen();

    Serial.println("Press START to begin!");
}

// ==========================================
// LOOP
// ==========================================

void loop()
{
    session.processSession();
    if (session.isSessionEnabled)
    {
        session.processDNSServer();
    }


    if (!config.endGameRunOnce)
    {
        gameMechanics.endGameOnePlayer();
    }

    /**
     * The START button is multi-functional. Its behavior is strictly guarded by
     * 'config.displayState' to ensure that game flow is preserved and
     * system configuration (like WiFi) is not accidentally toggled during active gameplay.
     */
    Inputs::ButtonEvent startEvent = inputs.processSmartButton();

    switch (startEvent)
    {
    case Inputs::ButtonEvent::SINGLE_CLICK:
        Serial.println("START pressed! Display state: " + String(config.displayState));

        /**
         * State-based Guarding for Single Click:
         * Each state defines a specific valid action for the START button.
         * This prevents the game from jumping states or triggering logic
         * out of order.
         */
        switch (config.displayState)
        {
        case SystemConfig::SHOW_QUESTION:
            // Guard: Ignore START during active questioning to prevent accidental skips
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
            gameLogics.tryNewSession(); // Confirming session.
            break;
        case SystemConfig::SHOW_UPLOADED:
            Serial.println("Session uploaded... Start the session");
            gameLogics.startQuiz();
            break;

        default:
            Serial.println("⚠️ Unknown display state");
            break;
        }
        break;

    case Inputs::ButtonEvent::DOUBLE_CLICK:
        Serial.println("⚡ START DOUBLE-CLICK DETECTED!");

        /**
         * WiFi Toggling Guard:
         * Double-click is used to enable/disable the WiFi Session.
         * CRITICAL: This is only allowed in SHOW_NO_CURRENT_SESSION state.
         */
        switch (config.displayState)
        {
        case SystemConfig::SHOW_NO_CURRENT_SESSION:
            if (!session.isSessionEnabled)
            {
                Serial.println("📡 Powering ON WiFi Session...");
                session.enableWifiSession();
            }
            else
            {
                Serial.println("💤 Powering OFF WiFi Session...");
                session.disableWifiSession();
            }
            break;

        default:
            // Guarded: WiFi cannot be toggled during any other state
            break;
        }
        break;

    case Inputs::ButtonEvent::LONG_PRESS:
        Serial.println("⚡ START LONG-PRESS DETECTED!");

        /**
         * Session Reset:
         * Long-press is used to instantly clear the current session.
         * This is useful if an incorrect session was uploaded.
         */
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
        break;

    case Inputs::ButtonEvent::NONE:
    default:
        break;
    }

    /**
     * Input Segregation:
     * Answer buttons (A, B, C, D) are processed independently of the START button.
     * They are only polled if the system is explicitly in the SHOW_QUESTION state
     * and the question hasn't been answered yet.
     */
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

    // ===== FEEDBACK TIMER =====
    gameLogics.handleFeedbackTimer();

    // ===== MEMORY CHECK (every 30 seconds) =====
    static unsigned long lastMemCheck = 0;
    if (millis() - lastMemCheck > 30000)
    {
        lastMemCheck = millis();
        gameLogics.checkMemory();
    }

    delay(10);
}