#include "IOs.h"

void initIO(void) {
    // Configure buttons as inputs
    TRISBbits.TRISB7 = 1; // PB1 (RB7) as input
    TRISBbits.TRISB4 = 1; // PB2 (RB4) as input  
    TRISAbits.TRISA4 = 1; // PB3 (RA4) as input
    
    // Enable weak pull-ups on button inputs
    CNPU2bits.CN23PUE = 1; // RB7 pull-up (PB1)
    CNPU1bits.CN1PUE = 1;  // RB4 pull-up (PB2)
    CNPU1bits.CN0PUE = 1;  // RA4 pull-up (PB3)
    
    // Enable change notification interrupts
    CNEN2bits.CN23IE = 1; // Enable CN for RB7 (PB1)
    CNEN1bits.CN1IE = 1;  // Enable CN for RB4 (PB2)  
    CNEN1bits.CN0IE = 1;  // Enable CN for RA4 (PB3)
    
    // Configure LEDs as outputs
    TRISBbits.TRISB9 = 0; // LED1 (RB9) as output
    TRISAbits.TRISA6 = 0; // LED2 (RA6) as output
    
    // Initialize LEDs to off
    LED1 = 0;
    LED2 = 0;
}