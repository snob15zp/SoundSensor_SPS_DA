//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
#include <systick.h>
#include "user_periph_setup.h"
#include "my_timer.h"
#include "ring_buff.h"
#include "ADC_flash.h"
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#define SPI_ADC_FLASH_EN_PORT             GPIO_PORT_0
#define SPI_ADC_FLASH_EN_PIN              GPIO_PIN_1

#define SPI_ADC_FLASH_CLK_PORT            GPIO_PORT_0
#define SPI_ADC_FLASH_CLK_PIN             GPIO_PIN_4

#define SPI_ADC_FLASH_DO_PORT             GPIO_PORT_0
#define SPI_ADC_FLASH_DO_PIN              GPIO_PIN_0

#define SPI_ADC_FLASH_DI_PORT             GPIO_PORT_0
#define SPI_ADC_FLASH_DI_PIN              GPIO_PIN_3

#define SPI_FLASH_OP_ByteProgramSec		  ((uint32_t)0xAD)

// Configuration struct for SPI
static const spi_cfg_t spi_adc_flash_cfg = {
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
#if defined (CFG_SPI_DMA_SUPPORT)
    .spi_dma_channel = SPI_DMA_CHANNEL_01,
    .spi_dma_priority = DMA_PRIO_0,
#endif
};

typedef union {
uint8_t	 masByte[4];
uint16_t mas_u16[2];
uint32_t data_u32;
} dc_t;

static dc_t dc; 	 
static uint32_t my_ssize;
//static uint32_t AddrNewRecord;
uint32_t AddrNewRecord;
static uint16_t F_PWR_on = 0;
static uint16_t F_Erase_on = 0;
static uint8_t byteADC[4];
const void *databyteADC = byteADC;

int rezult_find_AddrNewRecord;

bool ADCon;

int find_AddrNewRecord(uint32_t Addr_L, uint32_t Addr_R, uint32_t *ptr);
int8_t spi_flash_StartProgramByteSequent(uint8_t *wr_data_ptr, uint32_t address, uint16_t size);

//void AF_V_AddADCdataToFIFO(uint16_t A, uint16_t B)
//{
//	dc.mas_u16[0] = A;
//	dc.mas_u16[1] = B;	
//	RingBuffer_add_u32(dc.data_u32);
//}/

void AF_V_AddADCdataToFIFO(uint16_t A, uint16_t B)
{
	dc.masByte[0] = A;
	dc.masByte[1] = A | 0x01;
	dc.masByte[2] = A | 0x02;
	dc.masByte[3] = 0;	
	
	
	
//	dc.mas_u16[0] = A;
//	dc.mas_u16[1] = B;	
	RingBuffer_add_u32(dc.data_u32);
}



