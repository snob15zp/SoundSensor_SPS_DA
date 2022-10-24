#include "spi.h"
#include "spi_531.h"
#include "timer0_2.h"
#include "timer2.h"
#include "SPI_ADC.h"
#include "user_periph_setup.h"

#ifndef __NON_BLE_EXAMPLE__
#define def_dataRead_Size (1024*1)
#else
#define def_dataRead_Size (1024*16)
#endif

int32_t SA_dataRead_32[def_dataRead_Size/4];
int8_t dataRead_toy;
//bool SA_b_dataRead_full;
//bool SA_b_dataRead_empty;
volatile uint16_t SA_ui16_dataRead_index;
uint8_t *SA_dataRead=(uint8_t*)(&(SA_dataRead_32[0]));
/*
To configure the SPI controller in master mode, follow the steps below:
1. Set the appropriate GPIO ports in SPI clock mode (output), SPI Chip Select mode (output), SPI
Data Out mode (output), and SPI Data In mode (input).
2. Enable SPI clock by setting CLK_PER_REG[SPI_ENABLE] = 1.
3. Reset SPI FIFO by setting SPI_CTRL_REG[SPI_FIFO_RESET] = 1.
4. Set the SPI clock mode (synchronous or asynchronous with APB clock) by programming
SPI_CLOCK_REG[SPI_MASTER_CLK_MODE].
5. Set the SPI clock frequency by programming SPI_CLOCK_REG[SPI_CLK_DIV]. If SPI_CLK_DIV
is not 0x7F, SPI_CLK = module_clk/2 × (SPI_CLK_DIV + 1). If SPI_CLK_DIV = 0x7F, SPI_CLK =
module_clk.
6. Set the SPI mode (CPOL or CPHA) by programming SPI_CONFIG_REG[SPI_MODE].
7. Set the SPI controller in master mode by setting SPI_CONFIG_REG[SPI_SLAVE_EN] = 0.
8. Define the SPI word length (from 4-bit to 32-bit) by programming
SPI_CONFIG_REG[SPI_WORD_LENGTH]. SPI_WORD_LENGTH = word length - 1.
To read/write the following sequence has to be performed:
1. If a slave device is slow and does not give the data at the correct clock edge, configure the SPI
module to capture data at the next clock edge by setting
SPI_CTRL_REG[SPI_CAPTURE_AT_NEXT_EDGE] = 1. Otherwise, set
SPI_CTRL_REG[SPI_CAPTURE_AT_NEXT_EDGE] = 0.
2. Release FIFO reset by setting SPI_CTRL_REG[SPI_FIFO_RESET] = 0.
3. Enable SPI TX path by setting SPI_CTRL_REG[SPI_TX_EN] = 1.
4. Enable SPI RX path by setting SPI_CTRL_REG[SPI_RX_EN] = 1.
5. Enable the SPI chip select by programming the SPI_CS_CONFIG_REG[SPI_CS_SELECT] = 1
or 2. This option allows the master to select the slave that is connected to the GPIO that has the
function of SPI_CS0 or SPI_CS1.
6. Enable the SPI controller by setting SPI_CTRL_REG[SPI_EN] = 1.
7. Write to TX FIFO by programming SPI_FIFO_WRITE_REG[SPI_FIFO_WRITE]. Write access is
permitted only when SPI_FIFO_STATUS_REG[SPI_TX_FIFO_FULL] = 0.
8. Read from RX FIFO by programming SPI_FIFO_READ_REG[SPI_FIFO_READ]. Read is
permitted only when SPI_FIFO_STATUS_REG[SPI_RX_FIFO_EMPTY] = 0.
9. To disable the SPI chip select, set SPI_CS_CONFIG_REG[SPI_CS_SELECT] = 0 to deselect
the slave and set SPI_CTRL_REG[SPI_FIFO_RESET] = 1 to reset the SPI FIFO.


#define SPI_CTRL_REG                         (0x50001200) / Spi control register 
/// SPI registers (@ 0x50001200)
typedef struct
{
    volatile uint16_t SPI_CTRL_REGF;/// SPI Control Register (@ 0x00000000)
    uint16_t  RESERVED;
    volatile uint16_t SPI_CONFIG_REGF; /// SPI Configuration Register (@ 0x00000004)
    uint16_t  RESERVED1;
    volatile uint16_t SPI_CLOCK_REGF;/// SPI Clock Register (@ 0x00000008)
    uint16_t  RESERVED2;
    volatile uint16_t SPI_FIFO_CONFIG_REGF;/// SPI FIFO Configuration Register (@ 0x0000000C)
    uint16_t  RESERVED3;
    volatile uint16_t SPI_IRQ_MASK_REGF;/// SPI IRQ Mask Register (@ 0x00000010)
    uint16_t  RESERVED4;
    volatile uint16_t SPI_STATUS_REGF;/// SPI Status Register (@ 0x00000014)
    uint16_t  RESERVED5;
    volatile uint16_t SPI_FIFO_STATUS_REGF;/// SPI FIFO Status Register (@ 0x00000018)
    uint16_t  RESERVED6;
    volatile uint16_t SPI_FIFO_READ_REGF;/// SPI FIFO Read Register (@ 0x0000001C)
    uint16_t  RESERVED7;
    volatile uint16_t SPI_FIFO_WRITE_REGF; /// SPI FIFO Write Register (@ 0x00000020)
    uint16_t  RESERVED8;
    volatile uint16_t SPI_CS_CONFIG_REGF;/// SPI CS Configuration Register (@ 0x00000024)
    uint16_t  RESERVED9;
    volatile uint16_t SPI_FIFO_HIGH_REGF;/// SPI FIFO High Register (@ 0x00000028)
    uint16_t  RESERVED10;
    volatile uint16_t SPI_TXBUFFER_FORCE_L_REGF;/// SPI TXBUFFER_FORCE Low Register (@ 0x0000002C)
    uint16_t  RESERVED11;
    volatile uint16_t SPI_TXBUFFER_FORCE_H_REGF; /// SPI TXBUFFER_FORCE High Register (@ 0x00000030)
} spi_t;

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

// Define SPI Configuration
    #define SPI_MS_MODE             SPI_MS_MODE_MASTER
    #define SPI_CP_MODE             SPI_CP_MODE_0
    #define SPI_WSZ                 SPI_MODE_8BIT
    #define SPI_CS                  SPI_CS_0
    #define SPI_SPEED_MODE          SPI_SPEED_MODE_4MHz
    #define SPI_EDGE_CAPTURE        SPI_MASTER_EDGE_CAPTURE
		
/spi_initialize

   SetBits16(CLK_PER_REG, SPI_ENABLE, SPI_BIT_EN);


0x50001200 SPI_CTRL_REG Spi control register  
  spi_ctrl_reg_setf(SPI_FIFO_RESET)
	SetWord16(&spi->SPI_CTRL_REGF, spi_ctrl_reg);
	SetBits16(&spi->SPI_CTRL_REGF, SPI_CAPTURE_AT_NEXT_EDGE, capture_next_edge);
	SetBits16(&spi->SPI_CTRL_REGF, SPI_FIFO_RESET, spi_fifo_reset);
0x50001204 SPI_CONFIG_REG Spi control register
  spi_set_bitmode(spi_cfg->spi_wsz);
  SetBits16(&spi->SPI_CONFIG_REGF, SPI_WORD_LENGTH, xx); SPI_MODE_8BIT
	SetBits16(&spi->SPI_CONFIG_REGF, SPI_MODE, spi_cp); SPI_CP_MODE_0
	SetBits16(&spi->SPI_CONFIG_REGF, SPI_SLAVE_EN, spi_ms); SPI_MS_MODE_MASTER
0x50001208 SPI_CLOCK_REG Spi clock register
  SetBits16(&spi->SPI_CLOCK_REGF, SPI_MASTER_CLK_MODE, 1);
	SetBits16(&spi->SPI_CLOCK_REGF, SPI_CLK_DIV, spi_clk_div); SPI_SPEED_MODE_4MHz
0x5000120C SPI_FIFO_CONFIG_REG Spi fifo configuration register
	SetWord16(&spi->SPI_FIFO_CONFIG_REGF, 0);// Set SPI FIFO threshold levels to 0
0x50001210 SPI_IRQ_MASK_REG Spi interrupt mask register

  0x50001214 SPI_STATUS_REG Spi status register
  0x50001218 SPI_FIFO_STATUS_REG SPI RX/TX fifo status register
 0x5000121C SPI_FIFO_READ_REG Spi RX fifo read register
 0x50001220 SPI_FIFO_WRITE_REG Spi TX fifo wtite register
0x50001224 SPI_CS_CONFIG_REG Spi cs configuration register

    0x50001228 SPI_FIFO_HIGH_REG Spi TX/RX High 16bit word
    0x5000122C SPI_TXBUFFER_FORCE_L_REG SPI TX buffer force low value
    0x50001230 SPI_TXBUFFER_FOR CE_H_REG SPI TX buffer force high value
*/
#define ADC_SPI_WORD_LENGTH (7<<2)
#define ADC_SPI_RX_TL (2<<4)
#define ADC_SPI_TX_TL 0

