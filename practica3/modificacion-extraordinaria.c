//Modificación extraordinaria práctica 3 por Pablo García Fernández
#include <msp430.h>
#include <msp430fr6989.h>

/*
    MODIFICACIÓN EXTRAORDINARIA

Modificar el programa de la práctica 3 de la siguiente manera:
• Cambiar la funcionalidad del botón que resetea la cuenta para que al pulsarlo por primera vez incremente el contador de 2 en 2, la segunda vez de 5 en 5 y la tercera vez de 10 en 10. Si se pulsa una cuarta vez vuelve a contar de 1 en 1, y a partir de ahí las siguientes veces de 2 en 2, 5 en 5 y 10 en 10, repitiendo el comportamiento descrito anteriormente.
• Cambiar la funcionalidad del botón que para/reanuda la cuenta para que al pulsarse cuente de manera descendente hasta alcanzar el 0. Con cada pulsación se debe alternar el sentido de la cuenta descendente/ascendente. La cuenta descendente siempre para en el 0.

*/

//VARIABLES GLOBALES
int cnt = 0;
const unsigned char LCD_Num [10] = {0xFC , 0x60 , 0xDB , 0xF3 , 0x67 , 0xB7 , 0xBF , 0xE0 , 0xFF , 0xE7 };
unsigned int incremento = 1;
unsigned int valorSalto = 1;


//PROTOTIPOS
void config_ACLK_to_32KHz_crystal ();
inline void display_num_lcd ( unsigned int n );
void Initialize_LCD ();
void cambioValorSalto();

void main(void){
    // disable watchdog
    WDTCTL = WDTPW | WDTHOLD;
    // disable high impeadance on I / O
    PM5CTL0 &= ~LOCKLPM5;

    //INICIALIZACION DEL LCD Y MUESTREO INICIAL
    Initialize_LCD();

    //CONFIGURACION DEL TIMER0
    TA0CTL = TASSEL__ACLK | ID__1 | MC__UP | TACLR ;
    TA0CCR0 = 40000;
    TA0CCTL0 = CCIE;

    config_ACLK_to_32KHz_crystal();

    //config leds y pushbuttons
    P1DIR = BIT0;
    P9DIR = BIT7;
    P1OUT &= ~ BIT0;
    P9OUT &= ~ BIT7;

    P1DIR &= ~ BIT1;
    P1DIR &= ~ BIT2;

    //CONFIGURAR BOTONES
    //P1.1
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

    //P1.2
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

    //__bis_SR_register ( LPM4_bits + GIE ) ;
    __enable_interrupt () ;

    while(1){
        display_num_lcd(cnt);
    }
}

//INTERRUPCIONES
# pragma vector = PORT1_VECTOR
__interrupt void Port_1 ( void ) {
    if(P1IFG & BIT1)//INT P1.1: cambio de modo de incremento a decremento y viceversa
    {
        if(incremento) incremento = 0;
        else{
            incremento = 1;
            if(TA0CTL & MC__STOP) TA0CTL |= MC__UP; //Si el timer se paró entonces lo reanudamos
        }

        //limpia el flag de interrupcion
        P1IFG &= ~BIT1 ;
    }
    else if (P1IFG & BIT2)//INT P1.2: cambia el valor de salto de la cuenta
    {
        cambioValorSalto();

        //limpia el flag de interrupcion
        P1IFG &= ~BIT2;
    }
}

# pragma vector = TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR (void) {
    if(incremento) cnt += valorSalto;
    else cnt -= valorSalto;

    if (cnt < 0) cnt = 0;

    if (cnt == 0) TA0CTL |= MC__STOP;
}

// **********************************
// Configures ACLK to 32 KHz crystal
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

// *************** function that displays any 16 - bit unsigned integer ************
inline void display_num_lcd ( unsigned int n ) {
    int i = 0;
    do {
        unsigned int digit = n % 10;
        switch ( i ) {
            case 0: LCDM8 = LCD_Num [ digit ]; break ; // first digit
            case 1: LCDM15 = LCD_Num [ digit ]; break ; // second digit
            case 2: LCDM19 = LCD_Num [ digit ]; break ; // third digit
            case 3: LCDM4 = LCD_Num [ digit ]; break ; // fourth digit
            case 4: LCDM6 = LCD_Num [ digit ]; break ; // fifth digit
        }
        n /= 10;
        i ++;
    } while ( i < 5) ;
}

// **********************************************************
// Initializes the LCD_C module
// *** Source : Function obtained from MSP430FR6989 Sample Code ***
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
void cambioValorSalto(){
    switch (valorSalto) {
        case 1: valorSalto = 2;
            break;
        case 2: valorSalto = 5;
            break;
        case 5: valorSalto = 10;
            break;
        case 10: valorSalto = 1;
            break;
        default: valorSalto = 1;
            break;
    }
}