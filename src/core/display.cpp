/**
 * @file core/display.cpp
 * @brief Display output implementation — landscape 320x240 (TFT_eSPI / ST7789)
 */
#include "core/display.h"
#include <algorithm>
#include "game/helper.h"

extern Helper helper;

// ==========================================
// LAYOUT CONSTANTS (landscape 320x240)
// ==========================================

static constexpr int SCREEN_W = 320;
static constexpr int SCREEN_H = 240;

// Centered X for a string of N chars at a given text size.
// TFT_eSPI default GLCD font: 6px per char per size unit.
static inline int centerX(int charCount, int textSize)
{
    const int charWidth = 6 * textSize;
    return (SCREEN_W - (charCount * charWidth)) / 2;
}

// ==========================================
// CONSTRUCTOR & DESTRUCTOR
// ==========================================

DisplayOutputs::DisplayOutputs()
    : config(SystemConfig::get())
{
    initialized = true;
}

DisplayOutputs::~DisplayOutputs() {}

// ==========================================
// SPLASH & RESTART
// ==========================================

void DisplayOutputs::showSplashScreen()
{
    tft.fillScreen(TFT_BLACK);

    // Title — large, Deep Indigo
    tft.setTextSize(4);
    tft.setTextColor(SystemConfig::COLOR_DEEP_INDIGO, TFT_BLACK);

    const char *title = "reviewQuest";
    tft.setCursor(centerX(strlen(title), 4), 50);
    tft.print(title);

    // Underline: bright purple over dimmer purple
    tft.drawFastHLine(60, 92, SCREEN_W - 120, SystemConfig::COLOR_NEBULA_PURPLE);
    tft.drawFastHLine(60, 93, SCREEN_W - 120, 0x5A6F);

    // Loading — size 2, Nebula Purple
    tft.setTextSize(2);
    tft.setTextColor(SystemConfig::COLOR_NEBULA_PURPLE, TFT_BLACK);

    const char *loading = "Loading... Please wait.";
    tft.setCursor(centerX(strlen(loading), 2), 120);
    tft.print(loading);

    // Brand — size 1, Starlight Blue
    tft.setTextSize(1);
    tft.setTextColor(SystemConfig::COLOR_STARLIGHT, TFT_BLACK);

    const char *brand = "storem";
    tft.setCursor(centerX(strlen(brand), 1), 170);
    tft.print(brand);

    // Three dots pulsing in the bottom-right
    for (int i = 0; i < 3; i++)
    {
        tft.fillCircle(SCREEN_W - 30 + i * 8, SCREEN_H - 15, 2,
                       SystemConfig::COLOR_STARLIGHT);
    }

    config.displayState = SystemConfig::SHOW_SPLASH;
}

void DisplayOutputs::showRestartGameScreen()
{
    tft.fillScreen(TFT_BLACK);

    // Two-line title, Deep Indigo
    tft.setTextSize(4);
    tft.setTextColor(SystemConfig::COLOR_DEEP_INDIGO, TFT_BLACK);

    const char *line1 = "REVIEW";
    const char *line2 = "QUEST";

    tft.setCursor(centerX(strlen(line1), 4), 40);
    tft.print(line1);

    tft.setCursor(centerX(strlen(line2), 4), 82);
    tft.print(line2);

    tft.drawFastHLine(80, 122, SCREEN_W - 160, SystemConfig::COLOR_NEBULA_PURPLE);

    // Status — size 2, Nebula Purple
    tft.setTextSize(2);
    tft.setTextColor(SystemConfig::COLOR_NEBULA_PURPLE, TFT_BLACK);

    const char *msg = "Game restarted";
    tft.setCursor(centerX(strlen(msg), 2), 145);
    tft.print(msg);

    // Countdown — size 1, Starlight Blue
    tft.setTextSize(1);
    tft.setTextColor(SystemConfig::COLOR_STARLIGHT, TFT_BLACK);

    const char *countdown = "Will start in 1 second";
    tft.setCursor(centerX(strlen(countdown), 1), 180);
    tft.print(countdown);

    config.displayState = SystemConfig::SHOW_RESTART_GAME;
    config.currentQuestionPos = 0;
    config.currentPlayerPos = 0;
    config.overallScore = 0;
    config.answered = false;
}

// ==========================================
// SESSION SCREENS
// ==========================================

void DisplayOutputs::showNewSessionScreen()
{
    SystemConfig &cfg = SystemConfig::get();
    tft.fillScreen(TFT_BLACK);

    const int PAD_X = 10;
    const int CONTENT_X = PAD_X;
    const int CONTENT_W = SCREEN_W - 2 * PAD_X;

    // Title
    tft.setTextSize(2);
    tft.setTextColor(SystemConfig::COLOR_DEEP_INDIGO, TFT_BLACK);

    const char *title = "NEW SESSION?";
    tft.setCursor(centerX(strlen(title), 2), 16);
    tft.print(title);

    tft.drawLine(CONTENT_X, 36, SCREEN_W - PAD_X, 36,
                 SystemConfig::COLOR_DEEP_INDIGO);

    // Prompt
    tft.setTextSize(2);
    tft.setTextColor(SystemConfig::COLOR_NEBULA_PURPLE, TFT_BLACK);

    const char *line1 = "Would you like to try";
    tft.setCursor(centerX(strlen(line1), 2), 68);
    tft.print(line1);

    const char *line2 = "another session?";
    tft.setCursor(centerX(strlen(line2), 2), 96);
    tft.print(line2);

    // Button geometry — two buttons, side by side
    const int btnW = 110;
    const int btnH = 46;
    const int gap = 20;
    const int totalW = btnW * 2 + gap;
    const int startX = (SCREEN_W - totalW) / 2;
    const int btnY = 148;

    const int yesX = startX;
    const int noX = startX + btnW + gap;

    auto drawFlatButton = [&](int x, int y, int w, int h,
                              const char *label,
                              uint16_t borderColor,
                              uint16_t textColor)
    {
        tft.fillRect(x, y, w, h, SystemConfig::COLOR_CARD_BG);

        // 2 px border for emphasis
        tft.drawRect(x, y, w, h, borderColor);
        tft.drawRect(x + 1, y + 1, w - 2, h - 2, borderColor);

        // Centered label
        tft.setTextSize(3);
        int labelW = strlen(label) * 18; // size 3 = 18 px/char
        int labelX = x + (w - labelW) / 2;
        int labelY = y + (h - 24) / 2;

        tft.setTextColor(textColor, SystemConfig::COLOR_CARD_BG);
        tft.setCursor(labelX, labelY);
        tft.print(label);
    };

    drawFlatButton(yesX, btnY, btnW, btnH, "YES",
                   SystemConfig::COLOR_NEBULA_PURPLE,
                   TFT_WHITE);

    drawFlatButton(noX, btnY, btnW, btnH, "NO",
                   SystemConfig::COLOR_DEEP_INDIGO,
                   0xCE79);

    // Save geometry for touch handling in Inputs
    cfg.newSessionYesX = yesX;
    cfg.newSessionNoX = noX;
    cfg.newSessionBtnY = btnY;
    cfg.newSessionBtnW = btnW;
    cfg.newSessionBtnH = btnH;

    // Footer
    tft.setTextSize(1);
    tft.setTextColor(SystemConfig::COLOR_STARLIGHT, TFT_BLACK);

    const char *hint = "Tap a button to choose";
    tft.setCursor(centerX(strlen(hint), 1), SCREEN_H - 12);
    tft.print(hint);

    config.displayState = SystemConfig::SHOW_NEW_SESSION;
}

