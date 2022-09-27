#include "i2c.h"
#include "i2c_eeprom.h"
#include "sx1502.h"
#include "gpio.h"
#include "ss_i2c.h"

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


static uint8_t i2cBuff[32];
static uint8_t* ptrBuff;
static uint8_t cnt;
static uint32_t sysTime;
static i2c_error_code err;


void ssi2c_init(void)
{
    
  // Initialize I2C
  i2c_eeprom_configure(&i2c_cfg, &sx1502_cfg);
  i2c_eeprom_initialize();
  
  // Set pad functionality
  ssi2c_set_pad_functions();
  // Enable the pads
	
	err = i2c_eeprom_write_byte(1, 0x0E);
  err = i2c_eeprom_write_byte(2, 0x00);
  err = i2c_eeprom_write_byte(3, 0x00);
	
	i2c_eeprom_write_byte(SX1502_REGDATA_ADDR, 0x7F); //0 - NC calibration
  
  i2cBuff[0] = 0x7F;
  i2cBuff[1] = 0xBF;
  i2cBuff[2] = 0xDF;
  i2cBuff[3] = 0x5F;
  i2cBuff[4] = 0x9F;
  i2cBuff[5] = 0x1F;
  cnt = 0;

}




void ss_i2c_test (void)
{
  //system_init();
  //SysTick_Config(SystemCoreClock / 1000);
  


      i2c_eeprom_write_byte(SX1502_REGDATA_ADDR, i2cBuff[cnt]);
      cnt++;
      if(cnt == 6){cnt = 0;}

}

//=======================================================================================================================



static uint32_t __attribute__((aligned(8))) sysTime;                           // ????? ?????? ?????????? ? ?? ? ??????? ?????????
static uint32_t __attribute__((aligned(8))) scanTime;                          // ?????? ???????????? ??????


static uint8_t   inpData;                                                      // ???? ?????? ?????? ?? ?????? SX1502 
static uint8_t   outData;                                                      // ????? ?????? ?????? ??? ?????? SX1502
static uint8_t   buttonsState;


/*************************************************************************************************
 * ????? ????????? 
 * ***********************************************************************************************/
volatile struct {
	uint16_t BTN_BT_PUSH		    : 1;				                              // 1 = ?????? (SW1) ??????
  uint16_t BTN_BT_UNPUSH      : 1;				                              // 1 = ?????? (SW1) ??????
	uint16_t BTN_PWR_PUSH		    : 1;				                              // 1 = ?????? B_PWR (SW3) ?????? 
	uint16_t BTN_PWR_UNPUSH		  : 1;				                              // 1 = ?????? B_PWR (SW3) ??????
	uint16_t PWR_ENABLED			  : 1;				                              // 1 = DC/DC ???????, 0 = ????????
}Status;


/**********************************************************************************************
*
* ????????? ?????? ?????? ?????? ???????????
*
**********************************************************************************************/



#define DEB_CNT         (uint8_t)3                                      // ?????????? ???????? ??? ???????? ?????????
#define SCAN_TIME       (uint8_t)10                                     // ???????? ?????? ??????
/******************************************************************************************
* @brief   SCAN_TIME 
 *****************************************************************************************/
static void scanInputs(void)
{
  static uint8_t debounceBtnPwr = 0;
  static uint8_t debounceBtnBt = 0;
  
  if(scanTime < SCAN_TIME) return;                                      // ?????, ???? ????? ???????? ????????? ?????? ?? ?????????
  
  scanTime = 0;
  if(i2c_eeprom_read_byte(SX1502_REGDATA_ADDR, &inpData) == I2C_NO_ERROR)  // ????????? ?????? ?? ????????
  {
    if(inpData & BTN_PWR_IN)                                            // ???? ?????? SW3 (B_PWR) ?????? (HiState)
    {
      if(debounceBtnPwr < DEB_CNT)
      {
        if(++debounceBtnPwr == DEB_CNT)                                 // ???? ?????? ??????  ? ??????? ???????????? ??????? 
        {
          buttonsState |= 0x01;                                         // ?????????? ??? ????????? ??????
          Status.BTN_PWR_PUSH = 1;                                      // ?????????? ???? ??????? ??????? ??????
        }
      }
    } 
    else                                                                // ???? ?????? SW3 (B_PWR) ??????
    {
      if(debounceBtnPwr > 0)
      {
        if(--debounceBtnPwr == 0)
        {
          buttonsState &= ~0x01;
//          Status.BTN_PWR_UNPUSH = 1;
        }
      }        
    }
    
    if((inpData & BTN_BT_IN) == 0)                                      // ???? ?????? SW1 ?????? (LowState)
    {
      if(debounceBtnBt < DEB_CNT)
      {
        if(++debounceBtnBt == DEB_CNT)
        {
          buttonsState |= 0x02;
          Status.BTN_BT_PUSH = 1;
        }
      }    
    }
    else
    {
      if(debounceBtnBt > 0)
      {
        if(--debounceBtnBt == 0)
        {
          buttonsState &= ~0x02;
//          Status.BTN_BT_UNPUSH = 1;
        }
      }        
    }
  }
}