int16_t tmp_SPI_CTRL_REG;
int16_t tmp_SPI_CONFIG_REG;
int16_t tmp_SPI_FIFO_CONFIG_REG;
int16_t tmp_SPI_CS_CONFIG_REG;

void SPI_ADC_init(void)
{
	//SA_dataRead_32=(int32_t*)(&(SA_dataRead[0]));
  SA_dataRead_32[0]=45;	
	SA_ui16_dataRead_index=4;
	
#ifdef __SoundSensor__	
//user_spi_init(SPI_FLASH_GPIO_MAP);	

	SetWord16(SPI_CTRL_REG, SPI_FIFO_RESET|SPI_RX_EN|SPI_TX_EN|SPI_EN); 
	SetWord16(SPI_CONFIG_REG, ADC_SPI_WORD_LENGTH);
	//SetWord16(SPI_CLOCK_REG, );
	SetWord16(SPI_FIFO_CONFIG_REG,ADC_SPI_RX_TL|ADC_SPI_TX_TL );
	//SetWord16(SPI_IRQ_MASK_REG, );
	SetWord16(SPI_CTRL_REG,                 SPI_RX_EN|SPI_TX_EN|SPI_EN); 
	SetWord16(SPI_CS_CONFIG_REG,SPI_CS_0);
	SetWord16(SPI_CS_CONFIG_REG,SPI_CS_1);
	//SetWord16(SPI_CS_CONFIG_REG,SPI_CS_NONE);
	//SetWord16(SPI_CS_CONFIG_REG,SPI_CS_GPIO);
	
////======================================================================================================= 
//	  //SetWord16(SPI_CS_CONFIG_REG, SPI_CS_0);	

//	//	Step	1 	SPI_MODE_8BIT
////			spi_set_bitmode(SPI_MODE_8BIT);	
////		SetBits16(&spi->SPI_CONFIG_REGF, SPI_WORD_LENGTH, 15);
////    spi_env.incr = 2;	
//	  SetBits16(&spi->SPI_CONFIG_REGF, SPI_WORD_LENGTH, 7);	

////----------------------------------------------------------------------------	
////	Step	2 spi_cs_low();			
//	  SetWord16(SPI_CTRL_REG, GetWord16(SPI_CTRL_REG) & (~SPI_FIFO_RESET)); 
//    SetWord16(SPI_CS_CONFIG_REG, SPI_CS_0);			
//	
////----------------------------------------------------------------------------	
////	Step	3 Control SPI  			
////    SetBits16(&spi->SPI_CTRL_REGF, SPI_EN | SPI_TX_EN | SPI_RX_EN| SPI_DMA_TX_EN | SPI_DMA_RX_EN, 0);		
//    SetWord16(SPI_CTRL_REG, 0x07);			
////========================================================================================================
tmp_SPI_CTRL_REG=GetWord16(SPI_CTRL_REG);
tmp_SPI_CONFIG_REG=GetWord16(SPI_CONFIG_REG);
tmp_SPI_FIFO_CONFIG_REG=GetWord16(SPI_FIFO_CONFIG_REG);
tmp_SPI_CS_CONFIG_REG=GetWord16(SPI_CS_CONFIG_REG);


	
////	while(1)
//{	
//	
	SetWord16(SPI_CS_CONFIG_REG,SPI_CS_0);
    SetWord16(&spi->SPI_FIFO_WRITE_REGF, (uint16_t)(0x55));	
//    SetWord16(&spi->SPI_FIFO_WRITE_REGF, (uint16_t)(0x33));
//    SetWord16(&spi->SPI_FIFO_WRITE_REGF, (uint16_t)(0x44));
//		
//	while ( GetWord16(SPI_FIFO_STATUS_REG) & SPI_TRANSACTION_ACTIVE )
//		{
//		
//		};
//	SetWord16(SPI_CS_CONFIG_REG,SPI_CS_1);	
//	}		
//		//SetWord16(SPI_CS_CONFIG_REG, SPI_CS_NONE);
#endif	
}

