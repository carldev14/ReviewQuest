/**
 * @file game_logic.cpp
 * @brief Game logic implementation
 */
#include "game_logic.h"

// ==========================================
// CONSTRUCTOR & DESTRUCTOR
// ==========================================

GameLogic::GameLogic()
    : config(SystemConfig::get()),
      display(DisplayOutputs::get()),
      actuators(Actuators::get()),
      helper(Helper::get()),
      gameMechanics(GameMechanics::get())
{
    Serial.println("🎯 GameLogic initialized!");
    initialized = true;
}

GameLogic::~GameLogic()
{
    Serial.println("GameLogic destroyed!");
}

// ==========================================
// PUBLIC METHODS
// ==========================================

void GameLogic::initialize()
{
    Serial.println("🔧 Initializing GameLogic...");
    resetGameState();
    // Set to SHOW_START initially
    config.displayState = SystemConfig::SHOW_START;
}

void GameLogic::startQuiz()
{
    Serial.println("========================================");
    Serial.println("🚀 STARTING QUIZ");
    Serial.println("========================================");

    // Safety check - make sure there are questions
    if (!canStartQuiz())
    {
        showError("⚠️ ERROR!", "Cannot start quiz!");
        // Reset to start screen
        resetToStartScreen();
        return;
    }

    // FOR DEBUG [PRINT ALL PLAYERS]
    helper.printAllPlayers();

    // Reset game state but don't set to SHOW_QUESTION yet
    config.currentQuestionPos = 0;
    config.currentPlayerPos = 0;
    config.overallScore = 0;
    config.answered = false;
    config.selectedAnswer = ' ';
    config.stateStartTime = 0;
    gameMechanics.resetPenaltyCount();

    // Shuffle questions/options and players
    helper.shuffleQuestions();
    helper.shuffleQuestionOptions();
    helper.shufflePlayers();

    // Get the first player
    config.currentPlayerName = helper.getNextPlayer();

    // Make sure questionOrder has at least one question
    if (config.questionOrder.empty())
    {
        Serial.println("❌ ERROR: Question order is empty!");
        showError("⚠️ ERROR!", "Question order is empty!");
        resetToStartScreen();
        return;
    }

    // Now set the state to SHOW_QUESTION
    config.displayState = SystemConfig::SHOW_QUESTION;

    int originalIndex = config.questionOrder[0];
    display.showQuestionScreen(originalIndex);
    Serial.printf("✅ Quiz started! First player: %s\n", config.currentPlayerName.c_str());
    helper.printPlayerScores();
    printSystemStatus();
}

void GameLogic::endGameOnePlayer()
{
    if (helper.getActivePlayerCount() <= 0)
    {
        Serial.println("End the game: Only one player stays");
        display.showCompletionScreen();
    }
}

