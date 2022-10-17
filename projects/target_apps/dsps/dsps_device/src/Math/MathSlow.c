#include <math.h>
#include "MathFast.h"
#include "MathSlow.h"

int32_t LogLevelA;
int32_t LogLevelC;	
int32_t LogLevelIntegrator;

#define D_LogLevelA_offset 0
#define  D_LogLevelC_offset	0
#define  D_LogLevelIntegrator_offset 0

uint32_t MS_Integrator_Hi_out;
int32_t MS_i32_fastAC;


void SM_catch(void)
{
MS_Integrator_Hi_out=Integrator_Hi_out;
MS_i32_fastAC=i32_fastAC;
}
	

void EvaluteLogLevel(void)
{
LogLevelA=            1000*log10(MS_i32_fastAC)+D_LogLevelA_offset;
LogLevelIntegrator=   1000*log10(MS_Integrator_Hi_out)+D_LogLevelIntegrator_offset;	
}	


//void MS_test_EvaluteLogLevel(void)
//{
//	EvaluteLogLevel(121,1092782150);
//}
