/**
 * @file display/display.cpp
 * @brief Display output implementation using SystemConfig singleton
 */
#include "display.h"
#include <algorithm>
#include "helper.h"

extern Helper helper; // Declare extern

// ==========================================
// CONSTRUCTOR
// ==========================================

DisplayOutputs::DisplayOutputs()
    : config(SystemConfig::get())
{
    // Serial.println("🖥️ DisplayOutputs initialized!");
    initialized = true;
}

DisplayOutputs::~DisplayOutputs()
{
}

// ==========================================
// PUBLIC METHODS
// ==========================================

void DisplayOutputs::showRestartGameScreen()
{
    tft.fillScreen(ST77XX_BLACK);
    tft.setTextSize(2);
    tft.setTextColor(ST77XX_CYAN);
    tft.setCursor(44, 20);
    tft.print("REVIEW");
    tft.setCursor(50, 50);
    tft.print("QUEST");

    tft.setTextSize(1);
    tft.setTextColor(ST77XX_WHITE);
    tft.setCursor(38, 80);
    tft.print("Game restarted");

    tft.setTextColor(ST77XX_YELLOW);
    tft.setCursor(8, 90);
    tft.print("Will starts in 1 seconds");
    config.displayState = SystemConfig::SHOW_RESTART_GAME;
    config.currentQuestionPos = 0;
    config.currentPlayerPos = 0;
    config.overallScore = 0;
    config.answered = false;
}

void DisplayOutputs::showStartScreen()
{
    tft.fillScreen(ST77XX_BLACK);
    tft.setTextSize(2);
    tft.setTextColor(ST77XX_CYAN);
    tft.setCursor(44, 20);
    tft.print("REVIEW");
    tft.setCursor(50, 50);
    tft.print("QUEST");

    tft.setTextSize(1);
    tft.setTextColor(ST77XX_YELLOW);
    tft.setCursor(20, 90);
    tft.print("Press START to begin");

    tft.setTextColor(ST77XX_WHITE);
    tft.setCursor(20, 100);
    tft.printf("Questions: %d", (int)config.questionList.size());

    tft.setTextColor(ST77XX_CYAN);
    tft.setCursor(20, 110);
    tft.printf("Players: %d", (int)config.playerScores.size());

    config.displayState = SystemConfig::SHOW_START;
    config.currentQuestionPos = 0;
    config.currentPlayerPos = 0;
    config.overallScore = 0;
    config.answered = false;
}

void DisplayOutputs::showQuestionScreen(int index)
{
    if (index >= (int)config.questionList.size())
    {
        showCompletionScreen();
        return;
    }

    SystemConfig::Question &q = config.questionList[index];
    config.answered = false;
    config.selectedAnswer = ' ';
    config.displayState = SystemConfig::SHOW_QUESTION;

    tft.fillScreen(ST77XX_BLACK);
    tft.setTextSize(1);

    int yPos = 5;

    // Header: Question number, overall score
    tft.setTextColor(ST77XX_CYAN);
    tft.setCursor(5, yPos);
    tft.printf("Q%d/%d  Score:%d", index + 1, (int)config.questionList.size(), config.overallScore);
    yPos += 12;

    // Show current player and their individual score
    tft.setTextColor(ST77XX_MAGENTA);
    tft.setCursor(5, yPos);
    int playerScore = getPlayerScore(config.currentPlayerName);
    tft.printf("%s: %d pts", config.currentPlayerName.c_str(), playerScore);
    yPos += 12;

    // Question text with word wrapping
    tft.setTextColor(ST77XX_WHITE);
    wrapText(q.text, 5, yPos, 21, tft.height() - 50, 10, ST77XX_WHITE);

    // Separator line
    yPos += 2;
    tft.drawLine(5, yPos, tft.width() - 5, yPos, ST77XX_BLUE);
    yPos += 8;

    // Choices
    tft.setTextColor(ST77XX_YELLOW);
    tft.setCursor(5, yPos);
    tft.print("A. ");
    tft.setTextColor(ST77XX_WHITE);
    tft.println(q.optionA);
    yPos += 10;

    tft.setTextColor(ST77XX_YELLOW);
    tft.setCursor(5, yPos);
    tft.print("B. ");
    tft.setTextColor(ST77XX_WHITE);
    tft.println(q.optionB);
    yPos += 10;

    tft.setTextColor(ST77XX_YELLOW);
    tft.setCursor(5, yPos);
    tft.print("C. ");
    tft.setTextColor(ST77XX_WHITE);
    tft.println(q.optionC);
    yPos += 10;

    tft.setTextColor(ST77XX_YELLOW);
    tft.setCursor(5, yPos);
    tft.print("D. ");
    tft.setTextColor(ST77XX_WHITE);
    tft.println(q.optionD);
    yPos += 10;

    // Instructions
    tft.setTextColor(ST77XX_YELLOW);
    tft.setCursor(5, 115);
    tft.print("Press A/B/C/D");
}

