//Práctica 3 por Pablo García Fernández
#include <msp430.h>
#include <msp430fr6989.h>

//INTERRUPCIONES
//Comentarlas si fuera necesario

//ej2
// Port 1 ISR
# pragma vector = PORT1_VECTOR
__interrupt void Port_1 ( void ) {

    if(P1IN & BIT1){//los pushbuttons funcionan en lógica inversa
        P1OUT &= ~BIT0;
    } else {
        P1OUT |= BIT0;
    }

    P1IFG &= ~BIT1 ; // P1 .1 clear pending interrupts
}

//FUNCIONES
void ej1(void){
    WDTCTL = WDTPW | WDTHOLD;
    PM5CTL0 &= ~LOCKLPM5;

    P1DIR = BIT0;
    P1REN = BIT1;
    P1OUT = BIT1;

    while(1){
        if(P1IN & BIT1){//los pushbuttons funcionan en lógica inversa
            P1OUT &= ~BIT0;
        } else {
            P1OUT |= BIT0;
        }
    }
}
void ej2(void){
    // disable watchdog
    WDTCTL = WDTPW | WDTHOLD;
    // disable high impeadance on I / O
    PM5CTL0 &= ~LOCKLPM5;

    // set P1 .0 as output , rest as inputs
    P1DIR = BIT0;
    // enable pull on P1 .1 , rest disabled
    P1REN = BIT1;
    // set pull - up on P1 .1
    P1OUT = BIT1;

    // P1 .1 IRQ enabled
    P1IE |= BIT1;

    // P1 .1 Falling edge
    P1IES |= BIT1;
    // P1 .1 clear pending interrupts
    P1IFG &= ~ BIT1 ;

    __bis_SR_register ( LPM4_bits + GIE ) ; // Enter LMP4 mode ( sleep )
}

void main(void){
    //ej1();
    ej2();
}
