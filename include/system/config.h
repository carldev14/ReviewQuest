/**
 * @file system/config.h
 * @brief System configuration singleton — pins, WiFi, game state
 */
#ifndef SYSTEM_CONFIG_H
#define SYSTEM_CONFIG_H

// ==========================================
// BUILD PROFILE
// ==========================================
// Set REVIEWQUEST_DEBUG via platformio.ini:
//     build_flags = -DREVIEWQUEST_DEBUG=1   (debug)
//     build_flags = -DREVIEWQUEST_DEBUG=0   (production)
//
// Safe fallback: production. Override only when you want logs.

#ifndef REVIEWQUEST_DEBUG
#define REVIEWQUEST_DEBUG 0
#endif

#if REVIEWQUEST_DEBUG

    #define RQ_LOG(x)              do { Serial.print(x);   } while (0)
    #define RQ_LOGLN(x)            do { Serial.println(x); } while (0)
    #define RQ_LOGF(...)           do { Serial.printf(__VA_ARGS__); } while (0)
    #define RQ_SERIAL_BEGIN(baud)  Serial.begin(baud)

#else

    #define RQ_LOG(x)              do {} while (0)
    #define RQ_LOGLN(x)            do {} while (0)
    #define RQ_LOGF(...)           do {} while (0)
    #define RQ_SERIAL_BEGIN(baud)  do {} while (0)

#endif

#include <Arduino.h>
#include <vector>
#include <algorithm>

class SystemConfig
{
public:
    // ==========================================
    // SINGLETON
    // ==========================================
    static SystemConfig &get();

    // ==========================================
    // LIFECYCLE
    // ==========================================
    void initialize();
    void initialValues();

    // ==========================================
    // HELPERS
    // ==========================================
    void ListQuestions();
    int getMaxPlayer();

    // ==========================================
    // PIN DEFINITIONS
    // ==========================================

    // Actuators
    static constexpr int CORRECT_LED   = 16;
    static constexpr int INCORRECT_LED = 17;
    static constexpr int BUZZER_PIN    = 21;

    // Physical inputs (A/B/C/D handled by touch)
    static constexpr int INPUT_START = 22;

    // TFT + Touch pins live in lib/TFT_eSPI/User_Setup.h:
    //   MISO 19  MOSI 23  SCLK 18  CS 5  DC 2  RST 4  TOUCH_CS 32
    // GPIO 19 is shared between TFT SDO and Touch DO.
    // Touch IRQ (33) is unused — TFT_eSPI polls over SPI.

    // ==========================================
    // COLOR PALETTE
    // ==========================================
    static constexpr uint16_t COLOR_DEEP_INDIGO   = 0x6B4F;
    static constexpr uint16_t COLOR_NEBULA_PURPLE = 0x7C9F;
    static constexpr uint16_t COLOR_STARLIGHT     = 0x4C6B;
    static constexpr uint16_t COLOR_CARD_BG       = 0x1082;
    static constexpr uint16_t COLOR_CARD_BG_DIM   = 0x4208;

    // ==========================================
    // DATA STRUCTURES
    // ==========================================

    struct Question
    {
        int    id;
        String text;
        String optionA;
        String optionB;
        String optionC;
        String optionD;
        char   initialCharAns;

        // Hint support — store originals for restore
        String originalOptionA;
        String originalOptionB;
        String originalOptionC;
        String originalOptionD;
        bool   hintUsed = false;
    };

    struct PlayerScore
    {
        String name;
        int    score;
        bool   isEliminated;
    };

    enum DisplayState
    {
        SHOW_SPLASH,
        SHOW_START,
        SHOW_QUESTION,
        SHOW_CORRECT,
        SHOW_INCORRECT,
        SHOW_COMPLETE,
        SHOW_LEADERBOARD,
        SHOW_RESTART_GAME,
        SHOW_UPLOADED,
        SHOW_CREDENTIALS,
        SHOW_NEW_SESSION,
        SHOW_NO_CURRENT_SESSION,
    };

