/**
 * @file core/Actuators.h
 * @brief Actuator control class for LEDs and Buzzer
 */
#ifndef ACTUATORS_H
#define ACTUATORS_H

#include <Arduino.h>
#include "system/config.h"

class Actuators
{
public:
    // ==========================================
    // SINGLETON
    // ==========================================
    static Actuators &get()
    {
        static Actuators instance;
        return instance;
    }

    // ==========================================
    // LIFECYCLE
    // ==========================================
    void initializeActuatorsPins();

    // ==========================================
    // FEEDBACK ACTIONS
    // ==========================================
    void runCorrectFeedbackAction();
    void runIncorrectFeedbackAction();

    // ==========================================
    // LED CONTROL
    // ==========================================
    void correctLEDOn();
    void correctLEDOff();
    void correctLEDToggle();

    void incorrectLEDOn();
    void incorrectLEDOff();
    void incorrectLEDToggle();

    // ==========================================
    // BUZZER CONTROL
    // ==========================================
    void buzzerOn();
    void buzzerOff();
    void buzzerToggle();

    // ==========================================
    // BUZZER PATTERNS
    // ==========================================
    void beep(unsigned long duration = 100);
    void playBeepPattern(int count,
                         unsigned long duration = 100,
                         unsigned long pause = 100);

    void playHappyMelody();
    void playSadMelody();
    void playVictoryMelody();
    void playWarningMelody();

    // ==========================================
    // STATE CONTROL
    // ==========================================
    void allOff();
    void reset();

private:
    // ==========================================
    // PRIVATE HELPERS
    // ==========================================
    bool isValidPin(int pin);
    void setLED(int pin, int state);
    void safeDelay(unsigned long ms);

    // ==========================================
    // MEMBER VARIABLES
    // ==========================================
    SystemConfig &config;
    bool initialized = false;

    bool correctLEDState = false;
    bool incorrectLEDState = false;
    bool buzzerState = false;

    Actuators();
    ~Actuators();
};

#endif // ACTUATORS_H