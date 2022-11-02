#ifndef __SS_main_h__
#define __SS_main_h__

#include "ss_global.h"

extern bool ssm_main_BLE_RDY;
extern uint8_t ssm_main_state;

e_FunctionReturnState ss_main(void);
e_FunctionReturnState ss_main_init(void);
e_FunctionReturnState ssm_main_ADC_prepare(void);
e_FunctionReturnState ssm_main_BLE_prepare(void);



#endif
