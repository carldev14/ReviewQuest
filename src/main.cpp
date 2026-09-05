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
#include "game_logic.h"
#include <driver/rtc_io.h>

// ==========================================
// ACCESS POINT CONFIG
// ==========================================
const char *AP_SSID = "ReviewQuest: ESP32";
const char *AP_PASSWORD = "12345678";
const byte DNS_PORT = 53;

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
GameLogic &gameLogic = GameLogic::get(); // <-- NEW
DisplayOutputs &display = DisplayOutputs::get();
Actuators &actuators = Actuators::get();

// Web Server
AsyncWebServer server(80);
DNSServer dnsServer;

// ==========================================
// FUNCTION DECLARATIONS
// ==========================================

void initializeDefaultQuestions();
void handleFeedbackTimer();

// ==========================================
// PRE-DEFINED QUESTIONS
// ==========================================

void initializeDefaultQuestions()
{
    Serial.println("📝 Initializing default questions...");

    config.questionList.clear();
    config.nextQuestionId = 1;

    SystemConfig::Question q1;
    q1.id = config.nextQuestionId++;
    q1.text = "What is the square root of 169?";
    q1.optionA = "11";
    q1.optionB = "12";
    q1.optionC = "13";
    q1.optionD = "14";
    q1.initialCharAns = 'C';
    config.questionList.push_back(q1);

    SystemConfig::Question q2;
    q2.id = config.nextQuestionId++;
    q2.text = "What is 15% of 200?";
    q2.optionA = "20";
    q2.optionB = "25";
    q2.optionC = "30";
    q2.optionD = "35";
    q2.initialCharAns = 'C';
    config.questionList.push_back(q2);

    SystemConfig::Question q3;
    q3.id = config.nextQuestionId++;
    q3.text = "What is the area of a rectangle with length 8 and width 5?";
    q3.optionA = "30";
    q3.optionB = "35";
    q3.optionC = "40";
    q3.optionD = "45";
    q3.initialCharAns = 'C';
    config.questionList.push_back(q3);

    SystemConfig::Question q4;
    q4.id = config.nextQuestionId++;
    q4.text = "What is the cube root of 27?";
    q4.optionA = "2";
    q4.optionB = "3";
    q4.optionC = "4";
    q4.optionD = "5";
    q4.initialCharAns = 'B';
    config.questionList.push_back(q4);

    SystemConfig::Question q5;
    q5.id = config.nextQuestionId++;
    q5.text = "What is the sum of angles in a triangle?";
    q5.optionA = "90°";
    q5.optionB = "120°";
    q5.optionC = "180°";
    q5.optionD = "360°";
    q5.initialCharAns = 'C';
    config.questionList.push_back(q5);

    SystemConfig::Question q6;
    q6.id = config.nextQuestionId++;
    q6.text = "What is 11 x 12?";
    q6.optionA = "121";
    q6.optionB = "132";
    q6.optionC = "143";
    q6.optionD = "144";
    q6.initialCharAns = 'B';
    config.questionList.push_back(q6);

    SystemConfig::Question q7;
    q7.id = config.nextQuestionId++;
    q7.text = "What is the perimeter of a square with side 6?";
    q7.optionA = "20";
    q7.optionB = "24";
    q7.optionC = "28";
    q7.optionD = "32";
    q7.initialCharAns = 'B';
    config.questionList.push_back(q7);

    SystemConfig::Question q8;
    q8.id = config.nextQuestionId++;
    q8.text = "What is 64 divided by 8?";
    q8.optionA = "6";
    q8.optionB = "7";
    q8.optionC = "8";
    q8.optionD = "9";
    q8.initialCharAns = 'C';
    config.questionList.push_back(q8);

    SystemConfig::Question q9;
    q9.id = config.nextQuestionId++;
    q9.text = "What is the next prime number after 7?";
    q9.optionA = "9";
    q9.optionB = "10";
    q9.optionC = "11";
    q9.optionD = "12";
    q9.initialCharAns = 'C';
    config.questionList.push_back(q9);

    SystemConfig::Question q10;
    q10.id = config.nextQuestionId++;
    q10.text = "What is 5 factorial (5!)?";
    q10.optionA = "60";
    q10.optionB = "100";
    q10.optionC = "120";
    q10.optionD = "150";
    q10.initialCharAns = 'C';
    config.questionList.push_back(q10);

    Serial.printf("✅ Added %d default questions\n", (int)config.questionList.size());
}

// ==========================================
// SETUP
// ==========================================

