#include <XC.h>
#include <stdio.h>
#include <stdlib.h>
#include "lcd.h"
 
// Configuration Bits (somehow XC32 takes care of this)
#pragma config FNOSC = FRCPLL       // Internal Fast RC oscillator (8 MHz) w/ PLL
#pragma config FPLLIDIV = DIV_2     // Divide FRC before PLL (now 4 MHz)
#pragma config FPLLMUL = MUL_20     // PLL Multiply (now 80 MHz)
#pragma config FPLLODIV = DIV_2     // Divide After PLL (now 40 MHz)
#pragma config FWDTEN = OFF         // Watchdog Timer Disabled
#pragma config FPBDIV = DIV_1       // PBCLK = SYCLK
#pragma config FSOSCEN = OFF        // Secondary Oscillator Enable (Disabled)

// Defines
#define SYSCLK 40000000L
#define Baud2BRG(desired_baud)( (SYSCLK / (16*desired_baud))-1)
 
void UART2Configure(int baud_rate)
{
    // Peripheral Pin Select
    U2RXRbits.U2RXR = 4;    //SET RX to RB8
    RPB9Rbits.RPB9R = 2;    //SET RB9 to TX

    U2MODE = 0;         // disable autobaud, TX and RX enabled only, 8N1, idle=HIGH
    U2STA = 0x1400;     // enable TX and RX
    U2BRG = Baud2BRG(baud_rate); // U2BRG = (FPb / (16*baud)) - 1
    
    U2MODESET = 0x8000;     // enable UART2
}

// Needed to by scanf() and gets()
int _mon_getc(int canblock)
{
	char c;
	
    if (canblock)
    {
	    while( !U2STAbits.URXDA); // wait (block) until data available in RX buffer
	    c=U2RXREG;
	    if(c=='\r') c='\n'; // When using PUTTY, pressing <Enter> sends '\r'.  Ctrl-J sends '\n'
		return (int)c;
    }
    else
    {
        if (U2STAbits.URXDA) // if data available in RX buffer
        {
		    c=U2RXREG;
		    if(c=='\r') c='\n';
			return (int)c;
        }
        else
        {
            return -1; // no characters to return
        }
    }
}

// Use the core timer to wait for 1 ms.
void wait_1ms(void)
{
    unsigned int ui;
    _CP0_SET_COUNT(0); // resets the core timer count

    // get the core timer count
    while ( _CP0_GET_COUNT() < (SYSCLK/(2*1000)) );
}

void waitms(int len)
{
	while(len--) wait_1ms();
}

#define PIN_PERIOD (PORTB&(1<<6))

// GetPeriod() seems to work fine for frequencies between 200Hz and 700kHz.
long int GetPeriod (int n)
{
	int i;
	unsigned int saved_TCNT1a, saved_TCNT1b;
	
    _CP0_SET_COUNT(0); // resets the core timer count
	while (PIN_PERIOD!=0) // Wait for square wave to be 0
	{
		if(_CP0_GET_COUNT() > (SYSCLK/4)) return 0;
	}

    _CP0_SET_COUNT(0); // resets the core timer count
	while (PIN_PERIOD==0) // Wait for square wave to be 1
	{
		if(_CP0_GET_COUNT() > (SYSCLK/4)) return 0;
	}
	
    _CP0_SET_COUNT(0); // resets the core timer count
	for(i=0; i<n; i++) // Measure the time of 'n' periods
	{
		while (PIN_PERIOD!=0) // Wait for square wave to be 0
		{
			if(_CP0_GET_COUNT() > (SYSCLK/4)) return 0;
		}
		while (PIN_PERIOD==0) // Wait for square wave to be 1
		{
			if(_CP0_GET_COUNT() > (SYSCLK/4)) return 0;
		}
	}

	return  _CP0_GET_COUNT();
}

void Timer4us(unsigned char t) 
{
	T4CON = 0x8000; // enable Timer4, source PBCLK, 1:1 prescaler
 
    // delay 100us per loop until less than 100us remain
    while( t >= 100){
        t-=100;
        TMR4=0;
        while(TMR4 < SYSCLK/10000L);
    }
 
    // delay 10us per loop until less than 10us remain
    while( t >= 10){
        t-=10;
        TMR4=0;
        while(TMR4 < SYSCLK/100000L);
    }
 
    // delay 1us per loop until finished
    while( t > 0)
    {
        t--;
        TMR4=0;
        while(TMR4 < SYSCLK/1000000L);
    }
    // turn off Timer4 so function is self-contained
    T4CONCLR=0x8000;
}

void waitmsn(unsigned int ms)
{
	unsigned int j;
	unsigned char k;
	for(j=0;j<ms;j++)
		for(k=0;k<4;k++)
			Timer4us(250);
}

