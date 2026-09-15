/**
 * @file game_mechanics.cpp
 * @brief Game mechanics implementation - FIXED PENALTY TRACKING
 */
#include "game_mechanics.h"

// ==========================================
// CONSTRUCTOR
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
    // Cleanup if needed
}

void GameMechanics::initialize()
{
    Serial.println("🔧 Initializing GameMechanics...");
    penaltyCount = 0;
    isPenaltyQuestionActive = false;
    isPenaltyAccomplished = false;
}

// ==========================================
// GET RANDOM QUESTION
// ==========================================

SystemConfig::Question GameMechanics::getRandomQuestion()
{
    SystemConfig::Question emptyQuestion;
    emptyQuestion.id = -1;
    emptyQuestion.text = "No question available!";
    emptyQuestion.optionA = "";
    emptyQuestion.optionB = "";
    emptyQuestion.optionC = "";
    emptyQuestion.optionD = "";
    emptyQuestion.initialCharAns = 'A';

    if (config.questionList.empty())
    {
        Serial.println("❌ No questions available!");
        return emptyQuestion;
    }

    int randomIndex = random(0, config.questionList.size());
    return config.questionList[randomIndex];
}

// ==========================================
// INCREMENT QUESTION - ONLY BAD LUCK FEATURE
// ==========================================

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

    // Safety check
    if (config.currentQuestionPos >= (int)config.questionOrder.size())
    {
        Serial.printf("❌ ERROR: currentQuestionPos %d out of bounds (size: %d)\n",
                      config.currentQuestionPos, (int)config.questionOrder.size());
        return;
    }

    Serial.printf("📊 Current penalty count: %d/%d\n", penaltyCount, MAX_PENALTY_COUNT);
    Serial.printf("📊 Current player: %s\n", config.currentPlayerName.c_str());
    Serial.printf("📊 Current question position: %d\n", config.currentQuestionPos);
    Serial.printf("📊 Total questions: %d\n", (int)config.questionList.size());

    display.showMessage("BAD LUCK!", "Penalty Question Added!", ST77XX_RED);
    delay(1500);

    // Pick a random question
    SystemConfig::Question penaltyQuestion = getRandomQuestion();
    penaltyQuestion.id = config.nextQuestionId++;
    penaltyQuestion.text = "PENALTY: " + penaltyQuestion.text;

    // INSERT AFTER CURRENT POSITION
    int insertPos = config.currentQuestionPos + 1;
    config.questionList.insert(config.questionList.begin() + insertPos, penaltyQuestion);

    // REBUILD questionOrder
    config.questionOrder.clear();
    for (int i = 0; i < (int)config.questionList.size(); i++)
    {
        config.questionOrder.push_back(i);
    }

    // Increment penalty counter and set flag
    penaltyCount++;
    isPenaltyQuestionActive = true;

    // MOVE TO THE PENALTY QUESTION
    config.currentQuestionPos = insertPos;

    Serial.println("========================================");
    Serial.printf("✅ Penalty question added! (Attempt %d/%d)\n", penaltyCount, MAX_PENALTY_COUNT);
    Serial.printf("✅ isPenaltyQuestionActive: %s\n", isPenaltyQuestionActive ? "TRUE" : "FALSE");
    Serial.printf("✅ Total questions now: %d\n", (int)config.questionList.size());
    Serial.printf("✅ MOVING to penalty question at position: %d\n", config.currentQuestionPos);
    Serial.println("========================================");

    // Show the PENALTY question
    int penaltyIndex = config.questionOrder[config.currentQuestionPos];
    config.answered = false;
    display.showQuestionScreen(penaltyIndex);
}