int AF_V_WriteStart(uint16_t callerFunction)
{
	return 0;//RDD
int rezult_Start; 	
// 	Configure SPI pins
    GPIO_ConfigurePin(SPI_ADC_FLASH_EN_PORT,  SPI_ADC_FLASH_EN_PIN,  OUTPUT, PID_SPI_EN, true);
    GPIO_ConfigurePin(SPI_ADC_FLASH_CLK_PORT, SPI_ADC_FLASH_CLK_PIN, OUTPUT, PID_SPI_CLK, false);
    GPIO_ConfigurePin(SPI_ADC_FLASH_DO_PORT,  SPI_ADC_FLASH_DO_PIN,  OUTPUT, PID_SPI_DO, false);
    GPIO_ConfigurePin(SPI_ADC_FLASH_DI_PORT,  SPI_ADC_FLASH_DI_PIN,  INPUT,  PID_SPI_DI, false);
// 	Initialize SPI
    spi_initialize(&spi_adc_flash_cfg);
//	Unlock flash	
	delay_10ms();			
	spi_set_bitmode(SPI_MODE_8BIT);
	spi_transaction(SPI_FLASH_OP_WRDI);	
	delay_10ms();	
	spi_flash_set_write_enable();
	delay_10ms();		
	spi_flash_write_status_reg(0);	
	delay_10ms();	
//	Find address of a new record	
	rezult_find_AddrNewRecord = find_AddrNewRecord(SPI_FLASH_ADDR_START_RECORD_ADC, SPI_FLASH_ADDR_END_RECORD_ADC, &AddrNewRecord);	
	if ( rezult_find_AddrNewRecord == 0 )
	{
//		AF_V_AddADCdataToFIFO(callerFunction, 0x80 | F_PWR_on);
		AF_V_AddADCdataToFIFO(0x66 , 0x30);		
		if ( RingBuffer_get_ch8(&byteADC[0]) == 0 )  
			spi_flash_StartProgramByteSequent(&byteADC[0], AddrNewRecord, 1);
//  Run ADC   ..........		
		rezult_Start = 0;
	}
	else
	{
		if ( F_Erase_on == 0 )
		{
/*			
			F_Erase_on = 1;
//Block64 erase			
			spi_flash_block_erase(SPI_FLASH_ADDR_START_RECORD_ADC, SPI_FLASH_OP_BE64);
			while ( spi_flash_is_busy() != SPI_FLASH_ERR_OK )
			{
				delay_10ms();
			}
			spi_flash_block_erase(0x30000, SPI_FLASH_OP_BE64);
			while ( spi_flash_is_busy() != SPI_FLASH_ERR_OK )
			{
				delay_10ms();
			}
			AF_V_AddADCdataToFIFO(callerFunction, 0x80 | F_PWR_on);
			if ( RingBuffer_get_ch8(&byteADC[0]) == 0 )  
				spi_flash_StartByteProgramSequent(&byteADC[0], SPI_FLASH_ADDR_START_RECORD_ADC, 1);
//  Run ADC   ..........					
*/			
			rezult_Start = 0;			
		}
		else
		{
			AddrNewRecord = SPI_FLASH_ADDR_END_RECORD_ADC; 
			rezult_Start = -1;
		}
	}
	F_PWR_on = 1;
	return rezult_Start;
}

void AF_V_WriteStop(uint16_t callerFunction)
{
	spi_set_bitmode(SPI_MODE_8BIT);
	spi_transaction(SPI_FLASH_OP_WRDI);
}
















int8_t my_spi_send(const void *data, uint16_t num, SPI_OP_CFG op);
int spi_flash_init(void);

int init_spi_task(void)
{
//	init_systick();
//	spi_flash_init();
	return 0;
}

/*
int spi_flash_init(void)
{
// 	Configure SPI pins
    GPIO_ConfigurePin(SPI_ADC_FLASH_EN_PORT,  SPI_ADC_FLASH_EN_PIN,  OUTPUT, PID_SPI_EN, true);
    GPIO_ConfigurePin(SPI_ADC_FLASH_CLK_PORT, SPI_ADC_FLASH_CLK_PIN, OUTPUT, PID_SPI_CLK, false);
    GPIO_ConfigurePin(SPI_ADC_FLASH_DO_PORT,  SPI_ADC_FLASH_DO_PIN,  OUTPUT, PID_SPI_DO, false);
    GPIO_ConfigurePin(SPI_ADC_FLASH_DI_PORT,  SPI_ADC_FLASH_DI_PIN,  INPUT,  PID_SPI_DI, false);

	
// 	Initialize SPI
    spi_initialize(&spi_adc_flash_cfg);
}
*/












