#ifndef __SS_main_h__
#define __SS_main_h__

#include "ss_global.h"

extern bool ssm_main_BLE_RDY;
extern uint8_t ssm_main_state;

e_FunctionReturnState ss_main(void);
e_FunctionReturnState ss_main_BLE(void);
e_FunctionReturnState ss_main_init(void);

e_FunctionReturnState SSM_ADCStop(void);
e_FunctionReturnState SSM_ADCStart(void);

e_FunctionReturnState SSM_BLEStop();
e_FunctionReturnState SSM_BLEStart();
void DisplayAlarm(void);



#endif
