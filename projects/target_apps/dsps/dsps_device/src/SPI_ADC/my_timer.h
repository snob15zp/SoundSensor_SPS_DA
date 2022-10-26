//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
#ifndef __MY_TIMER_H__
#define __MY_TIMER_H__
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
#include <stdint.h>



void led_ON(void);
void led_Off(void);

int check_timeOVER(uint16_t *time_stop, uint16_t delta);


void delay_ms(uint16_t time_delta);
void delay_200us(void);
//void delay_1ms(void);
void delay_10ms(void);
void delay_100ms(void);
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
#endif //	__MY_TIMER_H__
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
