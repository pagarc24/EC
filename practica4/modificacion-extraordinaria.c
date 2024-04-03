//Modificación extraordinaria práctica 4 por Pablo García Fernández
#include <msp430.h>

/*
MODIFICACION
Modificar el programa de la práctica 4 de la siguiente manera:
	1. Cuando se haya recibido a través del teclado la palabra STOP, dejará de enviarse el abecedario.

	2. Al pulsar uno de los botones deberá reanudarse el envío del abecedario. La detección de la pulsación del botón debe hacerse mediante interrupciones.

	3. Mientras el envío del abecedario esté interrumpido el led rojo debe estar encendido y el verde apagado. El resto del tiempo deberá estar encendido el led verde y el rojo apagado.
*/

//VARIABLES GLOBALES
int bufferLCD[6]={'Z','Z','Z','Z','Z','Z'};
unsigned int idxLetra = 0;
unsigned int cuenta = 1;
const char alphabetBig [26][2] ={ //A, B, C, ..., Z
{0xEF , 0x00 } ,
{0xF1 , 0x50 } ,
{0x9C , 0x00 } ,
{0xF0 , 0x50 } ,
{0x9F , 0x00 } ,
{0x8F , 0x00 } ,
{0xBD , 0x00 } ,
{0x6F , 0x00 } ,
{0x90 , 0x50 } ,
{0x78 , 0x00 } ,
{0x0E , 0x22 } ,
{0x1C , 0x00 } ,
{0x6C , 0xA0 } ,
{0x6C , 0x82 } ,
{0xFC , 0x00 } ,
{0xCF , 0x00 } ,
{0xFC , 0x02 } ,
{0xCF , 0x02 } ,
{0xB7 , 0x00 } ,
{0x80 , 0x50 } ,
{0x7C , 0x00 } ,
{0x0C , 0x28 } ,
{0x6C , 0x0A } ,
{0x00 , 0xAA } ,
{0x00 , 0xB0 } ,
{0x90 , 0x28 }};

//PROTOTIPOS
void Initialize_LCD();
void config_ACLK_to_32KHz_crystal();
void ShowBuffer (int buffer[]);
char siguienteLetra();

int main(void){
    WDTCTL = WDTPW | WDTHOLD;
    PM5CTL0 &= ~LOCKLPM5;

    P3SEL0 = BIT4 + BIT5;
    P3SEL1 = 0x00;

    Initialize_LCD();
    config_ACLK_to_32KHz_crystal();

    //config led
    P1DIR |= BIT0;
    P9DIR |= BIT7;

    P1OUT &= ~ BIT0;
    P9OUT |= BIT7;

    //p1.1 y p1.2 puestos como in
    P1DIR &= ~ BIT1;
    P1DIR &= ~ BIT2;

    //CONFIGURAR BOTONES
    //P1.1 botón de pausa y continuar
    // enable pull on P1 .1 , rest disabled
    P1REN |= BIT1;
    // set pull - up on P1 .1
    P1OUT |= BIT1;

    // P1 .1 IRQ enabled
    P1IE |= BIT1;

    // P1 .1 Falling edge
    P1IES |= BIT1;
    // P1 .1 clear pending interrupts
    P1IFG &= ~ BIT1 ;

    //P1.2 botón de pausa de recepcion en el lcd
    // enable pull on P1 .2 , rest disabled
    P1REN |= BIT2;
    // set pull - up on P1 .2
    P1OUT |= BIT2;

    // P1 .2 IRQ enabled
    P1IE |= BIT2;

    // P1 .2 Falling edge
    P1IES |= BIT2;
    // P1 .2 clear pending interrupts
    P1IFG &= ~ BIT2 ;

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
    __enable_interrupt();

    return 0;
}

