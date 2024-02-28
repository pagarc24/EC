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

// Generar un programa en C embebido que utilizando el contador TIMER0 del microprocesador (inicializado a 40.000) y a través
// del método de interrupciones, muestre el conteo en el LCD de la placa añadiendo el control de los dos switches (botones) para Parar/Continuar y Reiniciar el contador
// Botón1: La parada del contador cuando se pulsa el switch. Si se vuelve a pulsar continua el conteo desde el valor actual.
// Botón2: Reiniciar el contador a 0.

//VARIABLES GLOBALES
unsigned int cnt = 0;
unsigned int ctrl_cuenta = 1;
const unsigned char LCD_Num [10] = {0xFC , 0x60 , 0xDB , 0xF3 , 0x67 , 0xB7 , 0xBF , 0xE0 , 0xFF , 0xE7 };

//PROTOTIPOS
void config_ACLK_to_32KHz_crystal ();
inline void display_num_lcd ( unsigned int n );
void Initialize_LCD ();

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

    // set P1 .0 as output , rest as inputs (EXTRA)
    P1DIR = BIT0;
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

    //P1.2 botón de resetear la cuenta
    //P1.2 botón de pausa y continuar
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
    if(P1IFG & BIT1)//INT P1.1
    {
        //parada o reanudacion
        if(ctrl_cuenta) ctrl_cuenta = 0;
        else ctrl_cuenta = 1;

        //limpia el flag de interrupcion
        P1IFG &= ~BIT1 ;
    }
    else if (P1IFG & BIT2)//INT P1.2
    {
        //reseteo del timer
        TA0CTL = TASSEL__ACLK | ID__1 | MC__UP | TACLR ;

        //reseteo del valor a contar
        cnt = 0;

        //display_num_lcd(cnt);

        //limpia el flag de interrupcion
        P1IFG &= ~BIT2;
    }
}

# pragma vector = TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR (void) {
    if (ctrl_cuenta) cnt++;
    else P1OUT ^=BIT0;
    //display_num_lcd(cnt);
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
*/

/*
    MODIFICACIÓN

//Modificar el programa de la práctica 3 de la siguiente manera:
// • Cuando la cuenta llegue al valor 5 se encenderá el led rojo y permanecerá encendido hasta que la cuenta alcance el valor 15.
// • Cuando la cuenta alcance el valor 10 se encenderá el led verde y permanecerá encendido hasta que la cuenta alcance el valor 20.
// • Cuando la cuenta alcance el valor 20 se reseteará a 0.
// • El funcionamiento de los pulsadores sigue siendo el mismo. De modo que, al pausar la cuenta los leds permanecen en el estado en que estaban antes de la pausa; y al reiniciar la cuenta se apagan los 2 leds

//VARIABLES GLOBALES
unsigned int cnt = 0;
unsigned int ctrl_cuenta = 1;
const unsigned char LCD_Num [10] = {0xFC , 0x60 , 0xDB , 0xF3 , 0x67 , 0xB7 , 0xBF , 0xE0 , 0xFF , 0xE7 };

//PROTOTIPOS
void config_ACLK_to_32KHz_crystal ();
inline void display_num_lcd ( unsigned int n );
void Initialize_LCD ();

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

    //P1.2 botón de resetear la cuenta
    //P1.2 botón de pausa y continuar
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
    if(P1IFG & BIT1)//INT P1.1
    {
        //parada o reanudacion
        if(ctrl_cuenta) ctrl_cuenta = 0;
        else ctrl_cuenta = 1;

        //limpia el flag de interrupcion
        P1IFG &= ~BIT1 ;
    }
    else if (P1IFG & BIT2)//INT P1.2
    {
        //reseteo del timer
        TA0CTL = TASSEL__ACLK | ID__1 | MC__UP | TACLR ;

        //reseteo del valor a contar
        cnt = 0;

        //reseteo leds
        P1OUT &= ~ BIT0;
        P9OUT &= ~ BIT7;

        //limpia el flag de interrupcion
        P1IFG &= ~BIT2;
    }
}

# pragma vector = TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR (void) {
    if (ctrl_cuenta) {
        cnt++;
        if(cnt == 5){
            P1OUT |= BIT0;
        } else if (cnt == 10){
            P9OUT |= BIT7;
        } else if (cnt == 15){
            P1OUT &= ~ BIT0;
        } else if (cnt == 20){
            P9OUT &= ~ BIT7;
            cnt = 0;
        }
    }
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
*/