void LCD_pulse(void)
{
	LCD_E = 1;
	Timer4us(40);
	LCD_E = 0;
}

void LCD_byte(unsigned char x)
{
	LCD_D7=(x&0x80)?1:0;
	LCD_D6=(x&0x40)?1:0;
	LCD_D5=(x&0x20)?1:0;
	LCD_D4=(x&0x10)?1:0;
	LCD_pulse();
	Timer4us(40);
	LCD_D7=(x&0x08)?1:0;
	LCD_D6=(x&0x04)?1:0;
	LCD_D5=(x&0x02)?1:0;
	LCD_D4=(x&0x01)?1:0;
	LCD_pulse();
}

void WriteData(unsigned char x)
{
	LCD_RS = 1;
	LCD_byte(x);
	waitmsn(2);
}

void WriteCommand(unsigned char x)
{
	LCD_RS = 0;
	LCD_byte(x);
	waitmsn(5);
}

void LCD_4BIT(void)
{
	// Configure the pins used to communicate with the LCD as outputs
	LCD_RS_ENABLE = 0;
	LCD_E_ENABLE = 0;
	LCD_D4_ENABLE = 0;
	LCD_D5_ENABLE = 0;
	LCD_D6_ENABLE = 0;
	LCD_D7_ENABLE = 0;
	
	LCD_E = 0; // Resting state of LCD's enable is zero
	// LCD_RW = 0; Not used in this code.  Connect to ground.
	waitmsn(20);
	// First make sure the LCD is in 8-bit mdode, then change to 4-bit mode
	WriteCommand(0x33);
	WriteCommand(0x33);
	WriteCommand(0x32); // Change to 4-bit mode
	
	// Configure the LCD
	WriteCommand(0x28);
	WriteCommand(0x0c);
	WriteCommand(0x01); // Clear screen command (takes some time)
	waitmsn(20); // Wait for clear screen command to finish
	LATBbits.LATB0 = 	!LATBbits.LATB0;
}

void LCDprint(char * string, unsigned char line, unsigned char clear)
{
	int j;
	
	WriteCommand(line==2?0xc0:0x80);
	waitmsn(5);
	for(j=0;string[j]!=0;j++)
		WriteData(string[j]); //Write the message character by character
	if(clear)
		for(;j<CHARS_PER_LINE;j++)
			WriteData(' '); //Clear the rest of the line if clear is 1
}


void ADCConf(void)
{
    AD1CON1CLR = 0x8000;    // disable ADC before configuration
    AD1CON1 = 0x00E0;       // internal counter ends sampling and starts conversion (auto-convert), manual sample
    AD1CON2 = 0;            // AD1CON2<15:13> set voltage reference to pins AVSS/AVDD
    AD1CON3 = 0x0f01;       // TAD = 4*TPB, acquisition time = 15*TAD 
    AD1CON1SET=0x8000;      // Enable ADC
}

int ADCRead(char analogPIN)
{
    AD1CHS = analogPIN << 16;    // AD1CHS<16:19> controls which analog pin goes to the ADC
 
    AD1CON1bits.SAMP = 1;        // Begin sampling
    while(AD1CON1bits.SAMP);     // wait until acquisition is done
    while(!AD1CON1bits.DONE);    // wait until conversion done
 
    return ADC1BUF0;             // result stored in ADC1BUF0
}

void wait_debounce() {
    waitms(50); // Wait for 50ms (adjust as needed)
}

/* Pinout for DIP28 PIC32MX130:
                                          --------
                                   MCLR -|1     28|- AVDD 
  VREF+/CVREF+/AN0/C3INC/RPA0/CTED1/RA0 -|2     27|- AVSS 
        VREF-/CVREF-/AN1/RPA1/CTED2/RA1 -|3     26|- AN9/C3INA/RPB15/SCK2/CTED6/PMCS1/RB15
   PGED1/AN2/C1IND/C2INB/C3IND/RPB0/RB0 -|4     25|- CVREFOUT/AN10/C3INB/RPB14/SCK1/CTED5/PMWR/RB14
  PGEC1/AN3/C1INC/C2INA/RPB1/CTED12/RB1 -|5     24|- AN11/RPB13/CTPLS/PMRD/RB13
   AN4/C1INB/C2IND/RPB2/SDA2/CTED13/RB2 -|6     23|- AN12/PMD0/RB12
     AN5/C1INA/C2INC/RTCC/RPB3/SCL2/RB3 -|7     22|- PGEC2/TMS/RPB11/PMD1/RB11
                                    VSS -|8     21|- PGED2/RPB10/CTED11/PMD2/RB10
                     OSC1/CLKI/RPA2/RA2 -|9     20|- VCAP
                OSC2/CLKO/RPA3/PMA0/RA3 -|10    19|- VSS
                         SOSCI/RPB4/RB4 -|11    18|- TDO/RPB9/SDA1/CTED4/PMD3/RB9
         SOSCO/RPA4/T1CK/CTED9/PMA1/RA4 -|12    17|- TCK/RPB8/SCL1/CTED10/PMD4/RB8
                                    VDD -|13    16|- TDI/RPB7/CTED3/PMD5/INT0/RB7
                    PGED3/RPB5/PMD7/RB5 -|14    15|- PGEC3/RPB6/PMD6/RB6
                                          --------
*/

