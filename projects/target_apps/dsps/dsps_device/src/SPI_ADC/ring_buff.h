#ifndef __RING_BUFF_h__ 
#define __RING_BUFF_h__ 

#include "ss_global.h"


typedef union {
    uint8_t	masByte[4];
    uint32_t data_u32;
} uni_uint32_t;

#define RING_BUFF_SIZE           ((uint16_t)16)

int RingBuffer_is_empty(void);
int RingBuffer_add_u32(uint32_t data,uint32_t *adr, uint32_t margin);
int RingBuffer_get_ch8(uint8_t *data);
bool poll_newBytetoWriteFlash(void);

void delay_200us(void);
void delay_10ms(void);

#endif	//__RING_BUFF_h__ 