//void SPITreeByts (void)
//{
//#ifdef __DA14531__		
////	  SetWord16(SPI_CS_CONFIG_REG,SPI_CS_1);	
////	  SetWord16(SPI_CS_CONFIG_REG,SPI_CS_0);
//    SetWord16(&spi->SPI_FIFO_WRITE_REGF, (uint16_t)(0xff));	
//    //SetWord16(&spi->SPI_FIFO_WRITE_REGF, (uint16_t)(0x55));
//    //SetWord16(&spi->SPI_FIFO_WRITE_REGF, (uint16_t)(0x55));	
//#endif	
//}

#ifdef __SoundSensor__ 
static tim0_2_clk_div_config_t clk_div_config =
{
    .clk_div  = TIM0_2_CLK_DIV_8
};

static tim2_config_t config =
{
	  .clk_source = TIM2_CLK_SYS,
    .hw_pause = TIM2_HW_PAUSE_OFF
};
static tim2_pwm_config_t pwm_config;


void timer2_init(void)
{
	//set_pad_functions GPIO_PIN_6
	GPIO_ConfigurePin(GPIO_PORT_0, GPIO_PIN_6, OUTPUT, PID_PWM3, true);
//    printf_string(UART, "\n\r\n\r***************");
//    printf_string(UART, "\n\r* TIMER2 TEST *\n\r");
//    printf_string(UART, "***************\n\r");

    // Enable the Timer0/Timer2 input clock
    timer0_2_clk_enable();
    // Set the Timer0/Timer2 input clock division factor
    timer0_2_clk_div_set(&clk_div_config);

    timer2_config(&config);

    // System clock, divided by 8, is the Timer2 input clock source (according
    // to the clk_div_config struct above).
//    timer2_pwm_freq_set(PWM_FREQUENCY, 16000000 / 8);
    timer2_pwm_freq_set(4000000U, 16000000/2);

    timer2_start();
	
	
        timer2_pause();
	
        // Set PWM3 duty cycle
        pwm_config.pwm_signal = TIM2_PWM_3;
        pwm_config.pwm_dc     = 50;
        timer2_pwm_signal_config(&pwm_config);

        // Release sw pause to let PWM2, PWM3, and PWM4 run
        timer2_resume();
}