void GameLogic::advanceToNextQuestion()
{
    Serial.println("➡️ ADVANCING TO NEXT QUESTION");
    Serial.printf("📊 Current position: %d, Order size: %d\n",
                  config.currentQuestionPos, (int)config.questionOrder.size());

    // Safety checks
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

    // ===== CHECK IF ALL QUESTIONS ARE COMPLETED =====
    if (config.currentQuestionPos >= (int)config.questionOrder.size())
    {
        Serial.println("🏁 All questions completed!");
        gameMechanics.resetPenaltyCount();
        display.showCompletionScreen();
        config.displayState = SystemConfig::SHOW_COMPLETE;
        return;
    }

    // Validate question index
    int originalIndex = config.questionOrder[config.currentQuestionPos];
    if (originalIndex < 0 || originalIndex >= (int)config.questionList.size())
    {
        Serial.printf("❌ ERROR: Invalid originalIndex: %d\n", originalIndex);
        display.showCompletionScreen();
        config.displayState = SystemConfig::SHOW_COMPLETE;
        return;
    }

    // Get next player
    String nextPlayer = helper.getNextPlayer();
    if (nextPlayer == "")
    {
        Serial.println("❌ ERROR: Failed to get next player!");
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

    // Reset state and show question
    config.answered = false;
    config.displayState = SystemConfig::SHOW_QUESTION;
    display.showQuestionScreen(originalIndex);
    Serial.printf("📝 Question %d/%d - %s's turn\n",
                  config.currentQuestionPos + 1,
                  (int)config.questionOrder.size(),
                  config.currentPlayerName.c_str());
}

void GameLogic::retryQuestion()
{
    Serial.println("🔄 RETRYING QUESTION");

    // Safety check - make sure questionOrder is not empty
    if (config.questionOrder.empty())
    {
        Serial.println("❌ ERROR: Question order is empty! Cannot retry.");
        showError("⚠️ ERROR!", "No questions available!");
        return;
    }

    // Make sure currentQuestionPos is valid
    if (config.currentQuestionPos >= (int)config.questionOrder.size())
    {
        Serial.println("⚠️ Current position out of bounds. Showing completion.");
        display.showCompletionScreen();
        config.displayState = SystemConfig::SHOW_COMPLETE;
        return;
    }

    // Set state to SHOW_QUESTION for retry
    config.displayState = SystemConfig::SHOW_QUESTION;
    config.answered = false;
    config.selectedAnswer = ' ';

    int originalIndex = config.questionOrder[config.currentQuestionPos];
    display.showQuestionScreen(originalIndex);
    Serial.printf("🔄 Retrying Q%d - %s's turn\n", originalIndex + 1, config.currentPlayerName.c_str());
}

void GameLogic::handleFeedbackTimer()
{
    if (config.isLuckActive)
    {
        return;
    }

    if (config.displayState == SystemConfig::SHOW_CORRECT ||
        config.displayState == SystemConfig::SHOW_INCORRECT)
    {
        if (millis() - config.stateStartTime >= config.FEEDBACK_DURATION)
        {
            if (config.displayState == SystemConfig::SHOW_CORRECT)
            {
                advanceToNextQuestion();
            }
            else if (config.displayState == SystemConfig::SHOW_INCORRECT)
            {
                retryQuestion();
            }
        }
    }
}

void GameLogic::restartGame()
{
    Serial.println("🔄 RESTARTING GAME");

    // Reset to start screen first
    display.showRestartGameScreen();

    // ===== RESET SOMETHING BEFORE START FRESH
    gameMechanics.resetAllHints();

    // Then start quiz after a delay
    delay(1000);
    startQuiz();
    Serial.println("✅ Game restarted!");
}

void GameLogic::handleAnswer(char option)
{
    Serial.printf("🎯 GameLogic::handleAnswer called with: %c\n", option);
    gameMechanics.handleAnswer(option);
}

void GameLogic::printSystemStatus()
{
    Serial.println("=========================================");
    Serial.println("🔍 SYSTEM STATUS");
    Serial.println("=========================================");
    Serial.printf("Total Questions: %d\n", (int)config.questionList.size());
    Serial.printf("Questions Order: %d\n", (int)config.questionOrder.size());
    Serial.printf("Players: %d\n", (int)config.playerScores.size());
    Serial.printf("Current Position: %d\n", config.currentQuestionPos);
    Serial.printf("Overall Score: %d\n", config.overallScore);
    Serial.printf("Display State: %d\n", config.displayState);
    Serial.printf("Answered: %s\n", config.answered ? "TRUE" : "FALSE");
    Serial.printf("Free Heap: %d bytes\n", ESP.getFreeHeap());
    Serial.printf("Min Free Heap: %d bytes\n", ESP.getMinFreeHeap());
    Serial.printf("Free PSRAM: %d bytes\n", ESP.getFreePsram());
    Serial.println("=========================================");
}

void GameLogic::checkMemory()
{
    static int lastHeap = 0;
    int currentHeap = ESP.getFreeHeap();
    if (currentHeap < lastHeap - 1000)
    {
        Serial.printf("⚠️ Memory leak detected! %d -> %d\n", lastHeap, currentHeap);
    }
    lastHeap = currentHeap;
}

void GameLogic::resetToStartScreen()
{
    config.displayState = SystemConfig::SHOW_START;
    config.currentQuestionPos = 0;
    config.currentPlayerPos = 0;
    config.overallScore = 0;
    config.answered = false;
    config.selectedAnswer = ' ';
    config.isLuckActive = false;
    config.stateStartTime = 0;
    gameMechanics.resetPenaltyCount();
    display.showStartScreen();
    Serial.println("🔄 Reset to start screen!");
}

// ==========================================
// PRIVATE METHODS
// ==========================================

bool GameLogic::canStartQuiz()
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

void GameLogic::resetGameState()
{
    config.currentQuestionPos = 0;
    config.currentPlayerPos = 0;
    config.overallScore = 0;
    config.answered = false;
    config.selectedAnswer = ' ';
    config.stateStartTime = 0;
    gameMechanics.resetPenaltyCount();
    // DON'T set displayState here - let the calling function decide
}

void GameLogic::showError(const String &title, const String &message)
{
    display.showMessage(title, message, ST77XX_RED);
    delay(2000);
    resetToStartScreen();
}