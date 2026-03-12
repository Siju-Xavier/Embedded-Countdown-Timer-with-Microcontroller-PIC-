#include "StateMachine.h"
#include "UART2.h"
#include "IOs.h"
#include <stdio.h>

// Global state variables
volatile system_state_t current_state = STATE_IDLE;
volatile uint16_t minutes = 0;
volatile uint16_t seconds = 0;
volatile uint8_t timer_running = 0;
volatile uint8_t timer_paused = 0;
volatile uint16_t one_second_passed = 0;

// Button hold detection variables
volatile uint8_t pb1_hold_active = 0;
volatile uint8_t pb2_hold_active = 0;
volatile uint16_t pb1_hold_counter = 0;
volatile uint16_t pb2_hold_counter = 0;
volatile uint8_t pb1_was_pressed = 0;
volatile uint8_t pb2_was_pressed = 0;

// PB1+PB2 combination detection
volatile uint8_t pb1_pb2_combination = 0;
volatile uint16_t pb1_pb2_hold_counter = 0;

// LED control variables
volatile uint8_t led1_state = 0;
volatile uint8_t led2_state = 0;
volatile uint16_t led_blink_counter = 0;

// Hold time threshold (in timer ticks)
#define HOLD_THRESHOLD 1       // 2 seconds to activate +5 mode
#define PB1_PB2_HOLD_THRESHOLD 1  // 3 seconds for PB1+PB2 reset
#define HOLD_INCREMENT_RATE 1  // Increment every 1 timer tick (~0.5 seconds)

void initStateMachine(void) {
    current_state = STATE_IDLE;
    minutes = 0;
    seconds = 0;
    timer_running = 0;
    timer_paused = 0;
    one_second_passed = 0;
    
    // Initialize button hold variables
    pb1_hold_active = 0;
    pb2_hold_active = 0;
    pb1_hold_counter = 0;
    pb2_hold_counter = 0;
    pb1_was_pressed = 0;
    pb2_was_pressed = 0;
    pb1_pb2_combination = 0;
    pb1_pb2_hold_counter = 0;
    
    // Initialize LEDs to off
    LED1 = 0;
    LED2 = 0;
    led1_state = 0;
    led2_state = 0;
}

void handleButtonEvents(void) {
    // Read current button states
    uint8_t pb1_pressed = (PB1 == 0);
    uint8_t pb2_pressed = (PB2 == 0);
    uint8_t pb3_pressed = (PB3 == 0);
    
    int button_count = pb1_pressed + pb2_pressed + pb3_pressed;
    
    // Handle button combinations
    if (button_count == 3) {
        // All 3 buttons pressed - display group info
        Disp2String("\f\r2025 ENSF 460 L01 - Group 18");
        resetHoldStates();
        return;
    }
    else if (pb1_pressed && pb2_pressed && !pb3_pressed) {
        // PB1 + PB2 combination
        if (!pb1_pb2_combination) {
            // First detection of PB1+PB2
            pb1_pb2_combination = 1;
            pb1_pb2_hold_counter = 0;
        }
        // Let handleContinuousIncrement() handle the timing
        // Don't start timer immediately anymore
    }
    else if (pb1_pressed && !pb2_pressed && !pb3_pressed) {
        // PB1 alone - set seconds with hold detection
        handlePB1Press();
    }
    else if (pb2_pressed && !pb1_pressed && !pb3_pressed) {
        // PB2 alone - set minutes with hold detection
        handlePB2Press();
    }
    else if (pb3_pressed && !pb1_pressed && !pb2_pressed) {
        // PB3 alone - pause/resume
        if (current_state == STATE_COUNTING) {
            pauseTimer();
        }
        else if (current_state == STATE_PAUSED) {
            resumeTimer();
        }
        resetHoldStates();
    }
    else {
        // No buttons pressed or other combinations
        resetHoldStates();
    }
}

