//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
#include <systick.h>
#include "SS_sys.h"
#include "user_periph_setup.h"
#include "ring_buff.h"
#include "ADC_flash.h"
#include "SPI_ADC.h"
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------



#define SPI_FLASH_OP_ByteProgramSec		((uint32_t)0xAD)

typedef union {
	uint8_t	 masByte[4];
	uint16_t mas_u16[2];
	uint32_t data_u32;
} dc_t;

static dc_t dc; 	 
static uint32_t my_ssize;
uint32_t AddrNewRecord;
uint32_t AddrNewRecordWithOffset;
static uint16_t F_PWR_on = 0;
//static uint16_t F_Erase_on = 0;
static uint8_t byteADC[4];
const void *databyteADC = byteADC;

int rezult_find_AddrNewRecord;



int find_AddrNewRecord(uint32_t Addr_L, uint32_t Addr_R, uint32_t *ptr);
int8_t spi_flash_StartProgramByteSequent(uint8_t *wr_data_ptr, uint32_t address, uint16_t size);

void AF_V_AddADCdataToFIFO(uint16_t A, uint16_t B)
{
	dc.data_u32 = A& 0x7ff;		
	dc.data_u32= dc.data_u32|((B& 0x7ff)<<11);
	RingBuffer_add_u32(dc.data_u32,&AddrNewRecord,SPI_FLASH_ADDR_END_RECORD_ADC-SPI_FLASH_ADDR_END_MARGIN_ADC);
	
}

void AF_V_Adddatau8u16ToFIFO(uint8_t A, uint16_t B)
{
	dc.mas_u16[1] = A;		
	dc.mas_u16[0] = B;
	RingBuffer_add_u32(dc.data_u32,&AddrNewRecord,SPI_FLASH_ADDR_END_RECORD_ADC-SPI_FLASH_ADDR_END_MARGIN_ADC);
}


int AF_V_WriteStart(uint16_t callerFunction)
{
int rezult_Start; 	
 //   ss_spi_init(SPI_ADC_GPIO_MAP,&UPS_spi_cfg);		 
    user_spi_flash_init(SPI_FLASH_GPIO_MAP);		
//	Unlock flash	
	delay_10ms();			
	spi_set_bitmode(SPI_MODE_8BIT);
	spi_transaction(SPI_FLASH_OP_WRDI);//RDD???	
	delay_10ms();	
	spi_flash_set_write_enable();
	delay_10ms();		
	spi_flash_write_status_reg(0);	
	delay_10ms();	

//	Find address of a new record	
	rezult_find_AddrNewRecord = find_AddrNewRecord(SPI_FLASH_ADDR_START_RECORD_ADC, 
	                   SPI_FLASH_ADDR_END_RECORD_ADC-SPI_FLASH_ADDR_END_MARGIN_ADC, &AddrNewRecord);	
	if (( rezult_find_AddrNewRecord == 0 )&&
		(AddrNewRecord<(SPI_FLASH_ADDR_END_RECORD_ADC-SPI_FLASH_ADDR_END_MARGIN_ADC)))
	{ uint32_t firstrecordAddress=AddrNewRecord;
 		AF_V_Adddatau8u16ToFIFO(recordType_dummyPWRon | F_PWR_on, callerFunction);		
		if ( RingBuffer_get_ch8(&byteADC[0]) == 0 )
		{
			spi_flash_StartProgramByteSequent(&byteADC[0], firstrecordAddress, 1);		
			rezult_Start = 0;			
		}			
		else
		{
			rezult_Start = -1;					
		}
	}
	else
	{
/*		if ( F_Erase_on == 0 )
		{
			
			F_Erase_on = 1;
//Block64 erase			
			spi_flash_block_erase(SPI_FLASH_ADDR_START_RECORD_ADC, SPI_FLASH_OP_BE64);
			while ( spi_flash_is_busy() != SPI_FLASH_ERR_OK )
			{
				delay_10ms();
			}
			spi_flash_block_erase(0x30000, SPI_FLASH_OP_BE64);
			while ( spi_flash_is_busy() != SPI_FLASH_ERR_OK )
			{
				delay_10ms();
			}
			AF_V_AddADCdataToFIFO(callerFunction, 0x80 | F_PWR_on);
			if ( RingBuffer_get_ch8(&byteADC[0]) == 0 )  
				spi_flash_StartByteProgramSequent(&byteADC[0], SPI_FLASH_ADDR_START_RECORD_ADC, 1);

			
			rezult_Start = 0;			
		}
		else
		{
			AddrNewRecord = SPI_FLASH_ADDR_END_RECORD_ADC; 
			rezult_Start = -1;
		}
*/	AddrNewRecord = SPI_FLASH_ADDR_END_RECORD_ADC;
	}
	F_PWR_on = 1;
	return rezult_Start;
}