// Information here:
// http://umassamherstm5.org/tech-tutorials/pic32-tutorials/pic32mx220-tutorials/1-basic-digital-io-220
void main(void)
{
	long int count;
	float T, f, cap, R;
	char buffer1[16]; // Buffers for LCD printing
	char buffer2[16];
	char buffer3[16];
	LCD_4BIT(); // Initialize the LCD
	CFGCON = 0;
	
	int adcval;
    float voltage;
    int cap_mode = 1;
    int val = 102;

	char unit[3];  
    float displayCap;
  
    UART2Configure(115200);  // Configure UART2 for a baud rate of 115200
    
    ANSELB &= ~(1<<6); // Set RB6 as a digital I/O
    TRISB |= (1<<6);   // configure pin RB6 as input
    CNPUB |= (1<<6);   // Enable pull-up resistor for RB6
    
    ANSELB &= ~(1<<15); // Set RB15 as a digital I/O
    TRISB |= (1<<15);   // configure pin RB15 as input
    CNPUB |= (1<<15);   // Enable pull-up resistor for RB15
    
    TRISAbits.TRISA0 = 0;
	LATAbits.LATA0 = 0;	
	INTCONbits.MVEC = 1;
	
	// Configure pins as analog inputs
    ANSELBbits.ANSB2 = 1;   // set RB2 (AN4, pin 6 of DIP28) as analog pin
    TRISBbits.TRISB2 = 1;   // set RB2 as an input
    
    ADCConf(); // Configure ADC
 
	waitms(500);
	printf("PIC32MX130 Period measurement using the core timer free running counter.\r\n"
	       "Connect signal to RB6 (pin 15).\r\n");
    
    while(1)
    {
		count=GetPeriod(100);
		
		
		if(count>0)
		{
			adcval = ADCRead(4); // note that we call pin AN4 (RB2) by it's analog number
    		voltage=adcval*3.3/1023.0;
    	
    		R = (980.0*voltage)/(3.3-voltage);
    	
			
			T=(count*2.0)/(SYSCLK*100.0);
			f=1/T;
			
			
			cap =1.44/(f*5001.0);
			cap*=1000000.0;
			
			
			if (cap<0.15 && cap>0.08){ val = 104; }
			if (cap<1.05 && cap>0.90){	val = 105;	}
			
			if (cap==0.00103){	val= 102;}
			if (cap==0.0101){	val = 103;	}
			
			
			printf("f=%.2fHz, Count=%ld, Cap=%.4fuF , resistance=%.4fohms code=%d \r", f, count, cap,R, val);
        
        // Dynamically change unit
        	  
        	if (cap < 0.0500) {
            	displayCap = cap * 1000.0;  
            	unit[0] = 'n'; unit[1] = 'F'; unit[2] = '\0';
        	} else {
            	displayCap = cap;  // Keep in µF
            	unit[0] = 'u'; unit[1] = 'F'; unit[2] = '\0';
        	}
        
        	if (cap_mode == 1) {
        		// Display adjusted unit on LCD
        		sprintf(buffer1, "C: %.3f %s %d", displayCap, unit, val);
        		sprintf(buffer2, "F: %.3f Hz", f);
        		LCDprint(buffer1, 1, 1);
        		LCDprint(buffer2, 2, 1);
        	}
        	// check if there's  no capacitor being measured
        	else {
        	sprintf(buffer3, "R: %.2f", R);
        	LCDprint(buffer3, 1, 1);
        	LCDprint("   ohms", 2, 1);
        	}
        
        	if (f > 120000.0 && f < 180000.0) {
        	// turn on red led
        	LATAbits.LATA0 = 1;
        	cap_mode = 0;
        		
       		 }
        	else { cap_mode = 1;}
        
        
        LATAbits.LATA0 = !LATAbits.LATA0;
        
 
        
    }
    else
    {
        printf("NO SIGNAL                     \r");
        LCDprint("No Signal", 1, 1);
        LCDprint("", 2, 1);
    }
		fflush(stdout); // GCC peculiarities: need to flush stdout to get string out without a '\n'
		waitms(200);
    }
}
