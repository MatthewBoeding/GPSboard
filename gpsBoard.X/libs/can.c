#include <xc.h>
#include <stdint.h>
#include "can.h"

static volatile uint32_t fifoAddr;
extern uint8_t canMsg[16];

void canReceive(void)
{    
    uint32_t fifoAddr = C1FIFOBA;
    uint8_t * FIFO = fifoAddr;
    for(int i = 0; i < 16; i++)
    {
        canMsg[i] = FIFO[i];    
    }
}