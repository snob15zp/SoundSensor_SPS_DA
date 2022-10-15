#include "SS_sys.h"
#include "systick.h"
#include "gpio.h"
#include "MathFast.h"

#define SYSTICK_PERIOD_US   64     // period for systick timer in us, so 1000000ticks = 1second
#define SYSTICK_EXCEPTION   1           // generate systick exceptions



volatile uint32_t systick_time;

#ifndef __SoundSensor__
volatile static uint8_t i;
#endif

void systick_irq()
{
    systick_time++;	  
#ifndef __SoundSensor__
	  GPIO_SetActive(LED_PORT, LED_PIN); 
#endif	
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