void DisplayOutputs::showCorrectFeedback()
{
    tft.fillScreen(ST77XX_BLACK);
    tft.setTextSize(2);
    tft.setTextColor(ST77XX_GREEN);
    tft.setCursor(10, 20);
    tft.print("PASS TO");
    tft.setCursor(5, 50);
    tft.print("NEXT PLAYER!");

    tft.setTextSize(1);
    tft.setTextColor(ST77XX_WHITE);
    tft.setCursor(10, 80);
    tft.printf("Score: %d", config.overallScore);

    // Show current player's individual score
    tft.setTextColor(ST77XX_CYAN);
    tft.setCursor(10, 95);
    int playerScore = getPlayerScore(config.currentPlayerName);
    tft.printf("%s: %d pts", config.currentPlayerName.c_str(), playerScore);

    config.displayState = SystemConfig::SHOW_CORRECT;
    config.stateStartTime = millis();
}

void DisplayOutputs::showIncorrectFeedback()
{
    tft.fillScreen(ST77XX_BLACK);
    tft.setTextSize(2);
    tft.setTextColor(ST77XX_RED);
    tft.setCursor(20, 30);
    tft.print("INCORRECT!");

    tft.setTextSize(1);
    tft.setTextColor(ST77XX_YELLOW);
    tft.setCursor(10, 70);
    tft.printf("%s stays!", config.currentPlayerName.c_str());

    tft.setTextColor(ST77XX_WHITE);
    tft.setCursor(10, 85);
    tft.print("Try again...");

    config.displayState = SystemConfig::SHOW_INCORRECT;
    config.stateStartTime = millis();
}

void DisplayOutputs::showCompletionScreen()
{
    tft.fillScreen(ST77XX_BLACK);
    tft.setTextSize(2);
    tft.setTextColor(ST77XX_GREEN);
    tft.setCursor(26, 10);
    tft.print("ALL DONE!");

    tft.setTextSize(1.5);
    tft.setTextColor(ST77XX_WHITE);
    tft.setCursor(10, 40);
    tft.printf("Overall Score: %d/%d", config.overallScore, (int)config.questionList.size());

    // Show top player
    tft.setTextSize(1);
    tft.setTextColor(ST77XX_YELLOW);
    tft.setCursor(10, 50);

    // Find player with highest score
    int highestScore = 0;
    String topPlayer = "";
    for (int i = 0; i < (int)config.playerScores.size(); i++)
    {
        if (config.playerScores[i].score > highestScore)
        {
            highestScore = config.playerScores[i].score;
            topPlayer = config.playerScores[i].name;
        }
    }

    if (topPlayer != "")
    {
        tft.printf("Winner: %s (%d pts)", topPlayer.c_str(), highestScore);
    }
    else
    {
        tft.print("No winner found!");
    }

    tft.setTextColor(ST77XX_CYAN);
    tft.setCursor(10, 85);
    tft.print("Press START for");
    tft.setCursor(10, 100);
    tft.print("Leaderboard");

    //* Track current location screen
    config.displayState = SystemConfig::SHOW_COMPLETE;
}

/**
 * @file display_outputs.cpp
 * @brief Renders leaderboard ranking with active and eliminated player statuses.
 */
