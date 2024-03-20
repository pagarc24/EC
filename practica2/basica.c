//Pr�ctica 2 por Pablo Garc�a Fern�ndez
#include <msp430fr6989.h>
#include <msp430.h>

//Para ejecutar los distintos ejercicios descomentar en el main la llamada a la funci�n correspondiente

void ejercicio2(void){
    WDTCTL = WDTPW +WDTHOLD;
    PM5CTL0 &= ~LOCKLPM5;

    P1DIR |= BIT0;
    P1OUT = 0x0;

    while(1){
        P1OUT ^= BIT0;
        __delay_cycles(30000);
    }
}

void ejercicio3(void){
    WDTCTL = WDTPW +WDTHOLD;
    PM5CTL0 &= ~LOCKLPM5;

    P9DIR |= BIT7;
    P9OUT = 0x0;

    while(1){
        P9OUT ^= BIT7;
        __delay_cycles(30000);
    }
}

void ejercicio4(void){
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

void main (void){
    //ejercicio2();
    //ejercicio3();
    //ejercicio4();
}
