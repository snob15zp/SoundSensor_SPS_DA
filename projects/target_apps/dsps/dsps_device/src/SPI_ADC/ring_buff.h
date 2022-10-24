#ifndef __RING_BUFF_h__ 
#define __RING_BUFF_h__ 

#include <stdint.h>

#define RING_BUFF_SIZE           ((uint16_t)16)

int RingBuffer_add_u32(uint32_t data);
int RingBuffer_get_ch8(uint8_t *data);

#endif	//__RING_BUFF_h__ 
