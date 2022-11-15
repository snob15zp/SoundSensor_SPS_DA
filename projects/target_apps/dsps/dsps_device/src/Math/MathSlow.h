#ifndef __MathSlow_h__
#define __MathSlow_h__

#ifdef __NO_MATLAB__
#include "ss_global.h"
#include "ss_sys.h"
#else
#include <stdint.h>
#include <stdbool.h>

#define D_SYSTICK_PERIOD_US   32     // period for systick timer in us, so 1000000ticks = 1second
#endif


#define MF_ADCOverLoad_Flag_duration_US 1000000

#define MS_D_offset_FastA_dB 498.8 //0.1dB
#define MS_D_offset_Dose_dB  (625.869) //0.1dB 74.53
#define MS_D_offset_C_Peak_dB (-237.0202241) //0.1dB






//#define MS_D_dBscale 10
#define MS_D_AlertZone 30  //0.1dB
#define MS_D_AlertLevel_C_140bB_peak (1400) //0.1dB -> 153144400 ADC bits, factor 2, fp26
#define MS_D_AlertLevel_FastA        (850)//0.1dB
#define MS_D_AlertLevel_Overload      102469853 //ADC bits, fp27
#define MS_D_AlertLevel_Dose          MS_D_AlertLevel_FastA//0.1dB
//#define MS_D_AlertLevel_DoseM3dB     (MS_D_AlertLevel_Dose-(MS_D_AlertZone*MS_D_dBscale))//0.1dB	
#define MS_D_CalibrationFactor         (10000) // -> fp14

extern float MS_d_offset_FastA_dB;  //0.1dB
extern float MS_d_offset_Dose_dB;   //0.1dB 74.53
extern float MS_d_offset_C_Peak_dB; //0.1dB


extern int32_t MS_i32_AlertLevel_C140dB_Peak;//filterC bits
extern int32_t MS_i32_AlertLevel_FastA;//0.1dB
//extern int32_t MS_i32_AlertLevel_Overload;//ADC bits
extern int32_t MS_i32_AlertLevel_Dose;//0.1dB
extern int32_t MS_i32_AlertLevel_DoseM3dB;//0.1dB	
extern int32_t MS_i32_CalibrationFactor;

extern bool MS_b_alert_C140dBPeak;
extern bool MS_b_alert_FastA;
extern bool MS_b_alert_live;
extern bool MS_b_alert_Overload;
extern bool MS_b_alert_DoseM3dB;	
extern bool MS_b_alert_Dose;
extern bool MS_b_alert_hearing;
// bool 

extern int32_t MS_i32_Level_C_Peak_dB;//0.1dB
extern int32_t MS_i32_Level_FastA_dB;//0.1dB
extern int32_t MS_i32_Level_Dose_dB;//0.1dB

#ifndef __NO_MATLAB__
extern uint32_t MS_couter;
extern uint32_t MS_matlab_time;
extern uint32_t MS_matlab_time_out;

extern int32_t MS_i32_Level_C_Peak_dB_out;//0.1dB
extern int32_t MS_i32_Level_FastA_dB_out;//0.1dB
extern int32_t MS_i32_Level_Dose_dB_out;//0.1dB

extern bool MS_b_alert_C140dBPeak_out;
extern bool MS_b_alert_FastA_out;
#endif

//for external call and matlab;
void MS_SetUpDefaultValue(void);
void MS_init(void);
//for external call
void MS_main(void);
//for matlab
void MS_evalute(uint32_t c, int32_t a, uint32_t i,uint32_t t);
//not for external call
void MS_Alerts(uint32_t time);
void MS_catch(void);
void MS_EvaluteLogLevel(void);
void MS_EvaluteLogLevel_A(uint32_t c, int32_t a, uint32_t i);//for matlab
void MS_test_EvaluteLogLevel(void);

#endif
