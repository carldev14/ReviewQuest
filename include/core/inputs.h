/**
 * @file core/inputs.h
 * @brief Input handling — physical START button + touchscreen
 */
#ifndef INPUTS_H
#define INPUTS_H

#include <Arduino.h>
#include "system/config.h"

class Inputs
{
public:
    // ==========================================
    // SINGLETON
    // ==========================================
    static Inputs &get();

    // ==========================================
    // EVENTS
    // ==========================================
    enum class ButtonEvent
    {
        NONE,
        SINGLE_CLICK,
        DOUBLE_CLICK,
        LONG_PRESS
    };

    // ==========================================
    // LIFECYCLE
    // ==========================================
    void initializeInputsPins();

    // ==========================================
    // PHYSICAL START BUTTON
    // ==========================================
    ButtonEvent processSmartButton();

    // ==========================================
    // TOUCH INPUTS
    // ==========================================
    char choicesButtonProcessor();      // A/B/C/D from touch
    void choicesButtonSavior();         // elect / next / skip
    void advanceSaviorSelection();
    int  confirmationSessionButton();   // 1 = yes, 2 = no, 0 = nothing

    // ==========================================
    // TOUCH CONFIG
    // ==========================================
    void setTouchMirror(bool mirrorX, bool mirrorY)
    {
        mirrorX_ = mirrorX;
        mirrorY_ = mirrorY;
    }

private:
    // ==========================================
    // HELPERS
    // ==========================================
    char mapTouchToChoice(uint16_t x, uint16_t y);

    // ==========================================
    // MEMBER VARIABLES
    // ==========================================

    // Physical button state
    bool          lastStartButtonState = false;
    int           clickCount           = 0;
    bool          longPressHandled     = false;
    unsigned long buttonPressStartTime = 0;
    unsigned long lastClickReleaseTime = 0;

    static constexpr unsigned long LONG_PRESS_DELAY   = 800;
    static constexpr unsigned long DOUBLE_CLICK_DELAY = 200;
    static constexpr unsigned long debounceDelay      = 100;

    // Touch state
    bool          mirrorX_      = false;
    bool          mirrorY_      = false;
    unsigned long lastTouchTime = 0;

    static constexpr unsigned long TOUCH_DEBOUNCE = 250;

    // Button index / press tracking
    static constexpr int START_INDEX = 4;
    unsigned long lastButtonPressTime[5] = {0, 0, 0, 0, 0};

    Inputs()  = default;
    ~Inputs() = default;

    Inputs(const Inputs &)            = delete;
    Inputs &operator=(const Inputs &) = delete;
};

#endif // INPUTS_H