void spi_flashProgramByte_dma(uint8_t *wr_data_ptr, uint32_t address, uint16_t size)
{
//    printf_string(UART, "\n\r>>  Performing Program Page DMA...");
	
//    SPI_FLASH_ENABLE_POWER_PIN();

/*	
    // Check if SPI Flash is ready
    int8_t status = spi_flash_is_busy();
    if (status != SPI_FLASH_ERR_OK)
    {
        return status;
    }
*/
    uint16_t temp_size = size;

/*	
    // Send Write Enable instruction
    status = spi_flash_write_enable(SPI_FLASH_OP_WREN);
    if (status != SPI_FLASH_ERR_OK)
    {
        return status;
    }
*/
	spi_flash_set_write_enable();
//	delay_200us();	
/*	
    // Check for max page size
    if (temp_size > SPI_FLASH_PAGE_SIZE)
    {
        temp_size = SPI_FLASH_PAGE_SIZE;
    }
*/
    // Send command
//    spi_set_bitmode(SPI_MODE_32BIT);
    spi_set_bitmode(SPI_MODE_8BIT);
//    spi_cs_low();
	SetWord16(SPI_CTRL_REG, GetWord16(SPI_CTRL_REG) & (~SPI_FIFO_RESET)); 	
    SetWord16(SPI_CS_CONFIG_REG, SPI_CS_0);	
	
	
	
//    spi_access((SPI_FLASH_OP_PP << 24) | address);

    // Send data
//    spi_set_bitmode(SPI_MODE_8BIT);
//    my_spi_send(wr_data_ptr, temp_size, SPI_OP_DMA);
    my_spi_send(wr_data_ptr, temp_size, 2);

    // Wait for DMA to finish

//-----------------------------------------------------------------	
//-----------------------------------------------------------------
//    spi_wait_dma_write_to_finish();
//void spi_wait_dma_write_to_finish(void)
//{
//    while(dma_get_ctrl_reg(spi_env.spi_dma_tx_channel) & DMA_ON);
//}

//__STATIC_FORCEINLINE uint16_t dma_get_ctrl_reg(DMA_ID id){
//    return GetWord16(DMA(id)->DMA_CTRL_REG);
//}

    while(GetWord16(DMA1_CTRL_REG) & DMA_ON);
//-----------------------------------------------------------------	
//-----------------------------------------------------------------
	
	
	
	
	

//    spi_cs_high();
	  SetWord16(SPI_CS_CONFIG_REG, SPI_CS_NONE);		
	  SetWord16(SPI_CTRL_REG, GetWord16(SPI_CTRL_REG) | (SPI_FIFO_RESET)); 	



//-----------------------------------------------------------------	
//-----------------------------------------------------------------
//????????????????????????????????????????????????????????????????
//    return spi_flash_wait_till_ready();
//-----------------------------------------------------------------	
//-----------------------------------------------------------------
}






