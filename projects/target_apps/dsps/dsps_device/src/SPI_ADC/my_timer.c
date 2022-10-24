#include <stdio.h>
#include "arch_system.h"
#include "user_periph_setup.h"
#include "systick.h"
#include "gpio.h"


#include "my_timer.h"

//#define SYSTICK_PERIOD_US   16000    // period for systick timer in us, so 1000000ticks = 1second
#define SYSTICK_PERIOD_US   1000    // period for systick timer in us, so 1000000ticks = 1second
#define SYSTICK_EXCEPTION   1           // generate systick exceptions


static uint16_t systick_u16 = 0;
static uint16_t delta_time_stop = 0;

void led_ON(void)
{
//	GPIO_SetActive(LED_PORT, LED_PIN);
}

void led_Off(void)
{
//	GPIO_SetInactive(LED_PORT, LED_PIN);
}

void delay_200us(void)
{
    uint16_t i;
    for (i = 530; i != 0; --i)
		__asm__ volatile("nop");
}


void delay_10ms(void)
{
    uint16_t i;
    for (i = 50; i != 0; --i)
		delay_200us();
}

void delay_100ms(void)
{
	delay_10ms();
	delay_10ms();
	delay_10ms();
	delay_10ms();
	delay_10ms();
	delay_10ms();	
	delay_10ms();
	delay_10ms();
	delay_10ms();
	delay_10ms();
}











static void systick_isr(void)
{
	if ( systick_u16 == 65535 )
		systick_u16 = 0;		
	else
		systick_u16++;
}

void init_systick(void)
{
    systick_register_callback(systick_isr);
    // Systick will be initialized to use a reference clock frequency of 1 MHz
    systick_start(SYSTICK_PERIOD_US, SYSTICK_EXCEPTION);
}


uint16_t get_systick(void)
{
	return systick_u16;
}


int check_timeOVER(uint16_t *time_stop, uint16_t delta)
{
uint16_t time_tek;	
	
	time_tek = get_systick();
	if ( time_tek < *time_stop )
	{
		if ( ((0xffff - *time_stop) + time_tek) < delta )
			return 0;
	}
	else
	{
		if ( time_tek - *time_stop < delta )
			return 0;	
	}
	*time_stop = time_tek;
	return 1;	
}


void delay_ms(uint16_t time_delta)
{
uint16_t	time_tek;	
uint16_t	flaf_out = 1;	
	delta_time_stop = get_systick();
	do{
		time_tek = get_systick(); 		
		if ( time_tek < delta_time_stop )
		{
			if ( ((0xffff - delta_time_stop) + time_tek) >= time_delta )
				flaf_out = 0;
		}
		else
		{
			if ( time_tek - delta_time_stop >= time_delta )
				flaf_out = 0;
		}		
	} while ( flaf_out );
}