void DisplayOutputs::showNoCurrentSessionScreen()
{
    SystemConfig &cfg = SystemConfig::get();
    tft.fillScreen(TFT_BLACK);

    const int PAD_X = 10;
    const int CONTENT_X = PAD_X;
    const int CONTENT_W = SCREEN_W - 2 * PAD_X;

    // Title
    tft.setTextSize(2);
    tft.setTextColor(SystemConfig::COLOR_DEEP_INDIGO, TFT_BLACK);

    const char *title = "NO SESSION";
    tft.setCursor(centerX(strlen(title), 2), 6);
    tft.print(title);

    tft.drawLine(CONTENT_X, 26, SCREEN_W - PAD_X, 26,
                 SystemConfig::COLOR_DEEP_INDIGO);

    // Status line — differs by WiFi state
    tft.setTextSize(1);

    if (cfg.isSessionEnabled)
    {
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
        const char *status = "WiFi is active; ready to receive";
        tft.setCursor(CONTENT_X, 34);
        tft.print(status);
    }
    else
    {
        tft.setTextColor(SystemConfig::COLOR_NEBULA_PURPLE, TFT_BLACK);
        const char *status = "WiFi is off; no session uploaded";
        tft.setCursor(CONTENT_X, 34);
        tft.print(status);
    }

    // WiFi credentials card — only drawn when session is enabled
    const int cardY = 48;
    const int cardH = 96;

    int nextY = cardY;

    if (cfg.isSessionEnabled)
    {
        tft.fillRect(CONTENT_X, cardY, CONTENT_W, cardH,
                     SystemConfig::COLOR_CARD_BG);
        tft.drawRect(CONTENT_X, cardY, CONTENT_W, cardH,
                     SystemConfig::COLOR_NEBULA_PURPLE);

        const int labelX = CONTENT_X + 10;
        const int valueX = CONTENT_X + 10;

        // SSID
        tft.setTextSize(1);
        tft.setTextColor(SystemConfig::COLOR_STARLIGHT,
                         SystemConfig::COLOR_CARD_BG);
        tft.setCursor(labelX, cardY + 8);
        tft.print("WiFi");

        tft.setTextSize(2);
        tft.setTextColor(TFT_WHITE, SystemConfig::COLOR_CARD_BG);
        tft.setCursor(valueX, cardY + 22);
        tft.print(cfg.wifiSSID);

        // Password
        tft.setTextSize(1);
        tft.setTextColor(SystemConfig::COLOR_STARLIGHT,
                         SystemConfig::COLOR_CARD_BG);
        tft.setCursor(labelX, cardY + 54);
        tft.print("Password");

        tft.setTextSize(2);
        tft.setTextColor(TFT_WHITE, SystemConfig::COLOR_CARD_BG);
        tft.setCursor(valueX, cardY + 68);
        tft.print(cfg.wifiPassword);

        nextY = cardY + cardH + 10;
    }
    else
    {
        nextY = 56;
    }

    // Main instruction — differs by WiFi state
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    int yPos = nextY;

    if (cfg.isSessionEnabled)
    {
        wrapText(
            "Connect your phone to the WiFi above, then send the "
            "session from the ReviewQuest app.",
            CONTENT_X, yPos, CONTENT_W,
            SCREEN_H - 20,
            10,
            TFT_WHITE,
            1);
    }
    else
    {
        wrapText(
            "Press START twice to enable WiFi. The credentials "
            "will appear here. Then connect your phone and send "
            "the session from the ReviewQuest app.",
            CONTENT_X, yPos, CONTENT_W,
            SCREEN_H - 20,
            10,
            TFT_WHITE,
            1);
    }

    // Footer
    tft.setTextSize(1);
    tft.setTextColor(SystemConfig::COLOR_STARLIGHT, TFT_BLACK);

    const char *hint = cfg.isSessionEnabled
                           ? "Waiting for session upload..."
                           : "Waiting for input...";

    tft.setCursor(CONTENT_X, SCREEN_H - 12);
    tft.print(hint);

    config.displayState = SystemConfig::SHOW_NO_CURRENT_SESSION;
}

void DisplayOutputs::showStartSessionScreen()
{
    tft.fillScreen(TFT_BLACK);

    const int PAD_X = 10;
    const int CONTENT_X = PAD_X;
    const int CONTENT_W = SCREEN_W - 2 * PAD_X;

    // Title
    tft.setTextSize(2);
    tft.setTextColor(SystemConfig::COLOR_DEEP_INDIGO, TFT_BLACK);

    const char *title = "SESSION READY";
    tft.setCursor(centerX(strlen(title), 2), 16);
    tft.print(title);

    tft.drawLine(CONTENT_X, 36, SCREEN_W - PAD_X, 36,
                 SystemConfig::COLOR_DEEP_INDIGO);

    // Status
    tft.setTextSize(1);
    tft.setTextColor(SystemConfig::COLOR_NEBULA_PURPLE, TFT_BLACK);

    const char *status = "Uploaded and ready to play";
    tft.setCursor(CONTENT_X, 46);
    tft.print(status);

    // Session info card
    const int cardY = 66;
    const int cardH = 130;

    tft.fillRect(CONTENT_X, cardY, CONTENT_W, cardH,
                 SystemConfig::COLOR_CARD_BG);
    tft.drawRect(CONTENT_X, cardY, CONTENT_W, cardH,
                 SystemConfig::COLOR_NEBULA_PURPLE);

    const int labelX = CONTENT_X + 12;
    const int valueX = CONTENT_X + 12;

    // Session title
    tft.setTextSize(1);
    tft.setTextColor(SystemConfig::COLOR_STARLIGHT,
                     SystemConfig::COLOR_CARD_BG);
    tft.setCursor(labelX, cardY + 10);
    tft.print("Session");

    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE, SystemConfig::COLOR_CARD_BG);
    tft.setCursor(valueX, cardY + 24);
    tft.print(config.sessionTitle);

    // Questions count
    tft.setTextSize(1);
    tft.setTextColor(SystemConfig::COLOR_STARLIGHT,
                     SystemConfig::COLOR_CARD_BG);
    tft.setCursor(labelX, cardY + 52);
    tft.print("Questions");

    tft.setTextSize(3);
    tft.setTextColor(SystemConfig::COLOR_NEBULA_PURPLE,
                     SystemConfig::COLOR_CARD_BG);

    char qBuf[16];
    snprintf(qBuf, sizeof(qBuf), "%d", (int)config.questionList.size());
    tft.setCursor(valueX, cardY + 66);
    tft.print(qBuf);

    // Players count
    tft.setTextSize(1);
    tft.setTextColor(SystemConfig::COLOR_STARLIGHT,
                     SystemConfig::COLOR_CARD_BG);
    tft.setCursor(labelX + 100, cardY + 52);
    tft.print("Players");

    tft.setTextSize(3);
    tft.setTextColor(SystemConfig::COLOR_NEBULA_PURPLE,
                     SystemConfig::COLOR_CARD_BG);

    char pBuf[16];
    snprintf(pBuf, sizeof(pBuf), "%d", (int)config.playerScores.size());
    tft.setCursor(labelX + 100, cardY + 66);
    tft.print(pBuf);

    // Ready badge at bottom of card
    tft.drawFastHLine(labelX, cardY + cardH - 30,
                      CONTENT_W - 24, 0x39E7);

    tft.setTextSize(1);
    tft.setTextColor(SystemConfig::COLOR_STARLIGHT,
                     SystemConfig::COLOR_CARD_BG);
    tft.setCursor(labelX, cardY + cardH - 22);
    tft.print("Status");

    tft.setTextSize(2);
    tft.setTextColor(TFT_GREEN, SystemConfig::COLOR_CARD_BG);
    tft.setCursor(labelX + 60, cardY + cardH - 26);
    tft.print("Data uploaded");

    // Footer
    tft.setTextSize(1);
    tft.setTextColor(SystemConfig::COLOR_STARLIGHT, TFT_BLACK);

    const char *hint = "Press START to begin";
    tft.setCursor(CONTENT_X, SCREEN_H - 12);
    tft.print(hint);

    config.displayState = SystemConfig::SHOW_UPLOADED;
}

// ==========================================
// PLAYER ROSTER
// ==========================================

void DisplayOutputs::showListPlayers()
{
    tft.fillScreen(TFT_BLACK);

    const int PAD_X = 10;
    const int CONTENT_X = PAD_X;
    const int CONTENT_W = SCREEN_W - 2 * PAD_X;

    // Title
    tft.setTextSize(2);
    tft.setTextColor(SystemConfig::COLOR_DEEP_INDIGO, TFT_BLACK);

    const char *title = "PLAYERS";
    tft.setCursor(centerX(strlen(title), 2), 10);
    tft.print(title);

    tft.drawLine(CONTENT_X, 30, SCREEN_W - PAD_X, 30,
                 SystemConfig::COLOR_DEEP_INDIGO);

    // Player count subtitle
    tft.setTextSize(1);
    tft.setTextColor(SystemConfig::COLOR_NEBULA_PURPLE, TFT_BLACK);

    char subtitle[32];
    snprintf(subtitle, sizeof(subtitle), "%d player%s",
             (int)config.playerScores.size(),
             config.playerScores.size() == 1 ? "" : "s");
    tft.setCursor(CONTENT_X, 36);
    tft.print(subtitle);

    // Row layout
    const int rowTop = 52;
    const int rowH = 32;
    const int maxRows = (SCREEN_H - rowTop - 20) / rowH;

    for (int i = 0; i < (int)config.playerScores.size() && i < maxRows; i++)
    {
        const auto &p = config.playerScores[i];
        const int yPos = rowTop + i * rowH;

        const bool hasPartner = !p.pairedUpWith.isEmpty();

        // Row background — subtle tint for paired players
        uint16_t rowBg = hasPartner
                             ? SystemConfig::COLOR_CARD_BG
                             : TFT_BLACK;

        tft.fillRect(4, yPos - 2, SCREEN_W - 8, rowH - 2, rowBg);

        // Accent bar on the left for paired players
        if (hasPartner)
        {
            tft.fillRect(4, yPos - 2, 3, rowH - 2,
                         SystemConfig::COLOR_NEBULA_PURPLE);
        }

        // Slot number
        tft.setTextSize(2);
        tft.setTextColor(hasPartner
                             ? SystemConfig::COLOR_NEBULA_PURPLE
                             : TFT_LIGHTGREY,
                         rowBg);
        tft.setCursor(14, yPos + 8);
        tft.printf("%d", i + 1);

        // Player name
        tft.setTextColor(TFT_WHITE, rowBg);
        tft.setCursor(44, yPos + 8);
        tft.print(p.name);

        // Partner tag — right-aligned
        if (hasPartner)
        {
            String partner = p.pairedUpWith;
            if (partner.length() > 8)
            {
                partner = partner.substring(0, 7) + ".";
            }

            String tag = "-> " + partner;

            int tagW = tag.length() * 6; // size 1 = 6 px/char
            int tagX = SCREEN_W - PAD_X - tagW - 4;

            tft.setTextSize(1);
            tft.setTextColor(SystemConfig::COLOR_NEBULA_PURPLE, rowBg);
            tft.setCursor(tagX, yPos + 12);
            tft.print(tag);
        }
    }

    // Overflow footer — show count of hidden players
    const int total = (int)config.playerScores.size();
    if (total > maxRows)
    {
        tft.setTextSize(1);
        tft.setTextColor(TFT_YELLOW, TFT_BLACK);
        tft.setCursor(CONTENT_X, SCREEN_H - 12);
        tft.printf("+%d more players", total - maxRows);
    }

    // Hint
    tft.setTextSize(1);
    tft.setTextColor(SystemConfig::COLOR_STARLIGHT, TFT_BLACK);

    const char *hint = "Press START to begin";
    tft.setCursor(centerX(strlen(hint), 1), SCREEN_H - 12);
    tft.print(hint);

    config.displayState = SystemConfig::SHOW_LIST_PLAYERS;
}