int8_t my_spi_send(const void *data, uint16_t num, SPI_OP_CFG op)
{
	
	
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------		
    // Clear Tx, Rx and DMA enable paths in Control Register
//    spi_ctrl_reg_clear_enable_setf();
/*1    SetBits16(&spi->SPI_CTRL_REGF, SPI_EN | SPI_TX_EN | SPI_RX_EN| SPI_DMA_TX_EN | SPI_DMA_RX_EN, 0);*/
/*2	 SetWord16(SPI_CTRL_REG, 0);*/

    // Enable TX path
//    spi_ctrl_reg_spi_tx_en_setf(SPI_BIT_EN);
/*1    SetBits16(&spi->SPI_CTRL_REGF, SPI_TX_EN, SPI_BIT_EN);*/
/*2*/	 SetWord16(SPI_CTRL_REG, SPI_TX_EN); //(2)

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------	

	

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------	
//	
    // Non-blocking mode. Interrupt driven + DMA
//    if ((op == SPI_OP_DMA) && (spi_env.spi_wsz != SPI_MODE_32BIT))
//    {
//        dma_set_ainc(spi_env.spi_dma_tx_channel, DMA_INC_TRUE);
/*1*/	/*	  SetBits16(DMA1_CTRL_REG, AINC, DMA_INC_TRUE);*/

//        spi_dma_send((uint32_t *) data, num, spi_dma_send_cb, DMA_IRQ_STATE_ENABLED);
	
    // Update DMA bus data width
//    dma_set_bw(spi_env.spi_dma_tx_channel, spi_env.spi_wsz == SPI_MODE_8BIT ? DMA_BW_BYTE : DMA_BW_HALFWORD);
/*1*/  /*  SetBits16(DMA1_CTRL_REG, BW, DMA_BW_BYTE);*/
		 SetWord16(DMA1_CTRL_REG, 0x58);

//
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------	





//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------	
//	
    // Update DMA source address
//    dma_set_src(spi_env.spi_dma_tx_channel, (uint32_t) data);
//    SetWord16(DMA(DMA_CHANNEL_1)->DMA_A_STARTL_REG, (uint32_t)data & 0xFFFF);
//    SetWord16(DMA(DMA_CHANNEL_1)->DMA_A_STARTH_REG, (uint32_t)data >> 16);
	SetWord16(DMA1_A_STARTL_REG, (uint32_t)data & 0xFFFF);
	SetWord16(DMA1_A_STARTH_REG, (uint32_t)data >> 16);	
//
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------	


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------	
//	
    // Update Tx DMA INT
//    dma_set_int(spi_env.spi_dma_tx_channel, num);
//    SetWord16(DMA(DMA_CHANNEL_1)->DMA_INT_REG, num - 1);
	SetWord16(DMA1_INT_REG, num - 1);	
	
    // Update Tx DMA length
//    dma_set_len(spi_env.spi_dma_tx_channel, num);
//    SetWord16(DMA(DMA_CHANNEL_1)->DMA_LEN_REG, num - 1);
	SetWord16(DMA1_LEN_REG, num - 1);		
//
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------	



    // Update DMA callback
//    dma_register_callback(spi_env.spi_dma_tx_channel, spi_dma_send_cb, NULL);

	
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------	
//	
    // Enable Tx DMA
//    dma_channel_start(spi_env.spi_dma_tx_channel, DMA_IRQ_STATE_ENABLED);
//    SetBits16(DMA1_CTRL_REG, IRQ_ENABLE, DMA_IRQ_STATE_ENABLED);
//    SetBits16(DMA1_CTRL_REG, DMA_ON, DMA_STATE_ENABLED);
	  SetWord16(DMA1_CTRL_REG, 0x0659);	
//
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------	
	



//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------	
//	
    // Enable SPI DMA Tx Path
//    spi_ctrl_reg_spi_dma_tx_en_setf(SPI_BIT_EN);
//    SetBits16(&spi->SPI_CTRL_REGF, SPI_DMA_TX_EN, SPI_BIT_EN);
	  SetWord16(SPI_CTRL_REG, 0x0A);	// SPI_TX_EN | SPI_DMA_TX_EN
        // Enable SPI
//      spi_ctrl_reg_spi_en_setf(SPI_BIT_EN);
//		SetBits16(&spi->SPI_CTRL_REGF, SPI_EN, SPI_BIT_EN);
	  SetWord16(SPI_CTRL_REG, 0x0B);	// SPI_TX_EN | SPI_DMA_TX_EN | SPI_EN		
//
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------	
//		NVIC_EnableIRQ(DMA_IRQn);		
		
//    }
    return SPI_STATUS_ERR_OK;	
}

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------





//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
/*
 * SPI Flash Program functions
 ****************************************************************************************
 */
int8_t spi_flash_StartProgramByteSequent(uint8_t *wr_data_ptr, uint32_t address, uint16_t size)
{
//    SPI_FLASH_ENABLE_POWER_PIN();

    // Check if SPI Flash is ready
    int8_t status = spi_flash_is_busy();
    if (status != SPI_FLASH_ERR_OK)
    {
        return status;
    }
    uint16_t temp_size = size;
    // Send Write Enable instruction
	status = spi_flash_set_write_enable();	
    if (status != SPI_FLASH_ERR_OK)
    {
        return status;
    }
    // Send command
    spi_set_bitmode(SPI_MODE_32BIT);
    spi_cs_low();
    spi_access((SPI_FLASH_OP_ByteProgramSec << 24) | address);

    // Send data
    spi_set_bitmode(SPI_MODE_8BIT);
    while(temp_size > 0)
    {
        spi_access(*wr_data_ptr++);
        temp_size--;
    }
    spi_cs_high();
    return spi_flash_wait_till_ready();
}




