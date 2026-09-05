/**
 * @file helper.cpp
 * @brief Helper class implementation for game utilities
 */
#include "helper.h"

// ==========================================
// CONSTRUCTOR & DESTRUCTOR
// ==========================================

Helper::Helper()
{
    Serial.println("🔧 Helper initialized!");
    initialized = true;
}

Helper::~Helper()
{
    playerOrder.clear();
    Serial.println("Helper destroyed!");
}

// ==========================================
// PLAYER MANAGEMENT
// ==========================================

void Helper::shuffleQuestions()
{
    SystemConfig &config = SystemConfig::get();

    config.questionOrder.clear();
    for (size_t i = 0; i < config.questionList.size(); i++)
    {
        config.questionOrder.push_back(static_cast<int>(i));
    }

    for (int i = (int)config.questionOrder.size() - 1; i > 0; i--)
    {
        int j = random(0, i + 1);
        int temp = config.questionOrder[i];
        config.questionOrder[i] = config.questionOrder[j];
        config.questionOrder[j] = temp;
    }

    Serial.println("📝 Questions shuffled! Order:");
    for (size_t i = 0; i < config.questionOrder.size(); i++)
    {
        Serial.printf("  %d: Q%d\n", static_cast<int>(i + 1), config.questionOrder[i] + 1);
    }
}

void Helper::shuffleQuestionOptions()
{
    SystemConfig &config = SystemConfig::get();

    Serial.println("🔄 SHUFFLING QUESTION OPTIONS");

    // ===== SHUFFLE QUESTIONS ORDER =====
    config.questionOrder.clear();
    for (size_t i = 0; i < config.questionList.size(); i++)
    {
        config.questionOrder.push_back(static_cast<int>(i));
    }

    for (int i = (int)config.questionOrder.size() - 1; i > 0; i--)
    {
        int j = random(0, i + 1);
        int temp = config.questionOrder[i];
        config.questionOrder[i] = config.questionOrder[j];
        config.questionOrder[j] = temp;
    }

    Serial.println("📝 Questions shuffled! Order:");
    for (size_t i = 0; i < config.questionOrder.size(); i++)
    {
        Serial.printf("  %d: Q%d\n", static_cast<int>(i + 1), config.questionOrder[i] + 1);
    }

    // ===== SHUFFLE OPTIONS WITHIN EACH QUESTION =====
    Serial.println("🔄 Shuffling options within each question...");

    for (auto &q : config.questionList)
    {
        // ===== SAVE ORIGINAL OPTIONS FIRST =====
        q.originalOptionA = q.optionA;
        q.originalOptionB = q.optionB;
        q.originalOptionC = q.optionC;
        q.originalOptionD = q.optionD;
        
        // Reset hint flag
        q.hintUsed = false;

        // Store original options in an array
        String options[4] = {q.optionA, q.optionB, q.optionC, q.optionD};
        char labels[4] = {'A', 'B', 'C', 'D'};

        // Shuffle the options array
        for (int i = 3; i > 0; i--)
        {
            int j = random(0, i + 1);
            String temp = options[i];
            options[i] = options[j];
            options[j] = temp;

            char tempLabel = labels[i];
            labels[i] = labels[j];
            labels[j] = tempLabel;
        }

        // Find where the correct answer moved to
        char newCorrect = 'A';
        for (int i = 0; i < 4; i++)
        {
            if (labels[i] == q.initialCharAns)
            {
                newCorrect = 'A' + i;
                break;
            }
        }

        // Assign shuffled options back
        q.optionA = options[0];
        q.optionB = options[1];
        q.optionC = options[2];
        q.optionD = options[3];
        q.initialCharAns = newCorrect;

        Serial.printf("   Q%d shuffled: A)%s B)%s C)%s D)%s (Correct: %c)\n",
                      q.id,
                      q.optionA.c_str(),
                      q.optionB.c_str(),
                      q.optionC.c_str(),
                      q.optionD.c_str(),
                      q.initialCharAns);
    }

    Serial.println("✅ Questions and options shuffled!");
}

void Helper::shufflePlayers()
{
    SystemConfig &config = SystemConfig::get();

    // ===== SAFETY: Check if playerScores is valid =====
    if (config.playerScores.empty())
    {
        Serial.println("❌ ERROR: config.playerScores is EMPTY!");
        playerOrder.clear();
        currentPlayerPos = 0;
        return;
    }

    playerOrder.clear();

    // Filter for non-eliminated players only
    for (size_t i = 0; i < config.playerScores.size(); i++)
    {
        if (!config.playerScores[i].isEliminated)
        {
            playerOrder.push_back(static_cast<int>(i));
        }
    }

    if (playerOrder.empty())
    {
        Serial.println("⚠️ Warning: No active players available to shuffle!");
        currentPlayerPos = 0;
        return;
    }

    for (int i = (int)playerOrder.size() - 1; i > 0; i--)
    {
        int j = random(0, i + 1);
        int temp = playerOrder[i];
        playerOrder[i] = playerOrder[j];
        playerOrder[j] = temp;
    }

    currentPlayerPos = 0;

    Serial.printf("👥 Shuffled %d active players\n", (int)playerOrder.size());
}

