/**
 * @file game/helper.h
 * @brief Helper class for game utilities
 */
#ifndef HELPER_H
#define HELPER_H

#include <Arduino.h>
#include <vector>
#include <random>
#include <algorithm>
#include "system/config.h"

class Helper
{
public:
    // ==========================================
    // SINGLETON
    // ==========================================
    static Helper &get()
    {
        static Helper instance;
        return instance;
    }

    // ==========================================
    // PLAYER MANAGEMENT
    // ==========================================
    void initPlayerScores();
    void resetPlayerScores();
    void printPlayerScores();
    void printAllPlayers();

    bool updatePlayerScore(const String &playerName = "", int points = 1);
    int  getPlayerScore(const String &playerName);
    int  getActivePlayerCount();

    // ==========================================
    // SORTING
    // ==========================================
    void sortPlayersByScore();   // highest score first
    void sortPlayers();          // active players first

    // ==========================================
    // SHUFFLING
    // ==========================================
    void shuffleQuestions();
    void shuffleQuestionOptions();
    void shufflePlayers();

    // ==========================================
    // TURN ORDER
    // ==========================================
    String getNextPlayer();
    bool   isPlayerOrderComplete();

private:
    // ==========================================
    // HELPERS
    // ==========================================
    int  findPlayerIndex(const String &playerName);
    bool isValidPlayerName(const String &playerName);

    // ==========================================
    // MEMBERS
    // ==========================================
    std::vector<int> playerOrder;
    int  currentPlayerPos = 0;
    bool initialized      = false;

    Helper();
    ~Helper();

    Helper(const Helper &)            = delete;
    Helper &operator=(const Helper &) = delete;
};

#endif // HELPER_H