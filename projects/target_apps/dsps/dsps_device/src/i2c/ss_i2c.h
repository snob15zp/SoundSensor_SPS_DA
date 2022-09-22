#ifndef __ss_i2c_h__
#define __ss_i2c_h__

#include "i2c_eeprom.h"
// simple test
void ssi2c_init(void);
void ss_i2c_test (void);
//complex test
i2c_error_code sx1502_init(void);
int sx_main(void);
#endif
