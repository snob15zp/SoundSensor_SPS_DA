#ifndef __ss_i2c_h__
#define __ss_i2c_h__

#include "ss_global.h"
#include "sx1502.h"
#include "i2c_eeprom.h"

#ifdef D_sx_takt_call 
#define D_sx_takt_period D_PeriodSlowMath_us
#else
#define D_sx_takt_period  D_SYSTICK_PERIOD_US
#endif

#define    maxLEDS_NUM               2                                     // количество светодиодов в последовательности
//#define    LED_PULSE_TIME         (3000000/D_sx_takt_period)     //us                             // время цикла обработки всех светодиодов
#define    LED_SLOT_TIME          (1000000/D_sx_takt_period) // время работы одного светодиода 
#define DEB_CNT         ((uint8_t)2)                                      // 믫鸥񲢮 ౮㦰猪嬿 𨪱ᷨ衱ﲲ-鿍
#define SCAN_TIME       (125000/D_sx_takt_period)// 
#define D_pulseWidthMs (500000/D_sx_takt_period)
#define D_KL_long (5000000/D_sx_takt_period)



typedef struct
{
  uint32_t  pressEventTime;                                             // время фиксации нажатия кнопки
  uint32_t  pressedStateTime;                                           // время удержания кнопки в нажатом состоянии
  uint16_t  debounceCount;                                              // счетчик циклов устранения дребезга
  uint16_t  numOfClicks;                                                // счетчик количества нажатий
  bool      isPressed;                                                  // состояние кнопки
  bool      pressEventFixed;                                            // флаг события нажатия кнопки
  bool      unpressEventFixed;                                          // флаг события отпускания кнопки
}btn_t;

typedef enum
{
  BTN_CMD_NO,
  BTN_SW3_SHORT,                                                        // sw3 отжата и sw1.numOfClicks = 0
  BTN_SW1_ONE_CLICK,                                                    // sw3 отжата и sw1.numOfClicks = 1
  BTN_SW1_DBL_CLICK,                                                    // sw3 отжата и sw1.numOfClicks = 2
  BTN_SW1_THREE_CLICK,                                                  // sw3 отжата и sw1.numOfClicks = 3
  /* далее можно продолжать список команд с количеством нажатий sw1 (порядок важен для алгоритма) */  
  BTN_SW3_LONG,                                                         // sw3 нажата более 5 секунд
  BTN_SW1_SHORT                                                         // нажималась только sw1
} btnCmd_en;

typedef enum
{
  CL_RED =  RED_LED_OUT,
  CL_GREEN =  GREEN_LED_OUT,
  CL_BLUE =   BLUE_LED_OUT,
	CL_WHITE = RED_LED_OUT|GREEN_LED_OUT|BLUE_LED_OUT,
	CL_RG =RED_LED_OUT|GREEN_LED_OUT,
	CL_GB=GREEN_LED_OUT|BLUE_LED_OUT,
	CL_BR =RED_LED_OUT|BLUE_LED_OUT,
	CL_LD1   = SINGLE_LED_OUT,
	CL_MASK  =SX_D_LED_MASK
}color_en;


typedef enum
{
  TSM_IDLE,
  TSM_STRT,
  TSM_BUSY
}timeSlotMode_en;

typedef struct
{
  bool      isEnabled;                  // признак активности слота индикации
  uint16_t  pulseWidthMs;               // длительность включенного состояния светодиода
  color_en  color;                      // цвет светодиода в слоте  
}ledTimeSlot_t;

typedef struct
{
  timeSlotMode_en timeSlotMode;
	uint32_t ledsTime;
	uint16_t itemIndex;
	uint8_t LEDS_NUM;
	uint8_t colormask;
  ledTimeSlot_t   ledTimeSlot[maxLEDS_NUM];
}rgbLedTask_t;

extern ledTimeSlot_t const LED_ALARM_Empty;
extern ledTimeSlot_t const LED_ALARM_LiveSPL;
extern ledTimeSlot_t const LED_ALARM_Operatingstate;
extern ledTimeSlot_t const LED_ALARM_Overloadindicator;
extern ledTimeSlot_t const LED_ALARM_LAeqM3dB;
extern ledTimeSlot_t const LED_ALARM_LAeq;
extern ledTimeSlot_t const LED_ALARM_hearing;
extern ledTimeSlot_t const LED_ALARM_BLE;
extern ledTimeSlot_t const LED_ALARM_CalibrationLong;
extern ledTimeSlot_t const LED_ALARM_CalibrationShort;

extern rgbLedTask_t  rgbLedTaskD1;
extern rgbLedTask_t  rgbLedTaskLD1;
// simple test

void ss_i2c_test (void);
//complex test
void ssi2c_init(void);
i2c_error_code sx1502_init(void);//for inner
void sx_main(void);
void SX_PowerOff();
#endif