void DisplayOutputs::showLeaderboard()
{
    // 1. Sort players (active players first by score, then eliminated players)
    helper.sortPlayersByScore();

    tft.fillScreen(ST77XX_BLACK);

    // 2. Guard against empty player vector
    if (config.playerScores.empty())
    {
        tft.setTextColor(ST77XX_RED);
        tft.setCursor(10, 50);
        tft.print("No scores available.");
        config.displayState = SystemConfig::SHOW_LEADERBOARD;
        return;
    }

    int yPos = 10;
    tft.setTextSize(1);

    // Draw column headers
    tft.setTextColor(ST77XX_CYAN);
    tft.setCursor(5, yPos);
    tft.print("RANK");
    tft.setCursor(50, yPos);
    tft.print("PLAYER");
    tft.setCursor(105, yPos);
    tft.print("SCORE");
    yPos += 12;

    // Separator line
    tft.drawLine(5, yPos, tft.width() - 5, yPos, ST77XX_BLUE);
    yPos += 5;

    // Display each player up to screen height limits
    const int maxDisplay = min(static_cast<int>(config.playerScores.size()), 8);

    for (int i = 0; i < maxDisplay; i++)
    {
        const auto &player = config.playerScores[i];
        uint16_t color = ST77XX_WHITE;
        String rankStr;

        // Rank designation
        if (i == 0)
        {
            rankStr = "Top 1";
            color = ST77XX_YELLOW;
        }
        else if (i == 1)
        {
            rankStr = "Top 2";
            color = ST77XX_CYAN;
        }
        else if (i == 2)
        {
            rankStr = "Top 3";
            color = 0xCD71; // Bronze color
        }
        else
        {
            rankStr = String(i + 1) + ".";
            color = ST77XX_WHITE;
        }

        // Highlight current player active bar
        if (player.name == config.currentPlayerName)
        {
            tft.fillRect(3, yPos - 2, tft.width() - 6, 12, ST77XX_BLUE);
            tft.setTextColor(ST77XX_WHITE);
        }
        else
        {
            tft.setTextColor(player.isEliminated ? ST77XX_RED : color);
        }

        // Render Rank Column
        tft.setCursor(5, yPos);
        tft.print(rankStr.c_str());

        // Render Player Name Column
        tft.setCursor(50, yPos);
        String formattedName = player.name.length() == 0 ? "Player" : player.name;
        if (formattedName.length() > 10)
        {
            formattedName = formattedName.substring(0, 9) + ".";
        }
        tft.print(formattedName.c_str());

        // Render Score Column / Elimination Flag
        tft.setCursor(105, yPos);
        tft.print(player.score);

        // If eliminated
        if (player.isEliminated)
            tft.print(" (ELIM)");

        yPos += 15;
        if (yPos > tft.height() - 20)
        {
            break;
        }
    }

    // Overflow footer indicator
    const int totalPlayers = static_cast<int>(config.playerScores.size());
    if (totalPlayers > 8)
    {
        tft.setTextColor(ST77XX_YELLOW);
        tft.setCursor(5, tft.height() - 10);
        tft.printf("+%d more players", totalPlayers - 8);
    }

    tft.setTextColor(ST77XX_CYAN);
    tft.setCursor(10, tft.height() - 20);
    tft.print("Press START to replay");

    // Track current display state
    config.displayState = SystemConfig::SHOW_LEADERBOARD;
}

// ==========================================
// SHOW NO QUESTIONS SCREEN
// ==========================================

void DisplayOutputs::showNoQuestionsScreen()
{
    tft.fillScreen(ST77XX_BLACK);
    drawBorderedBox(10, 15, tft.width() - 20, 50, ST77XX_RED, ST77XX_RED);
    tft.setTextSize(2);
    tft.setTextColor(ST77XX_BLACK);
    tft.setCursor(30, 28);
    tft.print("ERROR!");

    tft.setTextSize(1.5);
    tft.setTextColor(ST77XX_YELLOW);
    tft.setCursor(10, 80);
    tft.print("No Questions Found!");

    tft.setTextSize(1);
    tft.setTextColor(ST77XX_WHITE);
    tft.setCursor(10, 105);
    tft.print("Please add questions");
    tft.setCursor(10, 120);
    tft.print("and restart the game.");

    config.displayState = SystemConfig::SHOW_COMPLETE;
}

