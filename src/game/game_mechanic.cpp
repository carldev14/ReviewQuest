/**
 * @file game_mechanics.cpp
 * @brief Luck system, penalties, and elimination
 */
#include "game/game_mechanics.h"

// ==========================================
// CONSTRUCTOR & LIFECYCLE
// ==========================================

GameMechanics::GameMechanics()
    : config(SystemConfig::get()),
      display(DisplayOutputs::get()),
      actuators(Actuators::get()),
      helper(Helper::get()),
      inputs(Inputs::get())
{
    Serial.println("🎮 GameMechanics initialized!");
    initialized = true;
}

GameMechanics::~GameMechanics()
{
    Serial.println("GameMechanics destroyed!");
}

void GameMechanics::initialize()
{
    Serial.println("🔧 Initializing GameMechanics...");
    penaltyCount            = 0;
    isPenaltyQuestionActive = false;
    isPenaltyAccomplished   = false;
}

// ==========================================
// MAIN ENTRY — ANSWER HANDLING
// ==========================================

void GameMechanics::handleAnswer(char option)
{
    Serial.println("========================================");
    Serial.println("🎯 HANDLE ANSWER CALLED");
    Serial.println("========================================");

    if (config.answered || config.displayState != SystemConfig::SHOW_QUESTION)
    {
        Serial.println("⚠️ Skipping - already answered or wrong state");
        return;
    }

    config.answered       = true;
    config.selectedAnswer = option;

    // ---- Validate state ----
    if (config.questionOrder.empty())
    {
        Serial.println("❌ ERROR: questionOrder is empty!");
        config.displayState = SystemConfig::SHOW_COMPLETE;
        display.showCompletionScreen();
        return;
    }

    if (config.currentQuestionPos >= (int)config.questionOrder.size())
    {
        Serial.printf("❌ ERROR: currentQuestionPos %d >= questionOrder size %d\n",
                      config.currentQuestionPos, (int)config.questionOrder.size());
        config.displayState = SystemConfig::SHOW_COMPLETE;
        display.showCompletionScreen();
        return;
    }

    int originalIndex = config.questionOrder[config.currentQuestionPos];
    if (originalIndex < 0 || originalIndex >= (int)config.questionList.size())
    {
        Serial.printf("❌ ERROR: originalIndex %d invalid (questionList size: %d)\n",
                      originalIndex, (int)config.questionList.size());
        config.displayState = SystemConfig::SHOW_COMPLETE;
        display.showCompletionScreen();
        return;
    }

    SystemConfig::Question &q = config.questionList[originalIndex];
    bool isCorrect = (option == q.initialCharAns);

    Serial.printf("👤 Player: %s\n", config.currentPlayerName.c_str());
    Serial.printf("🔤 Selected: %c, Correct: %c\n", option, q.initialCharAns);
    Serial.printf("📊 Result: %s\n", isCorrect ? "CORRECT" : "INCORRECT");

    // ---- Penalty question branch ----
    if (isPenaltyQuestion() || isPenaltyQuestionActive)
    {
        handlePenaltyQuestionResult(isCorrect);
        return;
    }

    // ---- Normal question ----
    if (isCorrect)
    {
        handleCorrectAnswer();
    }
    else
    {
        handleIncorrectAnswer();
    }
}

// ==========================================
// ANSWER RESULTS
// ==========================================

void GameMechanics::handleCorrectAnswer()
{
    Serial.println("✅ NORMAL QUESTION - CORRECT");

    config.overallScore++;
    helper.updatePlayerScore(config.currentPlayerName, 1);

    display.showCorrectFeedbackScreen();
    actuators.runCorrectFeedbackAction();

    config.currentQuestionPos++;
    Serial.printf("📊 Moved to question position: %d (of %d)\n",
                  config.currentQuestionPos, (int)config.questionOrder.size());

    if (config.currentQuestionPos >= (int)config.questionOrder.size())
    {
        Serial.println("🏁 ALL QUESTIONS COMPLETED!");
        config.displayState = SystemConfig::SHOW_COMPLETE;
        display.showCompletionScreen();
        return;
    }

    config.displayState   = SystemConfig::SHOW_CORRECT;
    config.stateStartTime = millis();
}

