/**
 * @file system/config.cpp
 * @brief SystemConfig implementation
 */
#include "system/config.h"

void SystemConfig::initialize()
{
    if (initialized) {
        // Serial.println("⚠️ System already initialized!");
        return;
    }
    
    // Serial.println("🔧 Initializing SystemConfig...");
    
    // Initialize pin modes
    pinMode(CORRECT_LED, OUTPUT);
    pinMode(INCORRECT_LED, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    
    pinMode(INPUT_A, INPUT_PULLUP);
    pinMode(INPUT_B, INPUT_PULLUP);
    pinMode(INPUT_C, INPUT_PULLUP);
    pinMode(INPUT_D, INPUT_PULLUP);
    pinMode(INPUT_START, INPUT_PULLUP);
    pinMode(DEEP_SLEEP_BUTTON, INPUT_PULLUP);
    
    // Set initial LED states
    digitalWrite(CORRECT_LED, LOW);
    digitalWrite(INCORRECT_LED, LOW);
    digitalWrite(BUZZER_PIN, LOW);
    
    // Initialize default values
    initialValues();
    
    initialized = true;
    // Serial.println("✅ SystemConfig initialized!");
}

void SystemConfig::initialValues()
{
    Serial.println("⚠️ SystemConfig::initialValues() called!");
    // Serial.println("🔄 Resetting to initial values...");

    // Clear all vectors
    questionList.clear();
    questionOrder.clear();
    playerOrder.clear();
    playerScores.clear();

    // Reset counters
    currentQuestionPos = 0;
    currentPlayerPos = 0;
    nextQuestionId = 1;
    currentIndex = 0;
    overallScore = 0;
    currentPlayerName = "";
    displayState = SHOW_START;
    stateStartTime = 0;
    answered = false;
    selectedAnswer = ' ';
    lastDebounceTime = 0;
}

void SystemConfig::ListQuestions()
{
    // Serial.println("\n=========================================");
    // Serial.printf("📚 TOTAL QUESTIONS: %d\n", (int)questionList.size());
    // Serial.println("=========================================");
    
    if (questionList.empty()) {
        // Serial.println("❌ No questions available!");
        return;
    }
    
    for (const auto &q : questionList) {
        // Serial.printf("\n🔹 #%d: %s\n", q.id, q.text.c_str());
        // Serial.printf("   A) %s %s\n", q.optionA.c_str(), q.initialCharAns == 'A' ? "✅ [CORRECT]" : "");
        // Serial.printf("   B) %s %s\n", q.optionB.c_str(), q.initialCharAns == 'B' ? "✅ [CORRECT]" : "");
        // Serial.printf("   C) %s %s\n", q.optionC.c_str(), q.initialCharAns == 'C' ? "✅ [CORRECT]" : "");
        // Serial.printf("   D) %s %s\n", q.optionD.c_str(), q.initialCharAns == 'D' ? "✅ [CORRECT]" : "");
        // Serial.println("-----------------------------------------");
    }
    // Serial.println("=========================================\n");
}
SystemConfig &SystemConfig::get()
{
    static SystemConfig instance;
    return instance;
}
