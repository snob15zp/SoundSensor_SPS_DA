#include "ss_global.h"
#include "SS_sys.h"
#include "systick.h"
#include "gpio.h"
#include "MathFast.h"
#include "MathSlow.h"
#include "ADC_flash.h"
#include "SPI_ADC.h"
#include "version.h"

#include "user_periph_setup.h"
#include "user_config_storage.h"


#define SYSTICK_EXCEPTION   1           // generate systick exceptions

bool SSS_CalibrationMode=true;

E_ADC_MODE_t	SS_ADC_MODE;
static uint16_t vars[VARS_CNT] = {0};
uint8_t verify_value_cb (uint8_t* param);

static const user_config_elem_t SSS_conf_table[] = 
{
	{0x1000, sizeof(MS_i32_AlertLevel_FastA), sizeof(MS_i32_AlertLevel_FastA), &MS_i32_AlertLevel_FastA, verify_value_cb, false},
	{0x1001, sizeof(MS_i32_AlertLevel_C140dB_Peak), sizeof(MS_i32_AlertLevel_C140dB_Peak), &MS_i32_AlertLevel_C140dB_Peak, verify_value_cb, false},
	{0x1002, sizeof(MS_i32_AlertLevel_Dose), sizeof(MS_i32_AlertLevel_Dose), &MS_i32_AlertLevel_Dose, verify_value_cb, false},
	{0x1003, sizeof(MS_i32_AlertLevel_DoseM3dB), sizeof(MS_i32_AlertLevel_DoseM3dB), &MS_i32_AlertLevel_DoseM3dB, verify_value_cb, false},
	{0x1004, sizeof(MS_i32_CalibrationFactor), sizeof(MS_i32_CalibrationFactor), &MS_i32_CalibrationFactor, verify_value_cb, false},
	{0x1005, sizeof(SSS_CalibrationMode), sizeof(SSS_CalibrationMode), &SSS_CalibrationMode, verify_value_cb, false},
};
static const user_config_elem_t SSS_RO_table[] = 
{
	{0x2000, sizeof(MS_i32_Level_FastA_dB), sizeof(MS_i32_Level_FastA_dB), &MS_i32_Level_FastA_dB, verify_value_cb, false},
	{0x2001, sizeof(MS_i32_Level_C_Peak_dB), sizeof(MS_i32_Level_C_Peak_dB), &MS_i32_Level_C_Peak_dB, verify_value_cb, false},
	{0x2002, sizeof(MS_i32_Level_Dose_dB), sizeof(MS_i32_Level_Dose_dB), &MS_i32_Level_Dose_dB, verify_value_cb, false},
};

static uint8_t SSS_u8_RW_vars_index_table;

static uint8_t SSS_u8_index_vars;
#define SSS_u8_index_vars_start 4;

void SSS_WriteToVar(void)
{ SSS_u8_index_vars=SSS_u8_index_vars_start;
  SSS_WriteTableToVar(SSS_conf_table,&SSS_u8_index_vars,sizeof(SSS_conf_table)/sizeof(user_config_elem_t));
	SSS_WriteTableToVar(SSS_RO_table,&SSS_u8_index_vars,sizeof(SSS_RO_table)/sizeof(user_config_elem_t));
}

void SSS_WriteTableToVar(const user_config_elem_t *t, uint8_t *var_index, uint8_t ms)
{
	SSS_u8_RW_vars_index_table=0;
	while(SSS_u8_RW_vars_index_table<ms)
	{
		memcpy(vars+(SSS_u8_index_vars/2),
		       t[SSS_u8_RW_vars_index_table].data, 
		       t[SSS_u8_RW_vars_index_table].cur_size
		      );
		SSS_u8_index_vars+=(((t[SSS_u8_RW_vars_index_table].cur_size)+1)&0xFE);
		SSS_u8_RW_vars_index_table++;
	}		
}

