#ifndef __ss_global_h__
#define __ss_global_h__

#include <stdint.h>
#include <stdbool.h>

#define D_ADCMODE 0
/*
0 - work mode
1 - live data
2 - SIN in ADC_IRQ
3 - SIN from timer
*/


#if	(D_ADCMODE!=3)
#define D_SYSTICK_PERIOD_US   1000     // period for systick timer in us, so 1000000ticks = 1second
#else
#define D_SYSTICK_PERIOD_US   64     // period for systick timer in us, so 1000000ticks = 1second
#endif
#define D_PeriodSlowMath_us (1000000/8)
#define D_PeriodSlowMath (D_PeriodSlowMath_us/(D_SYSTICK_PERIOD_US)) //ticks
#define D_sx_takt_call

//typedef union {
//    uint8_t	masByte[4];
//    uint32_t data_u32;
//} uni_uint32_t;

#define D_FlashMap 531
//#define D_FlashMap 585
/*
Standat 585			
Address		Size	
0x	0d	0d	
0	0	16384	Bootloader
4000	16384	32768	First copy
c000	49152	32768	Second copy
14000	81920	147456	file
38000	229376	4096	Header
39000	233472	4096	Config 1
3a000	237568	4096	Config 2
3b000	241664	20480	Config2 end
40000	262144	3932160	end2m
400000	4194304		end32m
*/








extern uint8_t dummy_function_status;
extern char SSG_ch_Error[40];




//=============================================FSM=====================================
typedef enum  {e_FRS_Not_Done, e_FRS_Done,e_FRS_DoneError} e_FunctionReturnState;

typedef uint32_t key_type;
typedef e_FunctionReturnState (*fp_FSM_Functions)(void */*FSM*/ );	

typedef
   struct { 
			key_type mFSM_Error;
		  uint8_t state;
		  uint8_t NumOfel;
		  uint8_t mainFMSstate;
		  uint8_t sign;
		  uint8_t keyrows;
      key_type *keys;		 
		  fp_FSM_Functions fs;
    } t_s_FSM;
	
e_FunctionReturnState  FSM_main(t_s_FSM *FSM);
		
//typedef e_FunctionReturnState (*fp_FSM_Transition)(t_s_FSM_Data * pFDMD_Power,key_type key,fp_FSM_Functions fp);
//extern e_FunctionReturnState  FSM_MainTransition(t_s_FSM_Data * pFDMD_Power,key_type key);		

/* parametes
extern bool ;
exten  bool 

1	LiveWarningLevelAF	Live warning level, weighting A, time weighting fast	xxx.x	dB(A)
extern int32_t MS_i32_AlertLevel_FastA;//0.1dB
2	LiveWarningLevelCP	Live warning level, weighting C, peak	xxx.x	dB(C)
3	LAeqLimit	The normalized 8-hour equivalentexposure limit	xxx.x	dB(A)
extern int32_t MS_i32_AlertLevel_Dose;
4	ClibrationMode	Turning on the calibration mode	Check box	-
5	CalibrationFactor	Calibration factor	x.xxxxx	 -

Table of reading parameters
#	Variable name	Description	Format	Units
1	LiveLevelAF	Live level, weighting A, time weighting fast	xxx.x	dB(A)
extern int32_t MS_i32_Level_FastA_dB;//0.1dB
2	LiveLevelCP	Live level, weighting C, peak	xxx.x	dB(C)
extern int32_t MS_i32_Level_C_Peak_dB;//0.1dB
3	LAeq	The normalized 8-hour equivalentexposure	xxx.x	dB(A)
extern int32_t MS_i32_Level_Dose_dB;//0.1dB



*/

#endif