void handlePB1Press(void) {
    if (current_state != STATE_COUNTING && current_state != STATE_PAUSED) {
        current_state = STATE_SET_SECONDS;
        
        if (!pb1_was_pressed) {
            // First press - increment by 1 immediately
            seconds++;
            if (seconds > 59) seconds = 0;
            pb1_was_pressed = 1;
            pb1_hold_counter = 0;
            pb1_hold_active = 0;
            displayTime();
        } else {
            // Subsequent quick presses - also increment by 1
            if (!pb1_hold_active) {
                seconds++;
                if (seconds > 59) seconds = 0;
                pb1_hold_counter = 0; // Reset counter for quick presses
                displayTime();
            }
        }
        // Continuous increment is handled in handleTimerUpdates()
    }
}

void handlePB2Press(void) {
    if (current_state != STATE_COUNTING && current_state != STATE_PAUSED) {
        current_state = STATE_SET_MINUTES;
        
        if (!pb2_was_pressed) {
            // First press - increment by 1 immediately
            minutes++;
            if (minutes > 59) minutes = 0;
            pb2_was_pressed = 1;
            pb2_hold_counter = 0;
            pb2_hold_active = 0;
            displayTime();
        } else {
            // Subsequent quick presses - also increment by 1
            if (!pb2_hold_active) {
                minutes++;
                if (minutes > 59) minutes = 0;
                pb2_hold_counter = 0; // Reset counter for quick presses
                displayTime();
            }
        }
        // Continuous increment is handled in handleTimerUpdates()
    }
}

void resetHoldStates(void) {
    pb1_hold_counter = 0;
    pb2_hold_counter = 0;
    pb1_hold_active = 0;
    pb2_hold_active = 0;
    pb1_was_pressed = 0;
    pb2_was_pressed = 0;
    pb1_pb2_combination = 0;
    pb1_pb2_hold_counter = 0;
}

void handleTimerUpdates(void) {
    if (one_second_passed) {
        // Handle continuous increment while buttons are held
        handleContinuousIncrement();
        
        // Handle PB1+PB2 combination
        handlePB1PB2Combination();
        
        if (timer_running && !timer_paused) {
            updateCountdown();
        }
        updateLEDs();
        one_second_passed = 0;
    }
}

void handleContinuousIncrement(void) {
    // Handle PB1 continuous increment
    if (PB1 == 0 && pb1_was_pressed && current_state != STATE_COUNTING && current_state != STATE_PAUSED) {
        pb1_hold_counter++;
        
        // Check if we should activate +5 mode
        if (pb1_hold_counter >= HOLD_THRESHOLD && !pb1_hold_active) {
            pb1_hold_active = 1;
            Disp2String("\r\nPB1: +5 seconds mode activated");
        }
        
        // If in +5 mode, increment continuously at faster rate
        if (pb1_hold_active) {
            // Increment every tick for maximum speed!
            seconds += 5;
            if (seconds > 59) seconds = seconds % 60;
            displayTime();
        }
    }
    
    // Handle PB2 continuous increment
    if (PB2 == 0 && pb2_was_pressed && current_state != STATE_COUNTING && current_state != STATE_PAUSED) {
        pb2_hold_counter++;
        
        // Check if we should activate +5 mode
        if (pb2_hold_counter >= HOLD_THRESHOLD && !pb2_hold_active) {
            pb2_hold_active = 1;
            Disp2String("\r\nPB2: +5 minutes mode activated");
        }
        
        // If in +5 mode, increment continuously at faster rate
        if (pb2_hold_active) {
            // Increment every tick for maximum speed!
            minutes += 5;
            if (minutes > 59) minutes = minutes % 60;
            displayTime();
        }
    }
}