// ==========================================
// QUESTION SCREEN
// ==========================================

void DisplayOutputs::showQuestionScreen(int index)
{
    if (index >= (int)config.questionList.size())
    {
        showCompletionScreen();
        return;
    }

    SystemConfig::Question &q = config.questionList[index];
    config.answered = false;
    config.selectedAnswer = ' ';
    config.displayState = SystemConfig::SHOW_QUESTION;

    tft.fillScreen(TFT_BLACK);

    const int PAD_X = 10;
    const int CONTENT_X = PAD_X;
    const int CONTENT_W = SCREEN_W - 2 * PAD_X;

    // Header — player left, instruction centred, Q counter right
    tft.setTextSize(1);
    tft.setTextColor(config.COLOR_DEEP_INDIGO, TFT_BLACK);

    // Build strings first so we can measure them
    int playerScore = getPlayerScore(config.currentPlayerName);

    char playerBuf[32];
    snprintf(playerBuf, sizeof(playerBuf), "%s: %d pts",
             config.currentPlayerName.c_str(), playerScore);

    char qCounter[16];
    snprintf(qCounter, sizeof(qCounter), "Q%d/%d",
             index + 1, (int)config.questionList.size());

    const char *instruction = "Tap an answer to select";

    // Zone boundaries
    const int playerLeft = CONTENT_X;
    const int playerRight = playerLeft + tft.textWidth(playerBuf);

    const int qRight = SCREEN_W - PAD_X;
    const int qLeft = qRight - tft.textWidth(qCounter);

    // Centre the instruction in the gap between the two
    const int gapMid = (playerRight + qLeft) / 2;
    const int instrX = gapMid - (tft.textWidth(instruction) / 2);

    tft.setCursor(playerLeft, 6);
    tft.print(playerBuf);

    tft.setCursor(instrX, 6);
    tft.print(instruction);

    tft.setCursor(qLeft, 6);
    tft.print(qCounter);

    tft.drawLine(CONTENT_X, 20, SCREEN_W - PAD_X, 20, config.COLOR_DEEP_INDIGO);

    // Question text — wrapped, size 2, white
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    int qY = 28;
    wrapText(q.text, CONTENT_X, qY, CONTENT_W, 96, 18, TFT_WHITE, 2);

    // Answer bands — 4 rows, 32 px tall, 38 px pitch
    const int gap = 10;
    const int bandY = qY + gap;
    const int rowH = 32;
    const int pitch = 38;

    config.choiceBandH = rowH - 4;
    config.choiceBandY[0] = bandY + 0 * pitch;
    config.choiceBandY[1] = bandY + 1 * pitch;
    config.choiceBandY[2] = bandY + 2 * pitch;
    config.choiceBandY[3] = bandY + 3 * pitch;

    auto drawChoice = [&](int i, char letter, const String &opt, bool removed)
    {
        RowStyle s;
        s.bg = removed ? config.COLOR_CARD_BG_DIM : config.COLOR_CARD_BG;
        s.border = config.COLOR_NEBULA_PURPLE;
        s.prefixColor = removed ? TFT_DARKGREY : config.COLOR_DEEP_INDIGO;
        s.textColor = removed ? TFT_DARKGREY : 0xCE79;

        String prefix = String(letter) + ".";
        String body = removed ? "(removed)" : opt;

        drawListRow(config.choiceBandY[i], prefix, body, rowH, s);
    };

    drawChoice(0, 'A', q.optionA, q.optionA == "[REMOVED]");
    drawChoice(1, 'B', q.optionB, q.optionB == "[REMOVED]");
    drawChoice(2, 'C', q.optionC, q.optionC == "[REMOVED]");
    drawChoice(3, 'D', q.optionD, q.optionD == "[REMOVED]");
}

// ==========================================
// ANSWER BAND RENDERER
// ==========================================

void DisplayOutputs::drawListRow(int y, const String &prefix,
                                 const String &text, int rowH,
                                 const RowStyle &style)
{
    const int PAD_X = 10;
    const int x = PAD_X;
    const int w = SCREEN_W - 2 * PAD_X;
    const int h = rowH - 4;

    uint16_t bg = style.selected ? style.selectBg : style.bg;
    uint16_t prefixColor = style.selected ? TFT_WHITE : style.prefixColor;
    uint16_t textColor = style.selected ? TFT_WHITE : style.textColor;

    tft.fillRect(x, y, w, h, bg);
    tft.drawRect(x, y, w, h, style.border);

    // Prefix (A. / B. / C. / D.)
    tft.setTextSize(style.textSize == TextSize::Small ? 1 : 2);
    tft.setTextColor(prefixColor, bg);
    tft.setCursor(x + 8, y + 6);
    tft.print(prefix);

    int prefixW = tft.textWidth(prefix);

    // Body text
    tft.setTextColor(textColor, bg);
    tft.setCursor(x + 8 + prefixW + 6, y + 6);

    // Truncate with ellipsis only if the body overflows the card
    String shown = text;
    const int maxWidth = w - (8 + prefixW + 6) - 8;
    while ((int)tft.textWidth(shown) > maxWidth && shown.length() > 3)
    {
        shown = shown.substring(0, shown.length() - 4) + "...";
    }
    tft.print(shown);
}

// ==========================================
// FEEDBACK SCREENS
// ==========================================

void DisplayOutputs::showCorrectFeedbackScreen()
{
    tft.fillScreen(TFT_BLACK);

    const int PAD_X = 10;
    const int CONTENT_X = PAD_X;
    const int CONTENT_W = SCREEN_W - 2 * PAD_X;

    // Title
    tft.setTextSize(2);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);

    const char *title = "CORRECT";
    tft.setCursor(centerX(strlen(title), 2), 16);
    tft.print(title);

    tft.drawLine(CONTENT_X, 36, SCREEN_W - PAD_X, 36,
                 SystemConfig::COLOR_DEEP_INDIGO);

    // Status
    tft.setTextSize(1);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);

    const char *status = "Answer accepted";
    tft.setCursor(CONTENT_X, 46);
    tft.print(status);

    // Message card
    const int cardY = 66;
    const int cardH = 130;

    tft.fillRect(CONTENT_X, cardY, CONTENT_W, cardH,
                 SystemConfig::COLOR_CARD_BG);
    tft.drawRect(CONTENT_X, cardY, CONTENT_W, cardH, TFT_GREEN);

    // Headline
    tft.setTextSize(3);
    tft.setTextColor(TFT_WHITE, SystemConfig::COLOR_CARD_BG);

    const char *line1 = "PASS TO";
    const char *line2 = "NEXT PLAYER!";

    tft.setCursor(CONTENT_X + 12, cardY + 16);
    tft.print(line1);

    tft.setCursor(CONTENT_X + 12, cardY + 46);
    tft.print(line2);

    tft.drawFastHLine(CONTENT_X + 12, cardY + 82,
                      CONTENT_W - 24, 0x39E7);

    // Stats — overall score + player's score
    tft.setTextSize(1);
    tft.setTextColor(SystemConfig::COLOR_STARLIGHT,
                     SystemConfig::COLOR_CARD_BG);
    tft.setCursor(CONTENT_X + 12, cardY + 90);
    tft.print("Score");

    char scoreBuf[16];
    snprintf(scoreBuf, sizeof(scoreBuf), "%d", config.overallScore);
    tft.setTextSize(3);
    tft.setTextColor(SystemConfig::COLOR_NEBULA_PURPLE,
                     SystemConfig::COLOR_CARD_BG);
    tft.setCursor(CONTENT_X + 12, cardY + 100);
    tft.print(scoreBuf);

    tft.setTextSize(1);
    tft.setTextColor(SystemConfig::COLOR_STARLIGHT,
                     SystemConfig::COLOR_CARD_BG);
    tft.setCursor(CONTENT_X + 100, cardY + 90);
    tft.print(config.currentPlayerName);

    int playerScore = getPlayerScore(config.currentPlayerName);
    snprintf(scoreBuf, sizeof(scoreBuf), "%d pts", playerScore);
    tft.setTextSize(3);
    tft.setTextColor(TFT_WHITE, SystemConfig::COLOR_CARD_BG);
    tft.setCursor(CONTENT_X + 100, cardY + 100);
    tft.print(scoreBuf);

    // Footer
    tft.setTextSize(1);
    tft.setTextColor(SystemConfig::COLOR_STARLIGHT, TFT_BLACK);

    const char *hint = "Hand the device to the next player";
    tft.setCursor(CONTENT_X, SCREEN_H - 12);
    tft.print(hint);

    config.displayState = SystemConfig::SHOW_CORRECT;
    config.stateStartTime = millis();
}

