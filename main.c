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
#include "libs/uart2.h"

#define _XTAL_FREQ 64000000

bool received_flag;
bool measure_flag;
bool validFrame = false;
bool fixed;
uint8_t canMsg[16];
uint8_t frameBuffer[100];
uint8_t lat[6];
uint8_t lon[6];



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

uint8_t getFrame(void)
{
    bool receive = true;
    while(receive){
        if(!uartRecvReady())
        {
          __delay_ms(1);
          if(!uartRecvReady())
          {
              receive = false;
          }
        }
        uint8_t frameSize = 0;
        frameBuffer[frameSize] = uartRecv();
        frameSize++;
        if(frameSize == 100){
            receive = false
        }
    }
}

uint8_t uartFrameBuffer(void){
    uint8_t frameSize = 0;
    bool receiving = true;
    while(receiving){
        if(uartRecvReady())
        {
            //First char of NMEA frames is $. The goal here is to catch the beginning of a frame
            //We will keep entering this loop until beginning of frame is captured
            if(uartRecv() == '$'){
                frameSize = getFrame();
            }
        } else
        {
            __delay_ms(1);
            if(!uartRecvReady())
            {
                receiving = false;
            }
        }
    }
    return frameSize;
}


void processFrame(uint8_t size)
{
    bool valid = true;
    uint8_t header[] = "GNGLL";
    for (int i=0; i<5; i++){
        if(header[i] != frameBuffer[i])
        {
            valid = false;
        }
    }
    if(valid){
        uint8_t commas[7];
        int i,j = 0;
        while(i<7 && j <100)
        {
            if(frameBuffer[j] == ',')
            {
                commas[i] = j;
                i++;
            }
            j++;
        }
        if(frameBuffer[(commas[5]+1)] == 'V' || (commas[1] - commas[0] < 2))
        {
            fixed = false;
            lat = ["0","0",".","0","0","N"];
            lon = ["0","0","0",".","0","E"];                
        } else
        {
            lat[0] = frameBuffer[commas[0]+1];
            lat[1] = frameBuffer[commas[0]+2];
            lat[2] = '.';
            lat[3] = frameBuffer[commas[0]+3];
            lat[4] = frameBuffer[commas[0]+4];
            lat[5] = frameBuffer[commas[1]+1];
            
            lon[0] = frameBuffer[commas[2]+1];
            lon[1] = frameBuffer[commas[2]+2];
            lon[2] = frameBuffer[commas[2]+3];
            lon[3] = '.';
            lon[4] = frameBuffer[commas[2]+4];
            lon[5] = frameBuffer[commas[3]+1];
        }
    }
}

void main(void) {
    //System Setup
    bool setup = boardInit();
    uint8_t frameSize = 0;
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
            frameSize = uartFrameBuffer();
            if (frameSize > 0)
            {
                measure_flag = false;
                processFrame(frameSize);
            }
        }
    }
}