void AF_V_ERASE_FILE_DataADC(void)
{
uint32_t qnt_sec4096;
uint32_t indx_sec4096;
	
	qnt_sec4096 = ((SPI_FLASH_ADDR_END_RECORD_ADC) - SPI_FLASH_ADDR_START_RECORD_ADC)/ 4096;
	for ( indx_sec4096 = 0; indx_sec4096 < qnt_sec4096; indx_sec4096++)
	{	
		spi_flash_block_erase(SPI_FLASH_ADDR_START_RECORD_ADC + indx_sec4096*4096, SPI_FLASH_OP_SE);
		delay_10ms();
		delay_10ms();
		delay_10ms();
		delay_10ms();
		delay_10ms();
		delay_10ms();		
	}
}

void delay_200ms(void)
{
    uint16_t i;
    for (i = 20; i != 0; --i)
		delay_10ms();
}

#define	adr4_start ((SPI_FLASH_ADDR_START_RECORD_ADC / 4096)*4096)
#define	adr64_start (((SPI_FLASH_ADDR_START_RECORD_ADC+65535) / 65536)*65536)
#define	adr64_stop ((SPI_FLASH_ADDR_END_RECORD_ADC / 65536)*65536)
#define	adr4_stop ((SPI_FLASH_ADDR_END_RECORD_ADC / 4096)*4096)
#if (adr64_start<SPI_FLASH_ADDR_END_RECORD_ADC)
#define endEraseAddress adr64_start
#else
#define endEraseAddress  SPI_FLASH_ADDR_END_RECORD_ADC
#endif
uint8_t EraseState;
//uint32_t ea;	
uint32_t ca;

void AF_V_ERASE_FILE_DataADC2_begin(void)
{
	EraseState=1;
}	

e_FunctionReturnState AF_V_ERASE_FILE_DataADC2(void)
{
	e_FunctionReturnState b_rv;
	b_rv=e_FRS_Not_Done;
	switch (EraseState)
		{case 1: ca = 	adr4_start;EraseState++;
			case 2: if ((ca < endEraseAddress))
							{   spi_flash_block_erase(ca, SPI_FLASH_OP_SE);ca += 4096;}
			        else
							{ca = 	adr64_start; EraseState++;};
							break;
			case 3: if ((ca < adr64_stop))
							{   spi_flash_block_erase(ca, SPI_FLASH_OP_BE64);ca += 65536;}
			        else
							{ca = 	adr64_stop; EraseState++;};
							break;				
		  case 4:	if ((ca < adr4_stop))
							{   spi_flash_block_erase(ca, SPI_FLASH_OP_SE);ca += 4096;}
			        else
							{EraseState=0;};
							break;			
		 default:		b_rv=e_FRS_Done	;				
	 }					
	return b_rv;
}



