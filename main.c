/*
 * File:   main.c
 * Author: Matthew
 *
 * Created on July 22, 2021, 2:13 PM
 */
// PIC18F26Q84 Configuration Bit Settings

// 'C' source line config statements

// CONFIG1
#pragma config FEXTOSC = OFF    // External Oscillator Selection (Oscillator not enabled)
#pragma config RSTOSC = HFINTOSC_64MHZ// Reset Oscillator Selection (HFINTOSC with HFFRQ = 64 MHz and CDIV = 1:1)

// CONFIG2
#pragma config CLKOUTEN = OFF   // Clock out Enable bit (CLKOUT function is disabled)
#pragma config MVECEN = OFF
#pragma config LVP = ON
// CONFIG5
#pragma config WDTCPS = WDTCPS_31// WDT Period selection bits (Divider ratio 1:65536; software control of WDTPS)
#pragma config WDTE = OFF       // WDT operating mode (WDT Disabled; SWDTEN is ignored)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>
#include "libs/boardInit.h"
#include "libs/can.h"
#include "libs/lcd.h"
#include "libs/lcd_regs.h"

#define _XTAL_FREQ 64000000

bool received_flag;
bool measure_flag;
uint8_t canMsg[16];


void TMR0_ISR(void){
    measure_flag = true;
    PIR3bits.TMR0IF = 0;
    TMR0H = 0xFF;
    TMR0L = 0xFF;
}

void CAN1_RXI_ISR(void){
    received_flag = true;
    PIR3bits.TMR0IF = 0;
}

void __interrupt(irq(TMR0), irq(CAN)) INTERRUPT_InterruptManager (void)
{    
    if(PIE3bits.TMR0IE == 1 && PIR3bits.TMR0IF == 1)
    {
        TMR0_ISR();
        PIR3bits.TMR0IF = 0;
    }
    else if(PIE4bits.CANRXIE == 1 && PIR4bits.CANRXIF == 1)
    {
        CAN1_RXI_ISR();
        PIR4bits.CANRXIF = 0;
    } 
    else
    {
        //Unhandled Interrupt
    }
}

void uartWrite(uint8_t txData)
{
    while(0 == PIR4bits.U1TXIF)
    {
    }

    U1TXB = txData;    // Write the data byte to the USART.
}

bool uartRecvReady(void)
{
    return (bool)(PIR4bits.U1RXIF);
}

uint8_t uartRecv(void)
{
    return U1RXB;
}

void main(void) {
    //System Setup
    bool setup = boardInit();
    lcd_init();
    fillScreen(GRAY);
    setTextColor(GRAY, RED);
    LCD_string_write("Begin writing:");
    while(1)
    {
        if (received_flag && setup)
        {
            canReceive();
            received_flag = false;
        }
        if (measure_flag)
        {
            measure_flag = false;
        }
    }
}