void SSS_ReadTableFromVar(const user_config_elem_t *t, uint8_t *var_index, uint8_t ms)
{
	SSS_u8_RW_vars_index_table=0;
	
	while(SSS_u8_RW_vars_index_table<ms)
	{
		memcpy(t[SSS_u8_RW_vars_index_table].data,
		       vars+(SSS_u8_index_vars/2), 
		       t[SSS_u8_RW_vars_index_table].cur_size
		      );
		SSS_u8_index_vars+=(((t[SSS_u8_RW_vars_index_table].cur_size)+1)&0xFE);
		SSS_u8_RW_vars_index_table++;
	}		
}

void SSS_ReadFromVar(void)
{ SSS_u8_index_vars=SSS_u8_index_vars_start;
  SSS_ReadTableFromVar(SSS_conf_table,&SSS_u8_index_vars,sizeof(SSS_conf_table)/sizeof(user_config_elem_t));
}


/* parametes

1	LiveWarningLevelAF	Live warning level, weighting A, time weighting fast	xxx.x	dB(A)
extern int32_t MS_i32_AlertLevel_FastA;//0.1dB MODBUS ADDRESS 2
2	LiveWarningLevelCP	Live warning level, weighting C, peak	xxx.x	dB(C)
extern int32_t MS_i32_AlertLevel_C140dB_Peak; MODBUS ADDRESS 4
3	LAeqLimit	The normalized 8-hour equivalentexposure limit	xxx.x	dB(A)
extern int32_t MS_i32_AlertLevel_Dose; MODBUS ADDRESS 6
4
extern int32_t MS_i32_AlertLevel_DoseM3dB MODBUS ADDRESS 8
5	CalibrationFactor	Calibration factor	x.xxxxx	 -
int32_t MS_i32_CalibrationFactor; MODBUS ADDRESS 10
6	ClibrationMode	Turning on the calibration mode	Check box	-
uint16_t SSS_calibration MODBUS ADDRESS 12

Table of reading parameters
#	Variable name	Description	Format	Units
1	LiveLevelAF	Live level, weighting A, time weighting fast	xxx.x	dB(A)
extern int32_t MS_i32_Level_FastA_dB;//0.1dB MODBUS ADDRESS 13
2	LiveLevelCP	Live level, weighting C, peak	xxx.x	dB(C)
extern int32_t MS_i32_Level_C_Peak_dB;//0.1dB MODBUS ADDRESS 15
3	LAeq	The normalized 8-hour equivalentexposure	xxx.x	dB(A)
extern int32_t MS_i32_Level_Dose_dB;//0.1dB MODBUS ADDRESS 17



*/

volatile uint32_t systick_time;

//#ifndef __SoundSensor__
//volatile static uint8_t i;
//#endif


static uint8_t verify_value_cb (uint8_t* param)
{
    return 0;
}


uint64_t get_systime(void) 
{
    return systick_time;
}

uint16_t* get_vars(void)
{
    return vars;
}


void SSS_SetUpTimeEvent(t_SSS_s_timeevent *s,uint32_t dt)
{
	s->dtime=dt;
	s->time=systick_time;
	s->enable=true;
};

void systick_irq()
{
    systick_time++;	  
#ifdef __DEVKIT_EXT__
	  GPIO_SetActive(LED_PORT, LED_PIN); 
#endif	
//	  if (SS_ADC_MODE==EAM_ADC_WORK)
//			//test_MF_main_ADCEmul();
//		ADC_IRQ();
#ifdef __DEVKIT_EXT__	
	  GPIO_SetInactive(LED_PORT, LED_PIN);
#endif	

//#ifndef
//    if (i == 0)
//    {
//        GPIO_SetActive(LED_PORT, LED_PIN);
//        i = 1;
//    }
//    else
//    {
//        GPIO_SetInactive(LED_PORT, LED_PIN);
//        i = 0;
//    }
//#endif		
	
}


void test_hnd_init(void)
{
	  systick_stop();
	  NVIC_SetPriority(SysTick_IRQn, 2);
    systick_register_callback(systick_irq);
    systick_start(D_SYSTICK_PERIOD_US, SYSTICK_EXCEPTION);
    
    uint8_t version_len = strlen(SDK_VERSION);
    char version[16] = {0};
    strncpy(version, SDK_VERSION, sizeof(version));

//    user_conf_storage_init((user_config_elem_t *)SSS_conf_table, sizeof(SSS_conf_table)/sizeof(user_config_elem_t), version, &version_len);
    vars[0] += 1;
//    user_config_save((user_config_elem_t *)SSS_conf_table, sizeof(SSS_conf_table)/sizeof(user_config_elem_t), version, &version_len);
}