#endif



//	  //----------------------------------------------------------------------------
////    SetWord16(&spi->SPI_FIFO_WRITE_REGF, (uint16_t)(SPI_FLASH_OP_RDSR << 8));
////	Step	4 Init read 3 byte  
//    SetWord16(&spi->SPI_FIFO_WRITE_REGF, (uint16_t)(0x55));	
//    SetWord16(&spi->SPI_FIFO_WRITE_REGF, (uint16_t)(0x33));
//    SetWord16(&spi->SPI_FIFO_WRITE_REGF, (uint16_t)(0x44));
//		


//   	NVIC_ClearPendingIRQ(SPI_IRQn);
//	  //SetWord16(SPI_CS_CONFIG_REG, SPI_CS_NONE);	
//#endif
//}

void intinit(void)
{
/*	
	24.3.2 GPIO Interrupts
1. Enable a GPIO interrupt for the P0_x by setting the
GPIO_IRQx_IN_SEL_REG[KBRD_IRQ0_SEL] bit.
2. Select the logic level by which the interrupt is generated
(GPIO_INT_LEVEL_CTRL_REG[INPUT_LEVELx]).
3. Select whether a key release is needed for an interrupt to be generated after a generated IRQ is
cleared (GPIO_INT_LEVEL_CTRL_REG[EDGE_LEVELNx]).
4. Set up the debounce time for GPIO trigger (GPIO_DEBOUNCE_REG[DEB_VALUE]).
5. Enable the debounce timer for the selected IRQ (GPIO_DEBOUNCE_REG[DEB_ENABLEx]).
	*/
/*	
	NVIC_ISER	Interrupt Set-Enable Register.
NVIC_ICER	Interrupt Clear-Enable Register.
NVIC_ISPR	Interrupt Set-Pending Register.
NVIC_ICPR	Interrupt Clear-Pending Register.
NVIC_IPR0-NVIC_IPR7	Interrupt Priority Registers.
	*/
	
//void GPIO_SetPinFunction(GPIO_PORT port, GPIO_PIN pin, GPIO_PUPD mode, GPIO_FUNCTION function)
//void GPIO_ConfigurePin(GPIO_PORT port, GPIO_PIN pin, GPIO_PUPD mode, GPIO_FUNCTION function,const bool high)
//void GPIO_ConfigurePinPower(GPIO_PORT port, GPIO_PIN pin, GPIO_POWER_RAIL power_rail)//PAD_WEAK_CTRL_REG
//void GPIO_EnableIRQ(GPIO_PORT port, GPIO_PIN pin, IRQn_Type irq, bool low_input,bool release_wait, uint8_t debounce_ms)	
//void GPIO_SetIRQInputLevel(IRQn_Type irq, GPIO_IRQ_INPUT_LEVEL level)//GPIO_INT_LEVEL_CTRL_REG
//void GPIO_ResetIRQ( IRQn_Type irq )	//GPIO_RESET_IRQ_REG

GPIO_ConfigurePin(GPIO_PORT_0,GPIO_PIN_7,INPUT, PID_GPIO,false);


GPIO_EnableIRQ(GPIO_PORT_0, //GPIOSetBits16(GPIO_IRQ0_IN_SEL_REG + 2*(irq-GPIO0_IRQn), KBRD_IRQn_SEL, KBRD_IRQn_SEL_BASE[port] + pin);
GPIO_PIN_7,
GPIO0_IRQn,
false,/* 0= selected input will generate GPIO IRQ0 if that input is high.
1 = selected input will generate GPIO IRQ0 if that input is low.*/
	false,/*0: do not wait for key release after interrupt was reset for GPIO IRQ0, so a new interrupt can be
initiated immediately 1: wait for key release after interrupt was reset for IRQ0*/
		0);//GPIO_DEBOUNCE_REG
//
	
}

