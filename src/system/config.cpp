/**
 * @file system/config.cpp
 * @brief SystemConfig implementation
 */
#include "system/config.h"
#include <Preferences.h>

// ==========================================
// SINGLETON
// ==========================================

SystemConfig &SystemConfig::get()
{
    static SystemConfig instance;
    return instance;
}

// ==========================================
// LIFECYCLE
// ==========================================

void SystemConfig::initialize()
{
    if (initialized)
    {
        // Serial.println("⚠️ System already initialized!");
        return;
    }

    // Serial.println("🔧 Initializing SystemConfig...");

    // Pin modes
    pinMode(CORRECT_LED, OUTPUT);
    pinMode(INCORRECT_LED, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(INPUT_START, INPUT_PULLUP);

    // Initial LED / buzzer states
    digitalWrite(CORRECT_LED, LOW);
    digitalWrite(INCORRECT_LED, LOW);
    digitalWrite(BUZZER_PIN, LOW);

    initialValues();

    initialized = true;
    // Serial.println("✅ SystemConfig initialized!");
}

void SystemConfig::initialValues()
{
    // Reset runtime state
    currentQuestionPos = 0;
    currentPlayerPos = 0;
    nextQuestionId = 1;
    currentIndex = 0;
    overallScore = 0;

    endGameRunOnce = false;
    isLuckActive = false;
    isNewSesion = false;

    currentPlayerName = "";
    saviorPlayerName = "";

    selectedSaviorIndex = 0;
    revivingProcess = false;
    refuseToRevive = false;

    displayState = SHOW_START;
    eliminationReason = ELIM_UNKNOWN;
    stateStartTime = 0;

    answered = false;
    selectedAnswer = ' ';

    lastDebounceTime = 0;
    debounceDelay = 200;

    // Clear containers
    questionList.clear();
    questionOrder.clear();
    questionOptionOrder.clear();
    playerOrder.clear();
    playerScores.clear();

    // Sample data — remove after testing
    sessionTitle = "Sample Quiz";

    auto addQuestion = [this](int id,
                              const char *text,
                              const char *a, const char *b,
                              const char *c, const char *d,
                              char correct)
    {
        Question q;
        q.id = id;
        q.text = text;
        q.optionA = a;
        q.optionB = b;
        q.optionC = c;
        q.optionD = d;
        q.initialCharAns = correct;
        q.originalOptionA = "";
        q.originalOptionB = "";
        q.originalOptionC = "";
        q.originalOptionD = "";
        q.hintUsed = false;
        questionList.push_back(q);
        questionOrder.push_back(id);
    };

    addQuestion(0,
                "Who led the Spanish expedition in 1521?",
                "Ferdinand Magellan",
                "Miguel Lopez de Legazpi",
                "Jose Rizal",
                "Andres Bonifacio",
                'A');

    addQuestion(1,
                "Who wrote Noli Me Tangere?",
                "Marcelo H. Del Pilar",
                "Jose Rizal",
                "Graciano Lopez-Jaena",
                "Andres Bonifacio",
                'B');

    addQuestion(2,
                "What is the Pasyon about?",
                "Love of a knight",
                "History of Spain",
                "Life of Jesus Christ",
                "Daily life in Manila",
                'C');

    addQuestion(3,
                "Who wrote Urbana at Feliza?",
                "Modesto de Castro",
                "Jose Rizal",
                "Marcelo H. Del Pilar",
                "Graciano Lopez-Jaena",
                'A');

    addQuestion(4,
                "What is Awit composed of?",
                "Octosyllabic quatrains",
                "Free verse lines",
                "Sonnet form stanzas",
                "Dodecasyllabic quatrains",
                'D');

    auto addPlayer = [this](const char *name)
    {
        PlayerScore p;
        p.name = name;
        p.score = 0;
        p.isEliminated = false;
        playerScores.push_back(p);
    };

    addPlayer("Alice");
    addPlayer("Bob");
    addPlayer("Charlie");
    addPlayer("Dave");

    if (!playerScores.empty())
    {
        currentPlayerName = playerScores[0].name;
    }

    Serial.println("✅ initialValues() complete");
    Serial.printf("   Questions: %d\n", (int)questionList.size());
    Serial.printf("   Players:   %d\n", (int)playerScores.size());
    Serial.printf("   Session:   %s\n", sessionTitle.c_str());
}

// ==========================================
// GETTERS
// ==========================================

int SystemConfig::getMaxPlayer()
{
    SystemConfig &config = SystemConfig::get();
    return min(static_cast<int>(config.playerScores.size()), 5);
}

// ==========================================
// DIAGNOSTICS
// ==========================================

void SystemConfig::ListQuestions()
{
    // Serial.println("\n=========================================");
    // Serial.printf("📚 TOTAL QUESTIONS: %d\n", (int)questionList.size());
    // Serial.println("=========================================");

    if (questionList.empty())
    {
        // Serial.println("❌ No questions available!");
        return;
    }

    for (const auto &q : questionList)
    {
        // Serial.printf("\n🔹 #%d: %s\n", q.id, q.text.c_str());
        // Serial.printf("   A) %s %s\n", q.optionA.c_str(), q.initialCharAns == 'A' ? "✅ [CORRECT]" : "");
        // Serial.printf("   B) %s %s\n", q.optionB.c_str(), q.initialCharAns == 'B' ? "✅ [CORRECT]" : "");
        // Serial.printf("   C) %s %s\n", q.optionC.c_str(), q.initialCharAns == 'C' ? "✅ [CORRECT]" : "");
        // Serial.printf("   D) %s %s\n", q.optionD.c_str(), q.initialCharAns == 'D' ? "✅ [CORRECT]" : "");
        // Serial.println("-----------------------------------------");
    }
    // Serial.println("=========================================\n");
}