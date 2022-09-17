/**
 ****************************************************************************************
 *
 * @file user_sps_schedule_dma.c
 *
 * @brief SPS Scheduling of DMA SPS operations
 *
 * Copyright (C) 2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information  
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup DSPS_SERVICE_COMMON
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stdlib.h>


#include "ke_mem.h"
#include "arch_console.h"
#include "user_periph_setup.h"
#include "user_sps_buffer_dma.h"
#include "gpio.h"
#include "app_easy_timer.h" 
#include "ke_msg.h" 
#include "prf.h" 
#include "prf_utils.h"
#include "lld_evt.h"
#include "dma.h"
#include "user_sps_utils.h"

#include "dma_uart_sps.h"
#include "user_sps_schedule_dma.h"

#if BLE_SPS_CLIENT
#include "sps_client.h"
#include "user_spsc.h"
#include "user_sps_host_dma.h"
#elif BLE_SPS_SERVER
#include "sps_server.h"
#include "user_spss.h"
#include "user_sps_device_dma.h"
#endif

/*
* GLOBAL VARIABLE DEFINITIONS
****************************************************************************************
*/

extern struct dma_uart_tag dma_uart;

extern bool dev_connected;
extern bool rx_cb_done;

extern uint8_t heap_usage;


static uint8_t power_on_state;
arch_main_loop_callback_ret_t user_on_ble_powered(void)
{
    uint8_t  sys_state;
    
    switch( power_on_state )
    {
        case PON_STATE_IDLE:
            sys_state=ke_state_get(TASK_APP);
            if( sys_state != APP_CONNECTED )
                break;
            if (app_default_sleep_mode == ARCH_EXT_SLEEP_ON)
            {
                    power_on_state = PON_STATE_WAIT_RTS;
            }
            else
            {
                power_on_state = PON_STATE_WAIT_RTS;
            }
        case PON_STATE_WAIT_RTS://RDD?!!!!!!!
              ;
        break;
        default:
        break;
    }   
    

    return GOTO_SLEEP;
}
/// @} DSPS_SERVICE_COMMON