void DisplayOutputs::showIncorrectFeedbackScreen()
{
    tft.fillScreen(TFT_BLACK);

    const int PAD_X = 10;
    const int CONTENT_X = PAD_X;
    const int CONTENT_W = SCREEN_W - 2 * PAD_X;

    // Title
    tft.setTextSize(2);
    tft.setTextColor(TFT_RED, TFT_BLACK);

    const char *title = "INCORRECT";
    tft.setCursor(centerX(strlen(title), 2), 16);
    tft.print(title);

    tft.drawLine(CONTENT_X, 36, SCREEN_W - PAD_X, 36,
                 SystemConfig::COLOR_DEEP_INDIGO);

    // Status
    tft.setTextSize(1);
    tft.setTextColor(TFT_RED, TFT_BLACK);

    const char *status = "Answer rejected";
    tft.setCursor(CONTENT_X, 46);
    tft.print(status);

    // Message card
    const int cardY = 66;
    const int cardH = 120;

    tft.fillRect(CONTENT_X, cardY, CONTENT_W, cardH,
                 SystemConfig::COLOR_CARD_BG);
    tft.drawRect(CONTENT_X, cardY, CONTENT_W, cardH, TFT_RED);

    // Player
    tft.setTextSize(1);
    tft.setTextColor(SystemConfig::COLOR_STARLIGHT,
                     SystemConfig::COLOR_CARD_BG);
    tft.setCursor(CONTENT_X + 12, cardY + 14);
    tft.print("Player");

    tft.setTextSize(3);
    tft.setTextColor(TFT_WHITE, SystemConfig::COLOR_CARD_BG);
    tft.setCursor(CONTENT_X + 12, cardY + 28);
    tft.print(config.currentPlayerName);

    // Status message
    tft.setTextSize(1);
    tft.setTextColor(SystemConfig::COLOR_STARLIGHT,
                     SystemConfig::COLOR_CARD_BG);
    tft.setCursor(CONTENT_X + 12, cardY + 68);
    tft.print("Status");

    tft.setTextSize(2);
    tft.setTextColor(TFT_RED, SystemConfig::COLOR_CARD_BG);
    tft.setCursor(CONTENT_X + 12, cardY + 82);
    tft.print("Still in play.");

    // Footer
    tft.setTextSize(1);
    tft.setTextColor(SystemConfig::COLOR_STARLIGHT, TFT_BLACK);

    const char *hint = "You can retry this question";
    tft.setCursor(CONTENT_X, SCREEN_H - 12);
    tft.print(hint);

    config.displayState = SystemConfig::SHOW_INCORRECT;
    config.stateStartTime = millis();
}

// ==========================================
// COMPLETION & WINNER
// ==========================================

void DisplayOutputs::showCompletionScreen()
{
    tft.fillScreen(TFT_BLACK);

    const int PAD_X = 10;
    const int CONTENT_X = PAD_X;
    const int CONTENT_W = SCREEN_W - 2 * PAD_X;

    // Title
    tft.setTextSize(2);
    tft.setTextColor(SystemConfig::COLOR_NEBULA_PURPLE, TFT_BLACK);

    const char *title = "ALL DONE!";
    tft.setCursor(centerX(strlen(title), 2), 16);
    tft.print(title);

    tft.drawLine(CONTENT_X, 36, SCREEN_W - PAD_X, 36,
                 SystemConfig::COLOR_DEEP_INDIGO);

    // Status
    tft.setTextSize(1);
    tft.setTextColor(SystemConfig::COLOR_NEBULA_PURPLE, TFT_BLACK);

    const char *status = "Quiz finished";
    tft.setCursor(CONTENT_X, 46);
    tft.print(status);

    // Stats card — overall score
    const int cardY = 66;
    const int cardH = 60;

    tft.fillRect(CONTENT_X, cardY, CONTENT_W, cardH,
                 SystemConfig::COLOR_CARD_BG);
    tft.drawRect(CONTENT_X, cardY, CONTENT_W, cardH,
                 SystemConfig::COLOR_NEBULA_PURPLE);

    tft.setTextSize(1);
    tft.setTextColor(SystemConfig::COLOR_STARLIGHT,
                     SystemConfig::COLOR_CARD_BG);
    tft.setCursor(CONTENT_X + 12, cardY + 10);
    tft.print("Overall");

    char scoreBuf[32];
    snprintf(scoreBuf, sizeof(scoreBuf), "%d / %d",
             config.overallScore, (int)config.questionList.size());

    tft.setTextSize(3);
    tft.setTextColor(TFT_WHITE, SystemConfig::COLOR_CARD_BG);
    tft.setCursor(CONTENT_X + 12, cardY + 24);
    tft.print(scoreBuf);

    // Winner card
    const int winY = 136;
    const int winH = 66;

    tft.fillRect(CONTENT_X, winY, CONTENT_W, winH,
                 SystemConfig::COLOR_CARD_BG);
    tft.drawRect(CONTENT_X, winY, CONTENT_W, winH, 0xFE60);

    int highestScore = 0;
    String topPlayer = "";
    for (int i = 0; i < (int)config.playerScores.size(); i++)
    {
        if (config.playerScores[i].score > highestScore)
        {
            highestScore = config.playerScores[i].score;
            topPlayer = config.playerScores[i].name;
        }
    }

    tft.setTextSize(1);
    tft.setTextColor(SystemConfig::COLOR_STARLIGHT,
                     SystemConfig::COLOR_CARD_BG);
    tft.setCursor(CONTENT_X + 12, winY + 10);
    tft.print("Winner");

    if (topPlayer != "")
    {
        tft.setTextSize(2);
        tft.setTextColor(0xFE60, SystemConfig::COLOR_CARD_BG);
        tft.setCursor(CONTENT_X + 12, winY + 24);
        tft.print(topPlayer);

        char ptsBuf[16];
        snprintf(ptsBuf, sizeof(ptsBuf), "%d pts", highestScore);
        tft.setTextSize(3);
        tft.setTextColor(TFT_WHITE, SystemConfig::COLOR_CARD_BG);
        tft.setCursor(CONTENT_X + 12, winY + 44);
        tft.print(ptsBuf);
    }
    else
    {
        tft.setTextSize(2);
        tft.setTextColor(TFT_RED, SystemConfig::COLOR_CARD_BG);
        tft.setCursor(CONTENT_X + 12, winY + 28);
        tft.print("No winner found");
    }

    // Footer
    tft.setTextSize(1);
    tft.setTextColor(SystemConfig::COLOR_STARLIGHT, TFT_BLACK);

    const char *hint = "Press START for leaderboard";
    tft.setCursor(CONTENT_X, SCREEN_H - 12);
    tft.print(hint);

    config.displayState = SystemConfig::SHOW_COMPLETE;
}

// ==========================================
// LEADERBOARD
// ==========================================

