#include <stdint.h>

struct canMsgObj
{
 uint32_t msgId;
 uint8_t msgField;
 uint8_t data[8];
 uint16_t crc;
 uint8_t ack;
};

void canReceive(void);
