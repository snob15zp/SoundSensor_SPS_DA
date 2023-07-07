#include "ss_global.h"
#include "ss_main.h"
//#include "SS_InterfaceToBLE.h"
#include "SS_sys.h"
#include "MathFast.h"
#include "MathSlow.h"
#include "ADC_flash.h"
#include "i2c.h"
#include "ss_i2c.h"
#include "SPI_ADC.h"
#include "ring_buff.h"
#include "da14531.h"
#include "SS_ADC.h"


//---------------------------------------------------------------------
//---------------------------------------------------------------------
//uint8_t rd_data1[512];
//uint32_t my_ssize;
//---------------------------------------------------------------------
//---------------------------------------------------------------------

bool SSM_b_alert_hearing;
static int32_t RareProcedureCalc;
static uint32_t systick_last;
int16_t SSM_Vdd;
//uint8_t ssm_main_state;
bool SSM_b_AlarmVddLow;
bool ssm_main_BLE_RDY;
//E_ADC_MODE_t SS_ADC_Active_MODE=EAM_ADCsystick;

t_SSS_s_timeevent SSM_erase_alarm_time_event;

//static int32_t time_start;
e_FunctionReturnState SSM_ADCStart(void)
{
	user_spi_flash_init(SPI_FLASH_GPIO_MAP);//RDD
#ifdef __SS_EXT__
	SX_CalibrationOnOff(SSS_CalibrationMode);
#endif	
	//systick_last=systick_time;
  //time_start=systick_time;
	AF_V_WriteStart((uint16_t) SSM_ADCStart);	

	MS_init();
	MF_main_init();
	ssm_main_BLE_RDY=false;
	SS_spi_switchoff_pins(SPI_FLASH_GPIO_MAP);
	
	SPI_ADC_init();
//	SS_ADC_MODE=SS_ADC_Active_MODE;
	systick_last=systick_time;
	RareProcedureCalc=0;
	return e_FRS_Done;
};

static uint8_t SM_FSM_state = 0;

e_FunctionReturnState SSM_ADCStop(void)
{ e_FunctionReturnState b_rv;
	b_rv=e_FRS_Not_Done;
	//time_start=systick_time;
	switch (SM_FSM_state)
	{	case 0: // 
			       SM_FSM_state++;
		case 1: if (poll_newBytetoWriteFlash())
			           SM_FSM_state++;
//			if (systick_time<(1000000/D_SYSTICK_PERIOD_US))  //debug
//			{
//				b_rv=e_FRS_Not_Done;
//				break;		
//			}
//		       if (RingBuffer_is_empty())
//            SM_FSM_state++;
			break;						
		default: 	//SS_ADC_MODE=EAM_IDLE;
			        SPI_ADC_deinit();
							ss_spi_init(SPI_FLASH_GPIO_MAP,&UPS_spi_cfg);		 
        	    AF_V_WriteStop((uint16_t) SSM_ADCStop);		 
	            user_spi_flash_init(SPI_FLASH_GPIO_MAP);
	            //write stop stamp
	          	
//---------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------			
//uint32_t AddrNewRecord_funStop;
//				AddrNewRecord_funStop = AddrNewRecord;
//			
//				spi_flash_read_data(rd_data1, SPI_FLASH_ADDR_START_RECORD_ADC, 256, &my_ssize);				
//---------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------			
			
			
	            ssm_main_BLE_RDY=true;	
	          
		          SM_FSM_state=0;
			        b_rv=e_FRS_Done;
	}
	return b_rv;
};



e_FunctionReturnState ss_main_init(void)
{

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
		if (!ssm_main_BLE_RDY)
		{	
#ifdef __NO_MATLAB__		
			MS_main();
#endif		
			AF_V_AddADCdataToFIFO((uint16_t) MS_i32_Level_FastA_dB, (uint16_t) MS_i32_Level_C_Peak_dB);
//		AF_V_AddADCdataToFIFO((uint16_t) (fifodebugcalc), (uint16_t) (fifodebugcalc+1));fifodebugcalc+=2;
		};	
		
#ifdef __DEVKIT_EXT__					
//					led_flash();
#endif
		
		   DisplayAlarm();
		
#ifdef __SS_EXT__		
//#ifndef __ADCTEST__		
					sx_main();
//#endif		
#endif					
		
    if (!ssm_main_BLE_RDY)
		{	
			if ((BTN_SW1_SHORT&btnCmd)!=0)		
			{	
				MS_b_alert_hearing=!MS_b_alert_hearing;
				btnCmd=0;
			}
		}
    else		
		{
			if ((BTN_SW1_DBL_CLICK&btnCmd)!=0)		
			{	
				//SSS_SetUpTimeEvent(&SSM_erase_alarm_time_event,(5000000/D_SYSTICK_PERIOD_US));
				//AF_V_ERASE_FILE_DataADC2();//Done TODO erase flash file
				AF_V_ERASE_FILE_DataADC2_begin();
				btnCmd=0;
			}	
		}
		
		if (!((RareProcedureCalc++)&0x7))
		{
		   SSM_Vdd=SSA_getVdd();
			 SSM_b_AlarmVddLow=(SSG_D_VddLow>SSM_Vdd);
			 uint16_t AlarmStatus=BuildAlarmRecord();
       if (!ssm_main_BLE_RDY)
			 {	//AF_V_Adddatau8u16ToFIFO(recordType_Vdd,SSM_Vdd);MS_i32_Level_Dose_dB
				  AF_V_Adddatau8u16ToFIFO(recordType_Vdd,MS_i32_Level_Dose_dB);//out dose to file for debug
					AF_V_Adddatau8u16ToFIFO(recordType_AlarmStatus,AlarmStatus);
			 };	 
		}
	};
	
	poll_newBytetoWriteFlash();