/*****************************************************************************************
* @brief  PWR ON/OFF (SW3)  
 *****************************************************************************************/
static void updatePowerState(void)
{
  if(Status.BTN_PWR_PUSH)                                               // ???? ????????????? ??????? ?????? SW3
  { 
    if(Status.PWR_ENABLED)                                              // ???? DC/DC ???????
    {
      outData &= ~PWR_FIX_OUT;                                          // ????? ??? ????????? DC/DC
      outData |= SINGLE_LED_OUT;                                        // ?????????? ??? ??? ?????????? ?????????? ??????????
      Status.PWR_ENABLED = 0;                                           // ????? ???? ??????????? DC/DC
    } 
    else 
    {
      outData |= PWR_FIX_OUT;                                           // ?????????? ??? ????????? DC/DC
      outData &= ~SINGLE_LED_OUT;                                       // ????? ??? ??? ????????? ?????????? ??????????
      Status.PWR_ENABLED = 1;                                           // ?????????? ???? ??????????? DC/DC
    }
    i2c_eeprom_write_byte(SX1502_REGDATA_ADDR, outData);                // ???????? ?????? ? ????
    Status.BTN_PWR_PUSH = 0;                                            // ????? ???? ???????? ??????? ??????
  }
}

/*****************************************************************************************
* @brief  SW1  
 *****************************************************************************************/
static void executeSwState(void)
{
  static uint8_t i = 0;
  
  if(Status.BTN_BT_PUSH)                                                // ???? ????????????? ??????? ?????? SW1
  {
    if(++i == 4) {i = 0;}
    outData |= RED_LED_OUT | GREEN_LED_OUT | BLUE_LED_OUT;              // ????????????? ?????? ??? RGB ??????????
    switch(i)                                                           
    {
      case 1:
        outData &= ~RED_LED_OUT;                                        // ????? ??? ??? ????????? RED_LED
      break;
      
      case 2:
        outData &= ~GREEN_LED_OUT;                                      // ????? ??? ??? ????????? GREEN_LED
      break;
      
      case 3:
        outData &= ~BLUE_LED_OUT;                                       // ????? ??? ??? ????????? GREEN_LED
      break;
      
      default: break;
    } 

    i2c_eeprom_write_byte(SX1502_REGDATA_ADDR, outData);                // ???????? ?????? ? ????
    Status.BTN_BT_PUSH = 0;                                             // ????? ???? ???????? ??????? ??????
  }
}

i2c_error_code sx1502_init(void)
{
	
	i2c_eeprom_configure(&i2c_cfg, &sx1502_cfg);
  i2c_eeprom_initialize();
  
  // Set pad functionality
  ssi2c_set_pad_functions();

	
  i2c_error_code err;
  uint8_t wByte = IO_OUTPUT << 7 | IO_OUTPUT << 6 |
                  IO_OUTPUT << 5 | IO_OUTPUT << 4 |
                  IO_OUTPUT << 3 | IO_INPUT << 2 |
                  IO_INPUT << 1 | IO_OUTPUT << 0;
  err = i2c_eeprom_write_byte(SX1502_REGDIR_ADDR, wByte);               // wByte = 0b00000110; 0 - OUTPUT, 1 - INPUT  
  if(err != I2C_NO_ERROR) return err;
  err = i2c_eeprom_write_byte(SX1502_REGPULLUP_ADDR, 0x00);
  if(err != I2C_NO_ERROR) return err;
  wByte = 1 << 2;                                                       // IO2 - PULL_DOWN
  err = i2c_eeprom_write_byte(SX1502_REGPULLDOWN_ADDR, wByte);
	
	i2c_eeprom_read_byte(SX1502_REGDATA_ADDR, &outData);
	
  return err;  
}

void sx_SysTick_Handler(void)
{
	sysTime++;
  scanTime++;
}

/******************************************************************************************
 * @brief 
 *****************************************************************************************/
void sx_main (void)
{
	sx_SysTick_Handler();
  
//  if(sx1502_init() == I2C_NO_ERROR)
//  {
//    i2c_eeprom_read_byte(SX1502_REGDATA_ADDR, &outData);
//  } else while(1);
  

    scanInputs();
    updatePowerState();
    executeSwState();    

}

