/**
 * @file inputs.h
 * @brief Input handling class for button management
 */
#ifndef INPUTS_H
#define INPUTS_H

#include <Arduino.h>
#include "system/config.h"

class Inputs
{
public:
    enum class ButtonEvent
    {
        NONE,
        SINGLE_CLICK,
        DOUBLE_CLICK,
        LONG_PRESS
    };

    /**
     * @brief Singleton accessor
     */
    static Inputs &get();

    /**
     * @brief Configures GPIO pin modes for all inputs
     */
    void initializeInputsPins();

    /**
     * @brief Processes choice buttons (A, B, C, D)
     * @return Character representing selected choice ('A', 'B', 'C', 'D', or ' ')
     */
    char choicesButtonProcessor();

    /**
     * @brief Processes smart button gesture events (Single, Double, Long Press)
     * @return ButtonEvent Enum value representing the detected gesture
     */
    ButtonEvent processSmartButton();

    unsigned long getLastDebounceTime() const { return lastDebounceTime; }
    void setLastDebounceTime(unsigned long time) { lastDebounceTime = time; }

private:
    Inputs() = default;
    ~Inputs() = default;

    Inputs(const Inputs &) = delete;
    Inputs &operator=(const Inputs &) = delete;

    // Timing constants
    const unsigned long debounceDelay = 50;       // Debounce interval (ms)
    const unsigned long DOUBLE_CLICK_DELAY = 300; // Window to wait for 2nd click (ms)
    const unsigned long LONG_PRESS_DELAY = 1000;  // Hold duration for long press (ms)

    // Debounce tracking arrays
    unsigned long lastDebounceTime = 0;
    unsigned long lastButtonPressTime[5] = {0, 0, 0, 0, 0};

    // Smart button gesture state tracking
    unsigned long buttonPressStartTime = 0;
    unsigned long lastClickReleaseTime = 0;
    int clickCount = 0;
    bool lastStartButtonState = false; // Active state (true = pressed)
    bool longPressHandled = false;     // Suppresses click release after long press

    // Choice button double-click tracking
    unsigned long lastAPressTime = 0;
    int aPressCount = 0;
    bool aDoubleClickDetected = false;
};

#endif // INPUTS_H