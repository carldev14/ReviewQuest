/**
 * @file core/display.h
 * @brief Display output class using SystemConfig singleton
 */
#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "system/config.h"
#include "Fonts/Free_Fonts.h"

// TFT instance lives in main.cpp
extern TFT_eSPI tft;

// ==========================================
// ROW STYLE — reusable list-row appearance
// ==========================================
enum class TextSize : uint8_t
{
    Small = 1, // size 1
    Medium = 2 // size 2
};

struct RowStyle
{
    uint16_t bg = 0x18E3;
    uint16_t border = TFT_WHITE;
    uint16_t prefixColor = TFT_GREEN;
    uint16_t textColor = TFT_WHITE;
    bool selected = false;
    uint16_t selectBg = TFT_RED;
    TextSize textSize = TextSize::Small;
};

class DisplayOutputs
{
public:
    // ==========================================
    // SINGLETON
    // ==========================================
    static DisplayOutputs &get()
    {
        static DisplayOutputs instance;
        return instance;
    }

    // ==========================================
    // PUBLIC SCREENS — gameplay
    // ==========================================
    void showSplashScreen();
    void showNoCurrentSessionScreen();
    void showStartSessionScreen();
    void showNewSessionScreen();
    void showQuestionScreen(int index);
    void showCorrectFeedbackScreen();
    void showIncorrectFeedbackScreen();
    void showCompletionScreen();
    void showLeaderboardScreen();
    void showRestartGameScreen();
    void showListPlayers();

    // ---- Events / feedback ----
    void showHintScreen(char removedOption);
    void showPassToPlayerScreen(const String &currentPlayer, const String &newPlayer);
    void showDeductPointsScreen(const String &playerName, int pointsDeducted);
    void showIncrementQuestionScreen();
    void showEliminationWarningScreen();
    void showEliminatedPlayerScreen(const String &playerName);
    void showNoneEliminatedScreen();
    void showNoQuestionsScreen();
    void showListPossibleSaviorScreen();
    void showWouldYouSaveThePlayerScreen();

    // ---- Utility ----
    void showMessage(const String &title, const String &message, uint16_t color);
    void clearDisplay();
    void updateTimer(int seconds);

    // ==========================================
    // REUSABLE COMPONENTS
    // ==========================================

    // List row — default height or custom
    void drawListRow(int y, const String &prefix, const String &text,
                     int rowH = 26,
                     const RowStyle &style = RowStyle());

    // Leaderboard medal helpers
    void drawMedalIcon(int cx, int cy, uint16_t medalColor);
    uint16_t rankColorFor(int rankIndex);
    String rankLabelFor(int rankIndex);

private:
    // ==========================================
    // PRIVATE METHODS
    // ==========================================

    // ---- Text layout ----
    int wrapText(const String &text, int x, int &y, int maxWidth,
                 int maxY = 0, int lineHeight = 10,
                 uint16_t textColor = TFT_WHITE, uint8_t size = 2);

    int wrapTextFF(const String &text, int x, int &y,
                   int maxWidth, int maxY, int lineHeight,
                   uint16_t textColor, const GFXfont *font);

    // ---- Drawing primitives ----
    void drawHeader(const String &text);
    void drawSeparator(int y);
    void drawBorderedBox(int x, int y, int w, int h,
                         uint16_t color, uint16_t fillColor = -1);
    void drawSaviorButtonBar();

    // ---- Lookup ----
    int getPlayerScore(const String &playerName);

    // ==========================================
    // MEMBER VARIABLES
    // ==========================================
    SystemConfig &config;
    bool initialized = false;

    DisplayOutputs();
    ~DisplayOutputs();
};

#endif // DISPLAY_H