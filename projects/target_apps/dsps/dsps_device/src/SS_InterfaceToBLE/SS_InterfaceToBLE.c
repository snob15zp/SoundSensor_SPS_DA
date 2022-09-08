#include "SS_InterfaceToBLE.h"

//#include "user_periph_setup.h"
//#include "datasheet.h"
#include "gpio.h"
//#include "uart.h"
//#include "syscntl.h"

#define ITB_pause 100
#define LED_PORT                GPIO_PORT_0
#define LED_PIN                 GPIO_PIN_9

static int8_t ITB_state;

timer_hnd ITB_timer  __SECTION_ZERO("retention_mem_area0"); //@RETENTION MEMORY;

static void adv_data_update_timer_cb()
{
if (ITB_state)
  {GPIO_SetActive(LED_PORT, LED_PIN);}
   else
	 {GPIO_SetInactive(LED_PORT, LED_PIN); };
	 ITB_state=!ITB_state;
    ITB_timer = app_easy_timer(ITB_pause, adv_data_update_timer_cb);
}

void user_app_adv_start(void)
{

ITB_timer = app_easy_timer(ITB_pause, adv_data_update_timer_cb);

}

//app_easy_timer_cancel(app_adv_data_update_timer_used);


//    #define LED_PORT                GPIO_PORT_0
//    #define LED_PIN                 GPIO_PIN_9
//GPIO_ConfigurePin(LED_PORT, LED_PIN, OUTPUT, PID_GPIO, false);
//GPIO_set_pad_latch_en(true);
//				GPIO_SetActive(LED_PORT, LED_PIN);
//				 Filter0_step();
//				GPIO_SetInactive(LED_PORT, LED_PIN);

void SS_InterfaceToBLE_init(void)
{

GPIO_ConfigurePin(LED_PORT, LED_PIN, OUTPUT, PID_GPIO, false);
GPIO_set_pad_latch_en(true);
				GPIO_SetActive(LED_PORT, LED_PIN);
	      GPIO_SetInactive(LED_PORT, LED_PIN);
user_app_adv_start();
}
