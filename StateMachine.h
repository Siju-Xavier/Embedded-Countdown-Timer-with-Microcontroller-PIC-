#ifndef STATEMACHINE_H
#define STATEMACHINE_H

#include <xc.h>
#include <stdint.h>

// State definitions
typedef enum {
    STATE_IDLE,
    STATE_SET_SECONDS,
    STATE_SET_MINUTES, 
    STATE_COUNTING,
    STATE_PAUSED,
    STATE_ALARM
} system_state_t;

// Hold detection constants (in timer ticks)
#define HOLD_THRESHOLD 2           // 2 seconds to activate +5 mode
#define PB1_PB2_HOLD_THRESHOLD 3   // 3 seconds for PB1+PB2 reset
#define HOLD_INCREMENT_RATE 1      // Increment every 1 timer tick (~0.5 seconds)

// Function prototypes
void initStateMachine(void);
void handleButtonEvents(void);
void handleTimerUpdates(void);
void handleOneSecondTick(void);
void displayTime(void);
void startTimer(void);
void pauseTimer(void);
void resumeTimer(void);
void stopTimer(void);
void resetTimer(void);
void updateCountdown(void);
void updateLEDs(void);
void triggerAlarm(void);

// Button hold detection functions
void handlePB1Press(void);
void handlePB2Press(void);
void resetHoldStates(void);
void handleContinuousIncrement(void);
void handlePB1PB2Combination(void);

// External variables
extern volatile system_state_t current_state;
extern volatile uint16_t minutes;
extern volatile uint16_t seconds;
extern volatile uint8_t timer_running;
extern volatile uint8_t timer_paused;

#endif