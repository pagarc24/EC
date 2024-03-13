//Práctica 4 por Pablo García Fernández
#include <msp430.h> 

/*
EJERCICIO 1. Realizar un programa para que la UART transmita el abecedario con la siguiente configuración: 9600, 8 bits de datos,
1 bit de stop, y sin paridad y que se pueda visualizar a través del USB en el hyperteminal. PISTA: se puede utilizar
un temporizador en cuya interrupción se mande una nueva letra a la UART solo si esta ha disparado previamente una interrupción de transmisión.
*/
unsigned int idxLetra = 0;
unsigned int cuenta = 1;
// diccionario para transformar letras a segmentos del LCD
const char alphabetBig [26][2] ={
{0xEF , 0x00 } , /* "A" LCD segments a+b+c+e+f+g+m */
{0xF1 , 0x50 } , /* "B" */
{0x9C , 0x00 } , /* "C" */
{0xF0 , 0x50 } , /* "D" */
{0x9F , 0x00 } , /* "E" */
{0x8F , 0x00 } , /* "F" */
{0xBD , 0x00 } , /* "G" */
{0x6F , 0x00 } , /* "H" */
{0x90 , 0x50 } , /* "I" */
{0x78 , 0x00 } , /* "J" */
{0x0E , 0x22 } , /* "K" */
{0x1C , 0x00 } , /* "L" */
{0x6C , 0xA0 } , /* "M" */
{0x6C , 0x82 } , /* "N" */
{0xFC , 0x00 } , /* "O" */
{0xCF , 0x00 } , /* "P" */
{0xFC , 0x02 } , /* "Q" */
{0xCF , 0x02 } , /* "R" */
{0xB7 , 0x00 } , /* "S" */
{0x80 , 0x50 } , /* "T" */
{0x7C , 0x00 } , /* "U" */
{0x0C , 0x28 } , /* "V" */
{0x6C , 0x0A } , /* "W" */
{0x00 , 0xAA } , /* "X" */
{0x00 , 0xB0 } , /* "Y" */
{0x90 , 0x28 } /* "Z" */};

char siguienteLetra();

# pragma vector = USCI_A1_VECTOR
__interrupt void USCI_A1_ISR ( void ) {
    char next = siguienteLetra();
    if(UCA1IFG & UCTXIFG){//transmisión
        UCA1TXBUF = next;
        UCA1IE &= ~ UCTXIE;
        UCA1IFG &= ~ UCTXIFG;
    } else if (UCA1IFG & UCRXIFG){//recepción
        UCA1IFG &= ~ UCRXIFG;
    }
}
# pragma vector = TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR (void) {
    if (cuenta==5) {
        cuenta = 1;
        UCA1IE |= UCTXIE;
    } else {
        cuenta++;
    }
}


int main(void){
    WDTCTL = WDTPW | WDTHOLD;
    PM5CTL0 &= ~LOCKLPM5;

    P3SEL0 = BIT4 + BIT5;
    P3SEL1 = 0x00;

    //CONFIGURACION DEL TIMER0
    TA0CTL = TASSEL__ACLK | ID__1 | MC__UP | TACLR ;
    TA0CCR0 = 3276;//0,1segundos según el enunciado de la práctica 3
    TA0CCTL0 = CCIE;

    CSCTL0_H = CSKEY >> 8; // Unlock clock registers
    CSCTL1 = DCOFSEL_3 | DCORSEL ; // Set DCO to 8 MHz
    CSCTL2 = SELA__VLOCLK | SELS__DCOCLK | SELM__DCOCLK ;
    CSCTL3 = DIVA__1 | DIVS__1 | DIVM__1 ; // Set all dividers
    CSCTL0_H = 0; // Lock CS registers

    // Configure USCI_A1 for UART mode
    UCA1CTLW0 = UCSWRST ; // Put eUSCI in reset
    UCA1CTLW0 |= UCSSEL__SMCLK ; // CLK = SMCLK
    // Baud Rate calculation
    // 8000000/(16*9600) = 52.083
    // Fractional portion = 0.083
    // User Guide Table 21 -4: UCBRSx = 0 x04
    // UCBRFx = int ( (52.083 -52) *16) = 1
    UCA1BR0 = 52; // 8000000/16/9600
    UCA1BR1 = 0x00;
    UCA1MCTLW |= UCOS16 | UCBRF_1 | 0x4900;
    UCA1CTLW0 &= ~ UCSWRST ; // Initialize eUSCI

    UCA1IE |= UCTXIE | UCRXIE; //enable interrupts

    UCA1IFG &= ~UCRXIFG;
    //UCA1IFG &= ~ (UCTXIFG | UCRXIFG);
    //__bis_SR_register ( LPM4_bits + GIE ) ;
    __enable_interrupt();

    return 0;
}


char siguienteLetra(){
    char ret = 'A' + idxLetra;//generamos el carácter a enviar

    //actualizamos el idxLetra para avanzar a la siguiente letra
    idxLetra++;
    if (idxLetra == 26) idxLetra = 0;

    return ret;//devolvemos la letra

}