uint32_t get_u32_flashDisk(uint32_t addr)
{
	spi_flash_read_data(&dc.masByte[0], addr, 3, &my_ssize);
	dc.masByte[3] = 0;
	return dc.data_u32;
}

	
int find_AddrNewRecord(uint32_t Addr_L, uint32_t Addr_R, uint32_t *ptr)
{
uint32_t Addr_delta;
uint32_t Addr_new, u32tmp;	
uint32_t V_L, V_R, V_new;	
//uint32_t cnt = 0;	

	V_L = get_u32_flashDisk(Addr_L);
	V_R = get_u32_flashDisk(Addr_R);
	if ( V_L >= SPI_FLASH_DATA_FLASH_ERASE )
	{
		*ptr = Addr_L;
		return 0; 	
	}
	if ( V_R < SPI_FLASH_DATA_FLASH_ERASE )
	{
		*ptr = NULL;
		return SPI_FLASH_FLAG_FLASH_FULL;		
	}		
	Addr_delta = Addr_R - Addr_L;
	while ( Addr_delta > 3  )
	{
		u32tmp = Addr_delta / 3;
		u32tmp >>= 1;
		Addr_new = Addr_L + u32tmp * 3;
		V_new = get_u32_flashDisk(Addr_new);
		if ( V_new < SPI_FLASH_DATA_FLASH_ERASE )
			Addr_L = Addr_new;
		else
			Addr_R = Addr_new;
		Addr_delta = Addr_R - Addr_L;
//		cnt++;
	}
	*ptr = Addr_R;
	return 0;
}


