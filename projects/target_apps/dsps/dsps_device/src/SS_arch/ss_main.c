#include "ss_global.h"
#include "ss_main.h"
#include "SS_InterfaceToBLE.h"
#include "SS_sys.h"
#include "MathFast.h"
#include "MathSlow.h"
#include "ADC_flash.h"
#include "i2c.h"
#include "ss_i2c.h"
#include "SPI_ADC.h"
#include "ring_buff.h"


//---------------------------------------------------------------------
//---------------------------------------------------------------------
uint8_t rd_data1[512];
uint32_t my_ssize;
//---------------------------------------------------------------------
//---------------------------------------------------------------------

bool SSM_b_alert_hearing;

uint8_t ssm_main_state;
bool ssm_main_BLE_RDY;
E_ADC_MODE_t SS_ADC_Active_MODE=EAM_ADCsystick;

t_SSS_s_timeevent SSM_erase_alarm_time_event;

static int32_t time_start;
e_FunctionReturnState SSM_ADCStart(void)
{
#ifdef __SS_EXT__
	SX_CalibrationOnOff(SSS_CalibrationMode);
#endif	
  time_start=systick_time;
	AF_V_WriteStart((uint16_t) SSM_ADCStart);	

	MS_init();
	MF_main_init();
	ssm_main_BLE_RDY=false;
	SS_spi_switchoff_pins(SPI_FLASH_GPIO_MAP);
	
	SPI_ADC_init();
	SS_ADC_MODE=SS_ADC_Active_MODE;
	return e_FRS_Done;
};

static uint8_t SM_FSM_state = 0;

e_FunctionReturnState SSM_ADCStop(void)
{ e_FunctionReturnState b_rv;
	b_rv=e_FRS_Not_Done;
	time_start=systick_time;
	switch (SM_FSM_state)
	{	case 0: // 
			       SM_FSM_state++;
		case 1: poll_newBytetoWriteFlash();
//			if (systick_time<(1000000/D_SYSTICK_PERIOD_US))  //debug
//			{
//				b_rv=e_FRS_Not_Done;
//				break;		
//			}
		       if (RingBuffer_is_empty())
            SM_FSM_state++;
			break;						
		default: 	//SS_ADC_MODE=EAM_IDLE;
			        SPI_ADC_deinit();
							ss_spi_init(SPI_FLASH_GPIO_MAP,&UPS_spi_cfg);		 
        	    AF_V_WriteStop((uint16_t) SSM_ADCStop);		 
	            user_spi_flash_init(SPI_FLASH_GPIO_MAP);
	            //write stop stamp
	          	
//---------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------			
uint32_t AddrNewRecord_funStop;
				AddrNewRecord_funStop = AddrNewRecord;
			
				spi_flash_read_data(rd_data1, SPI_FLASH_ADDR_START_RECORD_ADC, 256, &my_ssize);				
//---------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------			
			
			
	            ssm_main_BLE_RDY=true;	
	          
		          SM_FSM_state=0;
			        b_rv=e_FRS_Done;
	}
	return b_rv;
};

static uint32_t systick_last;

e_FunctionReturnState ss_main_init(void)
{
	//ssm_main_state=0;
	//ADCon=false;
	systick_last=systick_time;
	return e_FRS_Done;
}



//#define PeriodSlowMath (1000000/(D_SYSTICK_PERIOD_US*8)) //ticks
int32_t fifodebugcalc;


