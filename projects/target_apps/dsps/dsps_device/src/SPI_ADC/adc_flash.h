#ifndef __ADC_flash_h__ 
#define __ADC_flash_h__

#include <stdint.h>

#define AF_D_StartFirstRecording 1
#define AF_D_StartContinueRecording 2
#define AF_D_StopPowerOn 3
#define AF_D_StopPoewrOff 4 
#define AF_D_timestampLo 5
#define AF_D_timestampHi 6

void AF_V_WriteTimestamp(void);
void AF_V_WriteServiceRecord(uint8_t recodType, uint16_t callerFunction);
void AF_V_AddADCdataToFIFO(uint16_t A, uint16_t B);

#endif