    enum EliminationReason
    {
        ELIM_DEDUCT_POINTS,
        ELIM_PENALTY_QUESTION,
        ELIM_TASK_FAILED,
        ELIM_SACRIFICIAL_CONS,
        ELIM_REFUSE_TO_REVIVE,
        ELIM_UNKNOWN
    };

    // ==========================================
    // WIFI / SESSION
    // ==========================================
    String wifiSSID      = "ReviewQuest: ESP32";
    String wifiPassword  = "12345678";
    String mdnsHostname  = "reviewquest";
    int    dnsPort       = 53;
    bool   isSessionEnabled = false;

    // ==========================================
    // GAME DATA
    // ==========================================
    String sessionTitle = "";

    std::vector<Question>    questionList;
    std::vector<int>         questionOrder;
    std::vector<int>         questionOptionOrder;
    std::vector<int>         playerOrder;
    std::vector<PlayerScore> playerScores;

    // ==========================================
    // GAME STATE — cursors
    // ==========================================
    int currentQuestionPos = 0;
    int currentPlayerPos   = 0;
    int nextQuestionId     = 1;
    int currentIndex       = 0;
    int overallScore       = 0;

    // ==========================================
    // GAME STATE — actors
    // ==========================================
    String currentPlayerName = "";
    String saviorPlayerName  = "";

    // ==========================================
    // GAME STATE — flags
    // ==========================================
    bool endGameRunOnce = false;
    bool isLuckActive   = false;
    bool isNewSesion    = false;

    // ==========================================
    // GAME STATE — revive flow
    // ==========================================
    int  selectedSaviorIndex = 0;
    bool revivingProcess     = false;
    bool refuseToRevive      = false;

    // ==========================================
    // GAME STATE — answer tracking
    // ==========================================
    bool answered       = false;
    char selectedAnswer = ' ';

    // ==========================================
    // DISPLAY STATE
    // ==========================================
    DisplayState      displayState      = SHOW_START;
    EliminationReason eliminationReason = ELIM_UNKNOWN;

    unsigned long       stateStartTime    = 0;
    const unsigned long FEEDBACK_DURATION = 1500;

    // ==========================================
    // TOUCH GEOMETRY — written by display, read by inputs
    // ==========================================

    // Answer bands (question screen)
    static constexpr int CHOICE_COUNT = 4;
    int choiceBandY[CHOICE_COUNT] = {0, 0, 0, 0};
    int choiceBandH = 26;

    // Savior list rows
    static constexpr int MAX_SAVIOR_ROWS = 4;
    int saviorRowY[MAX_SAVIOR_ROWS] = {0, 0, 0, 0};
    int saviorRowH   = 40;
    int saviorRowCount = 0;

    // Savior button bar
    int saviorBtnBarY   = 0;
    int saviorBtnElectX = 0;
    int saviorBtnNextX  = 0;
    int saviorBtnSkipX  = 0;
    int saviorBtnW      = 0;
    int saviorBtnH      = 0;

    // New session buttons
    int newSessionYesX = 0;
    int newSessionNoX  = 0;
    int newSessionBtnY = 0;
    int newSessionBtnW = 0;
    int newSessionBtnH = 0;

    // Legacy footer zones (kept for compatibility)
    int saviorFooterY      = 210;
    int saviorFooterSplit1 = 106;
    int saviorFooterSplit2 = 213;

    // ==========================================
    // DEBOUNCE (physical START button)
    // ==========================================
    unsigned long lastDebounceTime = 0;
    unsigned long debounceDelay    = 200;

private:
    // ==========================================
    // CONSTRUCTOR & DESTRUCTOR
    // ==========================================
    SystemConfig()  = default;
    ~SystemConfig() = default;

    SystemConfig(const SystemConfig &)            = delete;
    SystemConfig &operator=(const SystemConfig &) = delete;

    bool initialized = false;
};

#endif // SYSTEM_CONFIG_H