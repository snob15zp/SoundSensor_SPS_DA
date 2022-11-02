#ifndef __RING_BUFF_h__ 
#define __RING_BUFF_h__ 

#include <stdint.h>
typedef union {
    uint8_t	masByte[4];
    uint32_t data_u32;
} uni_uint32_t;

#define RING_BUFF_SIZE           ((uint16_t)16)

int RingBuffer_is_empty(void);
int RingBuffer_add_u32(uint32_t data);
int RingBuffer_get_ch8(uint8_t *data);
void poll_newBytetoWriteFlash(void);

void delay_200us(void);
void delay_10ms(void);

#endif	//__RING_BUFF_h__ 
