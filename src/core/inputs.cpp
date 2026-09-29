/**
 * @file core/inputs.cpp
 * @brief Input handling — physical START button + touchscreen choices
 */
#include "core/inputs.h"
#include "game/helper.h"
#include "game/game_mechanics.h"
#include <TFT_eSPI.h>

// Global TFT object (defined in main.cpp)
extern TFT_eSPI tft;

// ==========================================
// LAYOUT CONSTANTS (landscape 320x240)
// ==========================================

static constexpr int SCREEN_W = 320;
static constexpr int SCREEN_H = 240;

// ==========================================
// SINGLETON
// ==========================================

Inputs &Inputs::get()
{
    static Inputs instance;
    return instance;
}

// ==========================================
// LIFECYCLE
// ==========================================

void Inputs::initializeInputsPins()
{
    Serial.println("🔘 Initializing input pins...");

    pinMode(SystemConfig::INPUT_START, INPUT_PULLUP);

    // Touchscreen is already initialized by tft.init() in main.cpp
    Serial.println("✅ Input pins initialized (START + touch)!");
}

// ==========================================
// START BUTTON
// ==========================================

Inputs::ButtonEvent Inputs::processSmartButton()
{
    const unsigned long currentTime = millis();
    const bool rawPressed = (digitalRead(SystemConfig::INPUT_START) == LOW);
    ButtonEvent eventToReturn = ButtonEvent::NONE;

    // 1. Debounced edge detection
    if (currentTime - lastButtonPressTime[START_INDEX] > debounceDelay)
    {
        // Falling edge — pressed
        if (rawPressed && !lastStartButtonState)
        {
            lastStartButtonState = true;
            lastButtonPressTime[START_INDEX] = currentTime;
            buttonPressStartTime = currentTime;
            longPressHandled = false;
        }
        // Rising edge — released
        else if (!rawPressed && lastStartButtonState)
        {
            lastStartButtonState = false;
            lastButtonPressTime[START_INDEX] = currentTime;

            if (!longPressHandled)
            {
                clickCount++;
                lastClickReleaseTime = currentTime;

                if (clickCount == 2)
                {
                    Serial.println("✅ START DOUBLE CLICK DETECTED!");
                    eventToReturn = ButtonEvent::DOUBLE_CLICK;
                    clickCount = 0;
                }
            }
        }
    }

    // 2. Long press while held
    if (lastStartButtonState && !longPressHandled)
    {
        if (currentTime - buttonPressStartTime >= LONG_PRESS_DELAY)
        {
            Serial.println("✅ START LONG PRESS DETECTED!");
            eventToReturn = ButtonEvent::LONG_PRESS;
            longPressHandled = true;
            clickCount = 0;
        }
    }

    // 3. Single click timeout
    if (clickCount == 1 && !lastStartButtonState)
    {
        if (currentTime - lastClickReleaseTime > DOUBLE_CLICK_DELAY)
        {
            Serial.println("✅ START SINGLE CLICK DETECTED!");
            eventToReturn = ButtonEvent::SINGLE_CLICK;
            clickCount = 0;
        }
    }

    return eventToReturn;
}

// ==========================================
// ANSWER CHOICE
// ==========================================

char Inputs::choicesButtonProcessor()
{
    SystemConfig &config = SystemConfig::get();
    const unsigned long currentTime = millis();

    // Only read answers while a question is active
    if (config.displayState != SystemConfig::SHOW_QUESTION ||
        config.answered ||
        config.currentQuestionPos >= static_cast<int>(config.questionOrder.size()))
    {
        return ' ';
    }

    if (currentTime - lastTouchTime < TOUCH_DEBOUNCE)
        return ' ';

    uint16_t x = 0, y = 0;
    if (!tft.getTouch(&x, &y))
        return ' ';

    // Apply mirror
    if (mirrorX_)
        x = SCREEN_W - 1 - x;
    if (mirrorY_)
        y = SCREEN_H - 1 - y;

    if (x >= SCREEN_W)
        x = SCREEN_W - 1;
    if (y >= SCREEN_H)
        y = SCREEN_H - 1;

    lastTouchTime = currentTime;

    char choice = mapTouchToChoice(x, y);
    if (choice != ' ')
    {
        Serial.printf("👆 Touch: (%d, %d) → %c\n", x, y, choice);
    }
    return choice;
}

char Inputs::mapTouchToChoice(uint16_t x, uint16_t y)
{
    SystemConfig &config = SystemConfig::get();

    // Band positions are written by DisplayOutputs::showQuestionScreen().
    // Each band spans [choiceBandY[i], choiceBandY[i] + choiceBandH + gap).
    // The +gap tolerance means taps in the gap between bands still
    // register on the band above — no dead zones.
    const int gap = 4; // must match the drawing gap between bands

    for (int i = 0; i < SystemConfig::CHOICE_COUNT; i++)
    {
        int top = config.choiceBandY[i];
        int bottom = top + config.choiceBandH + gap;

        if (y >= top && y < bottom)
        {
            return 'A' + i; // A, B, C, D
        }
    }

    return ' ';
}

