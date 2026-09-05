/**
 * @file actuators/Actuators.h
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
    // SINGLETON INSTANCE
    // ==========================================
    static Actuators &get()
    {
        static Actuators instance;
        return instance;
    }

    // ==========================================
    // PUBLIC METHODS
    // ==========================================
    
    /**
     * @brief Initialize actuator pins (LEDs, Buzzer)
     */
    void initializeActuatorsPins();
    
    // ==========================================
    // FEEDBACK ACTIONS
    // ==========================================
    
    /**
     * @brief Run correct feedback action (LED + Buzzer)
     */
    void runCorrectFeedbackAction();
    
    /**
     * @brief Run incorrect feedback action (LED + Buzzer)
     */
    void runIncorrectFeedbackAction();
    
    // ==========================================
    // CORRECT LED CONTROL
    // ==========================================
    
    /**
     * @brief Turn on correct LED
     */
    void correctLEDOn();
    
    /**
     * @brief Turn off correct LED
     */
    void correctLEDOff();
    
    /**
     * @brief Toggle correct LED
     */
    void correctLEDToggle();
    
    // ==========================================
    // INCORRECT LED CONTROL
    // ==========================================
    
    /**
     * @brief Turn on incorrect LED
     */
    void incorrectLEDOn();
    
    /**
     * @brief Turn off incorrect LED
     */
    void incorrectLEDOff();
    
    /**
     * @brief Toggle incorrect LED
     */
    void incorrectLEDToggle();
    
    // ==========================================
    // BUZZER CONTROL
    // ==========================================
    
    /**
     * @brief Turn on buzzer
     */
    void buzzerOn();
    
    /**
     * @brief Turn off buzzer
     */
    void buzzerOff();
    
    /**
     * @brief Toggle buzzer
     */
    void buzzerToggle();
    
    // ==========================================
    // BUZZER PATTERNS
    // ==========================================
    
    /**
     * @brief Play a single beep
     * @param duration Duration in milliseconds
     */
    void beep(unsigned long duration = 100);
    
    /**
     * @brief Play a pattern of beeps
     * @param count Number of beeps
     * @param duration Duration of each beep
     * @param pause Pause between beeps
     */
    void playBeepPattern(int count, unsigned long duration = 100, unsigned long pause = 100);
    
    /**
     * @brief Play a happy melody (correct answer)
     */
    void playHappyMelody();
    
    /**
     * @brief Play a sad melody (incorrect answer)
     */
    void playSadMelody();
    
    /**
     * @brief Play a victory melody (game complete)
     */
    void playVictoryMelody();
    
    /**
     * @brief Play a warning melody (elimination warning)
     */
    void playWarningMelody();
    
    // ==========================================
    // ALL OFF
    // ==========================================
    
    /**
     * @brief Turn off all actuators
     */
    void allOff();
    
    /**
     * @brief Turn off all actuators and reset to initial state
     */
    void reset();

private:
    // ==========================================
    // PRIVATE METHODS
    // ==========================================
    
    /**
     * @brief Check if a pin is valid (not -1)
     * @param pin The pin number to check
     * @return true if valid
     */
    bool isValidPin(int pin);
    
    /**
     * @brief Set LED state with validation
     * @param pin The LED pin
     * @param state HIGH or LOW
     */
    void setLED(int pin, int state);
    
    /**
     * @brief Delay with yield to allow other tasks
     * @param ms Delay in milliseconds
     */
    void safeDelay(unsigned long ms);
    
    // ==========================================
    // MEMBER VARIABLES
    // ==========================================
    SystemConfig& config;  // Reference to singleton
    bool initialized = false;
    bool correctLEDState = false;
    bool incorrectLEDState = false;
    bool buzzerState = false;

private:
    Actuators();
    ~Actuators();
};

#endif // ACTUATORS_H