void GameMechanics::handleIncorrectAnswer()
{
    Serial.println("❌ NORMAL QUESTION - INCORRECT");

    display.showIncorrectFeedbackScreen();
    actuators.runIncorrectFeedbackAction();

    config.displayState   = SystemConfig::SHOW_INCORRECT;
    config.stateStartTime = millis();

    runPenalty();
}

void GameMechanics::handlePenaltyQuestionResult(bool isCorrect)
{
    Serial.println("========================================");
    Serial.println("⚖️ PENALTY QUESTION ACTIVE");
    Serial.println("========================================");

    isPenaltyQuestionActive = true;

    if (isCorrect)
    {
        penaltyCount            = 0;
        isPenaltyQuestionActive = false;
        Serial.println("✅ Penalty question answered correctly! Penalty cleared.");

        config.overallScore++;
        helper.updatePlayerScore(config.currentPlayerName, 1);

        display.showCorrectFeedbackScreen();
        actuators.runCorrectFeedbackAction();

        config.currentQuestionPos++;
        Serial.printf("📊 Moved to question position: %d\n", config.currentQuestionPos);

        if (config.currentQuestionPos >= (int)config.questionOrder.size())
        {
            Serial.println("🏁 All questions completed!");
            config.displayState = SystemConfig::SHOW_COMPLETE;
            display.showCompletionScreen();
            return;
        }

        config.displayState   = SystemConfig::SHOW_CORRECT;
        config.stateStartTime = millis();
        return;
    }

    // Wrong on penalty
    penaltyCount++;
    Serial.printf("❌ Wrong answer! Penalty count: %d/%d\n", penaltyCount, MAX_PENALTY_COUNT);

    if (penaltyCount >= MAX_PENALTY_COUNT)
    {
        Serial.println("💀 ELIMINATED! Too many penalty questions wrong!");
        config.eliminationReason = SystemConfig::ELIM_PENALTY_QUESTION;
        penaltyCount            = 0;
        isPenaltyQuestionActive = false;
        eliminatePlayer();
    }
    else
    {
        display.showIncorrectFeedbackScreen();
        actuators.runIncorrectFeedbackAction();
        config.displayState   = SystemConfig::SHOW_INCORRECT;
        config.stateStartTime = millis();
    }
}

// ==========================================
// LUCK SYSTEM
// ==========================================

void GameMechanics::runPenalty()
{
    Serial.println("========================================");
    Serial.println("🎲 RUNNING PENALTY/LUCK SYSTEM");
    Serial.println("========================================");

    if (isPenaltyQuestionActive)
    {
        Serial.println("⚠️ Penalty question active - skipping luck system");
        return;
    }

    bool isLucky = random(0, 100) < 50;
    Serial.printf("🎲 Random result: %s\n", isLucky ? "LUCKY" : "BAD LUCK");

    if (isLucky)
    {
        runGoodLuck();
    }
    else
    {
        runBadLuck();
    }

    Serial.println("========================================");
}

void GameMechanics::runGoodLuck()
{
    Serial.println("🍀 GOOD LUCK");

    int activePlayers = helper.getActivePlayerCount();
    int roll = random(0, 100);

    if (roll < 10 && activePlayers > 1)
    {
        passToAnotherPlayer();      // 10%
    }
    else
    {
        giveHint();                 // 40% + fallback
    }
}

void GameMechanics::runBadLuck()
{
    Serial.println("💀 BAD LUCK");

    int roll = random(0, 100);

    if (roll < 10)
    {
        incrementQuestion();        // 10%
    }
    else
    {
        int pointsToDeduct = (random(0, 2) == 0) ? 1 : 2;
        deductPoints(pointsToDeduct);   // 40% + fallback
    }
}

// ==========================================
// LUCK — GOOD EFFECTS
// ==========================================

