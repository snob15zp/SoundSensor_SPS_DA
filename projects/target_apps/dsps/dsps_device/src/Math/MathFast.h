#ifndef __MathFast_h__
#define __MathFast_h__

#include <stdint.h>

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

void MF_main(void);
int32_T mul_u18s29sh(int32_T a, int32_T b, uint32_T aShift);
int32_T mul_u18s29shl(int32_T a, int32_T b, uint32_T aShift); //aShift=0 ->
int32_T mul_s32_loSR(int32_T a, int32_T b, uint32_T aShift);

#endif
