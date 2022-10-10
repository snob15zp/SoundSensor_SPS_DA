#include "MathFast.h"
#include "FilterC_s19s29_CG1.h"
#include "FilterAC_s19s29_CG.h"


t_U_MF_uint64 Integrator;
uint32_t Integrator_Hi;

uint64_t square;

uint8_t IntegratorIndex,IntegratorIndexOut;

void fast();

void MF_main(void)
{
	uint32_t t;
	
	FilterC_s19s29_CG1_step();
	
	FilterAC_s19s29_CG_U.Input=FilterC_s19s29_CG1_Y.Output;
	
	FilterAC_s19s29_CG_step();
	
	square=FilterAC_s19s29_CG_Y.Output*FilterAC_s19s29_CG_Y.Output;
	
	t=Integrator.num32[1];
	Integrator.num64=+FilterAC_s19s29_CG_Y.Output*FilterAC_s19s29_CG_Y.Output;
	if (t>Integrator.num32[1]) {Integrator_Hi++;};
	
	fast();
	
}
;



t_U_MF_int64 fastmul(int32_t A,uint16_t K);

uint64_t fastDelay;



t_U_MF_int64 MF_U_64_fastoutinner;
t_U_MF_int64 MF_U_64_fastoutouter;
t_U_MF_int64 MF_U_64_fastMulResult;
int64_t fastoutinner;

#define fastFactor 33000

void fast()
{
 MF_U_64_fastoutinner.num64=(square>>7)+fastDelay;
 MF_U_64_fastMulResult=fastmul(MF_U_64_fastoutouter.num32[1],fastFactor);
 fastDelay=square+(MF_U_64_fastoutouter.num64-MF_U_64_fastMulResult.num64);	
 MF_U_64_fastoutouter.num64= MF_U_64_fastoutinner.num64;
}


t_U_MF_int64 fastmul(int32_t A,uint16_t K)
{ 
	t_U_MF_int64 t;
	uint32_t r0;
	uint32_t uA;
	if (A<0) 
	{	uA=-A;
		uA=uA<<1;
		t.num32[1]=(uA>>16)*K;
		r0=(uA&0xffff)*K;
		t.num32[0]=r0<<16;
		t.num32[1]+=(uA>>16);
		t.num64=-t.num64;
	}
	else
	{ uA=A;
		uA=uA<<1;
		t.num32[1]=(uA>>16)*K;
		r0=(uA&0xffff)*K;
		t.num32[0]=r0<<16;
		t.num32[1]+=(uA>>16);
	}
	return t;
};

inline int32_T mul_u18s29sh(int32_T a, int32_T b, uint32_T aShift) 
{
	int32_T ab;
	uint32_T b_h;
	uint32_T b_l;
	if (b>=0)
	{
	b_h = ((uint32_T)b) >> 14;
  b_l = ((uint32_T )b) &	0x3fff;
	ab=( (b_h * a )>>(4+aShift) ) 	+  ( (b_l * a )>> (18+aShift) );
	}
	else
	{
		b=-b;
	b_h = ((uint32_T)b) >> 14;
  b_l = ((uint32_T )b) &	0x3fff;
	ab=( (b_h * a )>>(4+aShift) )	+ ( (b_l * a )>> (18+aShift));
	ab=-ab;	
	}
	return ab;
}