// ==========================================
// SAVIOR SELECTION
// ==========================================

String Inputs::findPartnerName(const String &playerName)
{
    SystemConfig &config = SystemConfig::get();

    for (const auto &p : config.playerScores)
    {
        if (p.name == playerName)
        {
            const String partner = p.pairedUpWith;
            if (partner.length() > 0 && partner != "Solo")
            {
                return partner;
            }
            return "";
        }
    }
    return "";
}

void Inputs::choicesButtonSavior()
{
    SystemConfig &config = SystemConfig::get();
    Helper &helper = Helper::get();
    const unsigned long currentTime = millis();

    const int activePlayers = helper.getActivePlayerCount();
    if (activePlayers == 0)
        return;

    if (currentTime - lastTouchTime < TOUCH_DEBOUNCE + 150)
        return;

    uint16_t x = 0, y = 0;
    if (!tft.getTouch(&x, &y))
        return;

    if (mirrorX_)
        x = SCREEN_W - 1 - x;
    if (mirrorY_)
        y = SCREEN_H - 1 - y;
    if (x >= SCREEN_W)
        x = SCREEN_W - 1;
    if (y >= SCREEN_H)
        y = SCREEN_H - 1;

    lastTouchTime = currentTime;
    Serial.printf("👆 Savior touch: (%d, %d)\n", x, y);

    // Tap a row → select that player
    for (int row = 0; row < config.saviorRowCount; row++)
    {
        int top = config.saviorRowY[row];
        int bottom = top + config.saviorRowH;

        if (y >= top && y < bottom)
        {
            int rowIndex = 0;
            for (int i = 0; i < config.getMaxPlayer(); i++)
            {
                const auto &p = config.playerScores[i];
                if (p.isEliminated)
                    continue;
                if (p.name == config.currentPlayerName)
                    continue;

                if (rowIndex == row)
                {
                    config.selectedSaviorIndex = i;
                    config.revivingProcess = false;
                    config.refuseToRevive = false;
                    Serial.printf("✅ Row %d selected: %s\n",
                                  row, p.name.c_str());
                    return;
                }
                rowIndex++;
            }
        }
    }

    // Tap the button bar
    if (y >= config.saviorBtnBarY &&
        y < config.saviorBtnBarY + config.saviorBtnH)
    {
        // Elect (left)
        if (x >= config.saviorBtnElectX &&
            x < config.saviorBtnElectX + config.saviorBtnW)
        {
            const String targetName = config.currentPlayerName;
            const String partnerName = findPartnerName(targetName);

            if (config.selectedSaviorIndex >= 0 &&
                config.selectedSaviorIndex < (int)config.playerScores.size())
            {
                const String electedSavior =
                    config.playerScores[config.selectedSaviorIndex].name;

                // A rival stepping in = the partner was displaced
                const bool rivalIntervened =
                    partnerName.length() > 0 &&
                    partnerName != electedSavior &&
                    partnerName != targetName;

                if (rivalIntervened)
                {
                    GameMechanics::get().recordBetrayal(electedSavior,
                                                        partnerName);
                }

                config.saviorPlayerName = electedSavior;
            }

            config.revivingProcess = true;
            config.refuseToRevive = false;
            Serial.println("✅ Elected current selection");
            return;
        }

        // Next (middle)
        if (x >= config.saviorBtnNextX &&
            x < config.saviorBtnNextX + config.saviorBtnW)
        {
            advanceSaviorSelection();
            config.revivingProcess = false;
            config.refuseToRevive = false;
            Serial.println("⬇️ Next player");
            return;
        }

        // Skip (right)
        if (x >= config.saviorBtnSkipX &&
            x < config.saviorBtnSkipX + config.saviorBtnW)
        {
            const String targetName = config.currentPlayerName;
            const String partnerName = findPartnerName(targetName);

            // If the target had a partner, the partner is the one
            // who should have stepped in. Skipping = the partner failed.
            const bool partnerRefused =
                partnerName.length() > 0 &&
                partnerName != targetName;

            if (partnerRefused)
            {
                GameMechanics::get().recordBetrayal(partnerName, targetName);
            }

            config.refuseToRevive = true;
            config.revivingProcess = true;
            config.selectedSaviorIndex = -1;
            config.saviorPlayerName = "";
            Serial.println("🚫 Skipped");
            return;
        }
    }
}

