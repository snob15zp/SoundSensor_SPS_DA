#include "ss_global.h"
#include "SS_sys.h"
#include "systick.h"
#include "gpio.h"
#include "MathFast.h"
#include "ADC_flash.h"
#include "version.h"

#include "user_periph_setup.h"
#include "user_config_storage.h"

#define SYSTICK_EXCEPTION   1           // generate systick exceptions


static uint16_t vars[VARS_CNT] = {0};
uint8_t verify_value_cb (uint8_t* param);

static const user_config_elem_t conf_table[] = {
    {0x1000, sizeof(uint16_t) * VARS_CNT, sizeof(uint16_t) * VARS_CNT, vars, verify_value_cb, false},
};

volatile uint32_t systick_time;

#ifndef __SoundSensor__
volatile static uint8_t i;
#endif


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


void systick_irq()
{
    systick_time++;	  
#ifndef __SoundSensor__
	  GPIO_SetActive(LED_PORT, LED_PIN); 
#endif	
	  if (ADCon)
	  test_MF_main_ADCEmul();
#ifndef __SoundSensor__	
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
    systick_register_callback(systick_irq);
    systick_start(SYSTICK_PERIOD_US, SYSTICK_EXCEPTION);
    
    uint8_t version_len = strlen(SDK_VERSION);
    char version[16] = {0};
    strncpy(version, SDK_VERSION, sizeof(version));

    user_conf_storage_init((user_config_elem_t *)conf_table, sizeof(conf_table)/sizeof(user_config_elem_t), version, &version_len);
    vars[0] += 1;
    user_config_save((user_config_elem_t *)conf_table, sizeof(conf_table)/sizeof(user_config_elem_t), version, &version_len);
}

void LEDinit (void)
{
#ifndef __SoundSensor__	
GPIO_ConfigurePin(LED_PORT, LED_PIN, OUTPUT, PID_GPIO, false);
GPIO_set_pad_latch_en(true);
				GPIO_SetActive(LED_PORT, LED_PIN);
	      GPIO_SetInactive(LED_PORT, LED_PIN);
#endif	
	
};
