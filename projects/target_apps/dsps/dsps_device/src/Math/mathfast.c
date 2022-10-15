#include "MathFast.h"
#include "FilterC_s19s29_CG1.h"
#include "FilterAC_s19s29_CG.h"

//const int sin1000_len=16;
//int32_t sin1000[sin1000_len]={          
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
////          0,
//};


//const int sin1000_len=sizeof(sin1000)/sizeof(int32_t);
//	
////int32_t r1[sin1000_len];
////int32_t r2[sin1000_len];					
//int64_t r64[sin1000_len];
					
static t_U_MF_uint64 Integrator;

uint32_t Integrator_Hi;
t_U_MF_int64 MF_U_64_fastAC;
//-------------- output fast----------
uint32_t Integrator_Hi_out;
int32_t filterCout;
int32_t filterAout;
int32_t i32_fastAC;
//-----------------------------------

//uint64_t fastdata;
					
//uint8_t IntegratorIndex,IntegratorIndexOut;


//void mathtest(void);
//void mathtest_FilterC(void);
//void mathtest_FilterAC(void);
//void mathtest_fast(void);			



void MF_main(int32_t adcinput)
{
static	uint32_t t;
	
static	uint64_t square;
	
	FilterC_s19s29_CG1_U.Input=adcinput;
	
	FilterC_s19s29_CG1_step_o();
	
	filterCout=FilterC_s19s29_CG1_Y.Output;
	
	FilterAC_s19s29_CG_U.Input=FilterC_s19s29_CG1_Y.Output;
	
	FilterAC_s19s29_CG_step_o();
	
	filterAout=FilterAC_s19s29_CG_Y.Output;
	
	square=((uint64_t)(FilterAC_s19s29_CG_Y.Output))*((uint64_t)(FilterAC_s19s29_CG_Y.Output));
	
	t=Integrator.num32[1];
	Integrator.num64+=square;
	if (t>Integrator.num32[1]) 
  {Integrator_Hi++;
	 Integrator_Hi_out=Integrator_Hi;	
	};
	
	MF_U_64_fastAC.num64=fast(square);
	i32_fastAC=(int32_t)(MF_U_64_fastAC.num32[1]);
	//fast();
	//mathtest();
	//mathtest_FilterC();
	//mathtest_FilterAC();
//	mathtest_fast();
}
;
void test_MF_main(void)
{
//	while(1)
//	{
//		for(int i=0;i<sin1000_len;i++)
//		{
//			MF_main(sin1000[i]);
//		}
//			for(int i=0;i<sin1000_len;i++)
//		{
//			MF_main(-sin1000[i]);
//		}
//	};
}

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

//void mathtest_fast(void)
//{
//	for(int i=0;i<sin1000_len;i++)
//	{
//		square=((uint64_t)sin1000[i])*((uint64_t)sin1000[i]);
//		r64[i]=fast(square); 
//		
//	}
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



uint64_t fastDelay;



t_U_MF_int64 MF_U_64_fastoutinner;
t_U_MF_int64 MF_U_64_fastoutouter;
t_U_MF_int64 MF_U_64_fastMulResult;
int64_t fastoutinner;

#define fastFactor 33550	

int64_t fast(uint64_t in)
{
 MF_U_64_fastoutinner.num64=(in>>7)+fastDelay;
 MF_U_64_fastMulResult=fastmul(fastFactor,MF_U_64_fastoutouter.num32[1]);
 fastDelay=(in>>7)+(MF_U_64_fastoutouter.num64-MF_U_64_fastMulResult.num64);	
 MF_U_64_fastoutouter.num64= MF_U_64_fastoutinner.num64;
 return MF_U_64_fastoutouter.num64;	
}

t_U_MF_int64 fastmul(uint16_t K,int32_t A)
{ 
static	t_U_MF_int64 t;
static	uint32_t r0,r1; 
static 	uint32_t uA;
	if (A<0) 
	{	uA=-A;
		r1=((uA>>16)*K);
		r0=(uA&0xffff)*K;
		//t.num32[0]+=r0<<5;
		t.num64=(r0<<5)+(r1<<(5+16));
		t.num64=-t.num64;
	}
	else
	{ uA=A;
		r1=(uA>>16)*K;
		r0=(uA&0xffff)*K;
		t.num32[0]=0;
		t.num32[1]=(r0>>(32-5))+(r1>>(16-5));
	}
	return t;
};


t_U_MF_int64 fastmul64(uint16_t K,int32_t A)
{ 
static	t_U_MF_int64 t;
static	uint64_t r0,r1; //TODO optimize
static 	uint32_t uA;
	if (A<0) 
	{	uA=-A;
		uA=uA<<1;
		r1=((uA>>16)*K);
		r0=(uA&0xffff)*K;
		//t.num32[0]+=r0<<5;
		t.num64=(r0<<5)+(r1<<(5+16));
		t.num64=-t.num64;
	}
	else
	{ uA=A;
		uA=uA<<1;
		t.num32[1]=(uA>>16)*K;
		r0=(uA&0xffff)*K;
		t.num32[0]=r0<<16;
		t.num32[1]+=(r0>>16);
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

