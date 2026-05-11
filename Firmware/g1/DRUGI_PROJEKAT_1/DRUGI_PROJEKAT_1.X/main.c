#include <stdio.h>
#include <stdlib.h>
#include <p30fxxxx.h>
#include <string.h>

#define FCY 10000000UL
#include <libpic30.h>

#include "timer1.h"
#include "timer2.h"

_FOSC(CSW_FSCM_OFF & XT_PLL4);
_FWDT(WDT_OFF);

#define PWM_PERIODA              2499

volatile unsigned char tempRX;
volatile unsigned int broj, broj1, broj2;


char buf[6];
int n = 0;

float cm_napred, cm_levi;


void initUART1(void) {
    U1BRG = 0x0040;
    U1MODEbits.ALTIO = 1;

    U1STA &= 0xfffc;

    U1MODEbits.UARTEN = 1;
    U1STAbits.UTXEN = 1;
}

void initUART2(void) {
    U2BRG = 0x0040;

    IEC1bits.U2RXIE = 1;

    U2STA &= 0xfffc;

    U2MODEbits.UARTEN = 1;
    U2STAbits.UTXEN = 1;
}

void __attribute__((__interrupt__)) _U1RXInterrupt(void) 
{
    IFS0bits.U1RXIF = 0;
    tempRX=U1RXREG;
}

void __attribute__((__interrupt__)) _U2RXInterrupt(void)
{
    IFS1bits.U2RXIF = 0;
    tempRX = U2RXREG;
}

void WriteUART1(unsigned int data)
{
	  while(!U1STAbits.TRMT);

    if(U1MODEbits.PDSEL == 3)
        U1TXREG = data;
    else
        U1TXREG = data & 0xFF;
}

void WriteUART2(unsigned int data) {
    while (!U2STAbits.TRMT);

    if (U2MODEbits.PDSEL == 3)
        U2TXREG = data;
    else
        U2TXREG = data & 0xFF;
}

void ispis2(const char *str) {
    while (*str) {
        WriteUART2(*str++);
    }
}

void WriteUART1dec2string(unsigned int data)
{
    unsigned char temp;

    temp = data / 1000;
    WriteUART1(temp + '0');
    data = data - temp * 1000;

    temp = data / 100;
    WriteUART1(temp + '0');
    data = data - temp * 100;

    temp = data / 10;
    WriteUART1(temp + '0');
    data = data - temp * 10;

    WriteUART1(data + '0');
}

void WriteUART2dec2string(unsigned int data)
{
    unsigned char temp;

    temp = data / 1000;
    WriteUART2(temp + '0');
    data = data - temp * 1000;

    temp = data / 100;
    WriteUART2(temp + '0');
    data = data - temp * 100;

    temp = data / 10;
    WriteUART2(temp + '0');
    data = data - temp * 10;

    WriteUART2(data + '0');
}

void pwm_init(void)
{
    TRISDbits.TRISD0 = 0;
    TRISDbits.TRISD1 = 0;

    OpenTimer2(T2_ON & T2_IDLE_CON & T2_GATE_OFF & T2_PS_1_1 & T2_SOURCE_INT, PWM_PERIODA);

    OC1R = 0;
    OC1RS = 0;
    OC1CONbits.OCTSEL = 0;
    OC1CONbits.OCM = 0b110;

    OC2R = 0;
    OC2RS = 0;
    OC2CONbits.OCTSEL = 0;
    OC2CONbits.OCM = 0b110;
}

void postavi_pwm(unsigned int levi, unsigned int desni)
{
    OC1RS = levi;
    OC2RS = desni;
}

void motori_stop(void)
{
    postavi_pwm(0,0);

    LATBbits.LATB11 = 0;
    LATBbits.LATB12 = 0;
    LATFbits.LATF0 = 0;
    LATFbits.LATF1 = 0;
}

void motori_napred(void)
{   
    postavi_pwm(2250,2250);
    
    LATBbits.LATB11 = 1;
    LATBbits.LATB12 = 0;

    LATFbits.LATF0 = 0;
    LATFbits.LATF1 = 1;
}

void motori_nazad(void)
{
    postavi_pwm(2300,2300);
    
    LATBbits.LATB11 = 0;
    LATBbits.LATB12 = 1;

    LATFbits.LATF0 = 1;
    LATFbits.LATF1 = 0;
}

void skreni_levo(void)
{
    LATBbits.LATB11 = 1;
    LATBbits.LATB12 = 0;

    LATFbits.LATF0 = 1;
    LATFbits.LATF1 = 0;
    postavi_pwm(2000,2000);
}

void skreni_levo_slow(void)
{
    LATBbits.LATB11 = 1;
    LATBbits.LATB12 = 0;

    LATFbits.LATF0 = 1;
    LATFbits.LATF1 = 0;
    postavi_pwm(1700,1700);
}