void IRQ_wrByteFromRecord(void)
{
	SetWord16(SPI_CTRL_REG, GetWord16(SPI_CTRL_REG) & (~SPI_FIFO_RESET));
	byteADC[0] = SPI_FLASH_OP_ByteProgramSec;
	byteADC[1] = 0x33;	
	if ( RingBuffer_get_ch8(&byteADC[2]) == 0 )
		SetWord16(SPI_CS_CONFIG_REG, SPI_CS_0);			
	else		
		SetWord16(SPI_CS_CONFIG_REG, SPI_CS_NONE);
	
//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------
//int8_t my_spi_send(const void *data, uint16_t num, SPI_OP_CFG op)
//{
	
	
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------		
    // Clear Tx, Rx and DMA enable paths in Control Register
//    spi_ctrl_reg_clear_enable_setf();
/*1    SetBits16(&spi->SPI_CTRL_REGF, SPI_EN | SPI_TX_EN | SPI_RX_EN| SPI_DMA_TX_EN | SPI_DMA_RX_EN, 0);*/
/*2	 SetWord16(SPI_CTRL_REG, 0);*/

    // Enable TX path
//    spi_ctrl_reg_spi_tx_en_setf(SPI_BIT_EN);
/*1    SetBits16(&spi->SPI_CTRL_REGF, SPI_TX_EN, SPI_BIT_EN);*/
/*2*/	 SetWord16(SPI_CTRL_REG, SPI_TX_EN); //(2)

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------	

	

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------	
//	
    // Non-blocking mode. Interrupt driven + DMA
//    if ((op == SPI_OP_DMA) && (spi_env.spi_wsz != SPI_MODE_32BIT))
//    {
//        dma_set_ainc(spi_env.spi_dma_tx_channel, DMA_INC_TRUE);
/*1*/	/*	  SetBits16(DMA1_CTRL_REG, AINC, DMA_INC_TRUE);*/

//        spi_dma_send((uint32_t *) data, num, spi_dma_send_cb, DMA_IRQ_STATE_ENABLED);
	
    // Update DMA bus data width
//    dma_set_bw(spi_env.spi_dma_tx_channel, spi_env.spi_wsz == SPI_MODE_8BIT ? DMA_BW_BYTE : DMA_BW_HALFWORD);
/*1*/  /*  SetBits16(DMA1_CTRL_REG, BW, DMA_BW_BYTE);*/
		 SetWord16(DMA1_CTRL_REG, 0x58);
//
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------	





//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------	
//	
    // Update DMA source address
//    dma_set_src(spi_env.spi_dma_tx_channel, (uint32_t) data);
//    SetWord16(DMA(DMA_CHANNEL_1)->DMA_A_STARTL_REG, (uint32_t)data & 0xFFFF);
//    SetWord16(DMA(DMA_CHANNEL_1)->DMA_A_STARTH_REG, (uint32_t)data >> 16);
	SetWord16(DMA1_A_STARTL_REG, (uint32_t)databyteADC & 0xFFFF);
	SetWord16(DMA1_A_STARTH_REG, (uint32_t)databyteADC >> 16);	
//
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------	


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------	
//	
    // Update Tx DMA INT
//    dma_set_int(spi_env.spi_dma_tx_channel, num);
//    SetWord16(DMA(DMA_CHANNEL_1)->DMA_INT_REG, num - 1);
//	SetWord16(DMA1_INT_REG, num - 1);	
	SetWord16(DMA1_INT_REG, 2);	
	
    // Update Tx DMA length
//    dma_set_len(spi_env.spi_dma_tx_channel, num);
//    SetWord16(DMA(DMA_CHANNEL_1)->DMA_LEN_REG, num - 1);
//	SetWord16(DMA1_LEN_REG, num - 1);
	SetWord16(DMA1_LEN_REG, 2);	
//
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------	



    // Update DMA callback
//    dma_register_callback(spi_env.spi_dma_tx_channel, spi_dma_send_cb, NULL);

	
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------	
//	
    // Enable Tx DMA
//    dma_channel_start(spi_env.spi_dma_tx_channel, DMA_IRQ_STATE_ENABLED);
//    SetBits16(DMA1_CTRL_REG, IRQ_ENABLE, DMA_IRQ_STATE_ENABLED);
//    SetBits16(DMA1_CTRL_REG, DMA_ON, DMA_STATE_ENABLED);
	  SetWord16(DMA1_CTRL_REG, 0x0659);	
//
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------	
	



//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------	
//	
    // Enable SPI DMA Tx Path
//    spi_ctrl_reg_spi_dma_tx_en_setf(SPI_BIT_EN);
//    SetBits16(&spi->SPI_CTRL_REGF, SPI_DMA_TX_EN, SPI_BIT_EN);
	  SetWord16(SPI_CTRL_REG, 0x0A);	// SPI_TX_EN | SPI_DMA_TX_EN
        // Enable SPI
//      spi_ctrl_reg_spi_en_setf(SPI_BIT_EN);
//		SetBits16(&spi->SPI_CTRL_REGF, SPI_EN, SPI_BIT_EN);
	  SetWord16(SPI_CTRL_REG, 0x0B);	// SPI_TX_EN | SPI_DMA_TX_EN | SPI_EN		
//
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------	
//		NVIC_EnableIRQ(DMA_IRQn);		
		
//    }
//  return SPI_STATUS_ERR_OK;	
//}


//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------

//    spi_wait_dma_write_to_finish();
//	while( GetWord16(DMA1_CTRL_REG) & DMA_ON )
//	{
//		__asm__ volatile("nop");
//	};

	
	delay_10ms();
	
//    spi_cs_high();
	  SetWord16(SPI_CS_CONFIG_REG, SPI_CS_NONE);		
	  SetWord16(SPI_CTRL_REG, GetWord16(SPI_CTRL_REG) | (SPI_FIFO_RESET)); 	
}


