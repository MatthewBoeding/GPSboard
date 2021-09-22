#include <xc.h>
#include <stdbool.h>
#include <pic18f26q84.h>
#include "boardInit.h"
#include "can.h"
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
    // CLKRMD CLKR enabled; SYSCMD SYSCLK enabled; SCANMD SCANNER enabled; FVRMD FVR enabled; IOCMD IOC enabled; CRCMD CRC enabled; HLVDMD HLVD enabled; 
    PMD0 = 0x00;
    // TMR0MD TMR0 enabled; TMR1MD TMR1 enabled; TMR4MD TMR4 enabled; SMT1MD SMT1 enabled; TMR5MD TMR5 enabled; TMR2MD TMR2 enabled; TMR3MD TMR3 enabled; TMR6MD TMR6 enabled; 
    PMD1 = 0x00;
    // TU1MD UT16A enabled; CANMD CAN enabled; TU2MD UT16B enabled; 
    PMD2 = 0x00;
    // ZCDMD ZCD enabled; DACMD DAC enabled; ADCMD ADC enabled; ACTMD ACT enabled; CM2MD CM2 enabled; CM1MD CM1 enabled; 
    PMD3 = 0x00;
    // NCO1MD NCO1 enabled; NCO2MD NCO2 enabled; DSM1MD DSM1 enabled; CWG3MD CWG3 enabled; CWG2MD CWG2 enabled; CWG1MD CWG1 enabled; NCO3MD NCO3 enabled; 
    PMD4 = 0x00;
    // CCP2MD CCP2 enabled; CCP1MD CCP1 enabled; PWM2MD PWM2 enabled; CCP3MD CCP3 enabled; PWM1MD PWM1 enabled; PWM4MD PWM4 enabled; PWM3MD PWM3 enabled; 
    PMD5 = 0x00;
    // U5MD UART5 enabled; U4MD UART4 enabled; U3MD UART3 enabled; U2MD UART2 enabled; U1MD UART1 enabled; SPI2MD SPI2 enabled; SPI1MD SPI1 enabled; I2C1MD I2C1 enabled; 
    PMD6 = 0x00;
    // CLC5MD CLC5 enabled; CLC6MD CLC6 enabled; CLC3MD CLC3 enabled; CLC4MD CLC4 enabled; CLC7MD CLC7 enabled; CLC8MD CLC8 enabled; CLC1MD CLC1 enabled; CLC2MD CLC2 enabled; 
    PMD7 = 0x00;
    // DMA5MD DMA5 enabled; DMA6MD DMA6 enabled; DMA8MD DMA8 enabled; DMA7MD DMA7 enabled; DMA1MD DMA1 enabled; DMA2MD DMA2 enabled; DMA3MD DMA3 enabled; DMA4MD DMA4 enabled; 
    PMD8 = 0x00;
}

void portInit(void){
    //Latch setup
    LATA = 0x00;
    LATB = 0x00;
    LATC = 0x80;

    //Tristate (input/output)
    TRISE = 0x08;
    TRISA = 0xFF;
    TRISB = 0xF7;
    TRISC = 0xB1;

    //Analog select
    ANSELC = 0x00;
    ANSELB = 0xEF;
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
    
    //can 1 RX -> RB4
    RB3PPS = 0x46;
    //can 1 TX -> RB3
    CANRXPPS = 0x0C;  
    
    RC6PPS = 0x20;   //RC6->UART1:TX1;    
    U1RXPPS = 0x17;   //RC7->UART1:RX1;   
    
    RC0PPS = 0x32;   
    //RC0->SPI1:SDO1;    
    SPI1SCKPPS = 0x11;   
    //RC1->SPI1:SCK1;
}

void uartInit(void){
    // Disable interrupts before changing states

    // Set the UART1 module to the options selected in the user interface.

    // P1L 0; 
    U1P1L = 0x00;
    // P1H 0; 
    U1P1H = 0x00;
    // P2L 0; 
    U1P2L = 0x00;
    // P2H 0; 
    U1P2H = 0x00;
    // P3L 0; 
    U1P3L = 0x00;
    // P3H 0; 
    U1P3H = 0x00;
    // BRGS high speed; MODE Asynchronous 8-bit mode; RXEN enabled; TXEN enabled; ABDEN disabled; 
    U1CON0 = 0xB0;
    // RXBIMD Set RXBKIF on rising RX input; BRKOVR disabled; WUE disabled; SENDB disabled; ON enabled; 
    U1CON1 = 0x80;
    // TXPOL not inverted; FLO off; C0EN Checksum Mode 0; RXPOL not inverted; RUNOVF RX input shifter stops all activity; STP Transmit 1Stop bit, receiver verifies first Stop bit; 
    U1CON2 = 0x00;
    // BRGL 130; 
    U1BRGL = 0xA0;
    // BRGH 6; 
    U1BRGH = 0x01;
    // STPMD in middle of first Stop bit; TXWRE No error; 
    U1FIFO = 0x00;
    // ABDIF Auto-baud not enabled or not complete; WUIF WUE not enabled by software; ABDIE disabled; 
    U1UIR = 0x00;
    // ABDOVF Not overflowed; TXCIF 0; RXBKIF No Break detected; RXFOIF not overflowed; CERIF No Checksum error; 
    U1ERRIR = 0x00;
    // TXCIE disabled; FERIE disabled; TXMTIE disabled; ABDOVE disabled; CERIE disabled; RXFOIE disabled; PERIE disabled; RXBKIE disabled; 
    U1ERRIE = 0x00;
    return;
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
    C1FIFOBA = 0x2600;
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
    TMR0H = 0X3C;
    TMR0L = 0XB0;
    
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
    oscInit();
    pmdInit();
    portInit();
    uartInit();
    bool success = CAN1_Initialize();
    //bool success = canInit();
    timerInit();
    __delay_ms(1);
    return success;
}