// ==========================================
// HANDLE ANSWER - WITH PROPER PENALTY TRACKING
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

    config.answered = true;
    config.selectedAnswer = option;

    // ===== SAFETY CHECK: Validate current position =====
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
    Serial.printf("📊 currentQuestionPos: %d, questionOrder size: %d\n",
                  config.currentQuestionPos, (int)config.questionOrder.size());

    // ===== HANDLE PENALTY QUESTION =====
    if (isPenaltyQuestion() || isPenaltyQuestionActive)
    {
        Serial.println("========================================");
        Serial.println("⚖️ PENALTY QUESTION ACTIVE");
        Serial.println("========================================");

        isPenaltyQuestionActive = true;

        if (isCorrect)
        {
            // Reset penalty state
            penaltyCount = 0;
            isPenaltyQuestionActive = false;
            Serial.println("✅ Penalty question answered correctly! Penalty cleared.");

            config.overallScore++;
            helper.updatePlayerScore(config.currentPlayerName, 1);
            display.showCorrectFeedbackScreen();
            actuators.runCorrectFeedbackAction();

            // MOVE TO NEXT QUESTION (skip the penalty question)
            config.currentQuestionPos++;
            Serial.printf("📊 Moved to question position: %d\n", config.currentQuestionPos);

            // Check if all questions are done
            if (config.currentQuestionPos >= (int)config.questionOrder.size())
            {
                Serial.println("🏁 All questions completed!");
                config.displayState = SystemConfig::SHOW_COMPLETE;
                display.showCompletionScreen();
                return;
            }

            config.displayState = SystemConfig::SHOW_CORRECT;
            config.stateStartTime = millis();
        }
        else
        {
            penaltyCount++;
            Serial.printf("❌ Wrong answer! Penalty count: %d/%d\n", penaltyCount, MAX_PENALTY_COUNT);

            if (penaltyCount >= MAX_PENALTY_COUNT)
            {
                Serial.println("💀 ELIMINATED! Too many penalty questions wrong!");
                eliminatePlayer();
                config.eliminationReason = SystemConfig::ELIM_PENALTY_QUESTION;
                penaltyCount = 0;
                isPenaltyQuestionActive = false;
            }
            else
            {
                display.showIncorrectFeedbackScreen();
                actuators.runIncorrectFeedbackAction();
                config.displayState = SystemConfig::SHOW_INCORRECT;
                config.stateStartTime = millis();
            }
        }
        return;
    }

    // ===== NORMAL QUESTION =====
    if (isCorrect)
    {
        Serial.println("✅ NORMAL QUESTION - CORRECT");
        config.overallScore++;
        helper.updatePlayerScore(config.currentPlayerName, 1);
        display.showCorrectFeedbackScreen();
        actuators.runCorrectFeedbackAction();

        // ====== MOVE TO NEXT QUESTION

        config.currentQuestionPos++;
        Serial.printf("📊 Moved to question position: %d (of %d)\n",
                      config.currentQuestionPos, (int)config.questionOrder.size());

        // Check if all questions are done
        if (config.currentQuestionPos >= (int)config.questionOrder.size())
        {
            Serial.println("🏁 ALL QUESTIONS COMPLETED!");
            config.displayState = SystemConfig::SHOW_COMPLETE;
            display.showCompletionScreen();
            return;
        }

        config.displayState = SystemConfig::SHOW_CORRECT;
        config.stateStartTime = millis();
    }
    else
    {
        Serial.println("❌ NORMAL QUESTION - INCORRECT");
        display.showIncorrectFeedbackScreen();
        actuators.runIncorrectFeedbackAction();
        config.displayState = SystemConfig::SHOW_INCORRECT;
        config.stateStartTime = millis();
        runPenalty();
    }
}

// ==========================================
// CHECK IF PENALTY QUESTION
// ==========================================

bool GameMechanics::isPenaltyQuestion()
{
    if (config.questionList.empty() || config.questionOrder.empty())
    {
        return false;
    }

    int currentIndex = config.questionOrder[config.currentQuestionPos];
    if (currentIndex < 0 || currentIndex >= (int)config.questionList.size())
    {
        return false;
    }

    String currentText = config.questionList[currentIndex].text;
    bool result = currentText.startsWith("PENALTY:");

    Serial.printf("🔍 isPenaltyQuestion check: index=%d, text='%s', result=%s\n",
                  currentIndex, currentText.c_str(), result ? "TRUE" : "FALSE");

    return result;
}

// ==========================================
// RUN PENALTY - ONLY BAD LUCK (INCREMENT)
// ==========================================

