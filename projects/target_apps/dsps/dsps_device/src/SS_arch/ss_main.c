#include "ss_main.h"
#include "SS_InterfaceToBLE.h"
#include "SS_sys.h"
#include "MathFast.h"
#include "MathSlow.h"
#include "ADC_flash.h"

uint8_t ssm_main_state;
bool ssm_main_BLE_RDY;


void ssm_main_ADC_prepare(void)
{
	
#ifdef __SoundSensor__
	AF_V_WriteStart(uint16_t callerFunction);	
#endif	
	MF_main_init();
	ADCon=true;
	ssm_main_BLE_RDY=false;
};

void ssm_main_BLE_prepare(void)
{
	ssm_main_BLE_RDY=true;
};

static uint32_t systick_last;

void ss_main_init(void)
{
	//ssm_main_state=0;
	ADCon=false;
	systick_last=systick_time;
}

#define PeriodSlowMath 4000

bool ss_main(void)
{
	bool b_rv;
	b_rv=true;
	
	if ((systick_time-systick_last)>PeriodSlowMath)
	{ 
		systick_last+=PeriodSlowMath;
		SM_catch();
		EvaluteLogLevel();
		AF_V_AddADCdataToFIFO((uint16_t) LogLevelA, (uint16_t) LogLevelC);
		
#ifndef __SoundSensor__					
					led_flash();
#endif
		
		
#ifdef __SoundSensor__			
					sx_main();
#endif					
		
	}
	
	
	
	if (systick_time>32000)  //debug
		b_rv=false;
	return b_rv; 
};
