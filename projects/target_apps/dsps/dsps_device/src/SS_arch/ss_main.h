#ifndef __SS_main_h__
#define __SS_main_h__

#include <stdint.h>
#include <stdbool.h>

extern bool ssm_main_BLE_RDY;
extern uint8_t ssm_main_state;

bool ss_main(void);
void ss_main_init(void);
void ssm_main_ADC_prepare(void);
void ssm_main_BLE_prepare(void);



#endif