void GameMechanics::runPenalty()
{
    Serial.println("========================================");
    Serial.println("🎲 RUNNING PENALTY/LUCK SYSTEM");
    Serial.println("========================================");

    // Check if penalty question is active
    if (isPenaltyQuestionActive)
    {
        Serial.println("⚠️ Penalty question active - skipping luck system");
        Serial.println("========================================");
        return;
    }

    // 50/50 chance for luck vs bad luck
    bool isLucky = random(0, 100) < 50;
    Serial.printf("🎲 Random result: %s\n", isLucky ? "LUCKY" : "BAD LUCK");

    // Overall luck roll: 50% good, 50% bad
    if (isLucky)
    {
        Serial.println("🍀 GOOD LUCK");

        int activePlayers = helper.getActivePlayerCount();

        // Roll 0-99 for the good luck outcome
        int roll = random(0, 100);

        if (roll < 10 && activePlayers > 1)
        {
            // 10% chance: pass to another player
            passToAnotherPlayer();
        }
        else if (roll < 50) // 10% to 49% = 40% chance
        {
            // 40% chance: give hint
            giveHint();
        }
        else
        {
            // 50% chance: this shouldn't happen if isLucky is true,
            // but if it does, fall back to hint
            giveHint();
        }
    }
    else
    {
        Serial.println("💀 BAD LUCK");

        int roll = random(0, 100);

        if (roll < 10)
        {
            // 10% chance: increment question
            incrementQuestion();
        }
        else if (roll < 50) // 10% to 49% = 40% chance
        {
            // 40% chance: deduct 1-2 points
            int pointsToDeduct = (random(0, 2) == 0) ? 1 : 2;
            deductPoints(pointsToDeduct);
        }
        else
        {
            // 50% chance: shouldn't happen, fall back to deduct
            int pointsToDeduct = (random(0, 2) == 0) ? 1 : 2;
            deductPoints(pointsToDeduct);
        }
    }
    Serial.println("========================================");
}

void GameMechanics::endGameOnePlayer()
{
    // Guard: Only check for game end if the game is actually running.
    // Skip if we are in pre-game states or already finished.
    if (config.displayState == SystemConfig::SHOW_START ||
        config.displayState == SystemConfig::SHOW_UPLOADED ||
        config.displayState == SystemConfig::SHOW_NO_CURRENT_SESSION ||
        config.displayState == SystemConfig::SHOW_CREDENTIALS ||
        config.displayState == SystemConfig::SHOW_NEW_SESSION ||
        config.displayState == SystemConfig::SHOW_COMPLETE ||
        config.displayState == SystemConfig::SHOW_LEADERBOARD)
    {
        return;
    }

    if (helper.getActivePlayerCount() <= 1)
    {
        Serial.println("End the game: Only one player stays");
        display.showCompletionScreen();
        config.endGameRunOnce = true;
        return; // Stopping whenever next execution is
    }
}

// ==========================================
// OTHER METHODS
// ==========================================

void GameMechanics::giveHint()
{
    Serial.println("💡 HINT: Removing one wrong answer!");

    int originalIndex = config.questionOrder[config.currentQuestionPos];
    SystemConfig::Question &q = config.questionList[originalIndex];

    // Store original options if not already stored
    if (q.originalOptionA.isEmpty())
    {
        q.originalOptionA = q.optionA;
        q.originalOptionB = q.optionB;
        q.originalOptionC = q.optionC;
        q.originalOptionD = q.optionD;
    }

    // Remove one wrong answer
    char correct = q.initialCharAns;
    char wrongOptions[3];
    int wrongIndex = 0;

    // Collect all wrong options
    if (correct != 'A' && q.optionA != "[REMOVED]")
        wrongOptions[wrongIndex++] = 'A';
    if (correct != 'B' && q.optionB != "[REMOVED]")
        wrongOptions[wrongIndex++] = 'B';
    if (correct != 'C' && q.optionC != "[REMOVED]")
        wrongOptions[wrongIndex++] = 'C';
    if (correct != 'D' && q.optionD != "[REMOVED]")
        wrongOptions[wrongIndex++] = 'D';

    // Randomly remove one wrong option
    if (wrongIndex > 0)
    {
        int remove = random(0, wrongIndex);
        char toRemove = wrongOptions[remove];
        Serial.printf("Removed option %c\n", toRemove);

        // Mark the option as removed
        switch (toRemove)
        {
        case 'A':
            q.optionA = "[REMOVED]";
            break;
        case 'B':
            q.optionB = "[REMOVED]";
            break;
        case 'C':
            q.optionC = "[REMOVED]";
            break;
        case 'D':
            q.optionD = "[REMOVED]";
            break;
        }

        display.showHintScreen(toRemove);
        delay(1500);
        display.showQuestionScreen(originalIndex);
    }
}