void GameMechanics::giveHint()
{
    Serial.println("💡 HINT: Removing one wrong answer!");

    int originalIndex         = config.questionOrder[config.currentQuestionPos];
    SystemConfig::Question &q = config.questionList[originalIndex];

    if (q.originalOptionA.isEmpty())
    {
        q.originalOptionA = q.optionA;
        q.originalOptionB = q.optionB;
        q.originalOptionC = q.optionC;
        q.originalOptionD = q.optionD;
    }

    char correct = q.initialCharAns;
    char wrongOptions[3];
    int  wrongIndex = 0;

    if (correct != 'A' && q.optionA != "[REMOVED]") wrongOptions[wrongIndex++] = 'A';
    if (correct != 'B' && q.optionB != "[REMOVED]") wrongOptions[wrongIndex++] = 'B';
    if (correct != 'C' && q.optionC != "[REMOVED]") wrongOptions[wrongIndex++] = 'C';
    if (correct != 'D' && q.optionD != "[REMOVED]") wrongOptions[wrongIndex++] = 'D';

    if (wrongIndex == 0) return;

    int  remove   = random(0, wrongIndex);
    char toRemove = wrongOptions[remove];
    Serial.printf("Removed option %c\n", toRemove);

    switch (toRemove)
    {
    case 'A': q.optionA = "[REMOVED]"; break;
    case 'B': q.optionB = "[REMOVED]"; break;
    case 'C': q.optionC = "[REMOVED]"; break;
    case 'D': q.optionD = "[REMOVED]"; break;
    }

    display.showHintScreen(toRemove);
    delay(1500);
    display.showQuestionScreen(originalIndex);
}

void GameMechanics::resetAllHints()
{
    for (auto &q : config.questionList)
    {
        q.hintUsed = false;

        if (q.optionA == "[REMOVED]") q.optionA = q.originalOptionA;
        if (q.optionB == "[REMOVED]") q.optionB = q.originalOptionB;
        if (q.optionC == "[REMOVED]") q.optionC = q.originalOptionC;
        if (q.optionD == "[REMOVED]") q.optionD = q.originalOptionD;
    }

    Serial.println("🔄 All hints reset!");
}

void GameMechanics::passToAnotherPlayer()
{
    Serial.println("🔄 PASSING QUESTION to another player!");

    String newPlayer = getRandomOtherPlayer();

    display.showPassToPlayerScreen(config.currentPlayerName, newPlayer);
    delay(1500);

    config.currentPlayerName = newPlayer;
    Serial.printf("Question passed to %s\n", newPlayer.c_str());

    int originalIndex = config.questionOrder[config.currentQuestionPos];
    display.showQuestionScreen(originalIndex);
}

// ==========================================
// LUCK — BAD EFFECTS
// ==========================================

void GameMechanics::deductPoints(int pointsToDeduct)
{
    Serial.println("💀 DEDUCTING POINTS!");

    display.showDeductPointsScreen(config.currentPlayerName, pointsToDeduct);
    delay(1500);

    int currentScore = 0;
    int playerIndex  = -1;

    for (int i = 0; i < (int)config.playerScores.size(); i++)
    {
        if (config.playerScores[i].name == config.currentPlayerName)
        {
            currentScore = config.playerScores[i].score;
            playerIndex  = i;
            break;
        }
    }

    if (playerIndex == -1) return;

    int newScore = currentScore - pointsToDeduct;
    if (newScore < 0) newScore = 0;
    config.playerScores[playerIndex].score = newScore;

    Serial.printf("%s lost %d points! New score: %d\n",
                  config.currentPlayerName.c_str(), pointsToDeduct, newScore);

    config.overallScore -= pointsToDeduct;
    if (config.overallScore < 0) config.overallScore = 0;

    if (newScore == 0)
    {
        config.eliminationReason = SystemConfig::ELIM_DEDUCT_POINTS;
        eliminatePlayer();
        return;
    }

    int originalIndex = config.questionOrder[config.currentQuestionPos];
    display.showQuestionScreen(originalIndex);
}

void GameMechanics::incrementQuestion()
{
    Serial.println("========================================");
    Serial.println("💀 INCREMENT QUESTION (BAD LUCK)");
    Serial.println("========================================");

    if (penaltyCount >= MAX_PENALTY_COUNT)
    {
        Serial.println("⚠️ MAX PENALTY REACHED - CANNOT INCREMENT");
        return;
    }

    if (config.questionList.empty())
    {
        Serial.println("❌ No questions available to increment!");
        return;
    }

    if (config.currentQuestionPos >= (int)config.questionOrder.size())
    {
        Serial.printf("❌ ERROR: currentQuestionPos %d out of bounds (size: %d)\n",
                      config.currentQuestionPos, (int)config.questionOrder.size());
        return;
    }

    display.showMessage("BAD LUCK!", "Penalty Question Added!", TFT_RED);
    delay(1500);

    SystemConfig::Question penaltyQuestion = getRandomQuestion();
    penaltyQuestion.id   = config.nextQuestionId++;
    penaltyQuestion.text = "PENALTY: " + penaltyQuestion.text;

    int insertPos = config.currentQuestionPos + 1;
    config.questionList.insert(config.questionList.begin() + insertPos, penaltyQuestion);

    // Rebuild order
    config.questionOrder.clear();
    for (int i = 0; i < (int)config.questionList.size(); i++)
        config.questionOrder.push_back(i);

    penaltyCount++;
    isPenaltyQuestionActive   = true;
    config.currentQuestionPos = insertPos;

    Serial.printf("✅ Penalty added! (Attempt %d/%d) — pos %d\n",
                  penaltyCount, MAX_PENALTY_COUNT, config.currentQuestionPos);

    int penaltyIndex     = config.questionOrder[config.currentQuestionPos];
    config.answered      = false;
    display.showQuestionScreen(penaltyIndex);
}

