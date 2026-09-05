/**
 * @file inputs.cpp
 * @brief Input handling implementation with smart button gestures
 */
#include "inputs.h"

Inputs &Inputs::get()
{
    static Inputs instance;
    return instance;
}

void Inputs::initializeInputsPins()
{
    Serial.println("🔘 Initializing input pins...");

    pinMode(SystemConfig::INPUT_A, INPUT_PULLUP);
    pinMode(SystemConfig::INPUT_B, INPUT_PULLUP);
    pinMode(SystemConfig::INPUT_C, INPUT_PULLUP);
    pinMode(SystemConfig::INPUT_D, INPUT_PULLUP);
    pinMode(SystemConfig::INPUT_START, INPUT_PULLUP);

    Serial.println("✅ Input pins initialized!");
}

Inputs::ButtonEvent Inputs::processSmartButton()
{
    const unsigned long currentTime = millis();
    const bool rawPressed = (digitalRead(SystemConfig::INPUT_START) == LOW);
    ButtonEvent eventToReturn = ButtonEvent::NONE;

    // 1. Debounced edge detection
    if (currentTime - lastButtonPressTime[4] > debounceDelay)
    {
        // Falling edge: Button pressed down
        if (rawPressed && !lastStartButtonState)
        {
            lastStartButtonState = true;
            lastButtonPressTime[4] = currentTime;

            buttonPressStartTime = currentTime;
            longPressHandled = false;
        }
        // Rising edge: Button released
        else if (!rawPressed && lastStartButtonState)
        {
            lastStartButtonState = false;
            lastButtonPressTime[4] = currentTime;

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

    // 2. Long press evaluation (Triggers while held)
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

    // 3. Single click timeout evaluation (Fires after click window expires)
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

char Inputs::choicesButtonProcessor()
{
    SystemConfig &config = SystemConfig::get();
    char ans = ' ';
    const unsigned long currentTime = millis();

    if (config.displayState == SystemConfig::SHOW_QUESTION &&
        !config.answered &&
        config.currentQuestionPos < static_cast<int>(config.questionOrder.size()))
    {
        if (digitalRead(SystemConfig::INPUT_A) == LOW &&
            currentTime - lastButtonPressTime[0] > debounceDelay)
        {
            lastButtonPressTime[0] = currentTime;
            ans = 'A';

            if (currentTime - lastAPressTime < DOUBLE_CLICK_DELAY)
            {
                aPressCount++;
                if (aPressCount >= 2)
                {
                    aDoubleClickDetected = true;
                }
            }
            else
            {
                aPressCount = 1;
            }
            lastAPressTime = currentTime;
        }
        else if (digitalRead(SystemConfig::INPUT_B) == LOW &&
                 currentTime - lastButtonPressTime[1] > debounceDelay)
        {
            lastButtonPressTime[1] = currentTime;
            ans = 'B';
        }
        else if (digitalRead(SystemConfig::INPUT_C) == LOW &&
                 currentTime - lastButtonPressTime[2] > debounceDelay)
        {
            lastButtonPressTime[2] = currentTime;
            ans = 'C';
        }
        else if (digitalRead(SystemConfig::INPUT_D) == LOW &&
                 currentTime - lastButtonPressTime[3] > debounceDelay)
        {
            lastButtonPressTime[3] = currentTime;
            ans = 'D';
        }

        if (!aDoubleClickDetected && currentTime - lastAPressTime > DOUBLE_CLICK_DELAY)
        {
            aPressCount = 0;
        }
    }

    return ans;
}