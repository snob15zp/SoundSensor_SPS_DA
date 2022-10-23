#include <math.h>
#include "MathFast.h"
#include "MathSlow.h"




#define D_LogLevelA_offset 0
#define  D_LogLevelC_offset	0
#define  D_LogLevelIntegrator_offset 0

bool MS_b_alert_C140dBPeak;
bool MS_b_alert_FastA;
bool MS_b_alert_live;
bool MS_b_alert_Overload;
bool MS_b_alert_DoseM3dB;	
bool MS_b_alert_Dose;

//extern int32_t MS_i32_AlertLevel_C140dB_Peak;//filterC bits
int32_t MS_i32_AlertLevel_FastA;//0.1dB
//extern int32_t MS_i32_AlertLevel_Overload;//ADC bits
int32_t MS_i32_AlertLevel_Dose;//0.1dB
int32_t MS_i32_AlertLevel_DoseM3dB;//0.1dB	

int32_t MS_i32_Level_C_Peak_dB;//0.1dB
int32_t MS_i32_Level_FastA_dB;//0.1dB
int32_t MS_i32_Level_Dose_dB;//0.1dB
// for catch
uint32_t MS_Integrator_Hi_out;
int32_t MS_i32_fastAC;

#ifndef __NO_MATLAB__
uint32_t MS_couter;
uint32_t MS_matlab_time;
uint32_t MS_matlab_time_out;

int32_t MS_i32_Level_C_Peak_dB_out;//0.1dB
int32_t MS_i32_Level_FastA_dB_out;//0.1dB
int32_t MS_i32_Level_Dose_dB_out;//0.1dB

bool MS_b_alert_C140dBPeak_out;
bool MS_b_alert_FastA_out;
#endif

#ifdef __NO_MATLAB__
void MS_main(void)
{
	MS_catch();
	MS_evalute(PeakC_max,MS_i32_fastAC,MS_Integrator_Hi_out,systick_time);
}
#endif

void MS_evalute(uint32_t c, int32_t a, uint32_t i,uint32_t t)
{
 MS_EvaluteLogLevel_A(c,a,i);
 MS_Alerts(t);	
}

void MS_catch(void)
{
	#ifndef __NO_MATLAB__
	MS_matlab_time++;
	MS_matlab_time_out=MS_matlab_time;
	if (++MS_couter==4000)
	{ MS_couter=0;
	#endif

	catch_flag=true;
	MS_Integrator_Hi_out=Integrator_Hi_out;
	MS_i32_fastAC=i32_fastAC;
	#ifndef __NO_MATLAB__
	};
	#endif
	
}
	
void MS_EvaluteLogLevel(void)
{
	MS_EvaluteLogLevel_A(PeakC_max,MS_i32_fastAC,MS_Integrator_Hi_out);
}

uint32_t MF_ADCOverLoad_Flag_old;
uint32_t MF_ADCOverLoad_Flag_time;

void MS_EvaluteLogLevel_A(uint32_t c, int32_t a, uint32_t i)
{

MS_i32_Level_FastA_dB=  MS_D_dBscale*(10.0*log10(a) + MS_D_offset_FastA_dB); //0.01dB
MS_i32_Level_Dose_dB=   MS_D_dBscale*(10.0*log10(i) + (74.53-9.0));	//0.01dBMS_D_offset_Dose_dB
MS_i32_Level_C_Peak_dB= MS_D_dBscale*(20.0*log10(c) + MS_D_offset_C_Peak_dB);//0.01dB
#ifndef __NO_MATLAB__	
MS_i32_Level_C_Peak_dB_out=MS_i32_Level_C_Peak_dB;//0.1dB
MS_i32_Level_FastA_dB_out=MS_i32_Level_FastA_dB;//0.1dB
MS_i32_Level_Dose_dB_out=MS_i32_Level_Dose_dB;//0.1dB	
#endif	
}	

void MS_Alerts(uint32_t time)
{
	MS_b_alert_FastA=(MS_i32_Level_FastA_dB>MS_i32_AlertLevel_FastA);
	MS_b_alert_C140dBPeak=(PeakC_max>MS_D_AlertLevel_C_140bB_peak);
	MS_b_alert_live=MS_b_alert_FastA||MS_b_alert_C140dBPeak;
	
#ifndef __NO_MATLAB__
MS_b_alert_FastA_out=MS_b_alert_FastA;
MS_b_alert_C140dBPeak_out=MS_b_alert_C140dBPeak;
#endif
	
	if (MF_ADCOverLoad_Flag_old!=MF_ADCOverLoad_Flag)
	{ 
		MF_ADCOverLoad_Flag_old=MF_ADCOverLoad_Flag;
		MF_ADCOverLoad_Flag_time=time;
	}
	if ((time-MF_ADCOverLoad_Flag_time)<(MF_ADCOverLoad_Flag_duration_US/SYSTICK_PERIOD_US))
	{
	 MS_b_alert_Overload=true;
	}
	else
	{ 
		MS_b_alert_Overload=false;
		MF_ADCOverLoad_Flag_time=time-(MF_ADCOverLoad_Flag_duration_US/SYSTICK_PERIOD_US)*2;
	}
	
	MS_b_alert_DoseM3dB=(MS_i32_Level_Dose_dB>(MS_i32_AlertLevel_DoseM3dB));
	
	MS_b_alert_Dose=(MS_i32_Level_Dose_dB>MS_i32_AlertLevel_Dose);
	
};





//void MS_test_EvaluteLogLevel(void)
//{
//	EvaluteLogLevel(121,1092782150);
//}

void MS_init(void)
{

//MS_i32_AlertLevel_C140dB_Peak=MS_D_AlertLevel_C_140bB_peak;//filterC bits
MS_i32_AlertLevel_FastA=(MS_D_AlertLevel_FastA)*(MS_D_dBscale);//0.1dB
//MS_i32_AlertLevel_Overload=MS_D_AlertLevel_Overload;//ADC bits
MS_i32_AlertLevel_Dose=(MS_D_AlertLevel_Dose)*(MS_D_dBscale);//0.1dB
MS_i32_AlertLevel_DoseM3dB=(MS_D_AlertLevel_DoseM3dB)*(MS_D_dBscale);//0.1dB	


MS_b_alert_C140dBPeak=false;
MS_b_alert_FastA=false;
MS_b_alert_live=false;	
MS_b_alert_Overload=false;
MS_b_alert_DoseM3dB=false;	
MS_b_alert_Dose=false;


MS_i32_Level_C_Peak_dB=0;//0.1dB
MS_i32_Level_FastA_dB=0;//0.1dB
MS_i32_Level_Dose_dB=0;//0.1dB
// for catch
MS_Integrator_Hi_out=0;
MS_i32_fastAC=0;

#ifndef __NO_MATLAB__
MS_couter=0;
MS_matlab_time=0;

//MS_b_alert_FastA_out=0;
//MS_b_alert_C140dBPeak_out=0;
#endif	
};
