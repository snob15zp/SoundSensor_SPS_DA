#ifndef __ADC_flash_h__ 
#define __ADC_flash_h__

#include <stdint.h>
#include <stdbool.h>
#include <ss_global.h>

#if (D_FlashMap==585)
#define SPI_FLASH_ADDR_START_RECORD_ADC			(0x14000) 
#define SPI_FLASH_ADDR_END_RECORD_ADC			  (0x37ffA)
#endif

#define SPI_FLASH_DATA_FLASH_ERASE				((uint32_t)0x00FFFFFF) 
#define SPI_FLASH_FLAG_FLASH_FULL				(-1)

#define AF_D_StartFirstRecording 1
#define AF_D_StartContinueRecording 2
#define AF_D_StopPowerOn 3
#define AF_D_StopPoewrOff 4 
#define AF_D_timestampLo 5
#define AF_D_timestampHi 6

extern bool ADCon;
extern uint32_t AddrNewRecord;

void AF_V_WriteTimestamp(void);
void AF_V_WriteStartServiceRecord(uint8_t recodType, uint16_t callerFunction);
void AF_V_WriteStopServiceRecord(uint8_t recodType, uint16_t callerFunction);

		void AF_V_AddADCdataToFIFO(uint16_t A, uint16_t B);
		void IRQ_wrByteFromRecord(void);
		int AF_V_WriteStart(uint16_t callerFunction);
		void AF_V_WriteStop(uint16_t callerFunction);

void delay_10ms(void);
void delay_100ms(void);

#endif