void DisplayOutputs::showLeaderboardScreen()
{
    helper.sortPlayersByScore();
    tft.fillScreen(TFT_BLACK);

    if (config.playerScores.empty())
    {
        tft.setTextSize(2);
        tft.setTextColor(TFT_RED, TFT_BLACK);
        const char *msg = "No scores available.";
        tft.setCursor(centerX(strlen(msg), 2), 110);
        tft.print(msg);
        config.displayState = SystemConfig::SHOW_LEADERBOARD;
        return;
    }

    // Title bar
    tft.fillRect(0, 0, SCREEN_W, 32, SystemConfig::COLOR_DEEP_INDIGO);
    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE, SystemConfig::COLOR_DEEP_INDIGO);
    const char *title = "LEADERBOARD";
    tft.setCursor(centerX(strlen(title), 2), 5);
    tft.print(title);

    tft.drawFastHLine(0, 32, SCREEN_W, SystemConfig::COLOR_NEBULA_PURPLE);
    tft.drawFastHLine(0, 33, SCREEN_W, 0x5A6F);

    // Row layout
    const int rowTop = 44;
    const int rowH = 36;
    const int maxRows = (SCREEN_H - rowTop - 24) / rowH;

    for (int i = 0; i < config.getMaxPlayer() && i < maxRows; i++)
    {
        const auto &player = config.playerScores[i];
        const int yPos = rowTop + i * rowH;

        const bool isTop3 = (i < 3);
        const bool isCurrent = (player.name == config.currentPlayerName);
        const bool isEliminated = player.isEliminated;

        uint16_t medalColor = rankColorFor(i);

        // Medal-tinted backgrounds for top 3, Card BG for current
        uint16_t rowBg = TFT_BLACK;

        if (isTop3 && !isEliminated)
        {
            rowBg = (i == 0)   ? 0x2965  // dark gold
                    : (i == 1) ? 0x39E7  // dark silver
                               : 0x3186; // dark bronze
        }
        else if (isCurrent)
        {
            rowBg = SystemConfig::COLOR_CARD_BG;
        }

        if (isTop3 || isCurrent)
        {
            tft.fillRect(4, yPos - 2, SCREEN_W - 8, rowH - 2, rowBg);
        }

        // Gold border ring around #1
        if (i == 0 && !isEliminated)
        {
            tft.drawRect(4, yPos - 2, SCREEN_W - 8, rowH - 2, medalColor);
        }

        // Medal icon for top 3, plain rank number otherwise
        if (isTop3 && !isEliminated)
        {
            drawMedalIcon(22, yPos + rowH / 2 - 2, medalColor);
        }
        else
        {
            tft.setTextSize(2);
            tft.setTextColor(isEliminated ? TFT_RED : TFT_LIGHTGREY, rowBg);
            tft.setCursor(18, yPos + 8);
            tft.printf("%d", i + 1);
        }

        // Rank label ("1st", "2nd", "3rd" — only for top 3)
        tft.setTextSize(2);
        uint16_t rankTextColor = isEliminated ? TFT_RED
                                 : isTop3     ? medalColor
                                              : TFT_LIGHTGREY;
        tft.setTextColor(rankTextColor, rowBg);

        const char *rankLbl = (i == 0)   ? "1st"
                              : (i == 1) ? "2nd"
                              : (i == 2) ? "3rd"
                                         : nullptr;

        if (rankLbl)
        {
            tft.setCursor(44, yPos + 8);
            tft.print(rankLbl);
        }

        // Player name (truncated with "." if >10 chars)
        uint16_t nameColor = isEliminated ? TFT_RED
                             : isCurrent  ? TFT_WHITE
                             : isTop3     ? TFT_WHITE
                                          : TFT_LIGHTGREY;

        String name = player.name.isEmpty() ? "Player" : player.name;
        if (name.length() > 10)
            name = name.substring(0, 9) + ".";

        tft.setTextColor(nameColor, rowBg);
        tft.setCursor(100, yPos + 8);
        tft.print(name.c_str());

        // Score — right-aligned
        tft.setTextSize(2);
        tft.setTextColor(isEliminated ? TFT_RED : TFT_WHITE, rowBg);

        char scoreBuf[8];
        snprintf(scoreBuf, sizeof(scoreBuf), "%d", player.score);
        int scoreW = strlen(scoreBuf) * 12; // size 2 = 12 px/char
        int scoreX = SCREEN_W - 10 - scoreW;

        if (isEliminated)
            scoreX -= 46;

        tft.setCursor(scoreX, yPos + 8);
        tft.print(scoreBuf);

        // Eliminated tag — append deceiver's name if someone deceived them
        if (isEliminated)
        {
            tft.setTextSize(1);
            tft.setTextColor(TFT_RED, rowBg);
            tft.setCursor(SCREEN_W - 44, yPos + 12);

            // Find who deceived this player (any player whose betrayedName
            // matches the current player)
            String deceiver = "";
            for (const auto &q : config.playerScores)
            {
                if (q.betrayedName == player.name && q.name != player.name)
                {
                    deceiver = q.name;
                    break;
                }
            }

            if (deceiver.length() > 0)
            {
                // Truncate so the tag stays on one line
                if (deceiver.length() > 8)
                {
                    deceiver = deceiver.substring(0, 7) + ".";
                }

                String tag = "(Elim by " + deceiver + ")";
                tft.print(tag);
            }
            else
            {
                tft.print("(Elim)");
            }
        }
    }

    // Overflow footer
    const int totalPlayers = static_cast<int>(config.playerScores.size());
    if (totalPlayers > maxRows)
    {
        tft.setTextSize(1);
        tft.setTextColor(TFT_YELLOW, TFT_BLACK);
        tft.setCursor(10, SCREEN_H - 12);
        tft.printf("+%d more players", totalPlayers - maxRows);
    }

    // Prompt
    tft.setTextSize(1);
    tft.setTextColor(SystemConfig::COLOR_STARLIGHT, TFT_BLACK);
    const char *prompt = "Press start to continue";
    tft.setCursor(centerX(strlen(prompt), 2), SCREEN_H - 22);
    tft.print(prompt);

    config.displayState = SystemConfig::SHOW_LEADERBOARD;
}

// ==========================================
// LEADERBOARD ICONS
// ==========================================

void DisplayOutputs::drawMedalIcon(int cx, int cy, uint16_t medalColor)
{
    // Brighter highlight for top-left arc
    uint8_t hr = (medalColor >> 11) & 0x1F;
    uint8_t hg = (medalColor >> 5) & 0x3F;
    uint8_t hb = medalColor & 0x1F;
    hr = (hr + 4) > 0x1F ? 0x1F : hr + 4;
    hg = (hg + 8) > 0x3F ? 0x3F : hg + 8;
    hb = (hb + 4) > 0x1F ? 0x1F : hb + 4;
    uint16_t highlight = (hr << 11) | (hg << 5) | hb;

    // Darker shadow for bottom-right arc
    uint8_t sr = (medalColor >> 11) & 0x1F;
    uint8_t sg = (medalColor >> 5) & 0x3F;
    uint8_t sb = medalColor & 0x1F;
    sr = (sr > 4) ? sr - 4 : 0;
    sg = (sg > 8) ? sg - 8 : 0;
    sb = (sb > 4) ? sb - 4 : 0;
    uint16_t shadow = (sr << 11) | (sg << 5) | sb;

    // Outer black ring
    tft.fillCircle(cx, cy, 14, TFT_BLACK);

    // Main medal body
    tft.fillCircle(cx, cy, 13, medalColor);

    // Highlight arc (top-left, offset -1)
    tft.drawCircle(cx - 1, cy - 1, 11, highlight);

    // Shadow arc (bottom-right, offset +1)
    tft.drawCircle(cx + 1, cy + 1, 11, shadow);

    // White 5-point star emblem
    const int R = 8;
    const int r = 3;

    for (int a = 0; a < 360; a += 72)
    {
        float p1 = (a - 90) * DEG_TO_RAD;
        float p2 = (a - 90 + 36) * DEG_TO_RAD;
        float p3 = (a - 90 + 72) * DEG_TO_RAD;

        tft.fillTriangle(
            cx + R * cos(p1), cy + R * sin(p1),
            cx + r * cos(p2), cy + r * sin(p2),
            cx + R * cos(p3), cy + R * sin(p3),
            TFT_WHITE);
    }
}

uint16_t DisplayOutputs::rankColorFor(int rankIndex)
{
    switch (rankIndex)
    {
    case 0:
        return 0xFE60; // gold
    case 1:
        return 0xC618; // silver
    case 2:
        return 0xCB22; // bronze
    default:
        return TFT_WHITE;
    }
}

String DisplayOutputs::rankLabelFor(int rankIndex)
{
    switch (rankIndex)
    {
    case 0:
        return "Top 1";
    case 1:
        return "Top 2";
    case 2:
        return "Top 3";
    default:
        return String(rankIndex + 1) + ".";
    }
}

// ==========================================
// EFFECT SCREENS
// ==========================================

void DisplayOutputs::showHintScreen(char /*removedOption*/)
{
    tft.fillScreen(TFT_BLACK);

    const int PAD_X = 10;
    const int CONTENT_X = PAD_X;
    const int CONTENT_W = SCREEN_W - 2 * PAD_X;

    // Title
    tft.setTextSize(2);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);

    const char *title = "HINT";
    tft.setCursor(centerX(strlen(title), 2), 16);
    tft.print(title);

    tft.drawLine(CONTENT_X, 36, SCREEN_W - PAD_X, 36,
                 SystemConfig::COLOR_DEEP_INDIGO);

    // Status
    tft.setTextSize(1);
    tft.setTextColor(SystemConfig::COLOR_NEBULA_PURPLE, TFT_BLACK);

    const char *status = "A clue has been revealed";
    tft.setCursor(CONTENT_X, 46);
    tft.print(status);

    // Message card
    const int cardY = 66;
    const int cardH = 70;

    tft.fillRect(CONTENT_X, cardY, CONTENT_W, cardH,
                 SystemConfig::COLOR_CARD_BG);
    tft.drawRect(CONTENT_X, cardY, CONTENT_W, cardH,
                 SystemConfig::COLOR_NEBULA_PURPLE);

    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE, SystemConfig::COLOR_CARD_BG);

    const char *line1 = "One wrong answer";
    const char *line2 = "has been removed!";

    tft.setCursor(CONTENT_X + 10, cardY + 16);
    tft.print(line1);

    tft.setCursor(CONTENT_X + 10, cardY + 40);
    tft.print(line2);

    // Footer
    tft.setTextSize(1);
    tft.setTextColor(SystemConfig::COLOR_STARLIGHT, TFT_BLACK);

    const char *hint = "Tap A / B / C / D to answer";
    tft.setCursor(CONTENT_X, SCREEN_H - 12);
    tft.print(hint);
}

void DisplayOutputs::showPassToPlayerScreen(const String &currentPlayer,
                                            const String &newPlayer)
{
    tft.fillScreen(TFT_BLACK);

    const int PAD_X = 10;
    const int CONTENT_X = PAD_X;
    const int CONTENT_W = SCREEN_W - 2 * PAD_X;

    // Title
    tft.setTextSize(2);
    tft.setTextColor(SystemConfig::COLOR_NEBULA_PURPLE, TFT_BLACK);

    const char *title = "HANDED OFF";
    tft.setCursor(centerX(strlen(title), 2), 16);
    tft.print(title);

    tft.drawLine(CONTENT_X, 36, SCREEN_W - PAD_X, 36,
                 SystemConfig::COLOR_DEEP_INDIGO);

    // Status
    tft.setTextSize(1);
    tft.setTextColor(SystemConfig::COLOR_NEBULA_PURPLE, TFT_BLACK);

    const char *status = "Question transferred";
    tft.setCursor(CONTENT_X, 46);
    tft.print(status);

    // Message card
    const int cardY = 66;
    const int cardH = 90;

    tft.fillRect(CONTENT_X, cardY, CONTENT_W, cardH,
                 SystemConfig::COLOR_CARD_BG);
    tft.drawRect(CONTENT_X, cardY, CONTENT_W, cardH,
                 SystemConfig::COLOR_NEBULA_PURPLE);

    // From
    tft.setTextSize(1);
    tft.setTextColor(SystemConfig::COLOR_STARLIGHT,
                     SystemConfig::COLOR_CARD_BG);
    tft.setCursor(CONTENT_X + 10, cardY + 12);
    tft.print("From");

    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE, SystemConfig::COLOR_CARD_BG);
    tft.setCursor(CONTENT_X + 10, cardY + 24);
    tft.print(currentPlayer);

    // To
    tft.setTextSize(1);
    tft.setTextColor(SystemConfig::COLOR_STARLIGHT,
                     SystemConfig::COLOR_CARD_BG);
    tft.setCursor(CONTENT_X + 10, cardY + 54);
    tft.print("To");

    tft.setTextSize(2);
    tft.setTextColor(SystemConfig::COLOR_NEBULA_PURPLE,
                     SystemConfig::COLOR_CARD_BG);
    tft.setCursor(CONTENT_X + 10, cardY + 66);
    tft.print(newPlayer);

    // Footer
    tft.setTextSize(1);
    tft.setTextColor(SystemConfig::COLOR_STARLIGHT, TFT_BLACK);

    const char *hint = "The question is now with the new player";
    tft.setCursor(CONTENT_X, SCREEN_H - 12);
    tft.print(hint);
}