// ==========================================
// PENALTY DETECTION
// ==========================================

bool GameMechanics::isPenaltyQuestion()
{
    if (config.questionList.empty() || config.questionOrder.empty())
        return false;

    int currentIndex = config.questionOrder[config.currentQuestionPos];
    if (currentIndex < 0 || currentIndex >= (int)config.questionList.size())
        return false;

    return config.questionList[currentIndex].text.startsWith("PENALTY:");
}

// ==========================================
// ELIMINATION
// ==========================================

void GameMechanics::eliminatePlayer()
{
    Serial.println("========================================");
    Serial.println("💀 RUNNING ELIMINATION");
    Serial.println("========================================");

    if (!canEliminate())
    {
        Serial.println("❌ Cannot eliminate - only one player remains!");
        display.showCompletionScreen();
        delay(1500);
        return;
    }

    // ---- Decide who gets eliminated ----
    String saviorPlayerName = reviveByPeerEliminate();
    String candidate;
    bool   hasSavior = !saviorPlayerName.isEmpty();

    if (hasSavior)
    {
        // 70% — savior falls instead
        if (random(0, 100) > 30)
        {
            candidate = saviorPlayerName;
            Serial.printf("🔄 Savior %s will be eliminated instead!\n", saviorPlayerName.c_str());
            config.eliminationReason = SystemConfig::ELIM_SACRIFICIAL_CONS;
        }
        else
        {
            Serial.printf("✅ Savior %s saved the current player!\n", saviorPlayerName.c_str());
            display.showNoneEliminatedScreen();
            return;
        }
    }
    else
    {
        candidate = config.currentPlayerName;
        Serial.printf("💀 No savior found! %s is eliminated.\n", config.currentPlayerName.c_str());
        config.eliminationReason = SystemConfig::ELIM_REFUSE_TO_REVIVE;
    }

    // ---- Mark player eliminated ----
    bool found = false;
    for (auto &player : config.playerScores)
    {
        if (player.name == candidate)
        {
            player.isEliminated = true;
            found = true;
            Serial.printf("💀 %s marked as eliminated!\n", candidate.c_str());
            break;
        }
    }

    helper.sortPlayers();

    if (!found)
    {
        Serial.printf("⚠️ Player '%s' not found!\n", candidate.c_str());
        return;
    }

    // ---- Show elimination ----
    display.showEliminatedPlayerScreen(candidate);
    actuators.runIncorrectFeedbackAction();
    delay(1500);

    // ---- Continue or end ----
    int activeCount = helper.getActivePlayerCount();
    Serial.printf("📊 Active players remaining: %d\n", activeCount);

    if (activeCount == 0)
    {
        Serial.println("🏁 All players eliminated! Game Over.");
        display.showCompletionScreen();
        config.displayState = SystemConfig::SHOW_COMPLETE;
        return;
    }

    // ---- Set next player ----
    helper.shufflePlayers();
    String nextPlayer = helper.getNextPlayer();

    if (nextPlayer.isEmpty())
    {
        Serial.println("❌ No next player available!");
        display.showCompletionScreen();
        config.displayState = SystemConfig::SHOW_COMPLETE;
        return;
    }

    config.currentPlayerName = nextPlayer;
    config.answered          = false;
    Serial.printf("🎮 Now playing: %s\n", config.currentPlayerName.c_str());

    // ---- Validate & show next question ----
    if (config.questionOrder.empty())
    {
        Serial.println("❌ questionOrder is empty!");
        display.showCompletionScreen();
        config.displayState = SystemConfig::SHOW_COMPLETE;
        return;
    }

    if (config.currentQuestionPos >= (int)config.questionOrder.size())
    {
        Serial.printf("⚠️ currentQuestionPos out of bounds, resetting to 0\n");
        config.currentQuestionPos = 0;
    }

    int originalIndex = config.questionOrder[config.currentQuestionPos];
    if (originalIndex < 0 || originalIndex >= (int)config.questionList.size())
    {
        Serial.printf("❌ Invalid originalIndex: %d\n", originalIndex);
        display.showCompletionScreen();
        config.displayState = SystemConfig::SHOW_COMPLETE;
        return;
    }

    config.displayState = SystemConfig::SHOW_QUESTION;
    display.showQuestionScreen(originalIndex);
}

