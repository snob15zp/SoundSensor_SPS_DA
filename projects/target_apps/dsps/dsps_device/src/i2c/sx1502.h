/**
 ****************************************************************************************
 *
 * @file sx1502.h
 *
 ****************************************************************************************
 */

#ifndef _SX1502_H_
#define _SX1502_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
 
#include "i2c_eeprom.h"


/*
 * DEFINES
 ****************************************************************************************
 */
 
 #define SX1502_ADDRESS                       (0x20)     // 0x20 (Addr=GND)

 #define SX1502_REGDATA_ADDR                  (0x00)     // RegData
 #define SX1502_REGDIR_ADDR                   (0x01)     // Direction
 #define SX1502_REGPULLUP_ADDR                (0x02)     // Pullups
 #define SX1502_REGPULLDOWN_ADDR              (0x03)     // Pulldowns
  
 #define SX1502_INTERRUPTMASK_ADDR            (0x05)     // Interrupt Mask
 #define SX1502_SENSEHIGH_ADDR                (0x06)     // Interrupt direction 7-4
 #define SX1502_SENSELOW_ADDR                 (0x07)     // Interrupt direction 3-0
   
 #define SX1502_INTERRUPTSOURCE_ADDR          (0x08)     // Interrupt Source 
 #define SX1502_EVENTSTATUS_ADDR              (0x09)     // Event Status of I/Os
  
 #define SX1502_REGPLDMODE_ADDR               (0x10)     // PLD Mode  
 #define SX1502_REGPLDPLDTABLE0_ADDR          (0x11)     // PLD Truth Table 
 #define SX1502_REGPLDPLDTABLE1_ADDR          (0x12)     // PLD Truth Table  
 #define SX1502_REGPLDPLDTABLE2_ADDR          (0x13)     // PLD Truth Table  
 #define SX1502_REGPLDPLDTABLE3_ADDR          (0x14)     // PLD Truth Table  
 #define SX1502_REGPLDPLDTABLE4_ADDR          (0x15)     // PLD Truth Table  
 
 #define SX1502_REGADVANCED_ADDR              (0xAB)     // Advanced Settings
 
 
 /*---------------------------------------------------------------------*/
 
 #define SX1502_CONFIG_RESET                  (0x8000)  // Reset Bit

 #define SX1502_IO7_Msk                       (0x80)  // Channel IO7
 #define SX1502_IO6_Msk                       (0x40)  // Channel IO6 
 #define SX1502_IO5_Msk                       (0x20)  // Channel IO5 
 #define SX1502_IO4_Msk                       (0x10)  // Channel IO4 
 #define SX1502_IO3_Msk                       (0x08)  // Channel IO3 
 #define SX1502_IO2_Msk                       (0x04)  // Channel IO2 
 #define SX1502_IO1_Msk                       (0x02)  // Channel IO1 
 #define SX1502_IO0_Msk                       (0x01)  // Channel IO0 
   
 #define SX1502_INPUT                         (0x01)  // 0 means input
 #define SX1502_OUTPUT                        (0x00)  // 1 means output
   
 #define SX1502_OFF                           (0x00)  // 0 means off
 #define SX1502_ON                            (0x01)  // 1 means on
      	
 #define SX1502_REG_SENS_NONE                 (0x0)   // None - Interrupt Edge Sensitivity
 #define SX1502_REG_SENS_RISING               (0x1)   // Rising - Interrupt Edge Sensitivity
 #define SX1502_REG_SENS_FALLING              (0x2)   // Falling - Interrupt Edge Sensitivity
 #define SX1502_REG_SENS_BOTH                 (0x3)   // None - Interrupt Edge Sensitivity
 
 
extern uint32_t sx1502_read_data(uint8_t *data, uint32_t address, uint32_t size);
 

#endif // _SX1502_H_
