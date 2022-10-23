#include "ss_global.h"
#include "ss_main.h"
#include "SS_InterfaceToBLE.h"
#include "SS_sys.h"
#include "MathFast.h"
#include "MathSlow.h"
#include "ADC_flash.h"
#include "i2c.h"

uint8_t ssm_main_state;
bool ssm_main_BLE_RDY;


e_FunctionReturnState ssm_main_ADC_prepare(void)
{
	
#ifdef __SoundSensor__
	AF_V_WriteStart((uint16_t) ssm_main_ADC_prepare);	
#endif	
	MS_init();
	MF_main_init();
	ADCon=true;
	ssm_main_BLE_RDY=false;
	return e_FRS_Done;
};

e_FunctionReturnState ssm_main_BLE_prepare(void)
{
	ssm_main_BLE_RDY=true;
	return e_FRS_Done;
};

static uint32_t systick_last;

e_FunctionReturnState ss_main_init(void)
{
	//ssm_main_state=0;
	ADCon=false;
	systick_last=systick_time;
	return e_FRS_Done;
}

#define PeriodSlowMath 4000

e_FunctionReturnState ss_main(void)
{
	e_FunctionReturnState b_rv;
	b_rv=e_FRS_Not_Done;
	
	if ((systick_time-systick_last)>PeriodSlowMath)
	{ 
		systick_last+=PeriodSlowMath;
#ifdef __NO_MATLAB__		
		MS_main();
#endif		
		AF_V_AddADCdataToFIFO((uint16_t) MS_i32_Level_FastA_dB, (uint16_t) MS_i32_Level_C_Peak_dB);
		
#ifndef __SoundSensor__					
					led_flash();
#endif
		
		
#ifdef __SoundSensor__			
					sx_main();
#endif					
		
	}
	
	
	
	if (systick_time>32000)  //debug
		b_rv=e_FRS_Done;
	return b_rv; 
};