void Inputs::choicesButtonLastChance()
{
    SystemConfig &config = SystemConfig::get();
    const unsigned long currentTime = millis();

    // Debounce — same as the other touch handlers, plus the extra
    // 150 ms that the savior screen uses to avoid double-fires
    // between the initial tap and the decision tap.
    if (currentTime - lastTouchTime < TOUCH_DEBOUNCE + 150)
        return;

    uint16_t x = 0, y = 0;
    if (!tft.getTouch(&x, &y))
        return;

    // Apply mirror
    if (mirrorX_) x = SCREEN_W - 1 - x;
    if (mirrorY_) y = SCREEN_H - 1 - y;

    if (x >= SCREEN_W) x = SCREEN_W - 1;
    if (y >= SCREEN_H) y = SCREEN_H - 1;

    lastTouchTime = currentTime;

    // ==========================================
    // HIT TEST — reuse the geometry saved by
    // DisplayOutputs::showWouldYouSaveThePlayerScreen()
    // ==========================================
    const int btnY = config.newSessionBtnY;
    const int btnH = config.newSessionBtnH;

    if (y < btnY || y >= btnY + btnH)
        return; // tap outside the button row

    const int yesX = config.newSessionYesX;
    const int noX  = config.newSessionNoX;
    const int btnW = config.newSessionBtnW;

    // ==========================================
    // SAVE (left button)
    // ==========================================
    if (x >= yesX && x < yesX + btnW)
    {
        // The surviving player is the savior. Find them.
        const String targetName = config.currentPlayerName;
        String savior = "";

        for (const auto &p : config.playerScores)
        {
            if (p.isEliminated) continue;
            if (p.name == targetName) continue;
            savior = p.name;
            break;
        }

        if (savior.isEmpty())
        {
            Serial.println("⚠️ Last-chance SAVE: no surviving player found");
            return;
        }

        config.saviorPlayerName = savior;
        config.refuseToRevive = false;
        config.revivingProcess = true;

        Serial.printf("✅ Last-chance: %s will save %s\n",
                      savior.c_str(),
                      targetName.c_str());
        return;
    }

    // ==========================================
    // REFUSE (right button)
    // ==========================================
    if (x >= noX && x < noX + btnW)
    {
        // The survivor is refusing. Record it as a betrayal against
        // the partner — but only if the target actually had a partner.
        const String targetName = config.currentPlayerName;
        const String partnerName = findPartnerName(targetName);

        if (partnerName.length() > 0 &&
            partnerName != "Solo" &&
            partnerName != targetName)
        {
            GameMechanics::get().recordBetrayal(partnerName, targetName);
        }

        config.saviorPlayerName = "";
        config.refuseToRevive = true;
        config.revivingProcess = true;

        Serial.println("🚫 Last-chance: refused to save");
        return;
    }
}

void Inputs::advanceSaviorSelection()
{
    SystemConfig &config = SystemConfig::get();

    const int maxPlayers = config.getMaxPlayer();
    if (maxPlayers == 0)
        return;

    int start = config.selectedSaviorIndex;

    for (int step = 0; step < maxPlayers; step++)
    {
        config.selectedSaviorIndex++;

        if (config.selectedSaviorIndex >= maxPlayers)
            config.selectedSaviorIndex = 0;

        const auto &p = config.playerScores[config.selectedSaviorIndex];

        // Stop at the first eligible player
        if (!p.isEliminated && p.name != config.currentPlayerName)
        {
            config.revivingProcess = false;
            config.refuseToRevive = false;
            return;
        }

        // Avoid infinite loop if nothing is eligible
        if (config.selectedSaviorIndex == start)
            break;
    }
}

// ==========================================
// NEW-SESSION CONFIRMATION
// ==========================================

int Inputs::confirmationSessionButton()
{
    SystemConfig &config = SystemConfig::get();
    const unsigned long currentTime = millis();

    if (currentTime - lastTouchTime < TOUCH_DEBOUNCE + 150)
        return 0;

    uint16_t x = 0, y = 0;
    if (!tft.getTouch(&x, &y))
        return 0;

    if (mirrorX_)
        x = SCREEN_W - 1 - x;
    if (mirrorY_)
        y = SCREEN_H - 1 - y;
    if (x >= SCREEN_W)
        x = SCREEN_W - 1;
    if (y >= SCREEN_H)
        y = SCREEN_H - 1;

    lastTouchTime = currentTime;

    // Use saved button geometry — no magic numbers
    const int btnY = config.newSessionBtnY;
    const int btnH = config.newSessionBtnH;

    if (y >= btnY && y < btnY + btnH)
    {
        // YES button (left)
        if (x >= config.newSessionYesX &&
            x <  config.newSessionYesX + config.newSessionBtnW)
        {
            config.isNewSesion = true;
            Serial.println("✅ YES — new session");
            return 1;
        }

        // NO button (right)
        if (x >= config.newSessionNoX &&
            x <  config.newSessionNoX + config.newSessionBtnW)
        {
            config.isNewSesion = false;
            Serial.println("❌ NO — reuse session");
            return 2;
        }
    }

    return 0;
}