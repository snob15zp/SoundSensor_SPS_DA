/**
 ****************************************************************************************
 *
 * @file app_task.h
 *
 * @brief Header file - APPTASK.
 *
 * Copyright (C) RivieraWaves 2009-2013
 * Copyright (C) 2017-2019 Modified by Dialog Semiconductor
 *
 ****************************************************************************************
 */

#ifndef _APP_TASK_H_
#define _APP_TASK_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"             // SW configuration

#if (BLE_APP_PRESENT)

#include <stdint.h>         // Standard Integer
#include "ke_msg.h"         // Kernel Message

/*
 * DEFINES
 ****************************************************************************************
 */

/// Number of APP Task Instances
#define APP_IDX_MAX                 (1)

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// States of APP task
enum APP_STATE
{
    /// Disabled State
    APP_DISABLED,

    /// Database Initialization State
    APP_DB_INIT,

    /// Connectable state
    APP_CONNECTABLE,

    /// Connected state
    APP_CONNECTED,

    /// Number of defined states.
    APP_STATE_MAX
};

/*
 * GLOBAL VARIABLE DECLARATIONS
 ****************************************************************************************
 */

/// State handlers
extern const struct ke_state_handler app_default_handler;
/// Array of task instance states
extern ke_state_t app_state[APP_IDX_MAX];

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Process handler for the Application GAP messages.
 * @param[in] msgid   Id of the message received
 * @param[in] param   Pointer to the parameters of the message
 * @param[in] dest_id ID of the receiving task instance (probably unused)
 * @param[in] src_id  ID of the sending task instance
 * @param[in] msg_ret Result of the message handler
 * @return Returns if the message is handled by the process handler
 ****************************************************************************************
 */
enum process_event_response app_gap_process_handler (ke_msg_id_t const msgid,
                                                     void const *param,
                                                     ke_task_id_t const dest_id,
                                                     ke_task_id_t const src_id,
                                                     enum ke_msg_status_tag *msg_ret);

/// @} APPTASK

int gapc_cmp_evt_handler(ke_msg_id_t const msgid,
                         struct gapc_cmp_evt const *param,
                         ke_task_id_t const dest_id,
                         ke_task_id_t const src_id);

int gapm_device_ready_ind_handler(ke_msg_id_t const msgid,
                                  void const *param,
                                  ke_task_id_t const dest_id,
                                  ke_task_id_t const src_id);

int gapm_cmp_evt_handler(ke_msg_id_t const msgid,
                         struct gapm_cmp_evt const *param,
                         ke_task_id_t const dest_id,
                         ke_task_id_t const src_id);

int gapc_connection_req_ind_handler(ke_msg_id_t const msgid,
                                    struct gapc_connection_req_ind const *param,
                                    ke_task_id_t const dest_id,
                                    ke_task_id_t const src_id);

int gapc_disconnect_ind_handler(ke_msg_id_t const msgid,
                                struct gapc_disconnect_ind const *param,
                                ke_task_id_t const dest_id,
                                ke_task_id_t const src_id);

int gapm_adv_report_ind_handler(ke_msg_id_t msgid,
                                struct gapm_adv_report_ind *param,
                                ke_task_id_t dest_id,
                                ke_task_id_t src_id);

int gapc_get_dev_info_req_ind_handler(ke_msg_id_t msgid,
                                      struct gapc_get_dev_info_req_ind *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id);

int gapc_set_dev_info_req_ind_handler(ke_msg_id_t msgid,
                                      struct gapc_set_dev_info_req_ind *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id);

int gapm_profile_added_ind_handler(ke_msg_id_t msgid,
                                   struct gapm_profile_added_ind *param,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id);

int gapc_param_update_req_ind_handler(ke_msg_id_t msgid,
                                      struct gapc_param_update_req_ind *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id);

int gapc_le_pkt_size_ind_handler(ke_msg_id_t msgid,
                                 struct gapc_le_pkt_size_ind *param,
                                 ke_task_id_t const dest_id,
                                 ke_task_id_t const src_id);

int gattc_svc_changed_cfg_ind_handler(ke_msg_id_t msgid,
                                      struct gattc_svc_changed_cfg *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id);

int gapc_peer_features_ind_handler(ke_msg_id_t msgid,
                                   struct gapc_peer_features_ind const *param,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id);
#endif //(BLE_APP_PRESENT)

#endif // _APP_TASK_H_
