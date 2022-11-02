#include "i2c.h"
#include "i2c_eeprom.h"
#include "sx1502.h"
#include "gpio.h"
#include "ss_i2c.h"
#include "SS_sys.h"
#include "ss_global.h"

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

//======================================================================================


static uint8_t   inpData;                                                      // ???? ?????? ?????? ?? ?????? SX1502 
static uint8_t   outData;                                                      // ????? ?????? ?????? ??? ?????? SX1502
static uint8_t   buttonsState;

static uint32_t systick_last_LED;//TODO to init
static uint32_t systick_last_SCAN;
static uint32_t ledsTime;     // ������ ����������� ()
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
static i2c_error_code err;

#define board_calibration_control 0x10


i2c_error_code sx1502_init(void)
{
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
  return err;  
}
//===================================================================================================================

void ssi2c_init(void)
{
systick_last_LED=systick_time;
systick_last_SCAN=systick_time;    
ledsTime = systick_time;	
	
  // Initialize I2C
  i2c_eeprom_configure(&i2c_cfg, &sx1502_cfg);
  i2c_eeprom_initialize();
  
  // Set pad functionality
  ssi2c_set_pad_functions();
  // Enable the pads
	
//	err = i2c_eeprom_write_byte(1, 0x0E);
//  err = i2c_eeprom_write_byte(2, 0x00);
//  err = i2c_eeprom_write_byte(3, 0x00);

	if(sx1502_init() == I2C_NO_ERROR)
  {
    i2c_eeprom_read_byte(SX1502_REGDATA_ADDR, &outData);
  }
	
	//i2c_eeprom_write_byte(SX1502_REGDATA_ADDR, 0x7F|board_calibration_control ); //IO4=1 1 - NO microphone
	i2c_eeprom_write_byte(SX1502_REGDATA_ADDR, 0xBF&(~board_calibration_control) ); //IO4: 0 - NC calibration
  
//  i2cBuff[0] = 0x7F;
//  i2cBuff[1] = 0xBF;
//  i2cBuff[2] = 0xDF;
//  i2cBuff[3] = 0x5F;
//  i2cBuff[4] = 0x9F;
//  i2cBuff[5] = 0x1F;
//  cnt = 0;
	

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


                           // ????? ?????? ?????????? ? ?? ? ??????? ?????????


//uint16_t  vddVoltage;                                                   // ����� ������ �������� ���������� ������� 



btn_t         sw1;
btn_t         sw3;
rgbLedTask_t  rgbLedTask;

/*************************************************************************************************
 * ????? ????????? 
 * ***********************************************************************************************/
volatile struct {
  uint16_t VLTG_READ_EN       : 1;                                      // 1 = ���� ������ ���������� �������
	uint16_t PWR_ENABLED			  : 1;				                              // 1 = DC/DC �������, 0 = ��������
  uint16_t ALARM_STATE        : 1;                                      // 1 = ���������� ��������� ���������
  uint16_t ERROR_STATE        : 1;
 }Status;


/**********************************************************************************************
*
* ????????? ?????? ?????? ?????? ???????????
*
**********************************************************************************************/
//void SysTick_Handler(void)
//{
//	sysTime++;
//  scanTime++;
//  if(sysTime % 200){Status.VLTG_READ_EN = 1;}
//  if(++ledsTime == LED_SLOT_TIME)
//  {
//    ledsTime = 0;
//    rgbLedTask.timeSlotMode = TSM_STRT;
//  }
//}


                                     // �������� ������ ������
/******************************************************************************************
* @brief  ��������� ��������� ������ ������ SCAN_TIME ��
 *****************************************************************************************/
static void scanInputs(void)
{
 // static uint8_t debounceBtnPwr = 0;
 // static uint8_t debounceBtnBt = 0;
  
//  if(scanTime < SCAN_TIME) return;                                      // ?????, ???? ????? ???????? ????????? ?????? ?? ?????????
//  
//  scanTime = 0;
  if(i2c_eeprom_read_byte(SX1502_REGDATA_ADDR, &inpData) == I2C_NO_ERROR)  // ????????? ?????? ?? ????????
  {
    if(inpData & BTN_SW3_IN)                                            // ���� ������ SW3 ������ (HiState)
    {
      if(sw3.debounceCount < DEB_CNT)
      {
        if(++sw3.debounceCount == DEB_CNT)                              // ���� ������ ������  � ������� ������������ ������� 
        {
          if(sw3.isPressed == false)
          {
            sw3.isPressed = true;
            sw3.pressEventFixed = true;
            sw3.pressEventTime = systick_time;
          }
        }
      }
    } 
    else                                                                // ���� ������ SW3 (B_PWR) ������
    {
      if(sw3.debounceCount > 0)
      {
        if(--sw3.debounceCount == 0)
        {
          if(sw3.isPressed)
          {
            sw3.isPressed = false;
            sw3.unpressEventFixed = true;
            sw3.pressedStateTime = systick_time - sw3.pressEventTime;
          }
        }
      }        
    }
    
    if((inpData & BTN_SW1_IN) == 0)                                     // ���� ������ SW1 ������ (LowState)
    {
      if(sw1.debounceCount < DEB_CNT)
      {
        if(++sw1.debounceCount == DEB_CNT)
        {
          if(!sw1.isPressed)
          {
            sw1.isPressed = true;
            sw1.pressEventFixed = true;
            sw1.pressEventTime = systick_time;
            sw1.numOfClicks++;          
          }
        }
      }    
    }
    else
    {
      if(sw1.debounceCount > 0)
      {
        if(--sw1.debounceCount == 0)
        {
          if(sw1.isPressed)
          {
            sw1.isPressed = false;
            sw1.unpressEventFixed = true;
            sw1.pressedStateTime = systick_time - sw1.pressEventTime;
          }
        }
      }        
    }
  }
}

/*****************************************************************************************
* @brief ��������� ������� ������� ������
 *****************************************************************************************/
static btnCmd_en decodeButtonsState(void)
{
  btnCmd_en result = BTN_CMD_NO;
  
  if(sw3.isPressed && sw3.pressEventFixed)                              // ���� sw3 ������
  {
    if(sw1.numOfClicks == 0)                                            // ���� sw1 �� ���������� �� ����
    {
      if((systick_time - sw3.pressEventTime) >= D_KL_long)                          // ���� ������ ����� 5 ������
      {
        sw3.pressEventFixed = false;                                    // ����� ������� �������� ������� ������� SW3
        return BTN_SW3_LONG;                                            // ���������� ��� �������
      }
    }
  }
  
  if(sw3.unpressEventFixed)                                             // ���� ������������� ������� "SW3 ������"
  {
    sw3.unpressEventFixed = false;
    result = BTN_CMD_NO;
    if(sw3.pressedStateTime < D_KL_long)
    {
      result = (btnCmd_en)(sw1.numOfClicks + 1);                        // ������� ��� �������                                             
    }
    sw1.numOfClicks = 0;   
  }
  
  if(sw1.unpressEventFixed)                                             // ���� ������������� ������� "SW1 ������"
  {
    sw1.unpressEventFixed = false;
    if(!sw3.isPressed)
    {
      sw1.numOfClicks = 0;  
      result = BTN_SW1_SHORT;
    }
  }   
  return result;
}

/*****************************************************************************************
* @brief ���������� ������� �������� ������� ������ PWR ON/OFF (SW3)  
 *****************************************************************************************/
static void togglePowerState(void)
{
  if(Status.PWR_ENABLED)                                                // ���� DC/DC �������
  {
    outData &= ~PWR_FIX_OUT;                                            // ����� ��� ��������� DC/DC
    outData |= SINGLE_LED_OUT;                                          // ���������� ��� ��� ���������� ���������� ����������
    Status.PWR_ENABLED = 0;                                             // ����� ���� ����������� DC/DC
  } 
  else
  {
    outData |= PWR_FIX_OUT;                                             // ���������� ��� ��������� DC/DC
    outData &= ~SINGLE_LED_OUT;                                         // ����� ��� ��� ��������� ���������� ����������
    Status.PWR_ENABLED = 1;                                             // ���������� ���� ����������� DC/DC
  }
  i2c_eeprom_write_byte(SX1502_REGDATA_ADDR, outData);                  // �������� ������ � ����
}

/*****************************************************************************************
* @brief ��������� ������� ������  
 *****************************************************************************************/
static void executeSwState(void)
{
  btnCmd_en btnCmd = decodeButtonsState();

  switch(btnCmd)                                                           
  {
    case BTN_SW3_LONG:                                                  // ���� ���� ������� ������� SW3
//      togglePowerState();                                               // �������� ��������� ������ �������� �������
      rgbLedTask.ledTimeSlot[0].isEnabled = false;
      rgbLedTask.ledTimeSlot[1].isEnabled = false;
      rgbLedTask.ledTimeSlot[2].isEnabled = false;
    break;
      
    case BTN_SW3_SHORT:                                                 // ���� ���� �������� ������� SW3
      rgbLedTask.ledTimeSlot[0].color = CL_RED;
      rgbLedTask.ledTimeSlot[0].pulseWidthMs = D_pulseWidthMs;
      rgbLedTask.ledTimeSlot[0].isEnabled = true;
    break;
    
    case BTN_SW1_SHORT:                                                 // ���� ���� ������� SW1 ��� SW3
      rgbLedTask.ledTimeSlot[1].color = CL_RED|CL_GREEN;
      rgbLedTask.ledTimeSlot[1].pulseWidthMs = D_pulseWidthMs;
      rgbLedTask.ledTimeSlot[1].isEnabled = true;      
    break;
    
    case BTN_SW1_ONE_CLICK:                                             // ���� ���� ���� ������� SW1 ��� ������� SW3
      rgbLedTask.ledTimeSlot[1].color = CL_GREEN;
      rgbLedTask.ledTimeSlot[1].pulseWidthMs = D_pulseWidthMs;
      rgbLedTask.ledTimeSlot[1].isEnabled = true;
    break;
      
    case BTN_SW1_DBL_CLICK:                                             // ���� ���� ��� ������� SW1 ��� ������� SW3
      rgbLedTask.ledTimeSlot[2].color = CL_BLUE;
      rgbLedTask.ledTimeSlot[2].pulseWidthMs = D_pulseWidthMs;
      rgbLedTask.ledTimeSlot[2].isEnabled = true;
    break;
    
    case BTN_SW1_THREE_CLICK:                                           // ���� ���� ��� ������� SW1 ��� ������� SW3

    break;
      
    default: break;
  } 
}

///*****************************************************************************************
//* @brief �������� ���������� �������  
// *****************************************************************************************/
//static void updateVddValue(void)
//{
//  uint16_t adc_sample;
//  
//  if(Status.VLTG_READ_EN)
//  {
//    Status.VLTG_READ_EN = 0;
//    
//    adc_sample = adc_get_sample();
//    adc_sample = adc_correct_sample(adc_sample);
//    // Divider depends on oversampling setting
//    vddVoltage = ((3600 * adc_sample) / 2047);
//  }
//}

/*****************************************************************************************
* @brief ���������� RGB �����������  
 *****************************************************************************************/
static void rgbLedServer(void)
{
  static uint16_t itemIndex = 0;
	uint32_t l_ledsTime;
	
	l_ledsTime=systick_time-ledsTime;
	if (l_ledsTime >= LED_SLOT_TIME)
  { 
    ledsTime = systick_time;
    rgbLedTask.timeSlotMode = TSM_STRT;
	}
  
  switch(rgbLedTask.timeSlotMode)
  {
    case TSM_STRT:
      
      if(rgbLedTask.ledTimeSlot[itemIndex].isEnabled)                   // ���� ��������� ���� �������
      {
        outData |= RED_LED_OUT | GREEN_LED_OUT | BLUE_LED_OUT;          // ������������� ������ ��� RGB ����������
        outData &= ~rgbLedTask.ledTimeSlot[itemIndex].color;            // ����� ��� ��� ��������� ������� ����������
        i2c_eeprom_write_byte(SX1502_REGDATA_ADDR, outData);            // �������� ������ � ����
//        outData |= RED_LED_OUT | GREEN_LED_OUT | BLUE_LED_OUT;          // ������������� ������ ��� RGB ����������
//        i2c_eeprom_write_byte(SX1502_REGDATA_ADDR, outData);            // �������� ������ � ����
        rgbLedTask.timeSlotMode = TSM_BUSY;                             // ����������� ����� ������ �����
      } 
      else 
      {
        rgbLedTask.timeSlotMode = TSM_IDLE;                             // ����������� ����� ������ �����
        if(++itemIndex == LEDS_NUM){itemIndex = 0;}                     // ���� ����� ���� ����������� ����������, ������� � ������
      }
    break;
    
    case TSM_BUSY:
      
    if(rgbLedTask.ledTimeSlot[itemIndex].pulseWidthMs >= l_ledsTime)      // ���� ����� ����������� ��������� ���������� �������
      {
        outData |= RED_LED_OUT | GREEN_LED_OUT | BLUE_LED_OUT;          // ������������� ������ ��� RGB ����������
        i2c_eeprom_write_byte(SX1502_REGDATA_ADDR, outData);            // �������� ������ � ����
        rgbLedTask.timeSlotMode = TSM_IDLE;;                            // ����������� ����� ������ �����
       if(++itemIndex == LEDS_NUM){itemIndex = 0;}                      // ���� ����� ���� ����������� ����������, ������� � ������        
      }
      
    break;
      
    default: break;
  }
}


/******************************************************************************************
 * @brief 
 *****************************************************************************************/


//#define Period_LED 200000//uS

void sx_main (void)
{
  
  //SysTick_Config(SystemCoreClock / 1000);
  
//  if(sx1502_init() == I2C_NO_ERROR)
//  {
//    i2c_eeprom_read_byte(SX1502_REGDATA_ADDR, &outData);
//  } else while(1);
   
//  while(1)
	if ((systick_time-systick_last_LED)>(LED_PULSE_TIME))
	{ 
		systick_last_LED+=(LED_PULSE_TIME);
	  rgbLedTask.timeSlotMode = TSM_STRT;
	}	
		if ((systick_time-systick_last_SCAN)>(SCAN_TIME))
	{ 
		systick_last_SCAN+=(SCAN_TIME);
	  scanInputs();
	}	

  {
  
    executeSwState();
//    updateVddValue();
    rgbLedServer();    
  }
}

