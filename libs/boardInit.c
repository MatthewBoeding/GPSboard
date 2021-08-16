#include <xc.h>
#include <stdbool.h>
#include "boardInit.h"

#define _XTAL_FREQ 64000000

void interruptInit(void){
    //Global interrupts, no priority
    INTCON0bits.GIE = 1;
    INTCON0bits.IPEN = 0;

}

void oscInit(void){
    /* 
     * set oscillator to use high frequency internal
     *  at 16 MHz
     */
    OSCCON1 = 0x60;
    OSCCON3 = 0x00;
    OSCTUNE = 0x00;
    OSCFRQ = 0x05;
    OSCEN = 0x00;
}

void pmdInit(void){
    PMD0 = 0x00;
    //Enable Timer 0
    PMD1 = 0xFE;
    //Enable CAN
    PMD2 = 0x00;
    PMD3 = 0xE7;
    PMD4 = 0x7F;
    PMD5 = 0xF7;
    // Only enable UART 1 and SPI1
    PMD6 = 0xF5; 
    PMD7 = 0xFF;
    PMD8 = 0xFF;   
}

void portInit(void){
    //Latch setup
    LATA = 0x00;
    LATB = 0x00;
    LATC = 0x00;

    //Tristate (input/output)
    TRISE = 0x08;
    TRISA = 0xFF;
    TRISB = 0xEF;
    TRISC = 0x70;

    //Analog select
    ANSELC = 0xB0;
    ANSELB = 0xF7;
    ANSELA = 0xFF;

    //Weak pull-up
    WPUE = 0x00;
    WPUB = 0x00;
    WPUA = 0x00;
    WPUC = 0x00;

    //inverted levels
    INLVLA = 0xFF;
    INLVLB = 0xFF;
    INLVLC = 0xFF;
    INLVLE = 0x08;

    //slew rate
    SLRCONA = 0xFF;
    SLRCONB = 0xFF;
    SLRCONC = 0xFF;
    
    //open drain
    ODCONA = 0x00;
    ODCONB = 0x00;
    ODCONC = 0x00;
    
    //can 1 RX -> RB3
    CANRXPPS = 0x0B;  
    //can 1 TX -> RB4
    RB4PPS = 0x46;
    //UART 1 RX -> RC6
    U1RXPPS = 0x16;
    //UART 1 TX -> RC7
    RC7PPS = 0x20;   
    //RC7->UART1:TX1; 
    
    RC0PPS = 0x32;   
    //RC0->SPI1:SDO1;    
    SPI1SCKPPS = 0x11;   
    //RC1->SPI1:SCK1;
}

void uartInit(void){
    //No need for parameters 
    U1P1L = 0x00;
    U1P1H = 0x00;
    U1P2L = 0x00; 
    U1P2H = 0x00;
    U1P3L = 0x00;
    U1P3H = 0x00;
    //Using 8-bit no parity no checksum
    U1CON0 = 0xB0;
    U1CON1 = 0x80;
    U1CON2 = 0x00;
    //no error checking interrupts
    U1ERRIR = 0x00;
    U1ERRIE = 0x00;
    //no auto-baud
    U1UIR = 0x00;
    //stop bit checking and no error checking
    U1FIFO = 0x00;
    //setup baud (9600 = (16MHz)*4/[16*BRG]) (BRG = 416)
    U1BRGL = 0xA0;
    U1BRGH = 0x01;
}

bool canSetOP(uint8_t code){
    bool success = true;
    uint16_t i = 0;
    C1CONTbits.REQOP = code;
    while (C1CONUbits.OPMOD != code){
        if (1 == C1INTHbits.SERRIF || i > 40000)
            {
                success = false;
                break;
            }
        i++;
    }
    return success;
}

bool canInit(void)
{
    bool complete = true;
    // Turn it on
    C1CONHbits.ON = 1;
    //Set to configuration
    complete = canSetOP(4);
    if (complete){
    //Starting memory base for FIFO
    C1FIFOBA = 0x3800;
    // Internal clock, exception is form error; ISOCRC enabled; devicenet disabled; 
    C1CONL = 0x60;
    // ON enabled; FRZ disabled; SIDL disabled; BRSDIS enabled; WFT T11 Filter; WAKFIL enabled; 
    C1CONH = 0x97;
    // Transmit Queue disabled 
    C1CONU = 0x00;
    //CAN-baud and sampling configuration
    
    // TX FIFO ENABLE
    // TXATIE disabled; TXQEIE disabled; TXQNIE disabled; 
    C1TXQCONL = 0x00;
    // FRESET enabled; UINC disabled; 
    C1TXQCONH = 0x04;
    // TXAT 3; TXPRI 1; 
    C1TXQCONU = 0x60;
    // PLSIZE 8; FSIZE 2; 
    C1TXQCONT = 0x01;
    
    
    // SJW 5; 
    C1NBTCFGL = 0x05;    
    // TSEG2 5; 
    C1NBTCFGH = 0x05;    
    // TSEG1 24; 
    C1NBTCFGU = 0x18;    
    // BRP 0; 
    C1NBTCFGT = 0x00;
    
    complete = canSetOP(5);
    }
    
    //Set as receive queue, interrupt for FIFO not empty
    C1FIFOCON1L = 0x01;
    // reset fifo
    C1FIFOCON1H = 0X04;
    //disable retransmission
    C1FIFOCON1U = 0X60;
    //FIFO1 8 byte payload, size: 2 (32 BYTES RAM)
    C1FIFOCON1T = 0x01;
    
    //CAN RX interrupt
    PIR4bits.CANRXIF = 0;
    PIE4bits.CANRXIE = 1; 

    
    return complete;
}

void spiInit(void){
    //EN disabled; LSBF MSb first; MST bus slave; BMODE last byte; 
    SPI1CON0 = 0x02;
    //SMP Middle; CKE Active to idle; CKP Idle:Low, Active:High; FST disabled; SSP active high; SDIP active high; SDOP active high; 
    SPI1CON1 = 0x40;
    //SSET disabled; TXR not required for a transfer; RXR data is not stored in the FIFO; 
    SPI1CON2 = 0x00;
    //CLKSEL FOSC; 
    SPI1CLK = 0x00;
    //BAUD 4; 
    SPI1BAUD = 0x04;
    TRISCbits.TRISC3 = 0;
	//Turn it on
	SPI1CON0bits.EN = 1;
    return;
}
void timerInit(void){
    //~4 SECOND DELAY
    
    
    //TIMER 0: INTERNAL CLOCK SYNC TO F0SC/4 1:64
    T0CON1 = 0X66;    
    //16MHz/64 = 250KHz
    //TMR0 = 65535 / 250KHZ = .026214 sec
    TMR0H = 0XFF;
    TMR0L = 0XFF;
    
    PIR3bits.TMR0IF = 0;
    //Timer 0 interrupt
    PIE3bits.TMR0IE = 1;
    
    //.026214*16 = 4.19 seconds between interrupts
    //TURN ON (16 BIT MODE) 1:16
    T0CON0 = 0X9f; 
    return;
}

bool boardInit(void){
    interruptInit();
    portInit();
    pmdInit();
    oscInit();
    uartInit();
    bool success = canInit();
    timerInit();
    __delay_ms(1);
    spiInit();
    __delay_ms(1);
    return success;
}