void DisplayOutputs::showHint(char removedOption)
{
    tft.fillScreen(ST77XX_BLACK);
    tft.setTextSize(2);
    tft.setTextColor(ST77XX_GREEN);
    tft.setCursor(20, 20);
    tft.print("HINT!");

    tft.setTextSize(1);
    tft.setTextColor(ST77XX_WHITE);
    tft.setCursor(10, 60);
    tft.print("One wrong answer");
    tft.setCursor(10, 75);
    tft.print("has been removed!");

    tft.setTextColor(ST77XX_YELLOW);
    tft.setCursor(10, 100);
    tft.print("Press A/B/C/D");
}

void DisplayOutputs::showPassToPlayer(const String &currentPlayer, const String &newPlayer)
{
    tft.fillScreen(ST77XX_BLACK);
    tft.setTextSize(2);
    tft.setTextColor(ST77XX_CYAN);
    tft.setCursor(10, 20);
    tft.print("PASSED!");

    tft.setTextSize(1);
    tft.setTextColor(ST77XX_WHITE);
    tft.setCursor(10, 60);
    tft.printf("%s passes to", currentPlayer.c_str());

    tft.setTextColor(ST77XX_YELLOW);
    tft.setTextSize(2);
    tft.setCursor(10, 85);
    tft.print(newPlayer);
}

void DisplayOutputs::showDeductPoints(const String &playerName, int pointsDeducted)
{
    tft.fillScreen(ST77XX_BLACK);
    tft.setTextSize(1);
    tft.setTextColor(ST77XX_RED);
    tft.setCursor(10, 20);
    tft.print("Ouch...");

    tft.setTextSize(1.5);
    tft.setTextColor(ST77XX_WHITE);
    tft.setCursor(10, 60);
    tft.printf("%s loses", playerName.c_str());

    tft.setTextColor(ST77XX_RED);
    tft.setTextSize(2);
    tft.setCursor(10, 85);
    tft.printf("%d pts", pointsDeducted);
}

void DisplayOutputs::showIncrementQuestion()
{
    tft.fillScreen(ST77XX_BLACK);
    tft.setTextSize(2);
    tft.setTextColor(ST77XX_RED);
    tft.setCursor(10, 20);
    tft.print("BAD LUCK!");

    tft.setTextSize(1.5);
    tft.setTextColor(ST77XX_WHITE);
    tft.setCursor(10, 60);
    tft.print("Penalty Question");
    tft.setCursor(10, 80);
    tft.print("Added!");
}

void DisplayOutputs::showLuckResult(bool isLucky, const String &message)
{
    config.isLuckActive = true;

    tft.fillScreen(ST77XX_BLACK);
    tft.setTextSize(2);

    if (isLucky)
    {
        tft.setTextColor(ST77XX_GREEN);
        tft.setCursor(10, 20);
        tft.print("LUCKY!");
    }
    else
    {
        tft.setTextColor(ST77XX_RED);
        tft.setCursor(10, 20);
        tft.print("BAD LUCK!");
    }

    int yPos = 60;
    wrapText(message, 10, yPos, 21, tft.height() - 20, 10, ST77XX_WHITE);

    unsigned long startTime = millis();
    while (millis() - startTime < 3000)
    {
        delay(10);
        yield();
    }

    config.isLuckActive = false;
}

void DisplayOutputs::showEliminationWarning()
{
    tft.fillScreen(ST77XX_BLACK);
    tft.setTextSize(2);
    tft.setTextColor(ST77XX_YELLOW);
    tft.setCursor(10, 20);
    tft.print("WARNING!");

    tft.setTextSize(1);
    tft.setTextColor(ST77XX_WHITE);
    tft.setCursor(10, 60);
    tft.print("Someone will be");
    tft.setCursor(10, 75);
    tft.print("ELIMINATED!");
}

