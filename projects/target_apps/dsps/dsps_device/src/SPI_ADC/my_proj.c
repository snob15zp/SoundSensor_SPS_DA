

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "arch_system.h"
#include "user_periph_setup.h"
#include "gpio.h"
#include "uart.h"
#include "uart_utils.h"
#include "spi.h"
#include "spi_flash.h"


#include "ADC_flash.h"
#include "systick.h"
//#include "my_debug.h"
//#include "my_flash.h"
#include "my_timer.h"
#include "my_proj.h"


#define SIZE_MASS	256


extern uint32_t AddrNewRecord;

uint8_t rd_data1[512];
uint8_t wr_data1[512];
uint32_t my_ssize; 
uint32_t addr_median;
uint32_t value;	

void out(uint16_t indx);

uint8_t mass[SIZE_MASS];

void init_mass(void)
{
uint32_t i;
	for ( i = 0; i < SIZE_MASS; i++ )
		mass[i] = i;
}

int8_t my_spi_flash_page_program_dma(uint8_t *wr_data_ptr, uint32_t address, uint16_t size);
int8_t my_spi_send_board(const void *data, uint16_t num, SPI_OP_CFG op);
int8_t my_spi_flash_page_program_next(uint8_t *wr_data_ptr, uint16_t size);
void my_spi_access(uint32_t dataToSend);
void task_project(void)
{
//uint32_t jedec_id_m;
uint16_t indx;
uint16_t status_reg;
int rezult_median;
//uint8_t dev_id_m;
uint8_t rezult;

	
//static uint16_t time_stop = 0; 	
static uint32_t addr = 0;		
static enum { sfm_idle,	sfm_init_addr, sfm_pg_erase, sfm_pg_init, sfm_pg_write, 
	sfm_pg_read, sfm_check_data, sfm_done } SFM = sfm_idle;

//	if ( check_timeOVER(&time_stop, 1000) == 0 )
//		return;
	
	delay_100ms();
	delay_100ms();
	delay_100ms();
	delay_100ms();
	delay_100ms();	
	switch ( SFM )
	{
		case sfm_idle:
			rezult_median = AF_V_WriteStart(0x55);
			addr = AddrNewRecord;
			addr = SPI_FLASH_ADDR_START_RECORD_ADC;			
/*		
//			spi_flash_auto_detect(&dev_id_m);
// Read SPI Flash JEDEC ID
			spi_flash_read_jedec_id(&jedec_id_m);
//			spi_set_bitmode(SPI_MODE_8BIT);
//			spi_transaction(SPI_FLASH_OP_WRDI);		
//			delay_10ms();		
		
			status_reg = spi_flash_read_status_reg();		
			delay_10ms();
			
			rezult = spi_flash_set_write_enable();
			delay_10ms();			

			rezult = spi_flash_write_status_reg(0);
			delay_10ms();

			status_reg = spi_flash_read_status_reg();		
			delay_10ms();	

			rezult = spi_flash_set_write_enable();		
			delay_10ms();	
*/			
			SFM = sfm_init_addr;
			break;
			
		case sfm_init_addr:
	
//			printf_string(UART, "\n\r>> addr = ");	
//			printf_byte(UART, 0xff & addr>>24);
//			printf_byte(UART, 0xff & addr>>16);
//			printf_byte(UART, 0xff & addr>>8);
//			printf_byte(UART, 0xff & addr);
			SFM = sfm_pg_erase;		
			break;
		
		case sfm_pg_erase:
/*			
			spi_set_bitmode(SPI_MODE_8BIT);
			spi_transaction(SPI_FLASH_OP_WRDI);			
			delay_10ms();
			rezult = spi_flash_set_write_enable();
			spi_flash_block_erase(SPI_FLASH_ADDR_START_RECORD_ADC, SPI_FLASH_OP_SE);			
*/		
			SFM = sfm_pg_init;				
			break;
		
		case sfm_pg_init:
//			for ( indx = 0; indx < 256; indx++ )
//				wr_data1[indx] = indx;
//			init_mass();
			SFM = sfm_pg_write;						
			break;
		
		case sfm_pg_write:
			mass[0] = 0x30;
			mass[1] = 0x31;
			mass[2] = 0x32;		
//			spi_flash_page_program(&mass[0], AddrNewRecord, 3);
//			printf_string(UART, "\n\r>> Page programmed.\n\r");
			SFM = sfm_pg_read;								
			break;		
		
		case sfm_pg_read:
//			rezult_median = task_half_div(ADDR_START, ADDR_END, &addr_median);
//			prtf( "\n\r>> addr_median = ", addr_median, "", 0);
//			printf_string(UART, "\n\r");		
			SFM = sfm_check_data;									
			break;
			
		case sfm_check_data:
//			IRQ_wrByteFromRecord();	
			mass[0] = 0xAD;
			mass[1] = 0x65;
			mass[2] = 0x37;		
//			my_spi_flash_page_program_dma(&mass[0], addr, 3);
			my_spi_flash_page_program_next(&mass[0], 3);
			delay_10ms();
			mass[0] = 0xAD;
			mass[1] = 0x67;
			mass[2] = 0x38;		
//			my_spi_flash_page_program_dma(&mass[0], addr, 3);
			my_spi_flash_page_program_next(&mass[0], 3);		
//			IRQ_wrByteFromRecord();			
			delay_10ms();		
			spi_set_bitmode(SPI_MODE_8BIT);
	     	spi_transaction(SPI_FLASH_OP_WRDI);			
			delay_10ms();					
			spi_flash_read_data(rd_data1, SPI_FLASH_ADDR_START_RECORD_ADC, 256, &my_ssize);			
  			for ( indx = 0; indx < 16; indx +=2 )
			{
//				printf_string(UART, "   ");		
//				out(indx*16);
//				out((indx+1)*16);
//				printf_string(UART, "\n\r");
			}
			
		
		
			
			rezult = rezult;
//			printf_string(UART, ">> DONE");							
			SFM = sfm_done;			
			break;
		
		case sfm_done:
			break;
	}
	return;
}


