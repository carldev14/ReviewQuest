/**
 * @file system/config.h
 * @brief System configuration singleton class
 * Manages all system-wide configuration, pin definitions, and game state
 */
#ifndef SYSTEM_CONFIG_H
#define SYSTEM_CONFIG_H

#include <Arduino.h>
#include <vector>

// Undefine any conflicting macros
#ifdef TFT_CS
#undef TFT_CS
#endif
#ifdef TFT_DC
#undef TFT_DC
#endif
#ifdef TFT_RST
#undef TFT_RST
#endif

class SystemConfig
{
public:
    // ==========================================
    // SINGLETON INSTANCE
    // ==========================================

    /**
     * @brief Get the single instance of SystemConfig
     * @return Reference to the singleton instance
     */
    static SystemConfig &get();

    // ==========================================
    // INITIALIZATION METHODS
    // ==========================================

    /**
     * @brief Initialize the system configuration
     * Sets up pin modes, default values, etc.
     */
    void initialize();

    /**
     * @brief Reset all values to initial state
     */
    void initialValues();

    /**
     * @brief Print all questions to Serial for debugging
     */
    void ListQuestions();

    // ==========================================
    // PIN DEFINITIONS
    // ==========================================

    //** ACTUATORS */
    static constexpr int CORRECT_LED = 16;
    static constexpr int INCORRECT_LED = 17;
    static constexpr int BUZZER_PIN = 19;

    //** INPUTS */
    static constexpr int INPUT_A = 21;
    static constexpr int INPUT_B = 22;
    static constexpr int INPUT_C = 25;
    static constexpr int INPUT_D = 26;
    static constexpr int INPUT_START = 27;       // ADDED: Start button
    static constexpr int DEEP_SLEEP_BUTTON = 34; // 10K RESISTOR TO GROUND, 3.3V TOLERANT

    // ==========================================
    // TFT DISPLAY PINS
    // ==========================================
    static constexpr int TFT_CS = 5;
    static constexpr int TFT_RST = 4;
    static constexpr int TFT_DC = 2; // CONNECTED TO 'A0' PIN

    // ==========================================
    // DATA STRUCTURES
    // ==========================================

    struct Question
    {
        int id;
        String text;
        String optionA;
        String optionB;
        String optionC;
        String optionD;
        char initialCharAns;

        // Store original options for reset
        String originalOptionA;
        String originalOptionB;
        String originalOptionC;
        String originalOptionD;
        bool hintUsed = false; // Track if hint was used
    };

    // Player Score Structure
    struct PlayerScore
    {
        String name;
        int score;
        bool isEliminated;
    };

    // Display States
    enum DisplayState
    {
        SHOW_START,
        SHOW_QUESTION,
        SHOW_CORRECT,
        SHOW_INCORRECT,
        SHOW_COMPLETE,
        SHOW_LEADERBOARD,
        SHOW_RESTART_GAME
    };

    enum EliminationReason
    {
        ELIM_DEDUCT_POINTS,
        ELIM_PENALTY_QUESTION,
        ELIM_TASK_FAILED,
        ELIM_UNKNOWN
    };

    // ==========================================
    // GAME VARIABLES
    // ==========================================

    // Question management
    std::vector<Question> questionList;
    std::vector<int> questionOrder;        // Shuffled question indices
    std::vector<int> questionOptionOrder; // Shuffled question option indices
    std::vector<int> playerOrder;          // Shuffled player indices
    std::vector<PlayerScore> playerScores; // Per-player scores (RAM)

    // State tracking
    int currentQuestionPos = 0; // Position in questionOrder
    int currentPlayerPos = 0;   // Position in playerOrder
    int nextQuestionId = 1;
    int currentIndex = 0; // Original question index
    int overallScore = 0; // Overall score (kept for display)
    String currentPlayerName = "";
    bool isLuckActive = false; // Flag to prevent timer interference

    // Display state
    DisplayState displayState = SHOW_START;
    EliminationReason eliminationReason = ELIM_UNKNOWN;

    unsigned long stateStartTime = 0;
    const unsigned long FEEDBACK_DURATION = 1500; // 1.5 seconds

    // Answer tracking
    bool answered = false;
    char selectedAnswer = ' ';

    // Debounce
    unsigned long lastDebounceTime = 0;
    unsigned long debounceDelay = 200;

private:
    // ==========================================
    // PRIVATE CONSTRUCTOR & DESTRUCTOR (Singleton)
    // ==========================================

    SystemConfig() = default;
    ~SystemConfig() = default;

    // Delete copy constructor and assignment operator
    SystemConfig(const SystemConfig &) = delete;
    SystemConfig &operator=(const SystemConfig &) = delete;

    // Private member variables
    bool initialized = false;
};

#endif // SYSTEM_CONFIG_H