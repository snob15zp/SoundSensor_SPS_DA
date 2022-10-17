#ifndef __ss_global_h__
#define __ss_global_h__

#include <stdint.h>
#include <stdbool.h>

#define SYSTICK_PERIOD_US   32     // period for systick timer in us, so 1000000ticks = 1second

extern uint8_t dummy_function_status;

typedef enum  {e_FRS_Not_Done, e_FRS_Done,e_FRS_DoneError} e_FunctionReturnState;


#endif
