/**
 * @file helper.h
 * @brief Helper class for game utilities
 */
#ifndef HELPER_H
#define HELPER_H

#include <Arduino.h>
#include <vector>
#include <random>
#include <algorithm>  // Added for std::sort
#include "system/config.h"

class Helper
{
public:
    static Helper &get()
    {
        static Helper instance;
        return instance;
    }

    /**
     * @brief Shuffle the order of questions
     */
    void shuffleQuestions();

    /**
     * @brief Shuffle the order of question options (A, B, C, D)
     */
    void shuffleQuestionOptions();

    /**
     * @brief Shuffle the order of players
     */
    void shufflePlayers();

    /**
     * @brief Get the next player in the shuffled order
     * @return String Name of the next player
     */
    String getNextPlayer();

    /**
     * @brief Check if all players have had their turn
     * @return bool True if all players have completed their turn
     */
    bool isPlayerOrderComplete();

    /**
     * @brief Reset all player scores to 0 and set eliminated to false
     * Preserves player names
     */
    void resetPlayerScores();

    /**
     * @brief Update a player's score
     * @param playerName Name of the player to update
     * @param points Points to add (can be negative)
     * @return bool True if successful, false if player not found
     */
    bool updatePlayerScore(const String &playerName = "", int points = 1);

    /**
     * @brief Get a player's current score
     * @param playerName Name of the player
     * @return int Player's score, or -1 if not found
     */
    int getPlayerScore(const String &playerName);

    /**
     * @brief Print all player scores to Serial for debugging
     */
    void printPlayerScores();

    /**
     * @brief Initialize player scores with default names
     */
    void initPlayerScores();

    /**
     * @brief Sort players by score (descending) - highest first
     */
    void sortPlayersByScore();

    /**
     * @brief Print all players with their status to Serial
     */
    void printAllPlayers();

    /**
     * @brief Get the count of active (non-eliminated) players
     * @return int Number of active players
     */
    int getActivePlayerCount();

    /**
     * @brief Sort players so that non-eliminated players come first
     * Active players (isEliminated = false) are placed at the beginning
     * Eliminated players (isEliminated = true) are placed at the end
     * If same status, sort alphabetically by name
     */
    void sortPlayers();

private:
    Helper();
    ~Helper();
    Helper(const Helper &) = delete;
    Helper &operator=(const Helper &) = delete;

    /**
     * @brief Find a player by name and return their index
     * @param playerName Name of the player to find
     * @return int Index of the player, or -1 if not found
     */
    int findPlayerIndex(const String &playerName);

    /**
     * @brief Check if a player name is valid (exists in playerScores)
     * @param playerName Name to validate
     * @return bool True if the player exists
     */
    bool isValidPlayerName(const String &playerName);

    std::vector<int> playerOrder;
    int currentPlayerPos = 0;
    bool initialized = false;
};

#endif // HELPER_H