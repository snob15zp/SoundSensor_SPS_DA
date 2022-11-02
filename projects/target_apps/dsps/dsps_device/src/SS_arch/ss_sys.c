#include "ss_global.h"
#include "SS_sys.h"
#include "systick.h"
#include "gpio.h"
#include "MathFast.h"
#include "ADC_flash.h"
#include "SPI_ADC.h"
#include "version.h"

#include "user_periph_setup.h"
#include "user_config_storage.h"


#define SYSTICK_EXCEPTION   1           // generate systick exceptions

//bool ADC_EMUL_mode;

E_ADC_MODE_t	SS_ADC_MODE;
static uint16_t vars[VARS_CNT] = {0};
uint8_t verify_value_cb (uint8_t* param);

static const user_config_elem_t conf_table[] = {
    {0x1000, sizeof(uint16_t) * VARS_CNT, sizeof(uint16_t) * VARS_CNT, vars, verify_value_cb, false},
};

volatile uint32_t systick_time;

//#ifndef __SoundSensor__
//volatile static uint8_t i;
//#endif


static uint8_t verify_value_cb (uint8_t* param)
{
    return 0;
}


uint64_t get_systime(void) 
{
    return systick_time;
}

uint16_t* get_vars(void)
{
    return vars;
}


void systick_irq()
{
    systick_time++;	  
#ifdef __DEVKIT_EXT__
	  GPIO_SetActive(LED_PORT, LED_PIN); 
#endif	
	  if (SS_ADC_MODE==EAM_ADCsystick)
			//test_MF_main_ADCEmul();
		ADC_IRQ();
#ifdef __DEVKIT_EXT__	
	  GPIO_SetInactive(LED_PORT, LED_PIN);
#endif	

//#ifndef
//    if (i == 0)
//    {
//        GPIO_SetActive(LED_PORT, LED_PIN);
//        i = 1;
//    }
//    else
//    {
//        GPIO_SetInactive(LED_PORT, LED_PIN);
//        i = 0;
//    }
//#endif		
	
}


void test_hnd_init(void)
{
	  systick_stop();
	  NVIC_SetPriority(SysTick_IRQn, 2);
    systick_register_callback(systick_irq);
    systick_start(SYSTICK_PERIOD_US, SYSTICK_EXCEPTION);
    
    uint8_t version_len = strlen(SDK_VERSION);
    char version[16] = {0};
    strncpy(version, SDK_VERSION, sizeof(version));

    user_conf_storage_init((user_config_elem_t *)conf_table, sizeof(conf_table)/sizeof(user_config_elem_t), version, &version_len);
    vars[0] += 1;
    user_config_save((user_config_elem_t *)conf_table, sizeof(conf_table)/sizeof(user_config_elem_t), version, &version_len);
}

void LEDinit (void)
{
#ifdef __DEVKIT_EXT__	
//GPIO_ConfigurePin(LED_PORT, LED_PIN, OUTPUT, PID_GPIO, false);
//GPIO_set_pad_latch_en(true);
//				GPIO_SetActive(LED_PORT, LED_PIN);
//	      GPIO_SetInactive(LED_PORT, LED_PIN);
#endif	
	
};

void SS_spi_switchoff_pins(uint32_t gpio_map)
{
    spi_gpio_config_t spi_conf;

    spi_conf.clk.port      = ((GPIO_PORT)((gpio_map & 0x000000f0) >>  4));
    spi_conf.clk.pin       = ((GPIO_PIN) ((gpio_map & 0x0000000f)));
    spi_conf.cs.port       = ((GPIO_PORT)((gpio_map & 0x0000f000) >> 12));
    spi_conf.cs.pin        = ((GPIO_PIN) ((gpio_map & 0x00000f00) >> 8));
    spi_conf.mosi.port     = ((GPIO_PORT)((gpio_map & 0x00f00000) >> 20));
    spi_conf.mosi.pin      = ((GPIO_PIN) ((gpio_map & 0x000f0000) >> 16));
    spi_conf.miso.port     = ((GPIO_PORT)((gpio_map & 0xf0000000) >> 28));
    spi_conf.miso.pin      = ((GPIO_PIN) ((gpio_map & 0x0f000000) >> 24));

    GPIO_ConfigurePin( spi_conf.cs.port, spi_conf.cs.pin, INPUT_PULLUP, PID_GPIO, true );
    GPIO_ConfigurePin( spi_conf.clk.port, spi_conf.clk.pin, INPUT_PULLDOWN, PID_GPIO, false );
    GPIO_ConfigurePin( spi_conf.mosi.port, spi_conf.mosi.pin, INPUT_PULLDOWN, PID_GPIO, false );
    GPIO_ConfigurePin( spi_conf.miso.port, spi_conf.miso.pin, INPUT_PULLDOWN, PID_GPIO, false );

}

void ss_spi_init(uint32_t gpio_map, const spi_cfg_t *spi_cfg)//
{
    spi_gpio_config_t spi_conf;

    spi_conf.clk.port      = ((GPIO_PORT)((gpio_map & 0x000000f0) >>  4));
    spi_conf.clk.pin       = ((GPIO_PIN) ((gpio_map & 0x0000000f)));
    spi_conf.cs.port       = ((GPIO_PORT)((gpio_map & 0x0000f000) >> 12));
    spi_conf.cs.pin        = ((GPIO_PIN) ((gpio_map & 0x00000f00) >> 8));
    spi_conf.mosi.port     = ((GPIO_PORT)((gpio_map & 0x00f00000) >> 20));
    spi_conf.mosi.pin      = ((GPIO_PIN) ((gpio_map & 0x000f0000) >> 16));
    spi_conf.miso.port     = ((GPIO_PORT)((gpio_map & 0xf0000000) >> 28));
    spi_conf.miso.pin      = ((GPIO_PIN) ((gpio_map & 0x0f000000) >> 24));

    GPIO_ConfigurePin( spi_conf.cs.port, spi_conf.cs.pin, OUTPUT, PID_SPI_EN, true );
    GPIO_ConfigurePin( spi_conf.clk.port, spi_conf.clk.pin, OUTPUT, PID_SPI_CLK, false );
    GPIO_ConfigurePin( spi_conf.mosi.port, spi_conf.mosi.pin, OUTPUT, PID_SPI_DO, false );
    GPIO_ConfigurePin( spi_conf.miso.port, spi_conf.miso.pin, INPUT, PID_SPI_DI, false );

    spi_initialize(spi_cfg);
}