void LEDinit (void)
{
#ifdef __DEVKIT_EXT__	
//GPIO_ConfigurePin(LED_PORT, LED_PIN, OUTPUT, PID_GPIO, false);
//GPIO_set_pad_latch_en(true);
//				GPIO_SetActive(LED_PORT, LED_PIN);
//	      GPIO_SetInactive(LED_PORT, LED_PIN);
#endif	
	
};

void SS_spi_switchoff_pins(uint32_t gpio_map)
{
    spi_gpio_config_t spi_conf;

    spi_conf.clk.port      = ((GPIO_PORT)((gpio_map & 0x000000f0) >>  4));
    spi_conf.clk.pin       = ((GPIO_PIN) ((gpio_map & 0x0000000f)));
    spi_conf.cs.port       = ((GPIO_PORT)((gpio_map & 0x0000f000) >> 12));
    spi_conf.cs.pin        = ((GPIO_PIN) ((gpio_map & 0x00000f00) >> 8));
    spi_conf.mosi.port     = ((GPIO_PORT)((gpio_map & 0x00f00000) >> 20));
    spi_conf.mosi.pin      = ((GPIO_PIN) ((gpio_map & 0x000f0000) >> 16));
    spi_conf.miso.port     = ((GPIO_PORT)((gpio_map & 0xf0000000) >> 28));
    spi_conf.miso.pin      = ((GPIO_PIN) ((gpio_map & 0x0f000000) >> 24));

    GPIO_ConfigurePin( spi_conf.cs.port, spi_conf.cs.pin, OUTPUT, PID_GPIO, true );
    GPIO_ConfigurePin( spi_conf.clk.port, spi_conf.clk.pin, INPUT_PULLDOWN, PID_GPIO, false );
    GPIO_ConfigurePin( spi_conf.mosi.port, spi_conf.mosi.pin, INPUT_PULLDOWN, PID_GPIO, false );
    GPIO_ConfigurePin( spi_conf.miso.port, spi_conf.miso.pin, INPUT_PULLDOWN, PID_GPIO, false );

}

void ss_spi_init(uint32_t gpio_map, const spi_cfg_t *spi_cfg)//
{
    spi_gpio_config_t spi_conf;

    spi_conf.clk.port      = ((GPIO_PORT)((gpio_map & 0x000000f0) >>  4));
    spi_conf.clk.pin       = ((GPIO_PIN) ((gpio_map & 0x0000000f)));
    spi_conf.cs.port       = ((GPIO_PORT)((gpio_map & 0x0000f000) >> 12));
    spi_conf.cs.pin        = ((GPIO_PIN) ((gpio_map & 0x00000f00) >> 8));
    spi_conf.mosi.port     = ((GPIO_PORT)((gpio_map & 0x00f00000) >> 20));
    spi_conf.mosi.pin      = ((GPIO_PIN) ((gpio_map & 0x000f0000) >> 16));
    spi_conf.miso.port     = ((GPIO_PORT)((gpio_map & 0xf0000000) >> 28));
    spi_conf.miso.pin      = ((GPIO_PIN) ((gpio_map & 0x0f000000) >> 24));

    GPIO_ConfigurePin( spi_conf.cs.port, spi_conf.cs.pin, OUTPUT, PID_SPI_EN, true );
    GPIO_ConfigurePin( spi_conf.clk.port, spi_conf.clk.pin, OUTPUT, PID_SPI_CLK, false );
    GPIO_ConfigurePin( spi_conf.mosi.port, spi_conf.mosi.pin, OUTPUT, PID_SPI_DO, false );
    GPIO_ConfigurePin( spi_conf.miso.port, spi_conf.miso.pin, INPUT, PID_SPI_DI, false );

    spi_initialize(spi_cfg);
}
