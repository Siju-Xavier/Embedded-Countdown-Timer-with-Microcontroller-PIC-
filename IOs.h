#ifndef IOS_H
#define IOS_H

#include <xc.h>
#include <stdint.h>

// Button definitions 
#define PB1 PORTBbits.RB7
#define PB2 PORTBbits.RB4  
#define PB3 PORTAbits.RA4

// LED definitions
#define LED1 LATBbits.LATB9
#define LED2 LATAbits.LATA6

// Function declarations
void initIO(void);

#endif