void DisplayOutputs::showEliminatedPlayer(const String &playerName)
{
    String message = "Failed to accomplished!";
    // Use a switch with the enum from SystemConfig
    switch (config.eliminationReason)
    {
    case SystemConfig::ELIM_DEDUCT_POINTS:
        message = "No points left...";
        break;

    case SystemConfig::ELIM_PENALTY_QUESTION:
        message = "Penalty: 2 wrong answers!";
        break;

    case SystemConfig::ELIM_TASK_FAILED:
        message = "Failed to accomplish task!";
        break;

    case SystemConfig::ELIM_UNKNOWN:
    default:
        message = "Eliminated!";
        break;
    }

    tft.fillScreen(ST77XX_BLACK);
    tft.setTextSize(2);
    tft.setTextColor(ST77XX_RED);
    tft.setCursor(10, 20);
    tft.print("ELIMINATED!");

    tft.setTextSize(1);
    tft.setTextColor(ST77XX_WHITE);
    tft.setCursor(10, 50);
    tft.printf("%s is out!", playerName.c_str());

    tft.setCursor(10, 70);
    tft.setTextColor(ST77XX_RED);
    tft.print(message);
}

void DisplayOutputs::showMessage(const String &title, const String &message, uint16_t color)
{
    tft.fillScreen(ST77XX_BLACK);
    drawBorderedBox(5, 5, tft.width() - 10, 50, color, color);

    tft.setTextSize(2);
    tft.setTextColor(ST77XX_BLACK);
    tft.setCursor(10, 18);
    tft.print(title);

    tft.setTextSize(1);
    tft.setTextColor(ST77XX_WHITE);
    tft.setCursor(10, 70);
    tft.print(message);
}

void DisplayOutputs::clearDisplay()
{
    tft.fillScreen(ST77XX_BLACK);
}

void DisplayOutputs::updateTimer(int seconds)
{
    tft.setTextSize(1);
    tft.setTextColor(ST77XX_YELLOW);
    tft.fillRect(tft.width() - 50, tft.height() - 20, 45, 15, ST77XX_BLACK);
    tft.setCursor(tft.width() - 50, tft.height() - 18);
    tft.printf("%ds", seconds);
}

// ==========================================
// PRIVATE METHODS
// ==========================================

int DisplayOutputs::wrapText(const String &text, int x, int &y, int maxChars,
                             int maxY, int lineHeight, uint16_t textColor)
{
    int linesPrinted = 0;
    String remaining = text;

    tft.setTextColor(textColor);
    tft.setTextSize(1);

    while (remaining.length() > 0 && (maxY == 0 || y < maxY))
    {
        String line;
        int splitPos = -1;

        if (remaining.length() > maxChars)
        {
            splitPos = remaining.lastIndexOf(' ', maxChars);
            if (splitPos == -1)
            {
                splitPos = maxChars;
            }
            line = remaining.substring(0, splitPos);
            remaining = remaining.substring(splitPos + 1);
        }
        else
        {
            line = remaining;
            remaining = "";
        }

        tft.setCursor(x, y);
        tft.println(line);
        y += lineHeight;
        linesPrinted++;
    }

    return linesPrinted;
}

void DisplayOutputs::drawHeader(const String &text)
{
    tft.fillRect(0, 0, tft.width(), 20, ST77XX_CYAN);
    tft.setTextColor(ST77XX_BLACK);
    tft.setTextSize(1);
    tft.setCursor(5, 5);
    tft.print(text);
}

void DisplayOutputs::drawSeparator(int y)
{
    tft.drawLine(5, y, tft.width() - 5, y, ST77XX_BLUE);
}

void DisplayOutputs::drawBorderedBox(int x, int y, int w, int h, uint16_t color, uint16_t fillColor)
{
    if (fillColor != -1)
    {
        tft.fillRect(x, y, w, h, fillColor);
    }
    tft.drawRect(x, y, w, h, color);
}

int DisplayOutputs::getPlayerScore(const String &playerName)
{
    for (int i = 0; i < (int)config.playerScores.size(); i++)
    {
        if (config.playerScores[i].name == playerName)
        {
            return config.playerScores[i].score;
        }
    }
    return 0;
}