//	if ((systick_time-time_start)>((10000000)/D_SYSTICK_PERIOD_US))
//	  	b_rv=e_FRS_Done;
	
	return b_rv; 
};

//-------------------------------------------------------------------
//-------------------------------------------------------------------

/*
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
			AF_V_ERASE_FILE_DataADC();//Done TODO erase flash file
			btnCmd=0;
		}
  }
//		if ((systick_time-time_start)>((10000000)/D_SYSTICK_PERIOD_US))
// 	  	b_rv=e_FRS_Done;
	return b_rv; 
}
*/

e_FunctionReturnState SSM_BLEStop()
{
  	SSS_ReadFromVar();
	//rwble_reset();
	//rwble_init();
	//SetWord16(SYS_CTRL_REG,GetWord16(SYS_CTRL_REG)| SW_RESET );
	//arch_ble_ext_wakeup_on();
	    uint16_t tmp;
    // Trigger SW reset
    tmp = GetWord16(SYS_CTRL_REG);
    tmp = (tmp & ~REMAP_ADR0) | 0; // Map ROM at address 0
    tmp |= SW_RESET;
    SetWord16(SYS_CTRL_REG, tmp);

	return e_FRS_Done;
};
e_FunctionReturnState SSM_BLEStart()
{
	SSS_WriteToVar();
	//arch_ble_ext_wakeup_off();
	return e_FRS_Done;
};


//================================ALARM===============================================
//#define  MS_b_alert_C140dBPeakD true
//#define  MS_b_alert_FastAD true
//#define  MS_b_alert_liveD false   1 e 
//#define MS_b_alert_OverloadD false  2 e
//#define  MS_b_alert_DoseM3dBD	true    6 r
//#define  MS_b_alert_DoseD false      5  r
//#define MS_b_alert_hearingD false  4     r
//SSS_CalibrationMode                3    r
//full memoy                             r 

#define MS_D_alert_live 0
#define MS_D_alert_Overload 1
#define SSS_D_CalibrationMode 2
#define MS_D_alert_hearing 3
#define MS_D_alert_DoseM3dB 4
#define MS_D_alert_Dose 5
#define SSM_D_AlarmVddLow 6
//#define MS_D_MemoryFull 7

//bool MS_b_MemoryFull;

uint16_t BuildAlarmRecord(void)
{ uint16_t AlamStatus=0;
	if (SSS_CalibrationMode)
		AlamStatus|=1<<SSS_D_CalibrationMode;
	if (SSM_b_AlarmVddLow)
		AlamStatus|=1<<SSM_D_AlarmVddLow;
	if (MS_b_alert_live)
		AlamStatus|=1<<MS_D_alert_live;
	if (MS_b_alert_Overload)
		AlamStatus|=1<<MS_D_alert_Overload;
	if (MS_b_alert_hearing) 
		AlamStatus|=1<<MS_D_alert_hearing;
	if (MS_b_alert_Dose) 
		AlamStatus|=1<<MS_D_alert_Dose;
	if (MS_b_alert_DoseM3dB) 
		AlamStatus|=1<<MS_D_alert_DoseM3dB;
	
	return AlamStatus;
}




void DisplayAlarm(void)
{
	rgbLedTaskD1.ledTimeSlot[0]=LED_ALARM_Operatingstate;
	if (SSS_CalibrationMode)
		      rgbLedTaskD1.ledTimeSlot[0].color|=RED_LED_OUT;
	if (RB_b_MemoryFull)
		      rgbLedTaskD1.ledTimeSlot[0].color|=GREEN_LED_OUT;
	if (ssm_main_BLE_RDY) 
		                     rgbLedTaskD1.ledTimeSlot[0]=LED_ALARM_BLE;
	if (SSM_b_AlarmVddLow)
	{	rgbLedTaskD1.ledTimeSlot[0].pulseWidthMs=D_pulseWidthMs;}
	else
	{	rgbLedTaskD1.ledTimeSlot[0].pulseWidthMs=D_pulseWidth875ms;};

		

	rgbLedTaskD1.ledTimeSlot[1]=LED_ALARM_Empty;
  if (MS_b_alert_live) 
		                     rgbLedTaskD1.ledTimeSlot[1]=LED_ALARM_LiveSPL;
	if (MS_b_alert_Overload) 
		                     rgbLedTaskD1.ledTimeSlot[1]=LED_ALARM_Overloadindicator;

	rgbLedTaskD1.ledTimeSlot[2]=LED_ALARM_Empty;
		if (MS_b_alert_hearing) 
			                    rgbLedTaskD1.ledTimeSlot[2]=LED_ALARM_hearing;
	



	rgbLedTaskLD1.ledTimeSlot[0]=LED_ALARM_Empty;
	if (MS_b_alert_DoseM3dB) 
		                       rgbLedTaskLD1.ledTimeSlot[0]=LED_ALARM_LAeqM3dB;
	if (MS_b_alert_Dose)     
		                       rgbLedTaskLD1.ledTimeSlot[0]=LED_ALARM_LAeq;
	
	
	if (0!=EraseState) //SSM_erase_alarm_time_event.enable)
		{	rgbLedTaskD1.ledTimeSlot[0]=LED_ALARM_erase;
			rgbLedTaskD1.ledTimeSlot[1]=LED_ALARM_erase;
			rgbLedTaskD1.ledTimeSlot[2]=LED_ALARM_erase;
			rgbLedTaskLD1.ledTimeSlot[0]=LED_ALARM_erase1;
//		if (systick_time-SSM_erase_alarm_time_event.time>SSM_erase_alarm_time_event.dtime)
//			SSM_erase_alarm_time_event.enable=false;
	};
	
};
