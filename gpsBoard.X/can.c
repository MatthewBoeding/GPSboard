#include <xc.h>
#include <stdint.h>
#include "can.h"

static volatile uint8_t rxMsg[16];
extern uint8_t canMsg[16];

void canReceive(void)
{
    uint32_t *FIFO = C1FIFOBA;
    canMsg = *FIFO;    
}