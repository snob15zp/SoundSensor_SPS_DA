#ifndef __MathFast_h__
#define __MathFast_h__

#ifdef __NO_MATLAB__
#include "ss_global.h"
#else
#include <stdint.h>
#include <stdbool.h>

//#define SYSTICK_PERIOD_US   32     // period for systick timer in us, so 1000000ticks = 1second
#endif

typedef uint32_t uint32_T;
typedef int32_t  int32_T;
typedef char     char_T;

typedef union 
{
	int64_t num64;
	uint32_t num32[2];
	uint16_t num16[4];
} t_U_MF_int64;

typedef union 
{
	uint64_t num64;
	uint32_t num32[2];
	uint16_t num16[4];
} t_U_MF_uint64;

extern uint32_t MF_ADCOverLoad_Flag;
extern bool catch_flag;


//=======================for MATLAB check==================================
extern uint32_t	PeakC_max;
extern uint32_t Integrator_Hi_out;
extern int32_t i32_fastAC;
extern int32_t filterCout;
extern int32_t filterAout;
#ifndef __NO_MATLAB__
extern int32_t filterCout_M;
extern int32_t filterAout_M;
#endif
//--------------------------------------------------------------------------
//=======================for colling from ADCinterrupt===============================
void MF_main(int32_t adcinput);
//=======================for ADC emulating===========================================
void test_MF_main_ADCEmul(void);
//======================= init=======================================================
void MF_main_init(void);
void MF_main_reset(void);
//========================general function for math modules==========================
int32_T mul_u18s29sh(int32_T a, int32_T b, uint32_T aShift);
int32_T mul_u18s29sh17(int32_T a, int32_T b);
int32_T mul_u18s29shl(int32_T a, int32_T b, uint32_T aShift); //aShift=0 ->
int32_T mul_u18s29sh13(int32_T a, int32_T b); //aShift=0 ->
int32_T mul_u18s29sh12(int32_T a, int32_T b); //aShift=0 ->
uint64_t MF_sqr(int32_T a);
int32_T mul_s32_loSR(int32_T a, int32_T b, uint32_T aShift);
//========================for test in MATLAB=========================================
int32_t filterC(int32_t in);
int32_t filterAC(int32_t in); //for test in matlab
t_U_MF_int64 fastmul(int32_t A);
int64_t fast(uint64_t in);
void PeakC(int32_t in);
//=========================for test in keil==========================================
void test_MF_main(void);
#endif
