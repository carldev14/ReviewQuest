/**
 * @file helper.h
 * @brief Helper class for game utilities
 */
#ifndef HELPER_H
#define HELPER_H

#include <Arduino.h>
#include <vector>
#include <random>
#include "system/config.h"

class Helper
{
public:
    static Helper &get()
    {
        static Helper instance;
        return instance;
    }

    void shuffleQuestions();
    void shuffleQuestionOptions();
    void shufflePlayers();
    String getNextPlayer();
    bool isPlayerOrderComplete();
    void resetPlayerScores();
    bool updatePlayerScore(const String &playerName = "", int points = 1);
    int getPlayerScore(const String &playerName);
    void printPlayerScores();
    void initPlayerScores();
    void sortPlayersByScore();
    void printAllPlayers();
    int getActivePlayerCount();

private:
    Helper();
    ~Helper();
    Helper(const Helper &) = delete;
    Helper &operator=(const Helper &) = delete;

    int findPlayerIndex(const String &playerName);
    bool isValidPlayerName(const String &playerName);

    std::vector<int> playerOrder;
    int currentPlayerPos = 0;
    bool initialized = false;
};

#endif // HELPER_H