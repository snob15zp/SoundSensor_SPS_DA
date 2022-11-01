#ifndef __ss_i2c_h__
#define __ss_i2c_h__

#include "i2c_eeprom.h"

#define    LEDS_NUM               3                                     // количество светодиодов в последовательности
#define    LED_PULSE_TIME         (3000000/SYSTICK_PERIOD_US)     //us                             // время цикла обработки всех светодиодов
#define    LED_SLOT_TIME          ((uint16_t)(LED_PULSE_TIME / LEDS_NUM)) // время работы одного светодиода 
#define DEB_CNT         ((uint8_t)3)                                      // 믫鸥񲢮 ౮㦰猪嬿 𨪱ᷨ衱ﲲ-鿍
#define SCAN_TIME       (10000/SYSTICK_PERIOD_US)//us 
#define D_pulseWidthMs (500000/SYSTICK_PERIOD_US)
#define D_KL_long (5000000/SYSTICK_PERIOD_US)

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
  CL_RED =    0x80,
  CL_GREEN =  0x40,
  CL_BLUE =   0x20
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
  ledTimeSlot_t   ledTimeSlot[LEDS_NUM];
}rgbLedTask_t;





// simple test

void ss_i2c_test (void);
//complex test
void ssi2c_init(void);
i2c_error_code sx1502_init(void);//for inner
void sx_main(void);
#endif
