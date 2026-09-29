/**
 * @file game/game_mechanics.h
 * @brief Luck system, penalties, and elimination
 */
#ifndef GAME_MECHANICS_H
#define GAME_MECHANICS_H

#include <Arduino.h>
#include <vector>
#include <random>
#include "system/config.h"
#include "core/display.h"
#include "core/inputs.h"
#include "core/actuators.h"
#include "game/helper.h"

class GameMechanics
{
public:
    // ==========================================
    // SINGLETON
    // ==========================================
    static GameMechanics &get()
    {
        static GameMechanics instance;
        return instance;
    }

    // ==========================================
    // LIFECYCLE
    // ==========================================
    void initialize();
    void handleAnswer(char option);
    void recordBetrayal(const String &perpetrator, const String &victim);
    
    // ==========================================
    // ANSWER RESULTS
    // ==========================================
    void handleCorrectAnswer();                     // ← ADDED
    void handleIncorrectAnswer();                   // ← ADDED
    void handlePenaltyQuestionResult(bool correct); // ← ADDED (see note below)

    // ==========================================
    // LUCK SYSTEM
    // ==========================================
    void runPenalty();
    void runGoodLuck();                     // ← ADDED
    void runBadLuck();                      // ← ADDED
    void handlePenaltyAnswer(bool correct); // ← kept (legacy alias?)
    void resetAllHints();

    // ==========================================
    // PENALTY STATE
    // ==========================================
    bool isPenaltyQuestion(); // existing accessor
    int getPenaltyCount() const { return penaltyCount; }
    int getMaxPenaltyCount() const { return MAX_PENALTY_COUNT; }
    void resetPenaltyCount() { penaltyCount = 0; }

    // ==========================================
    // Revival
    // ==========================================
    bool isValidSavior(const String &name);

    // ==========================================
    // END-GAME CHECK
    // ==========================================
    void endGameOnePlayer();

    // ==========================================
    // FLAGS
    // ==========================================
    bool isPenaltyAccomplished = false;

private:
    // ==========================================
    // LUCK — GOOD
    // ==========================================
    void giveHint();
    void passToAnotherPlayer();

    // ==========================================
    // LUCK — BAD
    // ==========================================
    void deductPoints(int pointsToDeduct = 1);
    void incrementQuestion();

    // ==========================================
    // ELIMINATION
    // ==========================================
    bool canEliminate();
    void eliminatePlayer();
    String reviveByPeerEliminate();

    // ==========================================
    // HELPERS
    // ==========================================
    String getRandomOtherPlayer();
    SystemConfig::Question getRandomQuestion();
    void showLuckResult(bool isLucky, String message);

    // ==========================================
    // MEMBERS
    // ==========================================
    SystemConfig &config;
    Inputs &inputs;
    DisplayOutputs &display;
    Actuators &actuators;
    Helper &helper;

    bool initialized = false;

    int penaltyCount = 0;
    const int MAX_PENALTY_COUNT = 2;
    bool isPenaltyQuestionActive = false;

    GameMechanics();
    ~GameMechanics();

    GameMechanics(const GameMechanics &) = delete;
    GameMechanics &operator=(const GameMechanics &) = delete;
};

#endif // GAME_MECHANICS_H