/**
 * @file display/display.h
 * @brief Display output class using SystemConfig singleton
 */
#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include "system/config.h"

// Forward declaration of TFT (defined in main)
extern Adafruit_ST7735 tft;

class DisplayOutputs
{
public:
    // ==========================================
    // SINGLETON INSTANCE
    // ==========================================
    static DisplayOutputs &get()
    {
        static DisplayOutputs instance;
        return instance;
    }

    // ==========================================
    // PUBLIC METHODS
    // ==========================================


    // ==========================================
    // PUBLIC METHODS
    // ==========================================

    /**
     * @brief Show start screen
     */
    void showStartScreen();

    /**
     * @brief Show question screen with choices
     * @param index Question index in questionList
     */
    void showQuestionScreen(int index);

    /**
     * @brief Show correct feedback
     */
    void showCorrectFeedback();

    /**
     * @brief Show incorrect feedback
     */
    void showIncorrectFeedback();

    /**
     * @brief Show leaderboard with player scores
     */
    void showLeaderboard();

    /**
     * @brief Show completion screen
     */
    void showCompletionScreen();

    /**
     * @brief Show hint feedback
     * @param removedOption The option letter that was removed (A, B, C, D)
     */
    void showHint(char removedOption);

    /**
     * @brief Show pass to another player
     * @param currentPlayer Current player name
     * @param newPlayer New player name
     */
    void showPassToPlayer(const String &currentPlayer, const String &newPlayer);

    /**
     * @brief Show deduct points feedback
     * @param playerName Player name
     * @param pointsDeducted Points deducted
     */
    void showDeductPoints(const String &playerName, int pointsDeducted);

    /**
     * @brief Show increment question feedback
     */
    void showIncrementQuestion();

    /**
     * @brief Show luck result
     * @param isLucky True if lucky
     * @param message Message to display
     */
    void showLuckResult(bool isLucky, const String &message);

    /**
     * @brief Show elimination warning
     */
    void showEliminationWarning();

    /**
     * @brief Show eliminated player
     * @param playerName The name of the eliminated player
     */
    void showEliminatedPlayer(const String &playerName);

    /**
     * @brief Show no questions screen
     */
    void showNoQuestionsScreen();

    /**
     * @brief Show a generic message
     * @param title The title text
     * @param message The message text
     * @param color The color of the title
     */
    void showMessage(const String &title, const String &message, uint16_t color);

    /**
     * @brief Clear the TFT display
     */
    void clearDisplay();

    /**
     * @brief Update the timer display
     * @param seconds Remaining seconds
     */
    void updateTimer(int seconds);

    /**
     * @brief Restart Game Screen
     */
    void showRestartGameScreen();

private:
    // ==========================================
    // PRIVATE METHODS
    // ==========================================

    /**
     * @brief Wrap text to fit on TFT
     */
    int wrapText(const String &text, int x, int &y, int maxChars = 21,
                 int maxY = 0, int lineHeight = 10, uint16_t textColor = ST77XX_WHITE);

    /**
     * @brief Draw a header on the screen
     */
    void drawHeader(const String &text);

    /**
     * @brief Draw a separator line
     */
    void drawSeparator(int y);

    /**
     * @brief Draw a bordered box
     */
    void drawBorderedBox(int x, int y, int w, int h, uint16_t color, uint16_t fillColor = -1);

    /**
     * @brief Get player's individual score
     */
    int getPlayerScore(const String &playerName);

    // ==========================================
    // MEMBER VARIABLES
    // ==========================================
    SystemConfig &config; // Reference to singleton
    bool initialized = false;

private:
    // Constructor and Destructor
    DisplayOutputs();
    ~DisplayOutputs();
};

#endif // DISPLAY_H