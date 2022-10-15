#ifndef __SS_sys_h__
#define __SS_sys_h__

#include "stdint.h"

#ifndef __SoundSensor__
    #define LED_PORT                GPIO_PORT_0
    #define LED_PIN                 GPIO_PIN_9
#endif


extern volatile uint32_t systick_time;

void LEDinit (void);
void test_hnd_init(void);


#endif

