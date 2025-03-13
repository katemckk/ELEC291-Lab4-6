#include <XC.h>
#include <stdio.h>
#include <stdlib.h>

#define LCD_RS LATBbits.LATB3
#define LCD_E  LATAbits.LATA2
#define LCD_D4 LATAbits.LATA3
#define LCD_D5 LATBbits.LATB4
#define LCD_D6 LATAbits.LATA4
#define LCD_D7 LATBbits.LATB5

#define LCD_RS_ENABLE TRISBbits.TRISB3
#define LCD_E_ENABLE  TRISAbits.TRISA2
#define LCD_D4_ENABLE TRISAbits.TRISA3
#define LCD_D5_ENABLE TRISBbits.TRISB4
#define LCD_D6_ENABLE TRISAbits.TRISA4
#define LCD_D7_ENABLE TRISBbits.TRISB5

#define CHARS_PER_LINE 16

void Timer4us(unsigned char t);
long int GetPeriod(int n);
void waitms(int len);
void wait_1ms(void);
void waitmsn(unsigned int ms);
void LCD_pulse (void);
void LCD_byte (unsigned char x);
void WriteData (unsigned char x);
void WriteCommand (unsigned char x);
void LCD_4BIT (void);
void LCDprint(char * string, unsigned char line, unsigned char clear);
