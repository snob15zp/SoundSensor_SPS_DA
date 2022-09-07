#include "SS_InterfaceToBLE.h"

//#include "user_periph_setup.h"
//#include "datasheet.h"
#include "gpio.h"
//#include "uart.h"
//#include "syscntl.h"


timer_hnd app_adv_data_update_timer_used;

//static void adv_data_update_timer_cb()
//{
//    // If mnd_data_index has MSB set, manufacturer data is stored in scan response
//    uint8_t *mnf_data_storage = (mnf_data_index & 0x80) ? stored_scan_rsp_data : stored_adv_data;

//    // Update manufacturer data
//    mnf_data_update();

//    // Update the selected fields of the advertising data (manufacturer data)
//    memcpy(mnf_data_storage + (mnf_data_index & 0x7F), &mnf_data, sizeof(struct mnf_specific_data_ad_structure));

//    // Update advertising data on the fly
//    app_easy_gap_update_adv_data(stored_adv_data, stored_adv_data_len, stored_scan_rsp_data, stored_scan_rsp_data_len);
//    
//    // Restart timer for the next advertising update
//    app_adv_data_update_timer_used = app_easy_timer(APP_ADV_DATA_UPDATE_TO, adv_data_update_timer_cb);
//}

//void user_app_adv_start(void)
//{
//    // Schedule the next advertising data update
//    app_adv_data_update_timer_used = app_easy_timer(APP_ADV_DATA_UPDATE_TO, adv_data_update_timer_cb);
//    
//    struct gapm_start_advertise_cmd* cmd;
//    cmd = app_easy_gap_undirected_advertise_get_active();
//    
//    // Add manufacturer data to initial advertising or scan response data, if there is enough space
//    app_add_ad_struct(cmd, &mnf_data, sizeof(struct mnf_specific_data_ad_structure), 1);

//    app_easy_gap_undirected_advertise_start();
//}

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
    #define LED_PORT                GPIO_PORT_0
    #define LED_PIN                 GPIO_PIN_9
GPIO_ConfigurePin(LED_PORT, LED_PIN, OUTPUT, PID_GPIO, false);
GPIO_set_pad_latch_en(true);
				GPIO_SetActive(LED_PORT, LED_PIN);
	      GPIO_SetInactive(LED_PORT, LED_PIN);

}