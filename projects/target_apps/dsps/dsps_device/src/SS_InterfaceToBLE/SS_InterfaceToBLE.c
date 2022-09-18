#include "SS_InterfaceToBLE.h"
#include "SPI_ADC.h"

//#include "user_periph_setup.h"
//#include "datasheet.h"
#include "gpio.h"
//#include "uart.h"
//#include "syscntl.h"
#include "user_sps_utils.h"
#include "dma_uart_sps.h"

#define ITB_pause 100
#define LED_PORT                GPIO_PORT_0
#define LED_PIN                 GPIO_PIN_9

static int8_t ITB_state;
static int8_t ITB_initstatus=0;

timer_hnd ITB_timer;//  __SECTION_ZERO("retention_mem_area0"); //@RETENTION MEMORY;

static void adv_data_update_timer_cb()
{
if (ITB_state)
  {
#ifdef __KIT__		
	GPIO_SetActive(LED_PORT, LED_PIN);
#endif	
	;}
   else
	 {
#ifdef __KIT__
	 GPIO_SetInactive(LED_PORT, LED_PIN);
#endif
		 ;};
	 ITB_state=!ITB_state;
    ITB_timer = app_easy_timer(ITB_pause, adv_data_update_timer_cb);
	 SS_InterfaceToBLE_SendThreeBytes();
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
if (ITB_initstatus==0)
{ITB_initstatus=1;
#ifdef __KIT__	
GPIO_ConfigurePin(LED_PORT, LED_PIN, OUTPUT, PID_GPIO, false);
GPIO_set_pad_latch_en(true);
//				GPIO_SetActive(LED_PORT, LED_PIN);
//	      GPIO_SetInactive(LED_PORT, LED_PIN);
#endif	
adv_data_update_timer_cb();
}
}

void SS_InterfaceToBLE_SendThreeBytes(void)
{	
                                    //sps_client_data_tx_req
//  struct UART_RX_STRUCT *p_buff = KE_MSG_ALLOC_DYN_NO_INIT(DATA_TX_REQ, //RDD id, dest, src, param_str,length
//                                                             prf_get_task_from_id(DEST_TASK_ID), //RDD #define DEST_TASK_ID TASK_ID_SPS_CLIENT
//                                                             TASK_APP,
//                                                             UART_RX_STRUCT,
//                                                             ELEMENT_SIZE_RX);

//	

//        p_buff->length = 3;
//	      p_buff->data[1]=1;
//	      p_buff->data[2]=2;
//	      p_buff->data[3]=3;
//        ASSERT_ERROR(p_buff);
//        ke_msg_send(p_buff);
	char a[]="012";
	user_send_ble_data((uint8_t*)a,3);
	SPITreeByts();
	
}