void GPIO0_Handler(void)
{

 SetWord16(SPI_CS_CONFIG_REG,SPI_CS_NONE);	
	
	if (SA_ui16_dataRead_index<(def_dataRead_Size-10))
	{
    
		SA_dataRead[SA_ui16_dataRead_index+3] = GetWord16(&spi->SPI_FIFO_READ_REGF) ;				
    SA_dataRead[SA_ui16_dataRead_index+2] = GetWord16(&spi->SPI_FIFO_READ_REGF) ;				
		SA_dataRead[SA_ui16_dataRead_index+1]= GetWord16(&spi->SPI_FIFO_READ_REGF) ;	
		SA_dataRead[SA_ui16_dataRead_index]=0;
		SA_ui16_dataRead_index+=4;
	}	
	else 
	{ //SA_b_dataRead_full=true;
		dataRead_toy = GetWord16(&spi->SPI_FIFO_READ_REGF) ;				
    dataRead_toy = GetWord16(&spi->SPI_FIFO_READ_REGF) ;				
		dataRead_toy = GetWord16(&spi->SPI_FIFO_READ_REGF) ;	
	};	
	SetWord16(SPI_CTRL_REG, SPI_FIFO_RESET|SPI_RX_EN|SPI_TX_EN|SPI_EN);
	SetWord16(SPI_CTRL_REG,                SPI_RX_EN|SPI_TX_EN|SPI_EN);
 SetWord16(SPI_CS_CONFIG_REG,SPI_CS_0); 	
 SetWord16(&spi->SPI_FIFO_WRITE_REGF, (uint16_t)(0x55));//SPITreeByts();
 SetWord16(&spi->SPI_FIFO_WRITE_REGF, (uint16_t)(0xff));
 SetWord16(&spi->SPI_FIFO_WRITE_REGF, (uint16_t)(0x55));	
 while ( GetWord16(SPI_FIFO_STATUS_REG) & SPI_TRANSACTION_ACTIVE )
		{
		};
 SetWord16(SPI_CS_CONFIG_REG,SPI_CS_NONE);		
	//    SetWord16(SPI_CS_CONFIG_REG,SPI_CS_0); 
 GPIO_ResetIRQ(GPIO0_IRQn);
 NVIC_ClearPendingIRQ(GPIO0_IRQn);

}

