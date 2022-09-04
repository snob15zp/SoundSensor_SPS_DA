/**
 ****************************************************************************************
 *
 * @file ble_flow_control.h
 *
 * @brief BLE Flow Control implementation - header file
 *
 * Copyright (c) 2012-2019 Dialog Semiconductor. All rights reserved.
 *
 * This software ("Software") is owned by Dialog Semiconductor.
 *
 * By using this Software you agree that Dialog Semiconductor retains all
 * intellectual property and proprietary rights in and to this Software and any
 * use, reproduction, disclosure or distribution of the Software without express
 * written permission or a license agreement from Dialog Semiconductor is
 * strictly prohibited. This Software is solely for use on or in conjunction
 * with Dialog Semiconductor products.
 *
 * EXCEPT AS OTHERWISE PROVIDED IN A LICENSE AGREEMENT BETWEEN THE PARTIES, THE
 * SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. EXCEPT AS OTHERWISE
 * PROVIDED IN A LICENSE AGREEMENT BETWEEN THE PARTIES, IN NO EVENT SHALL
 * DIALOG SEMICONDUCTOR BE LIABLE FOR ANY DIRECT, SPECIAL, INDIRECT, INCIDENTAL,
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
 * USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THE SOFTWARE.
 *
 ****************************************************************************************
 */
 
#ifndef _BLE_FLOW_CONTROL_H_
#define _BLE_FLOW_CONTROL_H_
 
#if (USE_BLE_FLOW_CONTROL)

typedef struct ble_flow_control_params
{
  uint8_t enable;
  uint8_t block_rx_threshold_high;
  uint8_t block_rx_threshold_low;
  uint8_t reduce_bw_threshold_high;
  uint8_t reduce_bw_threshold_low;
  //uint8_t reduce_bw_packets_allowed_per_event;
  uint8_t heap_percentage_allocated_current;
  uint8_t heap_percentage_allocated_max;
} ble_flow_control_params_t;

typedef enum {
    /// BLE flow cotnrol disabled. Normal operation
    FC_DISABLED = 0, 
    /// BLE bandwidth reduced. Only BLE_FC_REDUCED_RX_BW_PACKETS_PER_EVENT packets per connection event are allowed
    FC_REDUCE_BW,
    /// BLE Rx flow blocked. All Rx packets are rejected.
    FC_BLOCK_RX
} ble_flow_control_state_t;

/**
 ****************************************************************************************
 * @brief Manually set the state of BLE flow control
 *
 * @param state The state of the flow control. Set to FC_DISABLED for normal BLE 
 *              operation. Set to FC_REDUCE_BW to allow up to 
 *              BLE_FC_REDUCED_RX_BW_PACKETS_PER_EVENT packets per event.
 *              Set to FC_BLOCK_RX to completely block incoming Rx traffic.
 ****************************************************************************************
 */
void ble_fc_set_state(ble_flow_control_state_t state);

#ifndef BLE_FC_DISABLE_HEAP_MONITORING

/**
 ****************************************************************************************
 * @brief Get the state of BLE flow control
 *
 * @return The current BLE flow control state. This state is automatically changed when
 *         heap monitoring has been enabled
 ****************************************************************************************
 */
ble_flow_control_state_t ble_fc_get_state(void);

/**
 ****************************************************************************************
 * @brief Enable heap monitoring
 *
 * When heap monitoring is enabled BLE_FC_BLOCK_RX_THRESHOLD_HIGH, 
 * BLE_FC_BLOCK_RX_THRESHOLD_LOW, BLE_FC_REDUCE_BW_THRESHOLD_HIGH and
 * BLE_FC_REDUCE_BW_THRESHOLD_LOW thresholds are used to automatically change the BLE 
 * flow control state. BLE flow control enter a state when heap utilization exceeds the 
 * corresponding high threshold. BLE flow control returns to the previous state when 
 * heap utilization falls below the corresponding low threshold.
 ****************************************************************************************
 */
void ble_fc_enable_heap_monitoring(void);

/**
 ****************************************************************************************
 * @brief Disable heap monitoring
 
 * BLE flow control are not changed automatically. ble_fc_set_state() must be called from
 * the application to control the flow.
 ****************************************************************************************
 */
void ble_fc_disable_heap_monitoring(void);

uint8_t ble_fc_heap_monitoring_is_enabled(void);

#endif //  BLE_FC_DISABLE_HEAP_MONITORING


 
void ble_flow_control_apply_rx_buffers_limit(void);
void ble_flow_control_revert_rx_buffers_limit(void);
 
 
#endif  //(USE_BLE_FLOW_CONTROL)

#endif  //_BLE_FLOW_CONTROL_H_

