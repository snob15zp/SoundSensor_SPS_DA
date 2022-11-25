/**
 ****************************************************************************************
 *
 * @file user_profiles_config.h
 *
 * @brief Configuration file for the profiles used in the application. 
 *
 * Copyright (C) 2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup DSPS_DEVICE_CONFIG
 * @ingroup USER_CONFIGURATION
 * @{
 ****************************************************************************************
 */

#ifndef _USER_PROFILES_CONFIG_H_
#define _USER_PROFILES_CONFIG_H_

/*
 * DEFINITIONS
 ****************************************************************************************
 */

/***************************************************************************************/
/* Used BLE profiles (used by "rwprf_config.h").                                       */
/***************************************************************************************/
#define CFG_PRF_SPSS

#define CFG_PRF_SUOTAR

/*
 * PROFILE CONFIGURATION
 ****************************************************************************************
 */

/// Add profile specific configurations

/* SUOTAR configurations
 * -----------------------------------------------------------------------------------------
 */
 
#define SUOTAR_PATCH_AREA   1   // Placed in the RetRAM when SUOTAR_PATCH_AREA is 0 and in SYSRAM when 1

/*
 ****************************************************************************************
 * BASS application profile configuration
 ****************************************************************************************
 */

// Measured in timer units (10ms)
#define APP_BASS_POLL_INTERVAL           (6000)  //  (6000*10ms)/60sec = Every 1 minute

#endif // _USER_PROFILES_CONFIG_H_

/// @} DSPS_DEVICE_CONFIG
