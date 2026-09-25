/**
 * @file game/game_logic.h
 * @brief Game logic class for quiz game flow control
 */
#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

#include <Arduino.h>
#include "system/config.h"
#include "system/session.h"
#include "core/display.h"
#include "core/actuators.h"
#include "game/helper.h"
#include "game/game_mechanics.h"
#include "core/inputs.h"

class GameLogics
{
public:
    // ==========================================
    // SINGLETON
    // ==========================================
    static GameLogics &get()
    {
        static GameLogics instance;
        return instance;
    }

    // ==========================================
    // LIFECYCLE
    // ==========================================
    void initialize();
    void startQuiz();
    void advanceToNextQuestion();
    void retryQuestion();
    void tryNewSession();
    void clearSession();
    void restartGame();

    // ==========================================
    // RUNTIME
    // ==========================================
    void handleAnswer(char option);
    void handleFeedbackTimer();

    // ==========================================
    // DIAGNOSTICS
    // ==========================================
    void printSystemStatus();
    void checkMemory();

    // ==========================================
    // GETTERS
    // ==========================================
    int    getCurrentQuestionPos() const { return config.currentQuestionPos; }
    int    getOverallScore()       const { return config.overallScore; }
    String getCurrentPlayer()      const { return config.currentPlayerName; }

private:
    // ==========================================
    // INTERNAL HELPERS
    // ==========================================
    bool canStartQuiz();
    void resetGameState();
    void resetToStartScreen();
    void showError(const String &title, const String &message);

    // ==========================================
    // SINGLETONS
    // ==========================================
    SystemConfig  &config;
    DisplayOutputs &display;
    Actuators     &actuators;
    Helper        &helper;
    Inputs        &inputs;
    Session       &session;
    GameMechanics &gameMechanics;

    bool initialized = false;

    GameLogics();
    ~GameLogics();

    GameLogics(const GameLogics &)            = delete;
    GameLogics &operator=(const GameLogics &) = delete;
};

#endif // GAME_LOGIC_H