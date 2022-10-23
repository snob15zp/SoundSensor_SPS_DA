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
	systick_last=get_systime();
	return e_FRS_Done;
}

#define PeriodSlowMath 4000

e_FunctionReturnState ss_main(void)
{
	e_FunctionReturnState b_rv;
	b_rv=e_FRS_Done;
	
	if ((get_systime()-systick_last)>PeriodSlowMath)
	{ 
		systick_last+=PeriodSlowMath;
		SM_catch();
		EvaluteLogLevel();
		AF_V_AddADCdataToFIFO((uint16_t) LogLevelA, (uint16_t) LogLevelC);
		
#ifdef __SoundSensor__			
					sx_main();
#endif					
		
	}
	
	
	
	if (get_systime()>32000)  //debug
		b_rv=e_FRS_Not_Done;
	return b_rv; 
};