void skreni_desno(void)
{
    LATBbits.LATB11 = 0;
    LATBbits.LATB12 = 1;

    LATFbits.LATF0 = 0;
    LATFbits.LATF1 = 1;
    postavi_pwm(2000,2000);
}

float meri_levi_cm(void)
{
    unsigned int t;

    LATBbits.LATB1 = 0;
    __delay_us(2);
    LATBbits.LATB1 = 1;
    __delay_us(10);
    LATBbits.LATB1 = 0;

    while(PORTBbits.RB2 == 0);
    TMR1 = 0;
    while(PORTBbits.RB2 == 1);

    t = TMR1;

    return 0.01372 * t;
}

float meri_prednji_cm(void)
{
    unsigned int t;

    LATBbits.LATB9 = 0;
    __delay_us(2);
    LATBbits.LATB9 = 1;
    __delay_us(10);
    LATBbits.LATB9 = 0;

    while(PORTBbits.RB10 == 0);
    TMR1 = 0;
    while(PORTBbits.RB10 == 1);

    t = TMR1;

    return 0.01372 * t;
}

void korekcija_pravca(){
    int trenutno = meri_levi_cm();
    int prethodno = 10000;
    if(trenutno > 4){
        ispis2("Korigujem pravac\r\n");
        while(prethodno > trenutno){
            prethodno = trenutno;
            skreni_levo(); 
            __delay_ms(100); 
            motori_stop();
            __delay_ms(1000); 
            trenutno = meri_levi_cm();
        }
    }
    
}


int main(int argc, char** argv) {
    
    typedef enum {
        NAPRED,
        STOP,
        SKRENI_LEVO,
        SKRENI_DESNO
    } state;

    for (broj = 0; broj < 60000; broj++);

    ADPCFG = 0xFFFF;

    TRISAbits.TRISA11 = 0;
    LATAbits.LATA11 = 0;

    TRISCbits.TRISC13 = 0;
    TRISCbits.TRISC14 = 1;

    TRISFbits.TRISF5 = 0;
    TRISFbits.TRISF4 = 1;

    pwm_init();
    TRISBbits.TRISB0 = 0;
    TRISBbits.TRISB1 = 0;
    TRISBbits.TRISB2 = 1;
    TRISBbits.TRISB3 = 0;
    
    TRISBbits.TRISB11 = 0;
    TRISBbits.TRISB12 = 0;
    TRISFbits.TRISF0 = 0;
    TRISFbits.TRISF1 = 0;

    TRISDbits.TRISD3 = 0;
    LATDbits.LATD3 = 0;
    TRISDbits.TRISD2 = 1;

    TRISBbits.TRISB9 = 0;
    LATBbits.LATB9 = 0;
    TRISBbits.TRISB10 = 1;
    TRISAbits.TRISA11 = 0;
    
    LATBbits.LATB0 = 1;
    LATBbits.LATB3 = 0;
    
    LATDbits.LATD0 = 1;
    LATDbits.LATD1 = 1;
    
    motori_stop();

    initUART1();
    initUART2();
    Init_T1();
    LATAbits.LATA11 = 0;
    __builtin_enable_interrupts();

    state current = NAPRED, next;
   
    
    while (1) {
        
        cm_napred = meri_prednji_cm();
        cm_levi = meri_levi_cm();
        
        switch(current){
            case NAPRED:
                ispis2("Vozim napred\r\n");
                next = current;
                motori_napred();
                LATAbits.LATA11 = 1;
                if(cm_napred < 16){ next = STOP; }
                if(cm_levi > 18){ next = STOP; }
                break;
            case STOP:
                ispis2("Stajem\r\n");
                next = current;
                motori_stop();
                LATAbits.LATA11 = 0;
                __delay_ms(1200);
                if(cm_levi > 18){ next = SKRENI_LEVO; } else { next = SKRENI_DESNO; }
                break;
            case SKRENI_LEVO:
                ispis2("Skrecem levo\r\n");
                next = current;
                LATAbits.LATA11 = 1;
                skreni_levo();
                __delay_ms(930);
                motori_napred();
                __delay_ms(680);        
                motori_stop();
                __delay_ms(1000);
                korekcija_pravca();
                next = NAPRED;
                break;
            case SKRENI_DESNO:
                ispis2("Skrecem desno\r\n");
                next = current;
                LATAbits.LATA11 = 1;
                skreni_desno();
                __delay_ms(1100);   // ovo  bilo 1350  
                motori_stop();
                __delay_ms(1000); 
                korekcija_pravca();
                korekcija_pravca();
                next = NAPRED;
                break;
            default:
                next = current;
                current = STOP;
                break;
        }
        current = next;
    }   
    return (EXIT_SUCCESS);
}