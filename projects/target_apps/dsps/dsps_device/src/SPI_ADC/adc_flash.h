#ifndef __ADC_flash_h__ 
#define __ADC_flash_h__

#include <stdint.h>
#include <stdbool.h>
#include <ss_global.h>

#if (D_FlashMap==585)
#define SPI_FLASH_ADDR_START_RECORD_ADC			(0x14000) 
#define SPI_FLASH_ADDR_END_RECORD_ADC			  (0x15000)// (0x38000)
#endif

#if (D_FlashMap==531)
#define SPI_FLASH_ADDR_START_RECORD_ADC			(0x22000) 
#define SPI_FLASH_ADDR_END_RECORD_ADC			  (0x23000)// (0x400000)
#endif

#define SPI_FLASH_ADDR_END_MARGIN_ADC   		(0x200)
#define SPI_FLASH_ADDR_END_MARGIN_STAMP   	(0x100)





#define SPI_FLASH_DATA_FLASH_ERASE				((uint32_t)0x00FFFFFF) 
#define SPI_FLASH_FLAG_FLASH_FULL				(-1)

//#define AF_D_StartFirstRecording 1
//#define AF_D_StartContinueRecording 2
//#define AF_D_StopPowerOn 3
//#define AF_D_StopPoewrOff 4 
//#define AF_D_timestampLo 5
//#define AF_D_timestampHi 6

#define recordType_FirstPWRon			0x80
#define recordType_NextPWRon        	0x81
#define recordType_dummyPWRon        	0x80
#define recordType_StopPWRon        	0x82
#define recordType_StopPWRoff       	0x83
#define recordType_TimeStampL       	0x84
#define recordType_TimeStampH       	0x85
#define recordType_Vdd              	0x86
#define recordType_AlarmStatus       	0x87

typedef union {
    uint32_t u32;
	  uint16_t u16[2];
	  uint8_t	 u8[4];
} uni_u32_t;

extern bool RB_b_MemoryFull;
extern uint32_t AddrNewRecord;
extern uint32_t AddrNewRecordWithOffset;

void AF_V_WriteTimestamp(void);
void AF_V_WriteStartServiceRecord(uint8_t recodType, uint16_t callerFunction);
void AF_V_WriteStopServiceRecord(uint8_t recodType, uint16_t callerFunction);

		void AF_V_AddADCdataToFIFO(uint16_t A, uint16_t B);
    void AF_V_Adddatau8u16ToFIFO(uint8_t A, uint16_t B);
		void IRQ_wrByteFromRecord(void);
		int AF_V_WriteStart(uint16_t callerFunction);
		void AF_V_WriteStop(uint16_t callerFunction);

void AF_V_ERASE_FILE_DataADC(void);

void delay_10ms(void);
void delay_100ms(void);

#endif
