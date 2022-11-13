
#include "SPI_ADC.h"
#include "ring_buff.h"

#define RING_BUFF_MASK			(RING_BUFF_SIZE - 1)  

typedef enum {
    BUFF_NOT_FULL = 0,
    BUFF_FULL = 1            
} enum_ovr_buff_t;

typedef enum {
    LOCK_RECORD_INACTIVE = 0,
    LOCK_RECORD_ACTIVE = 1            
} enum_lockRecord_t;

typedef struct
{
	uint32_t buffer[RING_BUFF_SIZE];
	uni_uint32_t convrt;
	uint16_t idxIn;
	uint16_t idxOut;
	uint16_t cnt;
	uint32_t cnt_ovr;    
	enum_ovr_buff_t ovr_buff;
	enum_lockRecord_t lockRecord;	
} RING_sBuf_t;

static uint8_t byteFlash;
static RING_sBuf_t sBuf;
static RING_sBuf_t *ptr = (RING_sBuf_t *)&sBuf;

int RingBuffer_get_ch8(uint8_t *data);

bool poll_newBytetoWriteFlash(void)
{ bool rv;
	rv=true;
	if ( SA_flashbit == true)
		return false;	
	if ( RingBuffer_get_ch8(&byteFlash) == 0 )
	{
		SA_out.masByte[0] = 0xAD;
		SA_out.masByte[1] = 0x22;
		SA_out.masByte[2] = byteFlash;		
		SA_flashbit = true;
		rv=false;
	}
	return rv;
}

void RingBuffer_Clr(void)
{
    ptr->idxIn = 0;
	ptr->idxOut = 0;
	ptr->cnt = 0;
	ptr->convrt.data_u32 = 0;
    ptr->ovr_buff = BUFF_NOT_FULL;
    ptr->cnt_ovr = 0;
	ptr->lockRecord = LOCK_RECORD_INACTIVE;
}

int RingBuffer_is_empty(void)
{
    if ( (ptr->idxIn == ptr->idxOut) && (ptr->ovr_buff==BUFF_NOT_FULL) )
        return 1;       
    else
        return 0;        
}

int RingBuffer_is_full(void)
{
    if ( ptr->ovr_buff == BUFF_FULL )
        return 1;
    else
        return 0;        
}

uint16_t RingBuffer_get_qntNode(void)
{
    if ( ptr->ovr_buff == BUFF_FULL )
        return RING_BUFF_SIZE;
    if ( ptr->idxIn == ptr->idxOut )
        return 0;
    if ( ptr->idxIn > ptr->idxOut )
        return ( ptr->idxIn - ptr->idxOut );
    return (RING_BUFF_SIZE - ptr->idxOut) + ptr->idxIn;
}

int RingBuffer_get_i32(int32_t *data)
{
uint16_t idx;
    if ( RingBuffer_is_empty() )
        return -1;
    idx = ptr->idxOut;
    ptr->idxOut += 1;
	ptr->idxOut &= RING_BUFF_MASK;
    ptr->ovr_buff = BUFF_NOT_FULL;            
    ptr->cnt = 0;
    *data = ptr->buffer[idx]; 
    ptr->buffer[idx] = 0;
    return 0;	
}

int RingBuffer_get_ch8(uint8_t *data)
{
int32_t rd;

	switch(ptr->cnt)
	{
		case 0:
            if ( RingBuffer_get_i32(&rd) )
                return -1;
			ptr->convrt.data_u32 = rd; 	
			ptr->cnt = 1;
			*data = ptr->convrt.masByte[0];
			break;
		case 1:
			ptr->cnt = 2;
			*data = ptr->convrt.masByte[1];
			break;
		case 2:
			ptr->cnt = 0;
			*data = ptr->convrt.masByte[2];
			break;
	}
	return 0;
}

int RingBuffer_add_u32(uint32_t data,uint32_t *adr, uint32_t margin)
{
	  (*adr)+=3;
	  if (*adr>margin)
			return -3;
    if ( ptr->lockRecord == LOCK_RECORD_ACTIVE )
        return -1;    		
    if ( ptr->ovr_buff == BUFF_FULL )
    {   ptr->cnt_ovr += 1;        
        return -2;    
    }
	ptr->buffer[ptr->idxIn] = data;
    ptr->idxIn += 1;
    ptr->idxIn &= RING_BUFF_MASK;
    if ( ptr->idxIn == ptr->idxOut )
        ptr->ovr_buff = BUFF_FULL;        
    return 0;
}

void delay_200us(void)
{
    uint16_t i;
    for (i = 530; i != 0; --i)
		__asm__ volatile("nop");
}

void delay_10ms(void)
{
    uint16_t i;
    for (i = 50; i != 0; --i)
		delay_200us();
}