//void SPI_Handler(void)
//{
//#ifdef __SoundSensor__ 	
//	//----------------------------------------------------------------------------	
////	Step	2 spi_cs_low();			
//	  
//    //SetWord16(SPI_CS_CONFIG_REG, SPI_CS_0);		
//	
//	 /// SetWord16(SPI_CTRL_REG, GetWord16(SPI_CTRL_REG) | (SPI_FIFO_RESET)); 	
//		SetWord16(SPI_IRQ_MASK_REG, 0);	
////		NVIC_DisableIRQ(SPI_IRQn);	


////----------------------------------------------------------------------------		
////    dataRead = (GetWord16(&spi->SPI_FIFO_HIGH_REGF) << 16);		
////    dataRead += GetWord16(&spi->SPI_FIFO_READ_REGF) & 0xFFFF;		
////	Step	6 Read data (3 byte)		
//	if (SA_ui16_dataRead_index<(def_dataRead_Size-3))
//	{
//    SA_dataRead[SA_ui16_dataRead_index++] = GetWord16(&spi->SPI_FIFO_READ_REGF) ;				
//    SA_dataRead[SA_ui16_dataRead_index++] = GetWord16(&spi->SPI_FIFO_READ_REGF) ;				
//		SA_dataRead[SA_ui16_dataRead_index++]= GetWord16(&spi->SPI_FIFO_READ_REGF) ;	
//	}	
//	else 
//	{ //SA_b_dataRead_full=true;
//		dataRead_toy = GetWord16(&spi->SPI_FIFO_READ_REGF) ;				
//    dataRead_toy = GetWord16(&spi->SPI_FIFO_READ_REGF) ;				
//		dataRead_toy = GetWord16(&spi->SPI_FIFO_READ_REGF) ;	
//	};	
//	