String Helper::getNextPlayer()
{
    SystemConfig &config = SystemConfig::get();

    if (playerOrder.empty())
    {
        Serial.println("⚠️ Player order empty, shuffling...");
        shufflePlayers();
    }

    const int totalPlayers = static_cast<int>(playerOrder.size());
    if (totalPlayers == 0)
    {
        Serial.println("❌ ERROR: No active players remaining in queue!");
        return "";
    }

    int attempts = 0;

    while (attempts < totalPlayers)
    {
        // Wrap position when reaching end of queue
        if (currentPlayerPos >= static_cast<int>(playerOrder.size()))
        {
            Serial.println("🔄 Reached end of queue, wrapping around...");
            currentPlayerPos = 0;
        }

        int playerIndex = playerOrder[currentPlayerPos];

        // Bounds validation
        if (playerIndex >= 0 && playerIndex < static_cast<int>(config.playerScores.size()))
        {
            const auto &player = config.playerScores[playerIndex];

            Serial.printf("👤 [%d/%d] Name: %s | Score: %d | Status: %s\n",
                          attempts + 1,
                          totalPlayers,
                          player.name.c_str(),
                          player.score,
                          player.isEliminated ? "Eliminated ❌" : "Active ✅");

            if (!player.isEliminated)
            {
                String selectedPlayer = player.name;
                currentPlayerPos = (currentPlayerPos + 1) % totalPlayers;

                Serial.printf("🎯 Next active player: %s (pos: %d/%d)\n",
                              selectedPlayer.c_str(),
                              currentPlayerPos,
                              totalPlayers);

                return selectedPlayer;
            }

            Serial.printf("⏭️ Skipping eliminated player: %s\n", player.name.c_str());
        }

        // Always advance position and attempt counter outside conditional guards
        currentPlayerPos = (currentPlayerPos + 1) % totalPlayers;
        attempts++;
    }

    Serial.println("💀 All players are marked as eliminated!");
    return "";
}

bool Helper::isPlayerOrderComplete()
{
    return currentPlayerPos >= static_cast<int>(playerOrder.size());
}

void Helper::resetPlayerScores()
{
    SystemConfig &config = SystemConfig::get();

    for (auto &player : config.playerScores)
    {
        player.score = 0;
        player.isEliminated = false;
    }
    Serial.println("🔄 All player scores reset!");
}

bool Helper::updatePlayerScore(const String &playerName, int points)
{
    SystemConfig &config = SystemConfig::get();

    Serial.println("========================================");
    Serial.println("📊 updatePlayerScore CALLED");
    Serial.println("========================================");

    // ===== SAFETY CHECK: Check if playerScores is empty =====
    if (config.playerScores.empty())
    {
        Serial.println("⚠️ CRITICAL: playerScores is EMPTY!");
        Serial.println("🔄 Re-initializing player scores...");
        initPlayerScores();

        if (config.playerScores.empty())
        {
            Serial.println("❌ FATAL: Failed to initialize player scores!");
            return false;
        }
        Serial.println("✅ Player scores re-initialized successfully!");
    }

    // ===== DEBUG: Print input parameters =====
    Serial.printf("📥 playerName parameter: '%s' (len: %d)\n",
                  playerName.c_str(),
                  playerName.length());
    Serial.printf("📥 points parameter: %d\n", points);
    Serial.printf("📥 playerName.isEmpty(): %s\n", playerName.isEmpty() ? "TRUE" : "FALSE");

    // ===== DEBUG: Print current state =====
    Serial.printf("📊 config.currentPlayerName: '%s' (len: %d)\n",
                  config.currentPlayerName.c_str(),
                  config.currentPlayerName.length());
    Serial.printf("📊 config.playerScores size: %d\n", (int)config.playerScores.size());

    String targetName = playerName;

    // If targetName is empty, fallback to current player
    if (targetName.isEmpty())
    {
        targetName = config.currentPlayerName;
        Serial.printf("Using current player: %s\n", targetName.c_str());
    }

    if (targetName.isEmpty())
    {
        Serial.println("❌ ERROR: Player name is empty!");
        return false;
    }

    int index = findPlayerIndex(targetName);
    if (index == -1)
    {
        Serial.printf("❌ Player '%s' not found!\n", targetName.c_str());
        return false;
    }

    // Update score if active
    if (!config.playerScores[index].isEliminated)
    {
        int oldScore = config.playerScores[index].score;
        config.playerScores[index].score += points;

        if (config.playerScores[index].score < 0)
        {
            config.playerScores[index].score = 0;
        }

        Serial.printf("✅ Updated %s score: %d -> %d (added %d)\n",
                      targetName.c_str(),
                      oldScore,
                      config.playerScores[index].score,
                      points);

        return true;
    }
    else
    {
        Serial.printf("⚠️ Cannot update score for %s: Player is eliminated!\n",
                      config.playerScores[index].name.c_str());
        return false;
    }
}

