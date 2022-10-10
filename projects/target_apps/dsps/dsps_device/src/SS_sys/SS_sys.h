#ifndef __SS_sys_h__
#define __SS_sys_h__

#ifndef __SoundSensor__
    #define LED_PORT                GPIO_PORT_0
    #define LED_PIN                 GPIO_PIN_9
#endif

void LEDinit (void);
void test_hnd_init(void);


#endif

