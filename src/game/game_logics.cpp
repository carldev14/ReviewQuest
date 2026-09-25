/**
 * @file game_logic.cpp
 * @brief Game logic implementation
 */
#include "game/game_logics.h"

// ==========================================
// CONSTRUCTOR & DESTRUCTOR
// ==========================================

GameLogics::GameLogics()
    : config(SystemConfig::get()),
      display(DisplayOutputs::get()),
      actuators(Actuators::get()),
      helper(Helper::get()),
      gameMechanics(GameMechanics::get()),
      inputs(Inputs::get()),
      session(Session::get())
{
    Serial.println("🎯 GameLogics initialized!");
    initialized = true;
}

GameLogics::~GameLogics()
{
    Serial.println("GameLogics destroyed!");
}

// ==========================================
// LIFECYCLE
// ==========================================

void GameLogics::initialize()
{
    Serial.println("🔧 Initializing GameLogics...");
    resetGameState();
    config.displayState = SystemConfig::SHOW_START;
}

void GameLogics::startQuiz()
{
    Serial.println("========================================");
    Serial.println("🚀 STARTING QUIZ");
    Serial.println("========================================");

    if (!canStartQuiz())
    {
        showError("⚠️ ERROR!", "Cannot start quiz!");
        resetToStartScreen();
        return;
    }

    helper.printAllPlayers();

    // Reset state
    config.currentQuestionPos = 0;
    config.currentPlayerPos   = 0;
    config.overallScore       = 0;
    config.answered           = false;
    config.selectedAnswer     = ' ';
    config.stateStartTime     = 0;
    gameMechanics.resetPenaltyCount();

    // Shuffle everything
    helper.shuffleQuestions();
    helper.shuffleQuestionOptions();
    helper.shufflePlayers();

    // First player
    config.currentPlayerName = helper.getNextPlayer();

    if (config.questionOrder.empty())
    {
        Serial.println("❌ ERROR: Question order is empty!");
        showError("⚠️ ERROR!", "Question order is empty!");
        resetToStartScreen();
        return;
    }

    config.displayState = SystemConfig::SHOW_QUESTION;
    int originalIndex = config.questionOrder[0];
    display.showQuestionScreen(originalIndex);

    Serial.printf("✅ Quiz started! First player: %s\n", config.currentPlayerName.c_str());
    helper.printPlayerScores();
    printSystemStatus();
}

void GameLogics::restartGame()
{
    Serial.println("🔄 RESTARTING GAME");

    display.showRestartGameScreen();

    gameMechanics.resetAllHints();
    helper.resetPlayerScores();
    config.endGameRunOnce = false;

    delay(1000);
    startQuiz();
    Serial.println("✅ Game restarted!");
}

void GameLogics::clearSession()
{
    Serial.println("🗑️ Clearing Session...");

    config.initialValues();
    resetGameState();

    config.displayState = SystemConfig::SHOW_NO_CURRENT_SESSION;
    display.showNoCurrentSessionScreen();

    Serial.println("✅ Session cleared. System ready for new upload.");
}

void GameLogics::tryNewSession()
{
    display.showNewSessionScreen();

    // Block until the user picks YES or NO
    while (inputs.confirmationSessionButton() == 0)
    {
        delay(10);
    }

    if (config.isNewSesion)
    {
        session.enableWifiSession();
        display.showNoCurrentSessionScreen();
    }
    else
    {
        restartGame();
    }
}

// ==========================================
// QUESTION FLOW
// ==========================================

void GameLogics::advanceToNextQuestion()
{
    Serial.println("➡️ ADVANCING TO NEXT QUESTION");
    Serial.printf("📊 Current position: %d, Order size: %d\n",
                  config.currentQuestionPos, (int)config.questionOrder.size());

    if (config.questionOrder.empty())
    {
        Serial.println("⚠️ Question order is empty!");
        display.showNoQuestionsScreen();
        config.displayState = SystemConfig::SHOW_COMPLETE;
        return;
    }

    if (config.playerScores.empty())
    {
        Serial.println("❌ ERROR: No players available!");
        display.showCompletionScreen();
        config.displayState = SystemConfig::SHOW_COMPLETE;
        return;
    }

    if (config.currentQuestionPos >= (int)config.questionOrder.size())
    {
        Serial.println("🏁 All questions completed!");
        gameMechanics.resetPenaltyCount();
        display.showCompletionScreen();
        config.displayState = SystemConfig::SHOW_COMPLETE;
        return;
    }

    int originalIndex = config.questionOrder[config.currentQuestionPos];
    if (originalIndex < 0 || originalIndex >= (int)config.questionList.size())
    {
        Serial.printf("❌ ERROR: Invalid originalIndex: %d\n", originalIndex);
        display.showCompletionScreen();
        config.displayState = SystemConfig::SHOW_COMPLETE;
        return;
    }

    // Next player
    String nextPlayer = helper.getNextPlayer();
    if (nextPlayer == "")
    {
        Serial.println("❌ ERROR: Failed to get next player!");

        // Fallback — first active player
        for (const auto &player : config.playerScores)
        {
            if (!player.isEliminated)
            {
                config.currentPlayerName = player.name;
                break;
            }
        }

        if (config.currentPlayerName.isEmpty())
        {
            display.showCompletionScreen();
            config.displayState = SystemConfig::SHOW_COMPLETE;
            return;
        }
    }
    else
    {
        config.currentPlayerName = nextPlayer;
    }

    config.answered     = false;
    config.displayState = SystemConfig::SHOW_QUESTION;
    display.showQuestionScreen(originalIndex);

    Serial.printf("📝 Question %d/%d - %s's turn\n",
                  config.currentQuestionPos + 1,
                  (int)config.questionOrder.size(),
                  config.currentPlayerName.c_str());
}

