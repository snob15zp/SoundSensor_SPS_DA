#include "i2c.h"
#include "i2c_eeprom.h"
#include "sx1502.h"
#include "gpio.h"

/****************************************************************************************/
/* I2C configuration                                                                    */
/****************************************************************************************/
// Define I2C Pad  
  #define I2C_SDA_PIN             GPIO_PIN_8   
  #define I2C_SCL_PIN             GPIO_PIN_9



// Configuration struct for I2C
static const i2c_cfg_t i2c_cfg = {
  .clock_cfg.ss_hcnt = I2C_SS_SCL_HCNT_REG_RESET,
  .clock_cfg.ss_lcnt = I2C_SS_SCL_LCNT_REG_RESET,
  .clock_cfg.fs_hcnt = I2C_FS_SCL_HCNT_REG_RESET,
  .clock_cfg.fs_lcnt = I2C_FS_SCL_LCNT_REG_RESET,
  .restart_en = I2C_RESTART_ENABLE,
  .speed = I2C_SPEED_STANDARD,
  .mode = I2C_MODE_MASTER,
  .addr_mode = I2C_ADDRESSING_7B,
  .address = SX1502_ADDRESS,
  .tx_fifo_level = 1,
  .rx_fifo_level = 1,
};

// Configuration struct for SX1502
static const i2c_eeprom_cfg_t sx1502_cfg = {
  .size = 256,
  .page_size = 128,
  .address_size = I2C_1BYTE_ADDR
};

/*****************************************************************************************
*
*****************************************************************************************/
static void ssi2c_set_pad_functions(void)
{
  // Configure I2C pin functionality
  GPIO_ConfigurePin(GPIO_PORT_0, I2C_SCL_PIN, INPUT, PID_I2C_SCL, false);
  GPIO_ConfigurePin(GPIO_PORT_0, I2C_SDA_PIN, INPUT, PID_I2C_SDA, false);
  // Configure SPI pins
/*  
  GPIO_ConfigurePin(GPIO_PORT_0, SPI_CS0_PIN, OUTPUT, PID_SPI_EN,  true);
  GPIO_ConfigurePin(GPIO_PORT_0, SPI_CS1_PIN, OUTPUT, PID_SPI_EN,  true);
  GPIO_ConfigurePin(GPIO_PORT_0, SPI_CLK_PIN, OUTPUT, PID_SPI_CLK, false);
  GPIO_ConfigurePin(GPIO_PORT_0, SPI_MO_PIN,  OUTPUT, PID_SPI_DO,  false);
  GPIO_ConfigurePin(GPIO_PORT_0, SPI_MI_PIN,  INPUT,  PID_SPI_DI,  false);
*/
}

/*****************************************************************************************
*
*****************************************************************************************/
void ssi2c_init(void)
{
    
  // Initialize I2C
  i2c_eeprom_configure(&i2c_cfg, &sx1502_cfg);
  i2c_eeprom_initialize();
  
  // Set pad functionality
  ssi2c_set_pad_functions();
  // Enable the pads

}
