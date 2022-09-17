/**
 ****************************************************************************************
 *
 * @file user_sps_buffer_dma.c
 *
 * @brief SPS project source file for handling DMA buffers.
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
#include "gpio.h"
#include "app_easy_timer.h" 
#include "ke_msg.h" 
#include "prf.h" 
#include "prf_utils.h"
#include "lld_evt.h"
#include "dma.h"
#include "user_sps_utils.h"
#include "dma_uart_sps.h"
#include "user_sps_buffer_dma.h"
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
 

void user_ble_to_dma_uart(void *msg) //Receive packet
{
}
 
/// @} DSPS_SERVICE_COMMON
