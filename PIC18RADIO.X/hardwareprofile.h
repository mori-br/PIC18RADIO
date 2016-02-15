/*
    File:   harwareprofile.h
    Copyright 2016 Marcos Mori de Siqueira <mori.br@gmail.com>

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/
#ifndef HARWAREPROFILE_H
#define	HARWAREPROFILE_H

#include <p18f2520.h>

#ifdef	__cplusplus
extern "C" {
#endif

#define ROMCHAR  const far rom char*
    
    
#define GetSystemClock()        10000000UL
#define GetInstructionClock()   (GetSystemClock()/2)    // Normally GetSystemClock()/4 for PIC18, GetSystemClock()/2 for PIC24/dsPIC, and GetSystemClock()/1 for PIC32.  Might need changing if     
    
#define INPUT_PIN      1
#define OUTPUT_PIN     0    

#define LED1           LATCbits.LATC5
#define LED1_TRIS      (TRISCbits.TRISC5)
    
#define I2C_SCL_TRIS   (TRISCbits.TRISC3)
#define I2C_SDA_TRIS   (TRISCbits.TRISC4)
        
    
#define LCD_DATA_TRIS  TRISB  
#define LCD_DATA_IO    LATB
    

#ifndef LCD_DATA_TRIS
#define LCD_DATA0_TRIS (TRISBbits.TRISB0)
#define LCD_DATA1_TRIS (TRISBbits.TRISB1)
#define LCD_DATA2_TRIS (TRISBbits.TRISB2)
#define LCD_DATA3_TRIS (TRISBbits.TRISB3)
#define LCD_DATA4_TRIS (TRISBbits.TRISB4)
#define LCD_DATA5_TRIS (TRISBbits.TRISB5)
#define LCD_DATA6_TRIS (TRISBbits.TRISB6)
#define LCD_DATA7_TRIS (TRISBbits.TRISB7)
#endif
    
#define LCD_RS_TRIS    (TRISCbits.TRISC0)  
#define LCD_RS_IO      (LATCbits.LATC0) 

#define LCD_RD_WR_TRIS (TRISCbits.TRISC1)
#define LCD_RD_WR_IO   (LATCbits.LATC1) 

#define LCD_E_TRIS     (TRISCbits.TRISC2)
#define LCD_E_IO       (LATCbits.LATC2)
  
#ifndef LCD_DATA_IO
#define LCD_DATA0_IO   (LATBbits.LATB0) 
#define LCD_DATA1_IO   (LATBbits.LATB1) 
#define LCD_DATA2_IO   (LATBbits.LATB2)
#define LCD_DATA3_IO   (LATBbits.LATB3)
#define LCD_DATA4_IO   (LATBbits.LATB4)
#define LCD_DATA5_IO   (LATBbits.LATB5)
#define LCD_DATA6_IO   (LATBbits.LATB6)
#define LCD_DATA7_IO   (LATBbits.LATB7)
#endif    
    
#define KEYPAD_BTN1_TRIS    (TRISAbits.TRISA0)
#define KEYPAD_BTN2_TRIS    (TRISAbits.TRISA1)
#define KEYPAD_BTN3_TRIS    (TRISAbits.TRISA2)
#define KEYPAD_BTN4_TRIS    (TRISAbits.TRISA3)
#define KEYPAD_BTN5_TRIS    (TRISAbits.TRISA4)
#define KEYPAD_BTN6_TRIS    (TRISAbits.TRISA5)
    
#define KEYPAD_BTN1_IO      (PORTAbits.RA0)
#define KEYPAD_BTN2_IO      (PORTAbits.RA1)
#define KEYPAD_BTN3_IO      (PORTAbits.RA2)
#define KEYPAD_BTN4_IO      (PORTAbits.RA3)
#define KEYPAD_BTN5_IO      (PORTAbits.RA4)
#define KEYPAD_BTN6_IO      (PORTAbits.RA5)

    
#ifdef	__cplusplus
}
#endif

#endif	/* HARWAREPROFILE_H */

