/**
 * @file actuators/Actuators.cpp
 * @brief Actuator control implementation
 */
#include "actuators.h"

// ==========================================
// CONSTRUCTOR & DESTRUCTOR
// ==========================================

Actuators::Actuators() 
    : config(SystemConfig::get())
{
    // Serial.println("🔌 Actuators created!");
    initialized = false;
}

Actuators::~Actuators()
{
    // Turn off everything when destroyed
    allOff();
    // Serial.println("🔌 Actuators destroyed!");
}

// ==========================================
// INITIALIZATION
// ==========================================

void Actuators::initializeActuatorsPins()
{
    if (initialized) {
        // Serial.println("⚠️ Actuators already initialized!");
        return;
    }
    
    // Serial.println("🔧 Initializing actuator pins...");
    
    // Initialize correct LED pin
    if (isValidPin(config.CORRECT_LED)) {
        pinMode(config.CORRECT_LED, OUTPUT);
        digitalWrite(config.CORRECT_LED, LOW);
        // Serial.printf("  ✅ Correct LED (GPIO %d) initialized\n", config.CORRECT_LED);
    } else {
        // Serial.printf("  ❌ Invalid CORRECT_LED pin: %d\n", config.CORRECT_LED);
    }
    
    // Initialize incorrect LED pin
    if (isValidPin(config.INCORRECT_LED)) {
        pinMode(config.INCORRECT_LED, OUTPUT);
        digitalWrite(config.INCORRECT_LED, LOW);
        // Serial.printf("  ✅ Incorrect LED (GPIO %d) initialized\n", config.INCORRECT_LED);
    } else {
        // Serial.printf("  ❌ Invalid INCORRECT_LED pin: %d\n", config.INCORRECT_LED);
    }
    
    // Initialize buzzer pin
    if (isValidPin(config.BUZZER_PIN)) {
        pinMode(config.BUZZER_PIN, OUTPUT);
        digitalWrite(config.BUZZER_PIN, LOW);
        // Serial.printf("  ✅ Buzzer (GPIO %d) initialized\n", config.BUZZER_PIN);
    } else {
        // Serial.printf("  ❌ Invalid BUZZER_PIN pin: %d\n", config.BUZZER_PIN);
    }
    
    // Start with all off
    allOff();
    
    initialized = true;
    // Serial.println("✅ Actuator pins initialized!");
}

// ==========================================
// FEEDBACK ACTIONS
// ==========================================

void Actuators::runCorrectFeedbackAction()
{
    // Serial.println("✅ CORRECT feedback action!");
    
    // Flash correct LED
    correctLEDOn();
    
    // Play happy melody
    playHappyMelody();
    
    // Keep LED on for a moment
    safeDelay(300);
    
    // Turn off
    correctLEDOff();
}

void Actuators::runIncorrectFeedbackAction()
{
    // Serial.println("❌ INCORRECT feedback action!");
    
    // Flash incorrect LED
    incorrectLEDOn();
    
    // Play sad melody
    playSadMelody();
    
    // Keep LED on for a moment
    safeDelay(400);
    
    // Turn off
    incorrectLEDOff();
}

// ==========================================
// CORRECT LED CONTROL
// ==========================================

void Actuators::correctLEDOn()
{
    setLED(config.CORRECT_LED, HIGH);
    correctLEDState = true;
}

void Actuators::correctLEDOff()
{
    setLED(config.CORRECT_LED, LOW);
    correctLEDState = false;
}

void Actuators::correctLEDToggle()
{
    if (correctLEDState) {
        correctLEDOff();
    } else {
        correctLEDOn();
    }
}

// ==========================================
// INCORRECT LED CONTROL
// ==========================================

void Actuators::incorrectLEDOn()
{
    setLED(config.INCORRECT_LED, HIGH);
    incorrectLEDState = true;
}

void Actuators::incorrectLEDOff()
{
    setLED(config.INCORRECT_LED, LOW);
    incorrectLEDState = false;
}

void Actuators::incorrectLEDToggle()
{
    if (incorrectLEDState) {
        incorrectLEDOff();
    } else {
        incorrectLEDOn();
    }
}

// ==========================================
// BUZZER CONTROL
// ==========================================

void Actuators::buzzerOn()
{
    if (!isValidPin(config.BUZZER_PIN)) {
        // Serial.printf("⚠️ Invalid BUZZER_PIN: %d\n", config.BUZZER_PIN);
        return;
    }
    digitalWrite(config.BUZZER_PIN, HIGH);
    buzzerState = true;
}

void Actuators::buzzerOff()
{
    if (!isValidPin(config.BUZZER_PIN)) {
        return;
    }
    digitalWrite(config.BUZZER_PIN, LOW);
    buzzerState = false;
}

void Actuators::buzzerToggle()
{
    if (buzzerState) {
        buzzerOff();
    } else {
        buzzerOn();
    }
}

// ==========================================
// BUZZER PATTERNS
// ==========================================

void Actuators::beep(unsigned long duration)
{
    buzzerOn();
    safeDelay(duration);
    buzzerOff();
}

void Actuators::playBeepPattern(int count, unsigned long duration, unsigned long pause)
{
    for (int i = 0; i < count; i++) {
        beep(duration);
        if (i < count - 1) {
            safeDelay(pause);
        }
    }
}

void Actuators::playHappyMelody()
{
    // Happy pattern: two short beeps with a pause
    playBeepPattern(2, 100, 150);
}

void Actuators::playSadMelody()
{
    // Sad pattern: three short beeps
    playBeepPattern(3, 80, 100);
    safeDelay(100);
    playBeepPattern(3, 80, 100);
}

void Actuators::playVictoryMelody()
{
    // Victory: ascending beeps
    for (int i = 0; i < 3; i++) {
        beep(150);
        safeDelay(100);
    }
    safeDelay(200);
    beep(300);
}

void Actuators::playWarningMelody()
{
    // Warning: two long beeps
    playBeepPattern(2, 300, 200);
}

// ==========================================
// ALL OFF
// ==========================================

void Actuators::allOff()
{
    correctLEDOff();
    incorrectLEDOff();
    buzzerOff();
    correctLEDState = false;
    incorrectLEDState = false;
    buzzerState = false;
}

void Actuators::reset()
{
    allOff();
    initialized = false;
    // Serial.println("🔄 Actuators reset!");
}

// ==========================================
// PRIVATE METHODS
// ==========================================

bool Actuators::isValidPin(int pin)
{
    // ESP32 valid GPIO range
    return (pin >= 0 && pin <= 39);
}

void Actuators::setLED(int pin, int state)
{
    if (!isValidPin(pin)) {
        // Serial.printf("⚠️ Invalid pin: %d\n", pin);
        return;
    }
    digitalWrite(pin, state);
}

void Actuators::safeDelay(unsigned long ms)
{
    unsigned long start = millis();
    while (millis() - start < ms) {
        delay(1);
        yield();  // Allow other tasks to run
    }
}