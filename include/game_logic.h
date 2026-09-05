/**
 * @file game_logic.h
 * @brief Game logic class for quiz game flow control
 */
#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

#include <Arduino.h>
#include "system/config.h"
#include "display.h"
#include "actuators.h"
#include "helper.h"
#include "game_mechanics.h"

class GameLogic
{
public:
    // ==========================================
    // SINGLETON INSTANCE
    // ==========================================
    static GameLogic &get()
    {
        static GameLogic instance;
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
     * @brief Restart the game
     */
    void restartGame();

    /**
     * @brief Handle answer from player
     * @param option The selected answer (A, B, C, D)
     */
    void handleAnswer(char option);

    /**
     * @brief Print system status for debugging
     */
    void printSystemStatus();

    /**
     * @brief Check memory usage
     */
    void checkMemory();

    /**
     * @brief Check and run if there is only one player left in the session, therefore, end it.
     */
    void endGameOnePlayer();

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
    GameLogic();
    ~GameLogic();

    // Delete copy
    GameLogic(const GameLogic &) = delete;
    GameLogic &operator=(const GameLogic &) = delete;

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
     * @brief Show error message on screen
     * @param title Error title
     * @param message Error message
     */
    void showError(const String &title, const String &message);

    // ==========================================
    // MEMBER VARIABLES
    // ==========================================
    SystemConfig &config;
    DisplayOutputs &display;
    Actuators &actuators;
    Helper &helper;
    GameMechanics &gameMechanics;
    bool initialized = false;
};

#endif // GAME_LOGIC_H