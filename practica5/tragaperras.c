// Máquina tragaperras por Pablo García Fernández
#include <msp430.h> 

//VARIABLES GLOBALES
unsigned int cuenta = 0;//variable que almacenará el valor que va llevando el timer
unsigned int figurasPuestas = 0;//variable que indica el fin de una ronda contando el número de figuras ya puestas. También nos será útil para tras finalizar la ronda resetear los valores
int bufferLCD[6]={'O','I','O','I','O','I'};//Buffer visual en el LCD. La I representa una columna separadora, la O representa los huecos en los cuales se colocarán las figuras
int figuras[5]={'A','B','C','D','E'};//Figuras posibles
int huecos[3];//buffer con las 3 casillas/huecos de la tragaperras
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
    {0x90 , 0x28 }
};

//PROTOTIPOS
void Initialize_LCD();
void config_ACLK_to_32KHz_crystal();
void ShowBuffer (int buffer[]);
unsigned int compruebaVictoria(int buffer[]);
int max(int a, int b);
void resetBuffers();

int main(void){
    WDTCTL = WDTPW | WDTHOLD;
    PM5CTL0 &= ~LOCKLPM5;

    //Config LCD
    Initialize_LCD();
    config_ACLK_to_32KHz_crystal();

    //Config LEDs
    P1DIR = BIT0;
    P1OUT = 0x0;
    P9DIR = BIT7;
    P9OUT = 0x0;

    //Config pushbuttons
    // P1.1 is input
    P1DIR &= ~ BIT1;

    // enable pull on P1 .1 , rest disabled
    P1REN |= BIT1;
    // set pull - up on P1 .1
    P1OUT |= BIT1;

    // P1 .1 IRQ enabled
    P1IE |= BIT1;

    // P1 .1 Falling edge
    P1IES |= BIT1;
    // P1 .1 clear pending interrupts
    P1IFG &= ~ BIT1;

    // P1.2 is input
    P1DIR &= ~ BIT2;

    // enable pull on P1 .2 , rest disabled
    P1REN |= BIT2;
    // set pull - up on P1 .2
    P1OUT |= BIT2;

    // P1 .2 IRQ enabled
    P1IE |= BIT2;

    // P1 .2 Falling edge
    P1IES |= BIT2;
    // P1 .2 clear pending interrupts
    P1IFG &= ~ BIT2;

    //Config timer
    TA0CTL = TASSEL__ACLK | ID__1 | MC__UP | TACLR ;
    TA0CCR0 = 3276;//0,1segundos según el enunciado de la práctica 3
    TA0CCTL0 = CCIE;

    ShowBuffer(bufferLCD);

    __enable_interrupt();
	
	return 0;
}

//INTERRUPCIONES
# pragma vector = TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR (void) {
    cuenta++;
    if(cuenta>=5){
        cuenta = 0;
    }
}
# pragma vector = PORT1_VECTOR
__interrupt void Port_1 ( void ) {
    if(figurasPuestas == 4){//hemos vuelto ha pulsar tras colocar las 3 figuras, reseteo
        //APAGAMOS LEDS
        P1OUT&= ~BIT0;
        P9OUT&= ~BIT7;

        //RESETEAMOS BUFFERS
        resetBuffers();

        //RESETEAMOS VALORES DE CUENTA
        cuenta = 0;
        figurasPuestas = 0;
    } else {
        int elem = figuras[cuenta];//funcionamiento normal
        //int elem = 'A';//usado para la comprobación de victoria

        huecos[figurasPuestas] = elem;
        bufferLCD[figurasPuestas*2] = elem;
        figurasPuestas++;

        if(figurasPuestas==3){//hemos colocado las 3, comprobación de victoria
            switch (compruebaVictoria(huecos)) {
                case 1://DERROTA
                    //ENCIENDE LED ROJO
                    P1OUT |= BIT0;
                    break;
                case 3://VICTORIA
                    //ENCIENDE LED VERDE
                    P9OUT |= BIT7;
                    break;
                default: break;
            }
            figurasPuestas++;//para que la proxima vez que se pulse se resetee la partida
        }
    }

    ShowBuffer(bufferLCD);
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
unsigned int compruebaVictoria(int buffer[]){
    //calcula la figura que más veces se ha repetido y en base a ello devuelve los siguientes códigos:
    // 1: derrota
    // 2: empate
    // 3: victoria
    // (estos códigos coinciden con el valor de aquella figura más repetida)
    unsigned int repeticiones[5]={0,0,0,0,0};
    int i, maxValue;

    for(i = 0; i < 3; i++){
        switch (buffer[i]) {
            case 'A':
                repeticiones[0]++;
                break;
            case 'B':
                repeticiones[1]++;
                break;
            case 'C':
                repeticiones[2]++;
                break;
            case 'D':
                repeticiones[3]++;
                break;
            case 'E':
                repeticiones[4]++;
                break;
            default:
                break;
        }
    }

    maxValue = 0;
    for(i = 0; i < 5; i++){
        maxValue = max(maxValue, repeticiones[i]);
    }

    return maxValue;
}

int max(int a, int b){
    if (a>b) return a;
    else return b;
}

void resetBuffers(){
    int i;
    for(i=0; i<3; i++) huecos[i] = 0;
    for(i=0; i<6; i=i+2) bufferLCD[i] ='O';
}