//INTERRUPCIONES
# pragma vector = PORT1_VECTOR
__interrupt void Port_1 ( void ) {
    //INDEPENDIENTEMENTE DEL BOTÓN REANUDAMOS LA TRANSMISION
    //LUEGO LIMPIAMOS LA INT CORRESPONDIENTE

    TA0CTL &= ~(BIT5 | BIT4);
    TA0CTL |= MC__UP;

    P1OUT &= ~ BIT0;
    P9OUT |= BIT7;

    if(P1IFG & BIT1)//INT P1.1
    {
        //limpia el flag de interrupcion
        P1IFG &= ~BIT1 ;
    }
    else if (P1IFG & BIT2)//INT P1.2
    {
        //limpia el flag de interrupcion
        P1IFG &= ~BIT2;
    }
}
# pragma vector = USCI_A1_VECTOR
__interrupt void USCI_A1_ISR ( void ) {
    if(UCA1IFG & UCRXIFG){//recepción
        bufferLCD[5]=bufferLCD[4];
        bufferLCD[4]=bufferLCD[3];
        bufferLCD[3]=bufferLCD[2];
        bufferLCD[2]=bufferLCD[1];
        bufferLCD[1]=bufferLCD[0];
        bufferLCD[0]=UCA1RXBUF;

        ShowBuffer(bufferLCD);

        if(bufferLCD[3]=='S'&&bufferLCD[2]=='T'&&bufferLCD[1]=='O'&&bufferLCD[0]=='P'){ //Paramos la transmision parando el timer
            UCA1IE &= ~ UCTXIE;
            TA0CTL &= ~(BIT5 | BIT4);
            TA0CTL |= MC__STOP;
            P1OUT |= BIT0;
            P9OUT &= ~ BIT7;
        }

        UCA1IFG &= ~ UCRXIFG;
    } else if(UCA1IFG & UCTXIFG){//transmisión
        UCA1TXBUF = siguienteLetra();
        UCA1IE &= ~ UCTXIE;
        UCA1IFG &= ~ UCTXIFG;
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

//FUNCIONES
void Initialize_LCD () {
    PJSEL0 = BIT4 | BIT5 ; // For LFXT
    // Initialize LCD segments 0 - 21; 26 - 43
    LCDCPCTL0 = 0xFFFF ;
    LCDCPCTL1 = 0xFC3F ;
    LCDCPCTL2 = 0x0FFF ;

    // Configure LFXT 32 kHz crystal
    CSCTL0_H = CSKEY >> 8; // Unlock CS registers
    CSCTL4 &= ~ LFXTOFF ; // Enable LFXT

    do {
        CSCTL5 &= ~ LFXTOFFG ; // Clear LFXT fault flag
        SFRIFG1 &= ~ OFIFG ;
    } while ( SFRIFG1 & OFIFG ) ; // Test oscillator fault flag

    CSCTL0_H = 0; // Lock CS registers

    // Initialize LCD_C
    // ACLK , Divider = 1 , Pre - divider = 16; 4 - pin MUX
    LCDCCTL0 = LCDDIV__1 | LCDPRE__16 | LCD4MUX | LCDLP ;

    // VLCD generated internally ,
    // V2 -V4 generated internally , v5 to ground
    // Set VLCD voltage to 2.60 v
    // Enable charge pump and select internal reference for it
    LCDCVCTL = VLCD_1 | VLCDREF_0 | LCDCPEN ;
    LCDCCPCTL = LCDCPCLKSYNC ; // Clock synchronization enabled
    LCDCMEMCTL = LCDCLRM ; // Clear LCD memory

    // Turn LCD on
    LCDCCTL0 |= LCDON ;
    return ;
}
void config_ACLK_to_32KHz_crystal () {
    // By default , ACLK runs on LFMODCLK at 5 MHz /128 = 39 KHz
    // Reroute pins to LFXIN / LFXOUT functionality
    PJSEL1 &= ~ BIT4 ;
    PJSEL0 |= BIT4 ;

    // Wait until the oscillator fault flags remain cleared
    CSCTL0 = CSKEY ; // Unlock CS registers

    do {
        CSCTL5 &= ~ LFXTOFFG ; // Local fault flag
        SFRIFG1 &= ~ OFIFG ; // Global fault flag
    } while (( CSCTL5 & LFXTOFFG ) != 0) ;

    CSCTL0_H = 0; // Lock CS registers
    return ;
}
void ShowBuffer ( int buffer []) {
    LCDMEM [9] = alphabetBig[( buffer [5]) -65][0];
    LCDMEM [10] = alphabetBig[( buffer [5]) -65][1];
    LCDMEM [5] = alphabetBig[( buffer [4]) -65][0];
    LCDMEM [6] = alphabetBig[( buffer [4]) -65][1];
    LCDMEM [3] = alphabetBig[( buffer [3]) -65][0];
    LCDMEM [4] = alphabetBig[( buffer [3]) -65][1];
    LCDMEM [18] = alphabetBig[( buffer [2]) -65][0];
    LCDMEM [19] = alphabetBig[( buffer [2]) -65][1];
    LCDMEM [14] = alphabetBig[( buffer [1]) -65][0];
    LCDMEM [15] = alphabetBig[( buffer [1]) -65][1];
    LCDMEM [7] = alphabetBig[( buffer [0]) -65][0];
    LCDMEM [8] = alphabetBig[( buffer [0]) -65][1];
}
char siguienteLetra(){
    char ret = 'A' + idxLetra;//generamos el carácter a enviar

    //actualizamos el idxLetra para avanzar a la siguiente letra
    if (idxLetra == 25){//hemos llegado al final del abecedario
        idxLetra = 0;
    }
    else{
         idxLetra++;
    }

    return ret;//devolvemos la letra
}
