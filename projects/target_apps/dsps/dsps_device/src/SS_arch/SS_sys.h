#ifndef __SS_sys_h__
#define __SS_sys_h__

#include "ss_global.h"
#include "spi_531.h"
#include "user_config_storage.h"


#ifdef __DEVKIT_EXT__
    #define LED_PORT                GPIO_PORT_0
    #define LED_PIN                 GPIO_PIN_9
#endif

typedef enum {EAM_IDLE, EAM_ADCLIVE, EAM_ADC_WORK,EAM_ADC_sin, EAM_ADCsystick} E_ADC_MODE_t;

extern E_ADC_MODE_t	SS_ADC_MODE;
extern bool SSS_CalibrationMode;
extern volatile uint32_t systick_time;
uint16_t* get_vars(void);
uint64_t get_systime(void);

void LEDinit (void);
void test_hnd_init(void);
void ss_spi_init(uint32_t gpio_map,const spi_cfg_t *spi_cfg);
void SS_spi_switchoff_pins(uint32_t gpio_map);

void SSS_ReadFromVar(void);
void SSS_WriteToVar(void);
void SSS_WriteTableToVar(const user_config_elem_t* t, uint8_t *var_index, uint8_t ms);

#endif

