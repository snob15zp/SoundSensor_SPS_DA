#include <stdio.h>
//#include "arch_system.h"
//include "user_periph_setup.h"
#include "gpio.h"
#include "SS_sys.h"


#include "my_timer.h"

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


//int check_timeOVER(uint16_t *time_stop, uint16_t delta)
//{
//uint16_t time_tek;	
//	
//	time_tek = get_systick();
//	if ( time_tek < *time_stop )
//	{
//		if ( ((0xffff - *time_stop) + time_tek) < delta )
//			return 0;
//	}
//	else
//	{
//		if ( time_tek - *time_stop < delta )
//			return 0;	
//	}
//	*time_stop = time_tek;
//	return 1;	
//}


void delay_ms(uint16_t time_delta)
{
uint16_t	time_tek;	
uint16_t	flaf_out = 1;	
	delta_time_stop = systick_time;
	do{
		time_tek = systick_time; 		
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








