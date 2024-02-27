//Práctica 3 por Pablo García Fernández
#include <msp430.h>
#include <msp430fr6989.h>

/*
    EJERCICIO 1

// Mientras el botón P1.1 se mantiene pulsado el led P1.0 está encendido. En caso contrario se queda apagado
void main(void){
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

*/

/*
    EJERCICIO 2

// Cada vez que pulsemos el botón P1.1 el led P1.0 cambia de estado (de encendido a apagado y viceversa)
# pragma vector = PORT1_VECTOR
__interrupt void Port_1 ( void ) {
    P1OUT ^= BIT0; // Switch Bit1 value
    P1IFG &= ~BIT1 ; // P1 .1 clear pending interrupts
}

void main(void){
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

*/

/*
    EJERCICIO 3

// Cada vez que el TIMER0 se acabe (programado con un delay de 40.000) el led P1.0 cambia de estado (de encendido a apagado y viceversa)


# pragma vector = TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR (void) {
    P1OUT ^= BIT0; // Switch Bit1 value
}

void main(void){
    // disable watchdog
    WDTCTL = WDTPW | WDTHOLD;
    // disable high impeadance on I / O
    PM5CTL0 &= ~LOCKLPM5;

    // set P1 .0 as output , rest as inputs
    P1DIR = BIT0;

    //CONFIGURACION DEL TIMER0
    TA0CTL = TASSEL__ACLK | ID__1 | MC__UP | TACLR ;
    TA0CCR0 = 40000;
    TA0CCTL0 = CCIE;
    __bis_SR_register ( LPM4_bits + GIE ) ;
}
*/

/*
    EJERCICIO 4

*/

//