void DisplayOutputs::showDeductPointsScreen(const String &playerName,
                                            int pointsDeducted)
{
    tft.fillScreen(TFT_BLACK);

    const int PAD_X = 10;
    const int CONTENT_X = PAD_X;
    const int CONTENT_W = SCREEN_W - 2 * PAD_X;

    // Title
    tft.setTextSize(2);
    tft.setTextColor(TFT_RED, TFT_BLACK);

    const char *title = "POINTS LOST";
    tft.setCursor(centerX(strlen(title), 2), 16);
    tft.print(title);

    tft.drawLine(CONTENT_X, 36, SCREEN_W - PAD_X, 36,
                 SystemConfig::COLOR_DEEP_INDIGO);

    // Status
    tft.setTextSize(1);
    tft.setTextColor(TFT_RED, TFT_BLACK);

    const char *status = "Bad luck penalty";
    tft.setCursor(CONTENT_X, 46);
    tft.print(status);

    // Message card
    const int cardY = 66;
    const int cardH = 90;

    tft.fillRect(CONTENT_X, cardY, CONTENT_W, cardH,
                 SystemConfig::COLOR_CARD_BG);
    tft.drawRect(CONTENT_X, cardY, CONTENT_W, cardH, TFT_RED);

    // Player
    tft.setTextSize(1);
    tft.setTextColor(SystemConfig::COLOR_STARLIGHT,
                     SystemConfig::COLOR_CARD_BG);
    tft.setCursor(CONTENT_X + 10, cardY + 12);
    tft.print("Player");

    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE, SystemConfig::COLOR_CARD_BG);
    tft.setCursor(CONTENT_X + 10, cardY + 24);
    tft.print(playerName);

    // Points lost
    tft.setTextSize(1);
    tft.setTextColor(SystemConfig::COLOR_STARLIGHT,
                     SystemConfig::COLOR_CARD_BG);
    tft.setCursor(CONTENT_X + 10, cardY + 54);
    tft.print("Lost");

    tft.setTextSize(3);
    tft.setTextColor(TFT_RED, SystemConfig::COLOR_CARD_BG);

    char pts[16];
    snprintf(pts, sizeof(pts), "-%d pts", pointsDeducted);
    tft.setCursor(CONTENT_X + 10, cardY + 62);
    tft.print(pts);

    // Footer
    tft.setTextSize(1);
    tft.setTextColor(SystemConfig::COLOR_STARLIGHT, TFT_BLACK);

    const char *hint = "Keep going — you can recover!";
    tft.setCursor(CONTENT_X, SCREEN_H - 12);
    tft.print(hint);
}

void DisplayOutputs::showIncrementQuestionScreen()
{
    tft.fillScreen(TFT_BLACK);

    const int PAD_X = 10;
    const int CONTENT_X = PAD_X;
    const int CONTENT_W = SCREEN_W - 2 * PAD_X;

    // Title
    tft.setTextSize(2);
    tft.setTextColor(SystemConfig::COLOR_DEEP_INDIGO, TFT_BLACK);

    const char *title = "MISFORTUNE";
    tft.setCursor(centerX(strlen(title), 2), 16);
    tft.print(title);

    tft.drawLine(CONTENT_X, 36, SCREEN_W - PAD_X, 36,
                 SystemConfig::COLOR_DEEP_INDIGO);

    // Status
    tft.setTextSize(1);
    tft.setTextColor(TFT_RED, TFT_BLACK);

    const char *status = "Penalty question inserted";
    tft.setCursor(CONTENT_X, 46);
    tft.print(status);

    // Message card
    const int cardY = 66;
    const int cardH = 90;

    tft.fillRect(CONTENT_X, cardY, CONTENT_W, cardH,
                 SystemConfig::COLOR_CARD_BG);
    tft.drawRect(CONTENT_X, cardY, CONTENT_W, cardH, TFT_RED);

    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE, SystemConfig::COLOR_CARD_BG);

    const char *line1 = "An extra question";
    const char *line2 = "has been added.";

    tft.setCursor(CONTENT_X + 10, cardY + 20);
    tft.print(line1);

    tft.setCursor(CONTENT_X + 10, cardY + 46);
    tft.print(line2);

    // Footer
    tft.setTextSize(1);
    tft.setTextColor(SystemConfig::COLOR_STARLIGHT, TFT_BLACK);

    const char *hint = "Answer correctly to clear the penalty";
    tft.setCursor(CONTENT_X, SCREEN_H - 12);
    tft.print(hint);
}

// ==========================================
// SAVIOR SELECTION
// ==========================================

void DisplayOutputs::showListPossibleSaviorScreen()
{
    SystemConfig &cfg = SystemConfig::get();

    tft.fillScreen(TFT_BLACK);

    const int PAD_X = 10;
    const int CONTENT_X = PAD_X;
    const int CONTENT_W = SCREEN_W - 2 * PAD_X;

    // Title
    tft.setTextSize(2);
    tft.setTextColor(SystemConfig::COLOR_DEEP_INDIGO, TFT_BLACK);

    const char *title = "PICK A SAVIOR";
    tft.setCursor(centerX(strlen(title), 2), 6);
    tft.print(title);

    tft.drawLine(CONTENT_X, 26, SCREEN_W - PAD_X, 26,
                 SystemConfig::COLOR_DEEP_INDIGO);

    // Status
    tft.setTextSize(1);
    tft.setTextColor(SystemConfig::COLOR_NEBULA_PURPLE, TFT_BLACK);

    char status[64];
    snprintf(status, sizeof(status),
             "Do you still think, someone would save you, %s?",
             cfg.currentPlayerName.c_str());

    tft.setCursor(CONTENT_X, 34);
    tft.print(status);

    // Row geometry — tighter to leave room for button bar
    const int rowTop = 46;
    const int rowH = 32;
    const int maxRows = (SCREEN_H - rowTop - 60) / rowH;

    cfg.saviorRowH = rowH;
    cfg.saviorRowCount = 0;

    int playerId = 1;
    int rowIndex = 0;
    int maxDisplay = cfg.getMaxPlayer();

    // Draw one row per eligible player (skip eliminated + current)
    for (int i = 0; i < maxDisplay && rowIndex < maxRows; i++)
    {
        const auto &player = cfg.playerScores[i];

        if (player.isEliminated)
            continue;
        if (player.name == cfg.currentPlayerName)
            continue;

        int yPos = rowTop + rowIndex * rowH;

        cfg.saviorRowY[rowIndex] = yPos;

        RowStyle s;
        s.bg = SystemConfig::COLOR_CARD_BG;
        s.border = TFT_RED;
        s.prefixColor = SystemConfig::COLOR_DEEP_INDIGO;
        s.textColor = TFT_WHITE;
        s.selected = (i == cfg.selectedSaviorIndex);
        s.selectBg = TFT_RED;
        s.textSize = TextSize::Medium;

        String prefix = String(playerId) + ".";
        drawListRow(yPos, prefix, player.name, rowH, s);

        playerId++;
        rowIndex++;
    }

    cfg.saviorRowCount = rowIndex;

    // Empty list guard
    if (playerId == 1)
    {
        tft.setTextSize(2);
        tft.setTextColor(TFT_RED, TFT_BLACK);
        const char *msg = "No eligible saviors";
        tft.setCursor(centerX(strlen(msg), 2), SCREEN_H / 2);
        tft.print(msg);
    }

    drawSaviorButtonBar();
}