void GameMechanics::resetAllHints()
{
    for (auto &q : config.questionList)
    {
        q.hintUsed = false;
        // Restore options if they were removed
        if (q.optionA == "[REMOVED]")
            q.optionA = q.originalOptionA;
        if (q.optionB == "[REMOVED]")
            q.optionB = q.originalOptionB;
        if (q.optionC == "[REMOVED]")
            q.optionC = q.originalOptionC;
        if (q.optionD == "[REMOVED]")
            q.optionD = q.originalOptionD;
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

void GameMechanics::deductPoints(int pointsToDeduct)
{
    Serial.println("💀 DEDUCTING POINTS!");
    display.showDeductPointsScreen(config.currentPlayerName, pointsToDeduct);
    delay(1500);

    int currentScore = 0;
    int playerIndex = -1;

    for (int i = 0; i < (int)config.playerScores.size(); i++)
    {
        if (config.playerScores[i].name == config.currentPlayerName)
        {
            currentScore = config.playerScores[i].score;
            playerIndex = i;
            break;
        }
    }

    if (playerIndex != -1)
    {
        int newScore = currentScore - pointsToDeduct;
        if (newScore < 0)
            newScore = 0;
        config.playerScores[playerIndex].score = newScore;

        Serial.printf("%s lost %d points! New score: %d\n",
                      config.currentPlayerName.c_str(), pointsToDeduct, newScore);

        config.overallScore -= pointsToDeduct;
        if (config.overallScore < 0)
            config.overallScore = 0;

        if (newScore == 0)
        {
            eliminatePlayer();
            config.eliminationReason = SystemConfig::ELIM_DEDUCT_POINTS;
            return;
        }
    }

    int originalIndex = config.questionOrder[config.currentQuestionPos];
    display.showQuestionScreen(originalIndex);
}

// ==========================================
// PRIVATE HELPERS
// ==========================================

String GameMechanics::getRandomOtherPlayer()
{
    std::vector<String> otherPlayers;

    for (int i = 0; i < (int)config.playerScores.size(); i++)
    {
        if (config.playerScores[i].name != config.currentPlayerName && !config.playerScores[i].isEliminated)
        {
            otherPlayers.push_back(config.playerScores[i].name);
        }
    }

    if (otherPlayers.size() == 0)
    {
        return "";
    }

    int randomIndex = random(0, otherPlayers.size());
    return otherPlayers[randomIndex];
}

// ==========================================
// ELIMINATION
// ==========================================

bool GameMechanics::canEliminate()
{
    int activeCount = 0;
    for (const auto &player : config.playerScores)
    {
        if (!player.isEliminated)
        {
            activeCount++;
        }
    }
    return activeCount >= 2;
}

String GameMechanics::reviveByPeerEliminate()
{
    // Reset selection state
    config.revivingProcess = false;
    config.refuseToRevive = false;
    config.selectedSaviorIndex = 0; // Start at first player (not -1)

    // Show the list of possible saviors
    display.showListPossibleSaviorScreen();

    int lastIndex = -1; // Track last displayed index

    // WAIT for button press - UPDATE DISPLAY in loop
    while (!config.revivingProcess)
    {
        inputs.choicesButtonSavior();

        // ✅ Update display when selection changes
        if (lastIndex != config.selectedSaviorIndex)
        {
            lastIndex = config.selectedSaviorIndex;
            display.showListPossibleSaviorScreen();

            // Redraw footer
            tft.setTextColor(ST77XX_CYAN);
            tft.setCursor(10, 220);
            tft.print("B:Down  C:Up  A:Select  D:Skip");
        }

        delay(50);
    }

    // Check if player refused to save
    if (config.refuseToRevive)
    {
        Serial.println("🚫 Player refused to save!");
        // Return empty string - no one willing to save
        return "";
    }

    // Get the selected player name
    String saviorPlayerName = "";

    if (config.selectedSaviorIndex >= 0 && config.selectedSaviorIndex < config.getMaxPlayer())
    {
        saviorPlayerName = config.playerScores[config.selectedSaviorIndex].name;
    }

    // GUARD: Check if empty
    if (saviorPlayerName.isEmpty())
    {
        Serial.println("⚠️ Player candidate is empty!");
        return "";
    }

    // GUARD: Validate player exists
    bool playerExists = false;
    for (const auto &player : config.playerScores)
    {
        if (player.name == saviorPlayerName)
        {
            playerExists = true;
            break;
        }
    }

    if (!playerExists)
    {
        Serial.printf("⚠️ Player '%s' not found!\n", saviorPlayerName.c_str());
        return "";
    }

    // GUARD: Check if player is already eliminated
    for (const auto &player : config.playerScores)
    {
        if (player.name == saviorPlayerName && player.isEliminated)
        {
            Serial.printf("⚠️ Player '%s' is already eliminated!\n", saviorPlayerName.c_str());
            return "";
        }
    }

    Serial.printf("✅ Player '%s' selected for elimination\n", saviorPlayerName.c_str());
    return saviorPlayerName;
}

void GameMechanics::eliminatePlayer()
{
    Serial.println("========================================");
    Serial.println("💀 RUNNING ELIMINATION");
    Serial.println("========================================");

    // Check if can eliminate (at least 2 players remain)
    if (!canEliminate())
    {
        Serial.println("❌ Cannot eliminate - only one player remains!");
        display.showCompletionScreen();
        delay(1500);
        return;
    }

    // ===== Get eliminated player name =====
    String saviorPlayerName = reviveByPeerEliminate();
    String playerEliminatedCandidate = "";
    bool isSaviorEliminated = false;
    bool hasSavior = !saviorPlayerName.isEmpty();

    /**
     * If the SaviorPlayerName String is not empty.
     * Therefore, there is someone willing to save the player, so run the savior's destiny.
     */
    if (hasSavior)
    {
        // 70/30 chance: savior gets eliminated instead (HARDER)
        if (random(0, 100) > 30) // 70% chance of elimination
        {
            // The savior will be eliminated on their behalf
            isSaviorEliminated = true;
            playerEliminatedCandidate = saviorPlayerName;
            Serial.printf("🔄 Savior %s will be eliminated instead!\n", saviorPlayerName.c_str());
            config.eliminationReason = SystemConfig::ELIM_SACRIFICIAL_CONS;
        }
        else
        {
            // No one gets eliminated (savior saved the current player)
            isSaviorEliminated = false;
            Serial.printf("✅ Savior %s saved the current player! No elimination.\n", saviorPlayerName.c_str());
            display.showNoneEliminatedScreen();
            return;
        }
    }
    else
    {
        // No savior found - current player is eliminated
        playerEliminatedCandidate = config.currentPlayerName;
        Serial.printf("💀 No savior found! %s is eliminated.\n", config.currentPlayerName.c_str());
        config.eliminationReason = SystemConfig::ELIM_REFUSE_TO_REVIVE;
    }

    Serial.print("Player Candidate: ");
    Serial.println(playerEliminatedCandidate);

    Serial.printf("💀 Eliminating: %s\n", playerEliminatedCandidate.c_str());

    // ===== 1. Mark player as eliminated =====
    bool found = false;
    for (auto &player : config.playerScores)
    {
        if (player.name == playerEliminatedCandidate)
        {
            player.isEliminated = true;
            found = true;
            Serial.printf("💀 %s marked as eliminated!\n", playerEliminatedCandidate.c_str());
            break;
        }
    }

    // Resort players so active players are at the front
    helper.sortPlayers();

    if (!found)
    {
        Serial.printf("⚠️ Player '%s' not found in playerScores!\n", playerEliminatedCandidate.c_str());
        // Print all players for debugging
        for (const auto &p : config.playerScores)
        {
            Serial.printf("  Available: %s\n", p.name.c_str());
        }
        Serial.println("========================================");
        return;
    }

    // ===== 2. Show elimination on display =====

    display.showEliminatedPlayerScreen(playerEliminatedCandidate);
    actuators.runIncorrectFeedbackAction();
    delay(1500);

    // ===== 3. Check if any players remain =====
    int activeCount = helper.getActivePlayerCount();
    Serial.printf("📊 Active players remaining: %d\n", activeCount);

    if (activeCount == 0)
    {
        Serial.println("🏁 All players eliminated! Game Over.");
        display.showCompletionScreen();
        config.displayState = SystemConfig::SHOW_COMPLETE;
        return;
    }

    // ===== 4. Get next player =====
    helper.shufflePlayers();
    String nextPlayer = helper.getNextPlayer();

    if (nextPlayer.isEmpty())
    {
        Serial.println("❌ No next player available!");
        display.showCompletionScreen();
        config.displayState = SystemConfig::SHOW_COMPLETE;
        return;
    }

    // ===== 5. Set new current player =====
    config.currentPlayerName = nextPlayer;
    config.answered = false;
    Serial.printf("🎮 Now playing: %s\n", config.currentPlayerName.c_str());

    // ===== 6. Validate question position =====
    if (config.questionOrder.empty())
    {
        Serial.println("❌ questionOrder is empty!");
        display.showCompletionScreen();
        config.displayState = SystemConfig::SHOW_COMPLETE;
        return;
    }

    if (config.currentQuestionPos >= (int)config.questionOrder.size())
    {
        Serial.printf("⚠️ currentQuestionPos %d out of bounds, resetting to 0\n", config.currentQuestionPos);
        config.currentQuestionPos = 0;
    }

    // ===== 7. Show the question =====
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
    Serial.printf("📝 Showing question %d for %s\n", originalIndex + 1, config.currentPlayerName.c_str());
}

void GameMechanics::handlePenaltyAnswer(bool correct)
{
    // Redirect to handleAnswer logic
    Serial.println("⚠️ handlePenaltyAnswer called - redirecting");
    // This is kept for compatibility but not used directly
}