int Helper::getPlayerScore(const String &playerName)
{
    SystemConfig &config = SystemConfig::get();

    String targetName = playerName.isEmpty() ? config.currentPlayerName : playerName;

    if (targetName.isEmpty())
    {
        return -1;
    }

    int index = findPlayerIndex(targetName);
    return (index != -1) ? config.playerScores[index].score : -1;
}

void Helper::printPlayerScores()
{
    SystemConfig &config = SystemConfig::get();

    Serial.println("=== 📊 PLAYER SCORES ===");
    if (config.playerScores.empty())
    {
        Serial.println("No players found!");
        return;
    }

    for (const auto &player : config.playerScores)
    {
        Serial.printf("%s: %d pts %s\n",
                      player.name.c_str(),
                      player.score,
                      player.isEliminated ? "(ELIM)" : "");
    }
    Serial.println("=====================");
}

void Helper::printAllPlayers()
{
    SystemConfig &config = SystemConfig::get();

    Serial.println("=== 📊 ALL PLAYERS ===");
    if (config.playerScores.empty())
    {
        Serial.println("No players found!");
        return;
    }

    for (int i = 0; i < (int)config.playerScores.size(); i++)
    {
        Serial.printf("[%d] %s | Score: %d | Eliminated: %s\n",
                      i,
                      config.playerScores[i].name.c_str(),
                      config.playerScores[i].score,
                      config.playerScores[i].isEliminated ? "YES" : "NO");
    }
    Serial.println("=====================");
}

void Helper::initPlayerScores()
{
    SystemConfig &config = SystemConfig::get();

    Serial.println("⚠️ Helper::initPlayerScores() - Clearing playerScores!");
    config.playerScores.clear();

    const char *defaultNames[] = {"Alice", "Bob", "Charlie", "Diana", "Eve"};
    int numDefaultPlayers = sizeof(defaultNames) / sizeof(defaultNames[0]);

    for (int i = 0; i < numDefaultPlayers; i++)
    {
        SystemConfig::PlayerScore ps;
        ps.name = String(defaultNames[i]);
        ps.score = 0;
        ps.isEliminated = false;
        config.playerScores.push_back(ps);
    }

    Serial.printf("✅ Initialized %d player scores\n", static_cast<int>(config.playerScores.size()));
    printPlayerScores();
}

// ==========================================
// PRIVATE METHODS
// ==========================================

int Helper::findPlayerIndex(const String &playerName)
{
    SystemConfig &config = SystemConfig::get();

    // ===== SAFETY CHECK =====
    if (config.playerScores.empty())
    {
        Serial.println("❌ findPlayerIndex: playerScores is EMPTY!");
        return -1;
    }

    if (playerName.isEmpty())
    {
        return -1;
    }

    String trimmedName = playerName;
    trimmedName.trim();

    Serial.printf("🔍 Searching for player: '%s' (len: %d)\n", trimmedName.c_str(), trimmedName.length());
    Serial.printf("📊 Player list size: %d\n", (int)config.playerScores.size());

    for (size_t i = 0; i < config.playerScores.size(); i++)
    {
        const auto &p = config.playerScores[i];
        String storedName = p.name;
        storedName.trim();

        Serial.printf("   Index %d | Name: '%s' (len: %d) | Score: %d | Elim: %s\n",
                      (int)i,
                      storedName.c_str(),
                      storedName.length(),
                      p.score,
                      p.isEliminated ? "YES" : "NO");

        if (storedName == trimmedName)
        {
            Serial.printf("✅ Found player at index %d\n", (int)i);
            return static_cast<int>(i);
        }
    }
    Serial.printf("❌ Player '%s' not found in list!\n", trimmedName.c_str());
    return -1;
}

bool Helper::isValidPlayerName(const String &playerName)
{
    return findPlayerIndex(playerName) != -1;
}

void Helper::sortPlayersByScore()
{
    SystemConfig &config = SystemConfig::get();

    int totalPlayers = static_cast<int>(config.playerScores.size());

    for (int i = 0; i < totalPlayers - 1; i++)
    {
        for (int j = i + 1; j < totalPlayers; j++)
        {
            bool shouldSwap = false;

            // Priority 1: Non-eliminated players rank above eliminated players
            if (config.playerScores[i].isEliminated != config.playerScores[j].isEliminated)
            {
                if (config.playerScores[i].isEliminated && !config.playerScores[j].isEliminated)
                {
                    shouldSwap = true;
                }
            }
            // Priority 2: Sort descending by score within the same state
            else
            {
                if (config.playerScores[i].score < config.playerScores[j].score)
                {
                    shouldSwap = true;
                }
            }

            if (shouldSwap)
            {
                SystemConfig::PlayerScore temp = config.playerScores[i];
                config.playerScores[i] = config.playerScores[j];
                config.playerScores[j] = temp;
            }
        }
    }
}

int Helper::getActivePlayerCount()
{
    SystemConfig &config = SystemConfig::get();

    int count = 0;
    for (const auto &player : config.playerScores)
    {
        if (!player.isEliminated)
        {
            count++;
        }
    }
    return count;
}