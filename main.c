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
#pragma config PR1WAY = ON      // PRLOCKED One-Way Set Enable bit (PRLOCKED bit can be cleared and set only once)
#pragma config CSWEN = ON       // Clock Switch Enable bit (Writing to NOSC and NDIV is allowed)
#pragma config JTAGEN = ON      // JTAG Enable bit (Enable JTAG Boundary Scan mode and pins)
#pragma config FCMEN = ON       // Fail-Safe Clock Monitor Enable bit (Fail-Safe Clock Monitor enabled)
#pragma config FCMENP = ON      // Fail-Safe Clock Monitor -Primary XTAL Enable bit (FSCM timer will set FSCMP bit and OSFIF interrupt on Primary XTAL failure)
#pragma config FCMENS = ON      // Fail-Safe Clock Monitor -Secondary XTAL Enable bit (FSCM timer will set FSCMS bit and OSFIF interrupt on Secondary XTAL failure)

// CONFIG3
#pragma config MCLRE = EXTMCLR  // MCLR Enable bit (If LVP = 0, MCLR pin is MCLR; If LVP = 1, RE3 pin function is MCLR )
#pragma config PWRTS = PWRT_OFF // Power-up timer selection bits (PWRT is disabled)
#pragma config MVECEN = OFF     // Multi-vector enable bit (Interrupt contoller does not use vector table to prioritze interrupts)
#pragma config IVT1WAY = ON     // IVTLOCK bit One-way set enable bit (IVTLOCKED bit can be cleared and set only once)
#pragma config LPBOREN = OFF    // Low Power BOR Enable bit (Low-Power BOR disabled)
#pragma config BOREN = OFF      // Brown-out Reset Enable bits (Brown-out Reset disabled)

// CONFIG4
#pragma config BORV = VBOR_1P9  // Brown-out Reset Voltage Selection bits (Brown-out Reset Voltage (VBOR) set to 1.9V)
#pragma config ZCD = OFF        // ZCD Disable bit (ZCD module is disabled. ZCD can be enabled by setting the ZCDSEN bit of ZCDCON)
#pragma config PPS1WAY = ON     // PPSLOCK bit One-Way Set Enable bit (PPSLOCKED bit can be cleared and set only once; PPS registers remain locked after one clear/set cycle)
#pragma config STVREN = ON      // Stack Full/Underflow Reset Enable bit (Stack full/underflow will cause Reset)
#pragma config LVP = ON         // Low Voltage Programming Enable bit (Low voltage programming enabled. MCLR/VPP pin function is MCLR. MCLRE configuration bit is ignored)
#pragma config XINST = OFF      // Extended Instruction Set Enable bit (Extended Instruction Set and Indexed Addressing Mode disabled)

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
void main(void) {
    //System Setup
    bool setup = boardInit();
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
