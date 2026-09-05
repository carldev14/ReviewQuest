/**
 * @file game_mechanics.h
 * @brief Game mechanics class for luck system, penalties, and elimination
 */
#ifndef GAME_MECHANICS_H
#define GAME_MECHANICS_H

#include <Arduino.h>
#include <vector>
#include <random>
#include "system/config.h"
#include "display.h"
#include "actuators.h"
#include "helper.h"

// Forward declaration of TFT (defined in main)
extern Adafruit_ST7735 tft;

class GameMechanics
{
public:
    // ==========================================
    // SINGLETON INSTANCE
    // ==========================================
    static GameMechanics &get()
    {
        static GameMechanics instance;
        return instance;
    }

    // ==========================================
    // PUBLIC METHODS
    // ==========================================

    /**
     * @brief Run the penalty system (50/50 luck)
     * Called after a correct answer
     */
    void runPenalty();

    /**
     * @brief Run the elimination system
     * Called when a player reaches 0 or negative points
     */
    void runElimination();

    /**
     * @brief Initialize the game mechanics with default values
     */
    void initialize();

    /**
     * @brief Handle penalty question answer
     * @param correct True if answered correctly
     */
    void handlePenaltyAnswer(bool correct);

    /**
     * @brief Check if current question is a penalty question
     * @return bool True if current question is a penalty
     */
    bool isPenaltyQuestion();

    /**
     * @brief Get the current penalty count
     * @return int Number of consecutive wrong penalty answers
     */
    int getPenaltyCount() const { return penaltyCount; }

    /**
     * @brief Reset the penalty count
     */
    void resetPenaltyCount() { penaltyCount = 0; }

    /**
     * @brief Get the max penalty count before elimination
     * @return int Maximum penalty count
     */
    int getMaxPenaltyCount() const { return MAX_PENALTY_COUNT; }

    /**
     * @brief Handle answer selection (main game loop)
     * @param option The selected answer (A, B, C, D)
     */
    void handleAnswer(char option);

    /**
     * @brief Handle reset hint when the game restarted.
     * @param int question index
     */
    void resetAllHints();

    // Flag to track if penalty has been applied
    bool isPenaltyAccomplished = false;

private:
    // ==========================================
    // PRIVATE CONSTRUCTOR (Singleton)
    // ==========================================
    GameMechanics();
    ~GameMechanics();

    // Delete copy
    GameMechanics(const GameMechanics &) = delete;
    GameMechanics &operator=(const GameMechanics &) = delete;

    // ==========================================
    // LUCK SYSTEM (GOOD)
    // ==========================================

    /**
     * @brief Give a hint to the current player
     * Removes one wrong answer from the current question
     */
    void giveHint();

    /**
     * @brief Pass the current question to another random player
     */
    void passToAnotherPlayer();

    // ==========================================
    // BAD LUCK SYSTEM
    // ==========================================

    /**
     * @brief Deduct points from the current player
     */
    void deductPoints(int pointsToDeduct = 1);

    /**
     * @brief Increment question difficulty (add a penalty question)
     * Picks a random question from the existing list and adds it as a penalty
     */
    void incrementQuestion();

    // ==========================================
    // PRIVATE HELPERS
    // ==========================================

    /**
     * @brief Get a random other player (not the current one)
     * @return String Name of another player
     */
    String getRandomOtherPlayer();

    /**
     * @brief Check if elimination is valid (at least 2 players remain)
     * @return bool True if at least 2 players remain
     */
    bool canEliminate();

    /**
     * @brief Eliminate the current player
     */
    void eliminateCurrentPlayer();

    /**
     * @brief Show luck result on TFT
     * @param isLucky True if lucky, false if bad luck
     * @param message Message to display
     */
    void showLuckResult(bool isLucky, String message);

    /**
     * @brief Get a random question from the list
     * @return Question A random question from the list
     */
    SystemConfig::Question getRandomQuestion();

    // ==========================================
    // MEMBER VARIABLES
    // ==========================================
    SystemConfig &config;    // Reference to singleton
    DisplayOutputs &display; // Reference to singleton
    Actuators &actuators;    // Reference to singleton
    Helper &helper;          // Reference to singleton
    bool initialized = false;

    // Penalty tracking
    int penaltyCount = 0;                 // Track consecutive wrong answers on penalty questions
    const int MAX_PENALTY_COUNT = 2;      // Eliminate after 2 wrong answers
    bool isPenaltyQuestionActive = false; // Flag to track if current question is a penalty
};

#endif // GAME_MECHANICS_H