e_FunctionReturnState ss_main(void)
{
	e_FunctionReturnState b_rv;
	b_rv=e_FRS_Not_Done;
	
	if ((get_systime()-systick_last)>D_PeriodSlowMath)
	{ 
		systick_last+=D_PeriodSlowMath;
#ifdef __NO_MATLAB__		
		MS_main();
#endif		
//		AF_V_AddADCdataToFIFO((uint16_t) MS_i32_Level_FastA_dB, (uint16_t) MS_i32_Level_C_Peak_dB);
		AF_V_AddADCdataToFIFO((uint16_t) (fifodebugcalc), (uint16_t) (fifodebugcalc+1));fifodebugcalc+=2;
		
#ifdef __DEVKIT_EXT__					
//					led_flash();
#endif
		
		   DisplayAlarm();
		
#ifdef __SS_EXT__		
//#ifndef __ADCTEST__		
					sx_main();
//#endif		
#endif					
		
		//TODO call ADC
		
		if ((BTN_SW1_SHORT&btnCmd)!=0)		
		{	
			MS_b_alert_hearing=!MS_b_alert_hearing;
			btnCmd=0;
		}
		if ((BTN_SW3_SHOR&btnCmd)!=0)		
		{	
			MS_b_alert_hearing=!MS_b_alert_hearing;
			btnCmd=0;
		}
   
		
	};
	
	poll_newBytetoWriteFlash();
	
	

	if ((systick_time-time_start)>((10000000)/D_SYSTICK_PERIOD_US))
	  	b_rv=e_FRS_Done;
	return b_rv; 
//-------------------------------------------------------------------
//-------------------------------------------------------------------
};

e_FunctionReturnState ss_main_BLE(void)
{
	e_FunctionReturnState b_rv;
	b_rv=e_FRS_Not_Done;
	
	if ((get_systime()-systick_last)>D_PeriodSlowMath)
	{ 
		systick_last+=D_PeriodSlowMath;
		
#ifdef __DEVKIT_EXT__					
//	led_flash();
#endif
		
		DisplayAlarm();
		
#ifdef __SS_EXT__		
//#ifndef __ADCTEST__		
		sx_main();
//#endif		
#endif						
		if ((BTN_SW1_DBL_CLICK&btnCmd)!=0)		
		{	
			SSS_SetUpTimeEvent(&SSM_erase_alarm_time_event,(5000000/D_SYSTICK_PERIOD_US));
			//TODO erase flash file
			btnCmd=0;
		}
  }
		if ((systick_time-time_start)>((10000000)/D_SYSTICK_PERIOD_US))
 	  	b_rv=e_FRS_Done;
	return b_rv; 
}


e_FunctionReturnState SSM_BLEStop()
{
	return e_FRS_Done;
};
e_FunctionReturnState SSM_BLEStart()
{
	return e_FRS_Done;
};


//================================ALARM===============================================
#define  MS_b_alert_C140dBPeakD true
#define  MS_b_alert_FastAD true
#define  MS_b_alert_liveD false
#define MS_b_alert_OverloadD false
#define  MS_b_alert_DoseM3dBD	true
#define  MS_b_alert_DoseD false
#define MS_b_alert_hearingD false



void DisplayAlarm(void)
{
	rgbLedTaskD1.ledTimeSlot[0]=LED_ALARM_Operatingstate;
  if (MS_b_alert_liveD) rgbLedTaskD1.ledTimeSlot[0]=LED_ALARM_LiveSPL;
	if (MS_b_alert_OverloadD) rgbLedTaskD1.ledTimeSlot[0]=LED_ALARM_Overloadindicator;
	if (ssm_main_BLE_RDY) rgbLedTaskD1.ledTimeSlot[0]=LED_ALARM_BLE;
	if (SSM_erase_alarm_time_event.enable)
	{	rgbLedTaskD1.ledTimeSlot[0]=LED_ALARM_erase;
		if (systick_time-SSM_erase_alarm_time_event.time>SSM_erase_alarm_time_event.dtime)
			SSM_erase_alarm_time_event.enable=false;
	};

	rgbLedTaskD1.ledTimeSlot[1]=LED_ALARM_Empty;
	if (SSS_CalibrationMode)
	{
		if (MS_b_alert_hearing) 
		{rgbLedTaskD1.ledTimeSlot[1]=LED_ALARM_CalibrationLong;
		}
		else
		{rgbLedTaskD1.ledTimeSlot[1]=LED_ALARM_CalibrationShort; 
		}
	}
	else
	{
		if (MS_b_alert_hearing) rgbLedTaskD1.ledTimeSlot[1]=LED_ALARM_hearing;
	}



	rgbLedTaskLD1.ledTimeSlot[0]=LED_ALARM_Empty;
	if (MS_b_alert_DoseM3dBD) rgbLedTaskLD1.ledTimeSlot[0]=LED_ALARM_LAeqM3dB;
	if (MS_b_alert_DoseD) rgbLedTaskLD1.ledTimeSlot[0]=LED_ALARM_LAeq;
	
};