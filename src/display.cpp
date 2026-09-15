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

void DisplayOutputs::showSplashScreen()
{
    tft.fillScreen(ST77XX_BLACK);

    // Main Title: "reviewQuest"
    tft.setTextSize(2);
    tft.setTextColor(ST77XX_WHITE);
    // Width: 11 chars * 12px = 132px. Screen width 160. X = (160-132)/2 = 14
    tft.setCursor(14, 40);
    tft.print("reviewQuest");

    // Subtitle: "Loading... Please wait."
    tft.setTextSize(1);
    tft.setTextColor(ST77XX_YELLOW);
    // Width: 24 chars * 6px = 144px. X = (160-144)/2 = 8
    tft.setCursor(8, 65);
    tft.print("Loading... Please wait.");

    // Brand: "storem"
    tft.setTextColor(ST77XX_CYAN);
    // Width: 6 chars * 6px = 36px. X = (160-36)/2 = 62
    tft.setCursor(62, 110);
    tft.print("storem");

    config.displayState = SystemConfig::SHOW_SPLASH;
}

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

    // Header:  Show current player and their individual score / Question index
    tft.setTextColor(ST77XX_CYAN);
    tft.setCursor(5, yPos);
    int playerScore = getPlayerScore(config.currentPlayerName);
    tft.printf("%s: %d pts", config.currentPlayerName.c_str(), playerScore);

    // Placed at the end, same baseline as the player scores.
    tft.setCursor(120, yPos);
    tft.printf("Q%d/%d", index + 1, (int)config.questionList.size());

    yPos += 12;

    // Question text with word wrapping
    tft.setTextColor(ST77XX_WHITE);
    wrapText(q.text, 5, yPos, 21, tft.height() - 50, 10, ST77XX_WHITE);

    // Separator line
    yPos += 2;
    tft.drawLine(5, yPos, tft.width() - 5, yPos, ST77XX_RED);
    yPos += 8;

    // Choices
    tft.setTextColor(ST77XX_GREEN);
    tft.setCursor(5, yPos);
    tft.print("A. ");
    tft.setTextColor(ST77XX_WHITE);
    tft.println(q.optionA);
    yPos += 10;

    tft.setTextColor(ST77XX_GREEN);
    tft.setCursor(5, yPos);
    tft.print("B. ");
    tft.setTextColor(ST77XX_WHITE);
    tft.println(q.optionB);
    yPos += 10;

    tft.setTextColor(ST77XX_GREEN);
    tft.setCursor(5, yPos);
    tft.print("C. ");
    tft.setTextColor(ST77XX_WHITE);
    tft.println(q.optionC);
    yPos += 10;

    tft.setTextColor(ST77XX_GREEN);
    tft.setCursor(5, yPos);
    tft.print("D. ");
    tft.setTextColor(ST77XX_WHITE);
    tft.println(q.optionD);
    yPos += 10;
}