void AF_V_WriteStop(uint16_t callerFunction)
{
uint32_t systick_time_tek;
//uint8_t byteFlash;
//uint8_t dummy;	
	
uni_u32_t datal;	
uint32_t actual_size;	
	
	spi_set_bitmode(SPI_MODE_8BIT);
	spi_transaction(SPI_FLASH_OP_WRDI);
	
  systick_time_tek = systick_time;
	rezult_find_AddrNewRecord = find_AddrNewRecord(SPI_FLASH_ADDR_START_RECORD_ADC, 
	                 SPI_FLASH_ADDR_END_RECORD_ADC-SPI_FLASH_ADDR_END_MARGIN_STAMP, &AddrNewRecord);	
	if (( rezult_find_AddrNewRecord == 0 )&&
	   (AddrNewRecord<(SPI_FLASH_ADDR_END_RECORD_ADC-SPI_FLASH_ADDR_END_MARGIN_STAMP)))
	{ datal.u8[2]=recordType_StopPWRon;
		datal.u16[0]=callerFunction;
		spi_flash_write_data(&(datal.u8[0]),AddrNewRecord,3,&actual_size);
		AddrNewRecord+=3;
		datal.u8[2]=recordType_TimeStampL;
		datal.u16[0]=systick_time_tek & 0x00ffff;
		spi_flash_write_data(&(datal.u8[0]),AddrNewRecord,3,&actual_size);
		AddrNewRecord+=3;
		datal.u8[2]=recordType_TimeStampH;
		datal.u16[0]=(systick_time_tek>>16);
		spi_flash_write_data(&(datal.u8[0]),AddrNewRecord,3,&actual_size);
		AddrNewRecord+=3;
		AddrNewRecordWithOffset=AddrNewRecord-SPI_FLASH_ADDR_START_RECORD_ADC;
	}
	else
	{
		AddrNewRecord=SPI_FLASH_ADDR_END_RECORD_ADC-SPI_FLASH_ADDR_END_MARGIN_STAMP;
		strcpy(SSG_ch_Error,"Error AF_V_WriteStop find_AddrNewRecord " );
	}
}

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
/*
 * SPI Flash Program functions
 ****************************************************************************************
 */
int8_t spi_flash_StartProgramByteSequent(uint8_t *wr_data_ptr, uint32_t address, uint16_t size)
{
//    SPI_FLASH_ENABLE_POWER_PIN();

    // Check if SPI Flash is ready
    int8_t status = spi_flash_is_busy();
    if (status != SPI_FLASH_ERR_OK)
    {
        return status;
    }
    uint16_t temp_size = size;
    // Send Write Enable instruction
	status = spi_flash_set_write_enable();	
    if (status != SPI_FLASH_ERR_OK)
    {
        return status;
    }
    // Send command
    spi_set_bitmode(SPI_MODE_32BIT);
    spi_cs_low();
    spi_access((SPI_FLASH_OP_ByteProgramSec << 24) | address);

    // Send data
    spi_set_bitmode(SPI_MODE_8BIT);
    while(temp_size > 0)
    {
        spi_access(*wr_data_ptr++);
        temp_size--;
    }
    spi_cs_high();
    return spi_flash_wait_till_ready();
}

uint32_t get_u32_flashDisk(uint32_t addr)
{
	spi_flash_read_data(&dc.masByte[0], addr, 3, &my_ssize);
	dc.masByte[3] = 0;
	return dc.data_u32;
}
	
int find_AddrNewRecord(uint32_t Addr_L, uint32_t Addr_R, uint32_t *ptr)
{
uint32_t Addr_delta;
uint32_t Addr_new, u32tmp;	
uint32_t V_L, V_R, V_new;	

	V_L = get_u32_flashDisk(Addr_L);
	V_R = get_u32_flashDisk(Addr_R);
	if ( V_L >= SPI_FLASH_DATA_FLASH_ERASE )
	{
		*ptr = Addr_L;
		return 0; 	
	}
	if ( V_R < SPI_FLASH_DATA_FLASH_ERASE )
	{
		*ptr = NULL;
		return SPI_FLASH_FLAG_FLASH_FULL;		
	}		
	Addr_delta = Addr_R - Addr_L;
	while ( Addr_delta > 3  )
	{
		u32tmp = Addr_delta / 3;
		u32tmp >>= 1;
		Addr_new = Addr_L + u32tmp * 3;
		V_new = get_u32_flashDisk(Addr_new);
		if ( V_new < SPI_FLASH_DATA_FLASH_ERASE )
			Addr_L = Addr_new;
		else
			Addr_R = Addr_new;
		Addr_delta = Addr_R - Addr_L;
//		cnt++;
	}
	*ptr = Addr_R;
	return 0;
}

