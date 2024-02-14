//Práctica 2 por Pablo García Fernández
#include <msp430fr6989.h>
#include <msp430.h>

void ej2(void){
    WDTCTL = WDTPW +WDTHOLD;
    PM5CTL0 &= ~LOCKLPM5;

    P1DIR |= BIT0;
    P1OUT = 0x0;

    while(1){
        P1OUT ^= BIT0;
        __delay_cycles(30000);
    }
}

void ej3(void){
    WDTCTL = WDTPW +WDTHOLD;
    PM5CTL0 &= ~LOCKLPM5;

    P9DIR |= BIT7;
    P9OUT = 0x0;

    while(1){
        P9OUT ^= BIT7;
        __delay_cycles(30000);
    }
}

void ej4(void){
    WDTCTL = WDTPW +WDTHOLD;
    PM5CTL0 &= ~LOCKLPM5;

    P1DIR |= BIT0;
    P1OUT = 0x0;
    P9DIR |= BIT7;
    P9OUT = 0x0;

    while(1){
        P1OUT ^= BIT0;
        P9OUT ^= BIT7;
        //__delay_cycles(30000);
        __delay_cycles(150000);
    }
}

void modificacion_ej1(void){
    WDTCTL = WDTPW +WDTHOLD;
    PM5CTL0 &= ~LOCKLPM5;

    P1DIR |= BIT0;
    P1OUT = 0x0;
    P9DIR |= BIT7;
    P9OUT = 0x0 | BIT7;

    while(1){
        P1OUT ^= BIT0;
        P9OUT ^= BIT7;
        __delay_cycles(150000);
    }
}

void modificacion_ej2(void){
    unsigned int i = 0;
    WDTCTL = WDTPW +WDTHOLD;
    PM5CTL0 &= ~LOCKLPM5;

    P1DIR |= BIT0;
    P1OUT = 0x0;
    P9DIR |= BIT7;
    P9OUT = 0x0;

    while(1){
        if(i){
            i = 0;
            P1OUT ^= BIT0;
            P9OUT ^= BIT7;
            __delay_cycles(350000);
        } else {
            i = 1;
            P1OUT ^= BIT0;
            __delay_cycles(350000);
        }
    }
}

void main (void){
    //ej2();
    //ej3();
    //ej4();
    //modificacion_ej1();
    modificacion_ej2();
}
