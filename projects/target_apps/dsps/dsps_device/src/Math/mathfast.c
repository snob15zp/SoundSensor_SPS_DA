#include "MathFast.h"
#include "FilterC_s19s29_CG1.h"
#include "FilterAC_s19s29_CG.h"


//int32_t sin1000[]={          
//	        0,
//   13092290,
//   25681450,
//   37283687,
//   47453133,
//   55798981,
//   62000506,
//   65819386,
//   67108864,
//   65819386,
//   62000506,
//   55798981,
//   47453133,
//   37283687,
//   25681450,
//   13092290,
//          0,};


//const int sin1000_len=sizeof(sin1000)/sizeof(int32_t);
//	
//int32_t r1[sin1000_len];
//int32_t r2[sin1000_len];					
					
t_U_MF_uint64 Integrator;
uint32_t Integrator_Hi;

uint64_t square;

uint8_t IntegratorIndex,IntegratorIndexOut;

void fast(void);
void mathtest(void);
void mathtest_FilterC(void);
void mathtest_FilterAC(void);

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
	//mathtest();
	//mathtest_FilterC();
	//mathtest_FilterAC();
}
;

//void mathtest(void)
//{
//const int mi=2;//#define mi 2
//int32_t a[mi]={65536,-65536};
//int32_t k[mi]={1, 1};
//int32_t s[mi]={14, 14};	


//for(int i=0;i<mi;i++)
//{
//	r1[i]=mul_u18s29sh(k[i],a[i],s[i]);
//	r2[i]=mul_s32_loSR(k[i],a[i],s[i]);
//}

//}

//void mathtest_FilterC(void)
//{
//const int mi=2;//#define mi 2
//int32_t a[mi]={65536,-65536};
//	

//FilterC_s19s29_CG1_initialize();
//for(int i=0;i<mi;i++)
//{
//	FilterC_s19s29_CG1_U.Input=a[i];
//  FilterC_s19s29_CG1_step(); 
//	r1[i]=FilterC_s19s29_CG1_Y.Output;
//}
//FilterC_s19s29_CG1_initialize();
//for(int i=0;i<mi;i++)
//{
//	FilterC_s19s29_CG1_U.Input=a[i];
//  FilterC_s19s29_CG1_step_o(); 
//	r2[i]=FilterC_s19s29_CG1_Y.Output;
//}

//}

//void mathtest_FilterAC(void)
//{
//	

//FilterAC_s19s29_CG_initialize();
//for(int i=8;i<sin1000_len;i++)
//{
//	FilterAC_s19s29_CG_U.Input=sin1000[i];
//  FilterAC_s19s29_CG_step(); 
//	r1[i]=FilterAC_s19s29_CG_Y.Output;
//}
//FilterAC_s19s29_CG_initialize();
//for(int i=8;i<sin1000_len;i++)
//{
//	FilterAC_s19s29_CG_U.Input=sin1000[i];
//  FilterAC_s19s29_CG_step_o(); 
//	r2[i]=FilterAC_s19s29_CG_Y.Output;
//}

//}



int32_t filterC(int32_t in) //for test in matlab
{
	FilterC_s19s29_CG1_U.Input=in;
  FilterC_s19s29_CG1_step_o(); 
	return FilterC_s19s29_CG1_Y.Output;
}

int32_t filterAC(int32_t in) //for test in matlab
{
	FilterAC_s19s29_CG_U.Input=in;
  FilterAC_s19s29_CG_step_o(); 
	return FilterAC_s19s29_CG_Y.Output;
}

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

inline int32_T mul_u18s29sh(int32_T a, int32_T b, uint32_T aShift) //aShift=0 ->
{
	int32_T ab;
  uint32_T b_h;
	uint32_T b_l;
//static int32_T A;
//static int32_T B;
//	A=a;
//	B=b;
	if (b>=0)
	{
	b_h = ((uint32_T)b) >> 14;
  b_l = ((uint32_T )b) &	0x3fff;
	ab=( (b_h * a )>>(aShift-14) ) 	+  ( (b_l * a )>> (aShift) );
	}
	else
	{
		b=-b;
	b_h = ((uint32_T)b) >> 14;
  b_l = ((uint32_T )b) &	0x3fff;
	ab=( (b_h * a )>>(aShift-14) )	+ ( (b_l * a )>> (aShift));
	ab=-ab;	
	}
	return ab;
}

inline int32_T mul_u18s29shl(int32_T a, int32_T b, uint32_T aShift) //aShift=0 ->
{
	int32_T ab;
	uint32_T b_h;
	uint32_T b_l;
//static int32_T A;
//static int32_T B;
//	A=a;
//	B=b;
	if (b>=0)
	{
	b_h = ((uint32_T)b) >> 14;
  b_l = ((uint32_T )b) &	0x3fff;
	ab=( (b_h * a )<<(14-aShift) ) 	+  ( (b_l * a )>> (aShift) );
	}
	else
	{
		b=-b;
	b_h = ((uint32_T)b) >> 14;
  b_l = ((uint32_T )b) &	0x3fff;
	ab=( (b_h * a )<<(14-aShift) )	+ ( (b_l * a )>> (aShift));
	ab=-ab;	
	}
	return ab;
}

void mul_wide_s32(int32_T in0, int32_T in1, uint32_T *ptrOutBitsHi, uint32_T
                  *ptrOutBitsLo)
{
  uint32_T absIn0;
  uint32_T absIn1;
  uint32_T in0Hi;
  uint32_T in0Lo;
  uint32_T in1Hi;
  uint32_T productHiLo;
  uint32_T productLoHi;
  absIn0 = in0 < 0 ? ~(uint32_T)in0 + 1U : (uint32_T)in0;
  absIn1 = in1 < 0 ? ~(uint32_T)in1 + 1U : (uint32_T)in1;
  in0Hi = absIn0 >> 16U;
  in0Lo = absIn0 & 65535U;
  in1Hi = absIn1 >> 16U;
  absIn0 = absIn1 & 65535U;
  productHiLo = in0Hi * absIn0;
  productLoHi = in0Lo * in1Hi;
  absIn0 *= in0Lo;
  absIn1 = 0U;
  in0Lo = (productLoHi << 16U) + absIn0;
  if (in0Lo < absIn0) {
    absIn1 = 1U;
  }

  absIn0 = in0Lo;
  in0Lo += productHiLo << 16U;
  if (in0Lo < absIn0) {
    absIn1++;
  }

  absIn0 = (((productLoHi >> 16U) + (productHiLo >> 16U)) + in0Hi * in1Hi) +
    absIn1;
  if ((in0 != 0) && ((in1 != 0) && ((in0 > 0) != (in1 > 0)))) {
    absIn0 = ~absIn0;
    in0Lo = ~in0Lo;
    in0Lo++;
    if (in0Lo == 0U) {
      absIn0++;
    }
  }

  *ptrOutBitsHi = absIn0;
  *ptrOutBitsLo = in0Lo;
}

int32_T mul_s32_loSR(int32_T a, int32_T b, uint32_T aShift)//aShift=0 -> only low bits
{
  uint32_T u32_chi;
  uint32_T u32_clo;
  mul_wide_s32(a, b, &u32_chi, &u32_clo);
  u32_clo = u32_chi << (32U - aShift) | u32_clo >> aShift;
  return (int32_T)u32_clo;
}

