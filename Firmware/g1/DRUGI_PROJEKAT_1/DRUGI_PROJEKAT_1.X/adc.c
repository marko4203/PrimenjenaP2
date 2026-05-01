#include <p30fxxxx.h>
#include "adc.h"

void ADCinit(void)
{
    ADCON1bits.ADSIDL = 0;
    ADCON1bits.FORM   = 0;
    ADCON1bits.SSRC   = 7;
    ADCON1bits.SAMP   = 1;
    /* ADCON1bits.ADON = 1;  ukljucuje se kasnije u main-u */

    ADCON2bits.VCFG  = 7;
    ADCON2bits.CSCNA = 1;
    ADCON2bits.SMPI  = 3;   /* prekid posle 4 konverzije */
    ADCON2bits.BUFM  = 0;
    ADCON2bits.ALTS  = 0;

    ADCON3bits.SAMC = 31;
    ADCON3bits.ADRC = 1;
    ADCON3bits.ADCS = 31;

    ADCHSbits.CH0NB = 0;
    ADCHSbits.CH0NA = 0;
    ADCHSbits.CH0SA = 0;
    ADCHSbits.CH0SB = 0;

    /* RB0 -> AN0  zadnji
       RB1 -> AN1  desni
       RB8 -> AN8  prednji
       RB10 -> AN10 levi */
    ADPCFG = 0xFFFF;
    ADPCFGbits.PCFG0  = 0;
    ADPCFGbits.PCFG1  = 0;
    ADPCFGbits.PCFG8  = 0;
    ADPCFGbits.PCFG10 = 0;

    
    ADCSSL = 0b0000010100000011;

    ADCON1bits.ASAM = 1;

    IPC2bits.ADIP = 5;
    IFS0bits.ADIF = 0;
    IEC0bits.ADIE = 1;
}