void handlePB1PB2Combination(void) {
    if (PB1 == 0 && PB2 == 0 && PB3 == 1) {
        // PB1+PB2 are pressed together
        if (pb1_pb2_combination) {
            pb1_pb2_hold_counter++;
            
            // Check if held long enough for reset
            if (pb1_pb2_hold_counter >= PB1_PB2_HOLD_THRESHOLD) {
                // Reset timer to 00:00
                resetTimer();
                pb1_pb2_combination = 0; // Reset the flag
            }
        }
    } else {
        // PB1+PB2 are not pressed together anymore
        if (pb1_pb2_combination && pb1_pb2_hold_counter < PB1_PB2_HOLD_THRESHOLD) {
            // Short press PB1+PB2 - start timer
            if (current_state == STATE_IDLE || current_state == STATE_SET_SECONDS || current_state == STATE_SET_MINUTES) {
                startTimer();
            }
        }
        pb1_pb2_combination = 0;
        pb1_pb2_hold_counter = 0;
    }
}

void handleOneSecondTick(void) {
    one_second_passed = 1;
    led_blink_counter++;
}

// ... (keep all other functions the same)

void updateCountdown(void) {
    if (seconds > 0) {
        seconds--;
    } else if (minutes > 0) {
        minutes--;
        seconds = 59;
    } else {
        // Timer reached zero
        triggerAlarm();
        return;
    }
    
    displayTime();
}

void updateLEDs(void) {
    switch(current_state) {
        case STATE_COUNTING:
            // LED1 blinks every second (1s on, 1s off)
            if (led_blink_counter % 2 == 0) {
                LED1 = 1;
            } else {
                LED1 = 0;
            }
            LED2 = 0; // LED2 off during countdown
            break;
            
        case STATE_ALARM:
            // LED1 solid on, LED2 rapid blink
            LED1 = 1;
            if (led_blink_counter % 2 == 0) { // Rapid blink - twice per second
                LED2 = !LED2;
            }
            break;
            
        default:
            // All other states - LEDs off
            LED1 = 0;
            LED2 = 0;
            break;
    }
}

void startTimer(void) {
    if (minutes == 0 && seconds == 0) {
        // Don't start if time is 00:00
        Disp2String("\f\rError: Time is 00:00");
        return;
    }
    
    timer_running = 1;
    timer_paused = 0;
    current_state = STATE_COUNTING;
    resetHoldStates(); // Reset any hold states when timer starts
    Disp2String("\f\rTimer Started");
    displayTime();
}

void pauseTimer(void) {
    timer_paused = 1;
    current_state = STATE_PAUSED;
    Disp2String("\f\rTimer Paused");
    displayTime();
}

void resumeTimer(void) {
    timer_paused = 0;
    current_state = STATE_COUNTING;
    Disp2String("\f\rTimer Resumed");
    displayTime();
}

void stopTimer(void) {
    timer_running = 0;
    timer_paused = 0;
    current_state = STATE_IDLE;
    minutes = 0;
    seconds = 0;
    resetHoldStates();
    Disp2String("\f\rTimer Stopped");
    displayTime();
}

void resetTimer(void) {
    minutes = 0;
    seconds = 0;
    current_state = STATE_IDLE;
    resetHoldStates();
    Disp2String("\f\rTimer Reset to 00:00");
    displayTime();
}

void triggerAlarm(void) {
    timer_running = 0;
    current_state = STATE_ALARM;
    resetHoldStates();
    Disp2String("\f\rALARM! Time's up!");
    displayTime();
}

void displayTime(void) {
    char buffer[30];
    
    switch(current_state) {
        case STATE_SET_SECONDS:
        case STATE_SET_MINUTES:
            sprintf(buffer, "\rSET %02dm : %02ds", minutes, seconds);
            break;
        case STATE_COUNTING:
            sprintf(buffer, "\rCNT %02dm : %02ds", minutes, seconds);
            break;
        case STATE_PAUSED:
            sprintf(buffer, "\rPAUSE %02dm : %02ds", minutes, seconds);
            break;
        case STATE_ALARM:
            sprintf(buffer, "\rFIN %02dm : %02ds - ALARM", minutes, seconds);
            break;
        default:
            sprintf(buffer, "\rIDLE %02dm : %02ds", minutes, seconds);
            break;
    }
    
    Disp2String(buffer);
}