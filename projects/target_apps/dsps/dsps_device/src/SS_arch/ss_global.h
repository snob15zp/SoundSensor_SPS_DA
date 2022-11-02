#ifndef __ss_global_h__
#define __ss_global_h__

#include <stdint.h>
#include <stdbool.h>

#define SYSTICK_PERIOD_US   32     // period for systick timer in us, so 1000000ticks = 1second

//typedef union {
//    uint8_t	masByte[4];
//    uint32_t data_u32;
//} uni_uint32_t;

extern uint8_t dummy_function_status;

typedef enum  {e_FRS_Not_Done, e_FRS_Done,e_FRS_DoneError} e_FunctionReturnState;

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
