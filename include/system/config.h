/**
 * @file system/config.h
 * @brief System configuration singleton class
 * Manages all system-wide configuration, pin definitions, and game state
 */
#ifndef SYSTEM_CONFIG_H
#define SYSTEM_CONFIG_H

#include <Arduino.h>
#include <vector>
#include <algorithm>

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

    /**
     * @brief Get the maximum number of players allowed (limited to 5)
     * @return int Maximum player count (min of playerScores.size() or 5)
     */
    int getMaxPlayer();

    // ==========================================
    // PIN DEFINITIONS
    // ==========================================

    //** ACTUATORS */
    static constexpr int CORRECT_LED = 16;   ///< LED pin for correct answer feedback
    static constexpr int INCORRECT_LED = 17; ///< LED pin for incorrect answer feedback
    static constexpr int BUZZER_PIN = 19;    ///< Buzzer pin for sound effects

    //** INPUTS */
    static constexpr int INPUT_A = 21;           ///< Button A pin
    static constexpr int INPUT_B = 22;           ///< Button B pin
    static constexpr int INPUT_C = 25;           ///< Button C pin
    static constexpr int INPUT_D = 26;           ///< Button D pin
    static constexpr int INPUT_START = 27;       ///< Start button pin

    // ==========================================
    // TFT DISPLAY PINS
    // ==========================================
    static constexpr int TFT_CS = 5;  ///< TFT Chip Select pin
    static constexpr int TFT_RST = 4; ///< TFT Reset pin
    static constexpr int TFT_DC = 2;  ///< TFT Data/Command pin (connected to 'A0' pin)

    // ==========================================
    // WIFI CONFIGURATION
    // ==========================================
    String wifiSSID = "ReviewQuest: ESP32";
    String wifiPassword = "12345678";
    String mdnsHostname = "reviewquest";
    int dnsPort = 53;

    // ==========================================
    // DATA STRUCTURES
    // ==========================================

    // =====
    // Sessions (Questions, Players, and Title)
    String sessionTitle = "";

    /**
     * @brief Question structure containing all question data
     */
    struct Question
    {
        int id;              ///< Unique question ID
        String text;         ///< Question text
        String optionA;      ///< Option A text
        String optionB;      ///< Option B text
        String optionC;      ///< Option C text
        String optionD;      ///< Option D text
        char initialCharAns; ///< Correct answer character (A, B, C, or D)

        // Store original options for reset
        String originalOptionA; ///< Original Option A (for hint reset)
        String originalOptionB; ///< Original Option B (for hint reset)
        String originalOptionC; ///< Original Option C (for hint reset)
        String originalOptionD; ///< Original Option D (for hint reset)
        bool hintUsed = false;  ///< Flag to track if hint was used on this question
    };

    /**
     * @brief Player score structure tracking individual player data
     */
    struct PlayerScore
    {
        String name;       ///< Player's name
        int score;         ///< Player's current score
        bool isEliminated; ///< True if player has been eliminated
    };

    /**
     * @brief Display state enumeration for UI flow control
     */
    enum DisplayState
    {
        SHOW_SPLASH,       ///< Show splash screen
        SHOW_START,        ///< Show start screen
        SHOW_QUESTION,     ///< Show current question
        SHOW_CORRECT,      ///< Show correct feedback
        SHOW_INCORRECT,    ///< Show incorrect feedback
        SHOW_COMPLETE,     ///< Show completion/game over screen
        SHOW_LEADERBOARD,  ///< Show leaderboard
        SHOW_RESTART_GAME, ///< Show restart confirmation,
        SHOW_UPLOADED,
        SHOW_CREDENTIALS,
        SHOW_NEW_SESSION,
        SHOW_NO_CURRENT_SESSION,
    };

    /**
     * @brief Elimination reason enumeration for tracking why a player was eliminated
     */
    enum EliminationReason
    {
        ELIM_DEDUCT_POINTS,    ///< Eliminated due to points reaching zero
        ELIM_PENALTY_QUESTION, ///< Eliminated due to failing penalty questions
        ELIM_TASK_FAILED,      ///< Eliminated due to task failure
        ELIM_SACRIFICIAL_CONS, ///< Eliminated as a sacrifice (savior took the fall)
        ELIM_REFUSE_TO_REVIVE, ///< Eliminated because no one would save them
        ELIM_UNKNOWN           ///< Elimination reason unknown
    };

    // ==========================================
    // GAME VARIABLES
    // ==========================================

    // Question management
    std::vector<Question> questionList;    ///< Master list of all questions
    std::vector<int> questionOrder;        ///< Shuffled question indices (order of play)
    std::vector<int> questionOptionOrder;  ///< Shuffled question option indices
    std::vector<int> playerOrder;          ///< Shuffled player indices (turn order)
    std::vector<PlayerScore> playerScores; ///< Per-player scores in RAM

    // State tracking
    int currentQuestionPos = 0;  ///< Current position in questionOrder
    int currentPlayerPos = 0;    ///< Current position in playerOrder
    int nextQuestionId = 1;      ///< Next available question ID for new questions
    int currentIndex = 0;        ///< Original question index (unshuffled)
    int overallScore = 0;        ///< Overall score (kept for display)
    bool endGameRunOnce = false; ///< Flag to prevent end-game from running multiple times
    bool isLuckActive = false;   ///< Flag to prevent timer interference during luck events
    bool isNewSesion = false;

    // Strings
    String currentPlayerName = ""; ///< Name of the currently active player
    String saviorPlayerName = "";  ///< Name of the player who saves/revives another

    // Revive states
    int selectedSaviorIndex = 0;  ///< Currently selected savior index in the list
    bool revivingProcess = false; ///< True when reviving process is active
    bool refuseToRevive = false;  ///< True if current player refuses to save another

    // Display state
    DisplayState displayState = SHOW_START;             ///< Current display state
    EliminationReason eliminationReason = ELIM_UNKNOWN; ///< Reason for last elimination

    unsigned long stateStartTime = 0;             ///< Timestamp when current state started
    const unsigned long FEEDBACK_DURATION = 1500; ///< Duration to show feedback (1.5 seconds)

    // Answer tracking
    bool answered = false;     ///< True if current question has been answered
    char selectedAnswer = ' '; ///< The selected answer character (A, B, C, D)

    // Debounce
    unsigned long lastDebounceTime = 0; ///< Last debounce timestamp
    unsigned long debounceDelay = 200;  ///< Debounce delay in milliseconds

private:
    // ==========================================
    // PRIVATE CONSTRUCTOR & DESTRUCTOR (Singleton)
    // ==========================================

    SystemConfig() = default;  ///< Private constructor (singleton)
    ~SystemConfig() = default; ///< Private destructor (singleton)

    // Delete copy constructor and assignment operator
    SystemConfig(const SystemConfig &) = delete;
    SystemConfig &operator=(const SystemConfig &) = delete;

    // Private member variables
    bool initialized = false; ///< Flag to track if config has been initialized
};

#endif // SYSTEM_CONFIG_H