String GameMechanics::reviveByPeerEliminate()
{
    // Reset selection state
    config.revivingProcess     = false;
    config.refuseToRevive      = false;
    config.selectedSaviorIndex = 0;

    display.showListPossibleSaviorScreen();

    int lastIndex = -1;

    // Block until the user elects or skips
    while (!config.revivingProcess)
    {
        inputs.choicesButtonSavior();

        if (lastIndex != config.selectedSaviorIndex)
        {
            lastIndex = config.selectedSaviorIndex;
            display.showListPossibleSaviorScreen();
        }

        delay(50);
    }

    if (config.refuseToRevive)
    {
        Serial.println("🚫 Player refused to save!");
        return "";
    }

    // Resolve the selected index to a player name
    String name = "";
    if (config.selectedSaviorIndex >= 0 &&
        config.selectedSaviorIndex < config.getMaxPlayer())
    {
        name = config.playerScores[config.selectedSaviorIndex].name;
    }

    // Guards
    if (name.isEmpty())                                return "";
    if (!isValidSavior(name))                          return "";

    Serial.printf("✅ Player '%s' selected as savior\n", name.c_str());
    return name;
}

bool GameMechanics::isValidSavior(const String &name)
{
    for (const auto &player : config.playerScores)
    {
        if (player.name == name)
        {
            if (player.isEliminated)
            {
                Serial.printf("⚠️ Player '%s' is already eliminated!\n", name.c_str());
                return false;
            }
            return true;
        }
    }

    Serial.printf("⚠️ Player '%s' not found!\n", name.c_str());
    return false;
}

bool GameMechanics::canEliminate()
{
    int activeCount = 0;
    for (const auto &player : config.playerScores)
        if (!player.isEliminated) activeCount++;

    return activeCount >= 2;
}

// ==========================================
// END-GAME CHECK
// ==========================================

void GameMechanics::endGameOnePlayer()
{
    // Skip if not in an active game state
    switch (config.displayState)
    {
    case SystemConfig::SHOW_START:
    case SystemConfig::SHOW_UPLOADED:
    case SystemConfig::SHOW_NO_CURRENT_SESSION:
    case SystemConfig::SHOW_CREDENTIALS:
    case SystemConfig::SHOW_NEW_SESSION:
    case SystemConfig::SHOW_COMPLETE:
    case SystemConfig::SHOW_LEADERBOARD:
        return;
    default:
        break;
    }

    if (helper.getActivePlayerCount() <= 1)
    {
        Serial.println("End the game: Only one player stays");
        display.showCompletionScreen();
        config.endGameRunOnce = true;
    }
}

// ==========================================
// HELPERS
// ==========================================

SystemConfig::Question GameMechanics::getRandomQuestion()
{
    SystemConfig::Question empty;
    empty.id            = -1;
    empty.text          = "No question available!";
    empty.initialCharAns = 'A';

    if (config.questionList.empty())
    {
        Serial.println("❌ No questions available!");
        return empty;
    }

    int idx = random(0, config.questionList.size());
    return config.questionList[idx];
}

String GameMechanics::getRandomOtherPlayer()
{
    std::vector<String> others;

    for (const auto &p : config.playerScores)
    {
        if (p.name != config.currentPlayerName && !p.isEliminated)
            others.push_back(p.name);
    }

    if (others.empty()) return "";

    return others[random(0, others.size())];
}

// ==========================================
// LEGACY
// ==========================================

void GameMechanics::handlePenaltyAnswer(bool correct)
{
    Serial.println("⚠️ handlePenaltyAnswer called - redirecting");
}