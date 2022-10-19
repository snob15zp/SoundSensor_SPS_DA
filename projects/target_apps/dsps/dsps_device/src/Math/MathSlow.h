#ifndef __MathSlow_h__
#define __MathSlow_h__

#include <stdint.h>

extern int32_t LogLevelA;
extern int32_t LogLevelC;	
extern int32_t LogLevelIntegrator;

void MS_catch(void);
void MS_EvaluteLogLevel(void);
void MS_EvaluteLogLevel_A(int32_t a, uint32_t i);//for matlab
void MS_test_EvaluteLogLevel(void);

#endif
