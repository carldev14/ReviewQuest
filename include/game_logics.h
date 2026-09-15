/**
 * @file game_logic.h
 * @brief Game logic class for quiz game flow control
 */
#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

#include <Arduino.h>
#include "system/config.h"
#include "session.h"
#include "display.h"
#include "actuators.h"
#include "helper.h"
#include "game_mechanics.h"
#include "inputs.h"

class GameLogics
{
public:
    // ==========================================
    // SINGLETON INSTANCE
    // ==========================================
    static GameLogics &get()
    {
        static GameLogics instance;
        return instance;
    }

    // ==========================================
    // PUBLIC METHODS
    // ==========================================

    /**
     * @brief Initialize the game logic
     */
    void initialize();

    /**
     * @brief Start the quiz
     */
    void startQuiz();

    /**
     * @brief Advance to the next question
     */
    void advanceToNextQuestion();

    /**
     * @brief Retry the current question
     */
    void retryQuestion();

    /**
     * @brief Handle feedback timer
     */
    void handleFeedbackTimer();

    /**
     * @brief Try new session?
     */
    void tryNewSession();

    /**
     * @brief Handle answer from player
     * @param option The selected answer (A, B, C, D)
     */
    void handleAnswer(char option);

    /**
     * @brief Clear the current session (questions, players) and return to start screen
     */
    void clearSession();

    /**
     * @brief Print system status for debugging
     */
    void printSystemStatus();

    /**
     * @brief Check memory usage
     */
    void checkMemory();

    /**
     * @brief Get current game state
     */
    int getCurrentQuestionPos() const { return config.currentQuestionPos; }
    int getOverallScore() const { return config.overallScore; }
    String getCurrentPlayer() const { return config.currentPlayerName; }

private:
    // ==========================================
    // PRIVATE CONSTRUCTOR (Singleton)
    // ==========================================
    GameLogics();
    ~GameLogics();

    // Delete copy
    GameLogics(const GameLogics &) = delete;
    GameLogics &operator=(const GameLogics &) = delete;

    // ==========================================
    // PRIVATE METHODS
    // ==========================================

    /**
     * @brief Check if game can start
     * @return bool True if game can start
     */
    bool canStartQuiz();

    /**
     * @brief Reset game state
     */
    void resetGameState();
    void resetToStartScreen();

    /**
     * @brief Restart the game
     */
    void restartGame();

    /**
     * @brief Show error message on screen
     * @param title Error title
     * @param message Error message
     */
    void showError(const String &title, const String &message);

    // ==========================================
    // MEMBER VARIABLES: SINGLETONS (INITIALIZE ONCE)
    // ==========================================
    SystemConfig &config;
    DisplayOutputs &display;
    Actuators &actuators;
    Helper &helper;
    Inputs &inputs;
    Session &session;
    GameMechanics &gameMechanics;
    bool initialized = false;
};

#endif // GAME_LOGIC_H