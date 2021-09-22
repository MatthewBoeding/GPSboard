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
#pragma config JTAGEN = OFF     // JTAG Enable bit (Disable JTAG Boundary Scan mode, JTAG pins revert to user functions)
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
#include "libs/can_types.h"

#define _XTAL_FREQ 64000000

bool received_flag;
bool measure_flag;
bool validFrame = false;
bool fixed;
uint8_t canMsg[16];
uint8_t data[6];
uint8_t frameBuffer[100];
uint8_t lat[8];
uint8_t lon[8];
uint8_t fixedlat[8] = {"LA00.00N"};
uint8_t fixedlon[8] = {"LO000.0E"};  
uint16_t validReceived = 0;
uint8_t gpsOn[8] = {0xB5, 0x62, 0x06, 0x57, 0x00, 0x00, 0x5D, 0x1D};
uint8_t pwrControl[16] = {0xB5, 0x62, 0x06, 0x86, 0x08, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x97, 0x6F};
uint8_t pwrControl2[8] = {0xB5, 0x62, 0x06, 0x86, 0x00, 0x00, 0x8C, 0xAA};
uint8_t coldRestart [12] = {0xB5, 0x62, 0x06, 0x04, 0x04, 0x00, 0xFF, 0xFF, 0x02, 0x00, 0x0E, 0x61};

void TMR0_ISR(void){
    measure_flag = true;
    PIR3bits.TMR0IF = 0;
    TMR0H = 0xFF;
    TMR0L = 0xFF;
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
        PIR4bits.U1RXIF = 0;
        PIR4bits.U1TXIF = 0;
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
    while(!PIR4bits.U1RXIF);
    PIR4bits.U1RXIF = 0;
    return U1RXB;
}

uint8_t getFrame(void)
{
    uint8_t frameSize = 0;
    bool receive = true;
    bool new = false;
    uint8_t helper;
    while(receive){
        while(new == false)
        {
            data[0] = uartRecv();
            if(data[0] == '$')
            {
                uint8_t header[5] = {"GNGLL"};
                data[1] = uartRecv();
                data[2] = uartRecv();
                data[3] = uartRecv();
                data[4] = uartRecv();
                data[5] = uartRecv();
                new = true;
                for(int i = 0; i < 5; i++)
                {
                    if(data[i+1] != header[i])
                    {
                        new = false;
                    }
                }
                
            }
        }
        helper = uartRecv();
        if(helper == '$'){
            receive = false;
        }else
        {
            frameBuffer[frameSize] = helper;
            frameSize++;
        }
    }
    return frameSize;
}


void processFrame(uint8_t size)
{
    bool valid = true;
    uint8_t header[] = "GNGLL";
    LATCbits.LATC2 = 1;
    if(valid){
        LATCbits.LATC3 = 1;
        uint8_t commas[7];
        int i = 0;
        int j = 0;
        while(i<7 && j <= size)
        {
            if(frameBuffer[j] == ',')
            {
                commas[i] = j;
                i++;
            }
            j++;
        }
        uint8_t test = commas[5] +1; 
        if(frameBuffer[(commas[5]+1)] == 'V' || (commas[1] - commas[0] < 2))
        {
            fixed = false;
            for(int i = 0; i<8; i++)
            {
                lat[i] = fixedlat[i];
                lon[i] = fixedlon[i];
            }
        } else
        {
            lat[2] = frameBuffer[commas[0]+1];
            lat[3] = frameBuffer[commas[0]+2];
            lat[4] = '.';
            lat[5] = frameBuffer[commas[0]+3];
            lat[6] = frameBuffer[commas[0]+4];
            lat[7] = frameBuffer[commas[1]+1];
            
            lon[2] = frameBuffer[commas[2]+1];
            lon[3] = frameBuffer[commas[2]+2];
            lon[4] = frameBuffer[commas[2]+3];
            lon[5] = '.';
            lon[6] = frameBuffer[commas[2]+4];
            lon[7] = frameBuffer[commas[3]+1];
        }
        LATCbits.LATC2 = 0;
        LATCbits.LATC3 = 0;
    }
}

void gpsCommand(uint8_t* data, uint8_t len)
{
    for(int i = 0; i < len; i++)
    {
        uartWrite(*data);
        data++;
    }
}

void gpsInit()
{
    gpsCommand(gpsOn, sizeof(gpsOn));
    gpsCommand(pwrControl, sizeof(pwrControl));
    gpsCommand(pwrControl2, sizeof(gpsCommand));
    gpsCommand(coldRestart, sizeof(coldRestart));
}

void main(void) {
    //System Setup
    bool setup = boardInit();
    gpsInit();
    uint8_t frameSize = 0;
    while(1)
    {
        if (measure_flag)
        {
            frameSize = getFrame();
            if (frameSize > 0)
            {
                measure_flag = false;
                processFrame(frameSize);
                if(!setup){
                    setup = CAN1_Initialize();
                    LATCbits.LATC3 = 1;
                }
                if(setup)
                {
                    LATCbits.LATC3 = 0;
                    CAN_MSG_OBJ msg;
                    msg.msgId = 0x101;          // 29 bit (SID: 11bit, EID:18bit)
                    
                    msg.field.msgfields = 0x20;     // CAN TX/RX Message Object Control
                    msg.data = lat;           // Pointer to message data

                    CAN1_Transmit(0, &msg);
                    msg.data = lon;    
                    CAN1_Transmit(0, &msg);
                    LATCbits.LATC3 = 1;
                }
            }
        }
    }
}