void setup()
{
    // Check if system woke up from button press
    if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_EXT0)
    {
        Serial.println("⚡ Woke up from Deep Sleep via START button!");
        // Optional: show wake screen or restore state
    }

    Serial.begin(115200);
    delay(1000);
    Serial.flush();

    Serial.println("\n>>> SYSTEM BOOTING <<<");

    // Initialize System Config
    config.initialize();

    // Initialize default questions
    initializeDefaultQuestions();
    config.ListQuestions();

    // Initialize TFT
    tft.initR(INITR_GREENTAB);
    tft.setRotation(1);
    tft.fillScreen(ST77XX_BLACK);

    // Initialize Actuators
    actuators.initializeActuatorsPins();

    // Initialize Inputs
    inputs.initializeInputsPins();

    // Initialize Game Mechanics
    gameMechanics.initialize();

    // Initialize Game Logic
    gameLogic.initialize();

    // Initialize players
    helper.initPlayerScores();

    // Initialize random seed
    randomSeed(analogRead(0));

    // Show start screen
    display.showStartScreen();

    // Initialize Access Point
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID, AP_PASSWORD);
    IPAddress apIP = WiFi.softAPIP();

    dnsServer.start(DNS_PORT, "*", apIP);

    Serial.println("=================================");
    Serial.print("AP SSID: ");
    Serial.println(AP_SSID);
    Serial.print("AP IP:   ");
    Serial.println(apIP);
    Serial.println("=================================");
    Serial.printf("Total Questions: %d\n", (int)config.questionList.size());
    Serial.printf("Total Players: %d\n", (int)config.playerScores.size());
    Serial.println("Press START to begin!");
}

// ==========================================
// LOOP
// ==========================================

void loop()
{
    dnsServer.processNextRequest();

    //* Always run on loop to guard keep if there is 1 player in the session
    gameLogic.endGameOnePlayer(); 

    // ===== SMART START BUTTON EVENT PROCESSOR =====
    Inputs::ButtonEvent startEvent = inputs.processSmartButton();

    switch (startEvent)
    {
    case Inputs::ButtonEvent::SINGLE_CLICK:
        Serial.println("START pressed! Display state: " + String(config.displayState));

        switch (config.displayState)
        {
        case SystemConfig::SHOW_START:
            Serial.println("🚀 Starting quiz...");
            gameLogic.startQuiz();
            break;

        case SystemConfig::SHOW_QUESTION:
            Serial.println("Already showing a question - press A/B/C/D to answer");
            break;

        case SystemConfig::SHOW_CORRECT:
            Serial.println("➡️ Advancing to next question...");
            gameLogic.advanceToNextQuestion();
            break;

        case SystemConfig::SHOW_INCORRECT:
            Serial.println("➡️ Retrying question...");
            gameLogic.retryQuestion();
            break;

        case SystemConfig::SHOW_COMPLETE:
            Serial.println("Transitioning to Leaderboard view...");
            display.showLeaderboard();
            break;

        case SystemConfig::SHOW_LEADERBOARD:
            Serial.println("Leaderboard active — restarting session...");
            gameLogic.restartGame();
            break;

        default:
            Serial.println("⚠️ Unknown display state");
            break;
        }
        break;
    case Inputs::ButtonEvent::DOUBLE_CLICK:
        /**
         * @brief Handles double-click context navigation for the primary START button.
         *
         ** Navigates screens or resets state depending on current system context:
         ** - Completed screen   -> Displays top player leaderboard scores.
         ** - Leaderboard screen -> Resets state and initializes a new game session.
         *
         * @note Functionality can be expanded for fast-action shortcuts or secondary options.
         */
        Serial.println("⚡ START DOUBLE-CLICK DETECTED!");
        // Double-click shortcuts can be added here if needed in the future
        break;

    case Inputs::ButtonEvent::LONG_PRESS:
    {
        Serial.println("🔄 START LONG-PRESS DETECTED! Entering deep sleep...");

        // 1. Notify user & clear outputs
        display.showMessage("SLEEP", "Going to sleep...", ST77XX_CYAN);
        delay(1000);
        actuators.allOff();

        // 2. Configure wake-up pin
        gpio_num_t wakePin = (gpio_num_t)SystemConfig::INPUT_START;
        esp_sleep_enable_ext0_wakeup(wakePin, 0);

        // Keep internal pull-up powered during deep sleep
        rtc_gpio_pullup_en(wakePin);
        rtc_gpio_pulldown_dis(wakePin);

        // 3. Enter deep sleep
        esp_deep_sleep_start();
        break;
    }

    case Inputs::ButtonEvent::NONE:
    default:
        break;
    }

    // ===== ANSWER BUTTONS =====
    if (config.displayState == SystemConfig::SHOW_QUESTION &&
        !config.answered &&
        config.currentQuestionPos < (int)config.questionOrder.size())
    {
        char ans = inputs.choicesButtonProcessor();
        if (ans != ' ')
        {
            gameLogic.handleAnswer(ans);
        }
    }

    // ===== FEEDBACK TIMER =====
    gameLogic.handleFeedbackTimer();

    // ===== MEMORY CHECK (every 30 seconds) =====
    static unsigned long lastMemCheck = 0;
    if (millis() - lastMemCheck > 30000)
    {
        lastMemCheck = millis();
        gameLogic.checkMemory();
    }

    delay(10);
}