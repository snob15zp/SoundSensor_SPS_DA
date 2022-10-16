/**
 ****************************************************************************************
 *
 * @file periph_setup.c
 *
 * @brief Peripherals initialization functions
 *
 * Copyright (C) 2012-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "user_periph_setup.h"
#include "datasheet.h"
#include "gpio.h"
#include "uart.h"
#include "syscntl.h"
#include "spi.h"
#include "spi_531.h"

// Configuration struct for SPI
static const spi_cfg_t spi_cfg = {
    .spi_ms = SPI_MS_MODE,
    .spi_cp = SPI_CP_MODE,
    .spi_speed = SPI_SPEED_MODE,
    .spi_wsz = SPI_WSZ,
    .spi_cs = SPI_CS,
    .cs_pad.port = SPI_EN_PORT,
    .cs_pad.pin = SPI_EN_PIN,
#if defined (__DA14531__)
    .spi_capture = SPI_EDGE_CAPTURE,
#endif
};

static void set_pad_functions(void)
{
/*
    i.e. to set P0_1 as Generic purpose Output:
    GPIO_ConfigurePin(GPIO_PORT_0, GPIO_PIN_1, OUTPUT, PID_GPIO, false);
*/

#if defined (__DA14586__)
    // Disallow spontaneous DA14586 SPI Flash wake-up
    GPIO_ConfigurePin(GPIO_PORT_2, GPIO_PIN_3, OUTPUT, PID_GPIO, true);
#endif

    // Configure LED pin functionality
#ifndef __SoundSensor__
	GPIO_ConfigurePin(LED_PORT, LED_PIN, OUTPUT, PID_GPIO, false);
#endif	
}

void periph_init(void)
{
#if defined (__DA14531__)
    // In Boost mode enable the DCDC converter to supply VBAT_HIGH for the used GPIOs
    // Assumption: The connected external peripheral is powered by 3V
    syscntl_dcdc_turn_on_in_boost(SYSCNTL_DCDC_LEVEL_3V0);
#else
    // Power up peripherals' power domain
    SetBits16(PMU_CTRL_REG, PERIPH_SLEEP, 0);
    while (!(GetWord16(SYS_STAT_REG) & PER_IS_UP));
    SetBits16(CLK_16M_REG, XTAL16_BIAS_SH_ENABLE, 1);
#endif

    // Set pad functionality
    set_pad_functions();

    // Enable the pads
    GPIO_set_pad_latch_en(true);
}

void user_spi_init(uint32_t gpio_map)
{
    spi_gpio_config_t spi_conf;
    uint8_t dev_id;

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


    spi_initialize(&spi_cfg);
}