void DisplayOutputs::showWouldYouSaveThePlayerScreen()
{
    SystemConfig &cfg = SystemConfig::get();

    tft.fillScreen(TFT_BLACK);

    const int PAD_X = 10;
    const int CONTENT_X = PAD_X;
    const int CONTENT_W = SCREEN_W - 2 * PAD_X;

    // ==========================================
    // TITLE
    // ==========================================
    tft.setTextSize(2);
    tft.setTextColor(SystemConfig::COLOR_DEEP_INDIGO, TFT_BLACK);

    const char *title = "LAST CHANCE";
    tft.setCursor(centerX(strlen(title), 2), 10);
    tft.print(title);

    tft.drawLine(CONTENT_X, 30, SCREEN_W - PAD_X, 30,
                 SystemConfig::COLOR_DEEP_INDIGO);

    // ==========================================
    // WHO'S IN TROUBLE
    // ==========================================
    tft.setTextSize(1);
    tft.setTextColor(TFT_RED, TFT_BLACK);

    char trouble[48];
    snprintf(trouble, sizeof(trouble), "%s is about to be eliminated",
             cfg.currentPlayerName.c_str());
    tft.setCursor(CONTENT_X, 36);
    tft.print(trouble);

    // ==========================================
    // IDENTIFY THE OTHER PLAYER (the potential savior)
    // ==========================================
    String saviorName = "";
    for (const auto &p : cfg.playerScores)
    {
        if (p.isEliminated)
            continue;
        if (p.name == cfg.currentPlayerName)
            continue;
        saviorName = p.name;
        break;
    }

    // ==========================================
    // THE QUESTION
    // ==========================================
    const int cardY = 60;
    const int cardH = 90;

    tft.fillRect(CONTENT_X, cardY, CONTENT_W, cardH,
                 SystemConfig::COLOR_CARD_BG);
    tft.drawRect(CONTENT_X, cardY, CONTENT_W, cardH,
                 SystemConfig::COLOR_NEBULA_PURPLE);

    // Ask directly — no list, no ambiguity
    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE, SystemConfig::COLOR_CARD_BG);

    const char *qLine1 = "Only one player";
    const char *qLine2 = "remains.";

    tft.setCursor(CONTENT_X + 12, cardY + 12);
    tft.print(qLine1);

    tft.setCursor(CONTENT_X + 12, cardY + 36);
    tft.print(qLine2);

    tft.setTextSize(1);
    tft.setTextColor(SystemConfig::COLOR_STARLIGHT,
                     SystemConfig::COLOR_CARD_BG);
    tft.setCursor(CONTENT_X + 12, cardY + 64);
    tft.print("Will you save them?");

    // ==========================================
    // BUTTON GEOMETRY
    // ==========================================
    const int btnW = 110;
    const int btnH = 44;
    const int gap = 20;
    const int totalW = btnW * 2 + gap;
    const int startX = (SCREEN_W - totalW) / 2;
    const int btnY = 164;

    const int yesX = startX;
    const int noX = startX + btnW + gap;

    // ==========================================
    // DRAW BUTTONS — SAVE / REFUSE
    // ==========================================
    auto drawFlatButton = [&](int x, int y, int w, int h,
                              const char *label,
                              uint16_t borderColor,
                              uint16_t textColor)
    {
        tft.fillRect(x, y, w, h, SystemConfig::COLOR_CARD_BG);

        // 2 px border for emphasis
        tft.drawRect(x, y, w, h, borderColor);
        tft.drawRect(x + 1, y + 1, w - 2, h - 2, borderColor);

        // Centered label
        tft.setTextSize(2);
        int labelW = strlen(label) * 12; // size 2 = 12 px/char
        int labelX = x + (w - labelW) / 2;
        int labelY = y + (h - 16) / 2;

        tft.setTextColor(textColor, SystemConfig::COLOR_CARD_BG);
        tft.setCursor(labelX, labelY);
        tft.print(label);
    };

    drawFlatButton(yesX, btnY, btnW, btnH, "SAVE",
                   SystemConfig::COLOR_NEBULA_PURPLE,
                   TFT_WHITE);

    drawFlatButton(noX, btnY, btnW, btnH, "REFUSE",
                   TFT_RED,
                   TFT_RED);

    // ==========================================
    // SAVE GEOMETRY FOR TOUCH HANDLING
    // ==========================================
    // Reuse the existing new-session button geometry fields so the
    // touch handler doesn't need new state.
    cfg.newSessionYesX = yesX;
    cfg.newSessionNoX = noX;
    cfg.newSessionBtnY = btnY;
    cfg.newSessionBtnW = btnW;
    cfg.newSessionBtnH = btnH;

    // ==========================================
    // FOOTER
    // ==========================================
    tft.setTextSize(1);
    tft.setTextColor(SystemConfig::COLOR_STARLIGHT, TFT_BLACK);

    char hint[48];
    snprintf(hint, sizeof(hint), "%s, the choice is yours",
             saviorName.isEmpty() ? "Player" : saviorName.c_str());
    tft.setCursor(CONTENT_X, SCREEN_H - 12);
    tft.print(hint);

    // New display state
    config.displayState = SystemConfig::SHOW_WOULD_YOU_SAVE;
}

void DisplayOutputs::drawSaviorButtonBar()
{
    SystemConfig &cfg = SystemConfig::get();

    const int PAD_X = 10;
    const int CONTENT_X = PAD_X;
    const int CONTENT_W = SCREEN_W - 2 * PAD_X;

    // Bar geometry — 3 buttons split evenly with 6 px gaps
    const int barH = 34;
    const int gap = 6;
    const int barY = SCREEN_H - barH - 8;

    const int btnW = (CONTENT_W - gap * 2) / 3;
    const int btnH = barH;

    const int electX = CONTENT_X;
    const int nextX = CONTENT_X + btnW + gap;
    const int skipX = CONTENT_X + (btnW + gap) * 2;

    // Save geometry for touch handling
    cfg.saviorBtnBarY = barY;
    cfg.saviorBtnElectX = electX;
    cfg.saviorBtnNextX = nextX;
    cfg.saviorBtnSkipX = skipX;
    cfg.saviorBtnW = btnW;
    cfg.saviorBtnH = btnH;

    // Legacy fields
    cfg.saviorFooterY = barY;
    cfg.saviorFooterSplit1 = nextX;
    cfg.saviorFooterSplit2 = skipX;

    // Pill-shaped buttons — corner radius = half button height
    auto drawRoundedBtn = [&](int x, const char *label,
                              uint16_t borderColor,
                              uint16_t textColor)
    {
        const int radius = btnH / 2;

        tft.fillRoundRect(x, barY, btnW, btnH, radius,
                          SystemConfig::COLOR_CARD_BG);
        tft.drawRoundRect(x, barY, btnW, btnH, radius, borderColor);

        tft.setTextSize(1);
        int labelW = strlen(label) * 6;
        int labelX = x + (btnW - labelW) / 2;
        int labelY = barY + (btnH - 8) / 2;

        tft.setTextColor(textColor, SystemConfig::COLOR_CARD_BG);
        tft.setCursor(labelX, labelY);
        tft.print(label);
    };

    drawRoundedBtn(electX, "Elect",
                   SystemConfig::COLOR_NEBULA_PURPLE,
                   SystemConfig::COLOR_NEBULA_PURPLE);

    drawRoundedBtn(nextX, "Next",
                   SystemConfig::COLOR_DEEP_INDIGO,
                   SystemConfig::COLOR_DEEP_INDIGO);

    drawRoundedBtn(skipX, "Skip",
                   TFT_RED,
                   TFT_RED);
}

// ==========================================
// ELIMINATION SCREENS
// ==========================================

void DisplayOutputs::showNoneEliminatedScreen()
{
    tft.fillScreen(TFT_BLACK);

    const int PAD_X = 10;
    const int CONTENT_X = PAD_X;
    const int CONTENT_W = SCREEN_W - 2 * PAD_X;

    // Title
    tft.setTextSize(2);
    tft.setTextColor(SystemConfig::COLOR_DEEP_INDIGO, TFT_BLACK);

    const char *title = "STILL ALIVE";
    tft.setCursor(centerX(strlen(title), 2), 16);
    tft.print(title);

    tft.drawLine(CONTENT_X, 36, SCREEN_W - PAD_X, 36,
                 SystemConfig::COLOR_DEEP_INDIGO);

    // Status
    tft.setTextSize(1);
    tft.setTextColor(SystemConfig::COLOR_NEBULA_PURPLE, TFT_BLACK);

    const char *status = "Revival successful";
    tft.setCursor(CONTENT_X, 46);
    tft.print(status);

    // Message card
    const int cardY = 66;
    const int cardH = 100;

    tft.fillRect(CONTENT_X, cardY, CONTENT_W, cardH,
                 SystemConfig::COLOR_CARD_BG);
    tft.drawRect(CONTENT_X, cardY, CONTENT_W, cardH,
                 SystemConfig::COLOR_NEBULA_PURPLE);

    tft.setTextSize(3);
    tft.setTextColor(TFT_WHITE, SystemConfig::COLOR_CARD_BG);

    const char *line1 = "Congrats!";
    tft.setCursor(CONTENT_X + 12, cardY + 14);
    tft.print(line1);

    tft.setTextSize(2);
    tft.setTextColor(SystemConfig::COLOR_STARLIGHT,
                     SystemConfig::COLOR_CARD_BG);

    const char *line2 = "No one has been";
    const char *line3 = "eliminated.";

    tft.setCursor(CONTENT_X + 12, cardY + 48);
    tft.print(line2);

    tft.setCursor(CONTENT_X + 12, cardY + 68);
    tft.print(line3);

    // Footer
    tft.setTextSize(1);
    tft.setTextColor(SystemConfig::COLOR_STARLIGHT, TFT_BLACK);

    const char *hint = "Continuing the game...";
    tft.setCursor(CONTENT_X, SCREEN_H - 12);
    tft.print(hint);

    delay(1500);
}

