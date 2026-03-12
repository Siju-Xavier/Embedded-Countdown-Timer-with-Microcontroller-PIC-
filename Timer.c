#include "Timer.h"
#include "StateMachine.h"

void setupTimer(void) {
    // Timer 3 configuration for 1-second interrupts
    // Note: PIC24F16KA101 doesn't have T32 bit for 32-bit mode
    T3CONbits.TCKPS = 0b11; // 1:256 prescaler for 500kHz clock
    T3CONbits.TCS = 0; // Use internal clock
    T3CONbits.TSIDL = 0; // Operate in idle mode
    
    // For 500kHz clock with 1:256 prescaler:
    // Timer frequency = 500kHz / 256 = 1953.125 Hz
    // PR3 = 1953 for approximately 1 second
    PR3 = 1953;
    
    IPC2bits.T3IP = 2; // Timer 3 interrupt priority
    IFS0bits.T3IF = 0; // Clear Timer 3 interrupt flag
    IEC0bits.T3IE = 1; // Enable Timer 3 interrupt
    
    TMR3 = 0;
    T3CONbits.TON = 1; // Start Timer 3
}