void DisplayOutputs::showCorrectFeedbackScreen()
{
    tft.fillScreen(ST77XX_BLACK);
    tft.setTextSize(2);
    tft.setTextColor(ST77XX_GREEN);
    tft.setCursor(10, 20);
    tft.print("PASS TO");
    tft.setCursor(10, 50);
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

void DisplayOutputs::showIncorrectFeedbackScreen()
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
void DisplayOutputs::showLeaderboardScreen()
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
    tft.drawLine(5, yPos, tft.width() - 5, yPos, ST77XX_GREEN);
    yPos += 5;

    for (int i = 0; i < config.getMaxPlayer(); i++)
    {
        const auto &player = config.playerScores[i];
        String rankStr;
        uint16_t color = ST77XX_WHITE;

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
        if (player.name != config.currentPlayerName)
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
    tft.setCursor(17, tft.height() - 20);
    tft.print("Press START to replay");

    // Track current display state
    config.displayState = SystemConfig::SHOW_LEADERBOARD;
}

void DisplayOutputs::showNewSessionScreen()
{
    tft.fillScreen(ST77XX_BLACK);
    tft.setTextSize(2);
    tft.setTextColor(ST77XX_CYAN);
    tft.setCursor(15, 30);
    tft.print("SESSION?");

    tft.setTextSize(1);
    tft.setTextColor(ST77XX_WHITE);
    tft.setCursor(15, 60);
    tft.print("Would you like to try");
    tft.setCursor(15, 72);
    tft.print("another session?");

    tft.setTextColor(ST77XX_ORANGE);
    tft.setCursor(15, 100);
    tft.print("Press A.(YES) B.(NO)");

    config.displayState = SystemConfig::SHOW_NEW_SESSION;
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
    tft.setCursor(10, 20);
    tft.print("ERROR!");

    tft.setTextSize(1);
    tft.setTextColor(ST77XX_YELLOW);
    tft.setCursor(10, 60);
    tft.print("No Questions Found!");

    tft.setTextSize(1);
    tft.setTextColor(ST77XX_WHITE);
    tft.setCursor(10, 90);
    tft.print("Please add questions");
    tft.setCursor(10, 100);
    tft.print("and restart the game.");

    config.displayState = SystemConfig::SHOW_COMPLETE;
}

void DisplayOutputs::showNoCurrentSessionScreen()
{
    tft.fillScreen(ST77XX_BLACK);
    tft.setTextSize(2);
    tft.setTextColor(ST77XX_ORANGE);
    tft.setCursor(10, 20);
    tft.print("NO SESSION");

    tft.setTextSize(1);
    tft.setTextColor(ST77XX_WHITE);
    int yPos = 50;
    wrapText("Press START twice to enable wifi, allowing you to upload your own session.", 10, yPos, 21, tft.height() - 20, 12, ST77XX_WHITE);

    config.displayState = SystemConfig::SHOW_NO_CURRENT_SESSION;
}

void DisplayOutputs::showHintScreen(char removedOption)
{
    tft.fillScreen(ST77XX_BLACK);
    tft.setTextSize(2);
    tft.setTextColor(ST77XX_GREEN);
    tft.setCursor(10, 20);
    tft.print("Hint");

    tft.setTextSize(1);
    tft.setTextColor(ST77XX_WHITE);
    tft.setCursor(10, 50);
    tft.print("One wrong answer");
    tft.setCursor(10, 65);
    tft.print("has been removed!");

    tft.setTextColor(ST77XX_YELLOW);
    tft.setCursor(10, 100);
    tft.print("Press A/B/C/D");
}

void DisplayOutputs::showPassToPlayerScreen(const String &currentPlayer, const String &newPlayer)
{
    tft.fillScreen(ST77XX_BLACK);
    tft.setTextSize(2);
    tft.setTextColor(ST77XX_CYAN);
    tft.setCursor(10, 20);
    tft.print("Handed!");

    tft.setTextSize(1);
    tft.setTextColor(ST77XX_WHITE);
    tft.setCursor(10, 40);
    tft.printf("%s transferred to", currentPlayer.c_str());

    tft.setTextColor(ST77XX_YELLOW);
    tft.setTextSize(2);
    tft.setCursor(10, 70);
    tft.print(newPlayer);
}

void DisplayOutputs::showDeductPointsScreen(const String &playerName, int pointsDeducted)
{
    tft.fillScreen(ST77XX_BLACK);
    tft.setTextSize(2);
    tft.setTextColor(ST77XX_ORANGE);
    tft.setCursor(10, 20);
    tft.print("Oh bad...");

    tft.setTextSize(1);
    tft.setTextColor(ST77XX_WHITE);
    tft.setCursor(10, 50);
    tft.printf("%s loses", playerName.c_str());

    tft.setTextColor(ST77XX_RED);
    tft.setTextSize(2);
    tft.setCursor(10, 75);
    tft.printf("%d pts", pointsDeducted);
}

void DisplayOutputs::showIncrementQuestionScreen()
{
    tft.fillScreen(ST77XX_BLACK);
    tft.setTextSize(2);
    tft.setTextColor(ST77XX_RED);
    tft.setCursor(10, 20);
    tft.print("Misfortune");

    tft.setTextSize(1.5);
    tft.setTextColor(ST77XX_WHITE);
    tft.setCursor(10, 60);
    tft.print("Penalty Question");
    tft.setCursor(10, 80);
    tft.print("Added!");
}

void DisplayOutputs::showEliminationWarningScreen()
{
    tft.fillScreen(ST77XX_BLACK);
    tft.setTextSize(2);
    tft.setTextColor(ST77XX_YELLOW);
    tft.setCursor(10, 20);
    tft.print("WARNING!");

    tft.setTextSize(1);
    tft.setTextColor(ST77XX_RED);
    tft.setCursor(10, 60);
    tft.print("Someone will be");
    tft.setCursor(10, 75);
    tft.print("ELIMINATED!");
}

void DisplayOutputs::showListPossibleSaviorScreen()
{
    SystemConfig &config = SystemConfig::get();

    // Header
    tft.fillScreen(ST77XX_BLACK);
    tft.setCursor(5, 5);
    tft.setTextSize(1);
    tft.setTextColor(ST7735_ORANGE);
    tft.print("List of savior");

    int playerId = 1; // Start from 1 for display
    int yPos = 21;
    int maxDisplay = config.getMaxPlayer();

    for (int i = 0; i < maxDisplay; i++)
    {
        const auto &player = config.playerScores[i];
        //* Don't show the eliminated players.
        if (!player.isEliminated)
        {
            // Highlight if this is the selected index
            if (i == config.selectedSaviorIndex)
            {
                // Draw highlight background
                tft.fillRect(5, yPos - 2, 155, 14, ST77XX_RED);
                tft.fillRoundRect(5, yPos - 2, 155, 14, 3, ST77XX_RED);
                tft.setTextColor(ST77XX_WHITE);
                tft.setCursor(10, yPos);
                tft.print("> "); // Arrow indicator
            }
            else
            {
                tft.setTextColor(ST77XX_WHITE);
                tft.setCursor(10, yPos);
                tft.print("  "); // Spacing for alignment
            }

            tft.print(playerId);
            tft.print(". ");
            tft.print(player.name);

            playerId++;
            yPos += 18;
        }
    }

    tft.setTextColor(ST7735_ORANGE);
    tft.setCursor(9.5, 110);
    tft.print("A.(Sel) B.(Pick) C.(Skip)");
}

void DisplayOutputs::showNoneEliminatedScreen()
{
    tft.fillScreen(ST77XX_BLACK);
    tft.setTextSize(2);

    tft.setTextColor(ST77XX_GREEN);
    tft.setCursor(10, 10);
    tft.print("Congrats!");

    tft.setTextSize(1);
    tft.setTextColor(ST77XX_WHITE);
    tft.setCursor(10, 40);
    tft.print("No one has eliminated.");

    tft.setTextColor(ST77XX_ORANGE);
    tft.setCursor(10, 60);
    tft.print("Continuing the game");

    // Delay for 1500
    delay(1500);
}

void DisplayOutputs::showEliminatedPlayerScreen(const String &playerName)
{
    String message = "Failed to accomplished!";
    // Use a switch with the enum from SystemConfig
    switch (config.eliminationReason)
    {
    case SystemConfig::ELIM_DEDUCT_POINTS:
        message = "I lost all my points...";
        break;

    case SystemConfig::ELIM_PENALTY_QUESTION:
        message = "I failed to answer correctly.";
        break;

    case SystemConfig::ELIM_TASK_FAILED:
        message = "Failed to accomplish task!";
        break;
    case SystemConfig::ELIM_SACRIFICIAL_CONS:
        message = "I sacrificed!";
        break;
    case SystemConfig::ELIM_REFUSE_TO_REVIVE:
        message = "I'll hunt you all down.";
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

    tft.setTextColor(ST77XX_ORANGE);
    tft.setCursor(10, 70);
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

// WIFI

void DisplayOutputs::showCredentialsScreen()
{
    tft.fillScreen(ST77XX_BLACK);
    drawHeader("WiFi Configuration");

    tft.setTextSize(1);
    int yPos = 30;

    tft.setTextColor(ST77XX_CYAN);
    tft.setCursor(10, yPos);
    tft.print("SSID: ");
    tft.setTextColor(ST77XX_WHITE);
    tft.println(config.wifiSSID);

    yPos += 15;
    tft.setTextColor(ST77XX_CYAN);
    tft.setCursor(10, yPos);
    tft.print("PASS: ");
    tft.setTextColor(ST77XX_WHITE);
    tft.println(config.wifiPassword);

    yPos += 30;
    tft.setTextColor(ST77XX_YELLOW);
    tft.setCursor(10, yPos);
    tft.println("Connect to configure");
    tft.setCursor(10, yPos + 12);
    tft.println("via ReviewQuest App");

    config.displayState = SystemConfig::SHOW_CREDENTIALS;
}

void DisplayOutputs::showStartSessionScreen()
{
    tft.fillScreen(ST77XX_BLACK);
    drawHeader("Session Ready!");

    tft.setTextSize(1);
    int yPos = 30;

    tft.setTextColor(ST77XX_CYAN);
    tft.setCursor(10, yPos);
    tft.print("Title: ");
    tft.setTextColor(ST77XX_WHITE);
    tft.println(config.sessionTitle);

    yPos += 15;
    tft.setTextColor(ST77XX_CYAN);
    tft.setCursor(10, yPos);
    tft.printf("Questions: %d", (int)config.questionList.size());

    yPos += 15;
    tft.setTextColor(ST77XX_CYAN);
    tft.setCursor(10, yPos);
    tft.printf("Players: %d", (int)config.playerScores.size());

    yPos += 30;
    tft.setTextColor(ST77XX_GREEN);
    tft.setCursor(10, yPos);
    tft.println("Data uploaded!");
    tft.setCursor(10, yPos + 12);
    tft.println("Press START to begin");

    config.displayState = SystemConfig::SHOW_UPLOADED;
}