void GameLogics::retryQuestion()
{
    Serial.println("🔄 RETRYING QUESTION");

    if (config.questionOrder.empty())
    {
        Serial.println("❌ ERROR: Question order is empty! Cannot retry.");
        showError("⚠️ ERROR!", "No questions available!");
        return;
    }

    if (config.currentQuestionPos >= (int)config.questionOrder.size())
    {
        Serial.println("⚠️ Current position out of bounds. Showing completion.");
        display.showCompletionScreen();
        config.displayState = SystemConfig::SHOW_COMPLETE;
        return;
    }

    config.displayState   = SystemConfig::SHOW_QUESTION;
    config.answered       = false;
    config.selectedAnswer = ' ';

    int originalIndex = config.questionOrder[config.currentQuestionPos];
    display.showQuestionScreen(originalIndex);

    Serial.printf("🔄 Retrying Q%d - %s's turn\n",
                  originalIndex + 1, config.currentPlayerName.c_str());
}

void GameLogics::handleAnswer(char option)
{
    Serial.printf("🎯 GameLogics::handleAnswer called with: %c\n", option);
    gameMechanics.handleAnswer(option);
}

void GameLogics::handleFeedbackTimer()
{
    if (config.isLuckActive) return;

    if (config.displayState != SystemConfig::SHOW_CORRECT &&
        config.displayState != SystemConfig::SHOW_INCORRECT)
    {
        return;
    }

    if (millis() - config.stateStartTime < config.FEEDBACK_DURATION)
    {
        return;
    }

    if (config.displayState == SystemConfig::SHOW_CORRECT)
    {
        advanceToNextQuestion();
    }
    else
    {
        retryQuestion();
    }
}

// ==========================================
// DIAGNOSTICS
// ==========================================

void GameLogics::printSystemStatus()
{
    Serial.println("=========================================");
    Serial.println("🔍 SYSTEM STATUS");
    Serial.println("=========================================");
    Serial.printf("Total Questions: %d\n", (int)config.questionList.size());
    Serial.printf("Questions Order: %d\n", (int)config.questionOrder.size());
    Serial.printf("Players:         %d\n", (int)config.playerScores.size());
    Serial.printf("Current Pos:     %d\n", config.currentQuestionPos);
    Serial.printf("Overall Score:   %d\n", config.overallScore);
    Serial.printf("Display State:   %d\n", config.displayState);
    Serial.printf("Answered:        %s\n", config.answered ? "TRUE" : "FALSE");
    Serial.printf("Free Heap:       %d bytes\n", ESP.getFreeHeap());
    Serial.printf("Min Free Heap:   %d bytes\n", ESP.getMinFreeHeap());
    Serial.printf("Free PSRAM:      %d bytes\n", ESP.getFreePsram());
    Serial.println("=========================================");
}

void GameLogics::checkMemory()
{
    static int lastHeap = 0;
    int currentHeap = ESP.getFreeHeap();

    if (currentHeap < lastHeap - 1000)
    {
        Serial.printf("⚠️ Memory leak detected! %d -> %d\n", lastHeap, currentHeap);
    }

    lastHeap = currentHeap;
}

// ==========================================
// STATE RESET
// ==========================================

void GameLogics::resetGameState()
{
    config.currentQuestionPos = 0;
    config.currentPlayerPos   = 0;
    config.overallScore       = 0;
    config.answered           = false;
    config.selectedAnswer     = ' ';
    config.stateStartTime     = 0;
    gameMechanics.resetPenaltyCount();
    // Note: displayState is set by the caller
}

void GameLogics::resetToStartScreen()
{
    config.displayState       = SystemConfig::SHOW_START;
    config.currentQuestionPos = 0;
    config.currentPlayerPos   = 0;
    config.overallScore       = 0;
    config.answered           = false;
    config.selectedAnswer     = ' ';
    config.isLuckActive       = false;
    config.stateStartTime     = 0;
    gameMechanics.resetPenaltyCount();

    display.showStartSessionScreen();
    Serial.println("🔄 Reset to start screen!");
}

// ==========================================
// HELPERS
// ==========================================

bool GameLogics::canStartQuiz()
{
    if (config.questionList.empty())
    {
        Serial.println("❌ ERROR: No questions available!");
        return false;
    }

    if (config.playerScores.empty())
    {
        Serial.println("❌ ERROR: No players available!");
        return false;
    }

    return true;
}

void GameLogics::showError(const String &title, const String &message)
{
    display.showMessage(title, message, TFT_RED);
    delay(2000);
    resetToStartScreen();
}