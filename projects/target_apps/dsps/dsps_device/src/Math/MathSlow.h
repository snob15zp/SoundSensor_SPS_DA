#ifndef __MathSlow_h__
#define __MathSlow_h__

#include <stdint.h>

extern int32_t LogLevelA;
extern int32_t LogLevelC;	
extern int32_t LogLevelIntegrator;

void SM_catch(void);
void EvaluteLogLevel(uint32_t i, int32_t a);
void MS_test_EvaluteLogLevel(void);

#endif