/*
 * SPI Flash Program functions
 ****************************************************************************************
 */
int8_t my_spi_flash_page_program_next(uint8_t *wr_data_ptr, uint16_t size)
{

    // Send command
    spi_set_bitmode(SPI_MODE_8BIT);
//    spi_cs_low();
		SetWord16(SPI_CTRL_REG, GetWord16(SPI_CTRL_REG) & (~SPI_FIFO_RESET)); 	
    SetWord16(SPI_CS_CONFIG_REG, SPI_CS_0);			
//-------------------------------------------------------------------
//-------------------------------------------------------------------
	SetWord16(SPI_CTRL_REG, 0x07);
	SetWord16(&spi->SPI_FIFO_WRITE_REGF, 0xAD);
	SetWord16(&spi->SPI_FIFO_WRITE_REGF, 0x33);
	SetWord16(&spi->SPI_FIFO_WRITE_REGF, wr_data_ptr[2]);	

//while (spi_transaction_status_getf() == SPI_TRANSACTION_IS_ACTIVE);
	while (GetBits16(&spi->SPI_FIFO_STATUS_REGF, SPI_TRANSACTION_ACTIVE) == SPI_TRANSACTION_IS_ACTIVE);

//-------------------------------------------------------------------
//-------------------------------------------------------------------
	
//    spi_cs_high();
	  SetWord16(SPI_CS_CONFIG_REG, SPI_CS_NONE);		
	  SetWord16(SPI_CTRL_REG, GetWord16(SPI_CTRL_REG) | (SPI_FIFO_RESET)); 

    return spi_flash_wait_till_ready();
}



void out(uint16_t indx)
{

}


int8_t my_spi_flash_page_program_dma(uint8_t *wr_data_ptr, uint32_t address, uint16_t size)
{

	
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
	delay_200us();	
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
    my_spi_send_board(wr_data_ptr, temp_size, 2);


//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // Wait for DMA to finish
//    spi_wait_dma_write_to_finish();
	delay_10ms();
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!	
	

//    spi_cs_high();
	  SetWord16(SPI_CS_CONFIG_REG, SPI_CS_NONE);		
	  SetWord16(SPI_CTRL_REG, GetWord16(SPI_CTRL_REG) | (SPI_FIFO_RESET)); 	




    return spi_flash_wait_till_ready();
	
}

int8_t my_spi_send_board(const void *data, uint16_t num, SPI_OP_CFG op)
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