void DisplayOutputs::showEliminatedPlayerScreen(const String &playerName)
{
    // ==========================================
    // REASON MESSAGE (fallback)
    // ==========================================
    String message = "Eliminated!";
    switch (config.eliminationReason)
    {
    case SystemConfig::ELIM_DEDUCT_POINTS:
        message = "My points... It's gone...";
        break;
    case SystemConfig::ELIM_PENALTY_QUESTION:
        message = "Why it's so difficult?!";
        break;
    case SystemConfig::ELIM_TASK_FAILED:
        message = "I forgot the task!";
        break;
    case SystemConfig::ELIM_SACRIFICIAL_CONS:
        message = "I'm too kind to save them...";
        break;
    case SystemConfig::ELIM_REFUSE_TO_REVIVE:
        message = "Mark my word. I shall return!";
        break;
    case SystemConfig::ELIM_UNKNOWN:
    default:
        message = "Eliminated!";
        break;
    }

    // ==========================================
    // BETRAYAL ANALYSIS
    // ==========================================
    // Two questions to answer:
    //   1. Did anyone betray this player?     → betrayedBy
    //   2. Did this player betray anyone?     → betrayedWho
    //
    // The betrayal record lives on the PERPETRATOR's PlayerScore,
    // in a `betrayedName` field. See GameMechanics::recordBetrayal.

    String betrayedBy = "";  // name of who betrayed this player
    String betrayedWho = ""; // name of who this player betrayed

    // Pass 1 — did anyone betray this player?
    for (const auto &p : config.playerScores)
    {
        if (p.betrayedName == playerName && p.name != playerName)
        {
            betrayedBy = p.name;
            break;
        }
    }

    // Pass 2 — did this player betray anyone?
    for (const auto &p : config.playerScores)
    {
        if (p.name == playerName && p.betrayedName.length() > 0)
        {
            betrayedWho = p.betrayedName;
            break;
        }
    }

    // ==========================================
    // MESSAGE OVERRIDE
    // ==========================================
    // Priority order:
    //   1. This player betrayed someone — the last thing they did
    //      is the story the screen should tell.
    //   2. Someone betrayed this player — the victim's angle.
    //   3. No betrayal — fall back to the mechanical reason.

    const bool isPerpetrator = betrayedWho.length() > 0;
    const bool isVictim = betrayedBy.length() > 0;

    if (isPerpetrator)
    {
        message = "Too bad... How naive, " + betrayedWho + ".";
    }
    else if (isVictim)
    {
        message = "Liar. I'll remember this, " + betrayedBy + ".";
    }

    // ==========================================
    // SCREEN RENDER
    // ==========================================
    tft.fillScreen(TFT_BLACK);

    const int PAD_X = 10;
    const int CONTENT_X = PAD_X;
    const int CONTENT_W = SCREEN_W - 2 * PAD_X;

    // Title — Red for negative outcome
    tft.setTextSize(2);
    tft.setTextColor(TFT_RED, TFT_BLACK);

    const char *title = "ELIMINATED";
    tft.setCursor(centerX(strlen(title), 2), 16);
    tft.print(title);

    tft.drawLine(CONTENT_X, 36, SCREEN_W - PAD_X, 36, TFT_RED);

    // Status line — describes the role
    tft.setTextSize(1);
    tft.setTextColor(SystemConfig::COLOR_NEBULA_PURPLE, TFT_BLACK);

    const char *status =
        isPerpetrator ? "You broke a promise"
        : isVictim    ? "Your teammate refused to save you"
                      : "Player is out of the game";
    tft.setCursor(CONTENT_X, 46);
    tft.print(status);

    // Card
    const int cardY = 66;
    const int cardH = 110;

    tft.fillRect(CONTENT_X, cardY, CONTENT_W, cardH,
                 SystemConfig::COLOR_CARD_BG);
    tft.drawRect(CONTENT_X, cardY, CONTENT_W, cardH, TFT_RED);

    // Player name
    tft.setTextSize(1);
    tft.setTextColor(SystemConfig::COLOR_STARLIGHT,
                     SystemConfig::COLOR_CARD_BG);
    tft.setCursor(CONTENT_X + 12, cardY + 12);
    tft.print("Player");

    tft.setTextSize(3);
    tft.setTextColor(TFT_RED, SystemConfig::COLOR_CARD_BG);
    tft.setCursor(CONTENT_X + 12, cardY + 24);
    tft.print(playerName);

    tft.drawFastHLine(CONTENT_X + 12, cardY + 60,
                      CONTENT_W - 24, 0x4208);

    // Message
    tft.setTextSize(1);
    tft.setTextColor(SystemConfig::COLOR_STARLIGHT,
                     SystemConfig::COLOR_CARD_BG);
    tft.setCursor(CONTENT_X + 12, cardY + 68);
    tft.print("Message from the character");

    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE, SystemConfig::COLOR_CARD_BG);

    int msgY = cardY + 82;
    wrapText(message, CONTENT_X + 12, msgY,
             CONTENT_W - 24, cardY + cardH - 6,
             14, TFT_WHITE, 2);

    // Footer
    tft.setTextSize(1);
    tft.setTextColor(SystemConfig::COLOR_STARLIGHT, TFT_BLACK);

    const char *hint =
        isPerpetrator ? "The game remembers what you did"
        : isVictim    ? "The game continues — remember this"
                      : "The game continues with the remaining players";
    tft.setCursor(CONTENT_X, SCREEN_H - 12);
    tft.print(hint);
}

// ==========================================
// UTILITY SCREENS
// ==========================================

void DisplayOutputs::showNoQuestionsScreen()
{
    tft.fillScreen(TFT_BLACK);

    drawBorderedBox(30, 30, SCREEN_W - 60, 60, TFT_RED, TFT_RED);

    tft.setTextSize(3);
    tft.setTextColor(TFT_BLACK);
    tft.setCursor(centerX(6, 3), 45);
    tft.print("ERROR!");

    tft.setTextSize(2);
    tft.setTextColor(TFT_YELLOW);
    tft.setCursor(centerX(18, 2), 120);
    tft.print("No Questions Found!");

    tft.setTextColor(TFT_WHITE);
    tft.setCursor(centerX(20, 2), 160);
    tft.print("Please add questions");

    tft.setCursor(centerX(20, 2), 190);
    tft.print("and restart the game.");

    config.displayState = SystemConfig::SHOW_COMPLETE;
}

void DisplayOutputs::showMessage(const String &title, const String &message,
                                 uint16_t color)
{
    tft.fillScreen(TFT_BLACK);

    drawBorderedBox(20, 20, SCREEN_W - 40, 70, color, color);

    tft.setTextSize(3);
    tft.setTextColor(TFT_BLACK);
    tft.setCursor(centerX(title.length(), 3), 45);
    tft.print(title);

    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE);
    tft.setCursor(centerX(message.length(), 2), 130);
    tft.print(message);
}

void DisplayOutputs::clearDisplay()
{
    tft.fillScreen(TFT_BLACK);
}

void DisplayOutputs::updateTimer(int seconds)
{
    tft.setTextSize(2);
    tft.setTextColor(TFT_YELLOW);
    tft.fillRect(SCREEN_W - 80, SCREEN_H - 30, 70, 24, TFT_BLACK);
    tft.setCursor(SCREEN_W - 75, SCREEN_H - 26);
    tft.printf("%ds", seconds);
}

// ==========================================
// HELPERS (PRIVATE)
// ==========================================

int DisplayOutputs::wrapText(const String &text, int x, int &y, int maxWidth,
                             int maxY, int lineHeight, uint16_t textColor,
                             uint8_t size)
{
    int linesPrinted = 0;
    String remaining = text;

    tft.setTextColor(textColor, TFT_BLACK);
    tft.setTextSize(size);

    while (remaining.length() > 0 && (maxY == 0 || y < maxY))
    {
        // Measure how many chars fit in maxWidth
        int charsFit = 0;
        int width = 0;

        for (int i = 0; i < (int)remaining.length(); i++)
        {
            int charW = tft.textWidth(String(remaining[i]));
            if (width + charW > maxWidth)
                break;
            width += charW;
            charsFit++;
        }

        if (charsFit == 0)
            break;

        // Prefer to break at a space
        int splitPos = remaining.lastIndexOf(' ', charsFit);
        if (splitPos <= 0)
            splitPos = charsFit;

        String line = remaining.substring(0, splitPos);
        remaining = remaining.substring(splitPos + 1);

        tft.setCursor(x, y);
        tft.print(line);

        y += lineHeight;
        linesPrinted++;
    }

    return linesPrinted;
}

void DisplayOutputs::drawHeader(const String &text)
{
    tft.fillRect(0, 0, SCREEN_W, 30, TFT_CYAN);
    tft.setTextColor(TFT_BLACK);
    tft.setTextSize(2);
    tft.setCursor(8, 6);
    tft.print(text);
}

void DisplayOutputs::drawSeparator(int y)
{
    tft.drawLine(8, y, SCREEN_W - 8, y, TFT_BLUE);
}

void DisplayOutputs::drawBorderedBox(int x, int y, int w, int h,
                                     uint16_t color, uint16_t fillColor)
{
    // 0xFFFF sentinel means "no fill"
    if (fillColor != (uint16_t)-1 && fillColor != 0xFFFF)
    {
        tft.fillRect(x, y, w, h, fillColor);
    }
    tft.drawRect(x, y, w, h, color);
}

int DisplayOutputs::getPlayerScore(const String &playerName)
{
    for (int i = 0; i < (int)config.playerScores.size(); i++)
    {
        if (config.playerScores[i].name == playerName)
            return config.playerScores[i].score;
    }
    return 0;
}