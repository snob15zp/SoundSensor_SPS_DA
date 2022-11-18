#include <systick.h>
#include <user_periph_setup.h>
#include "ss_hnd.h"
#include "cmd_hnd.h"
#include "version.h"
#include "SS_sys.h"
#include "user_config_storage.h"
#include "ADC_flash.h"

int8_t on_int_rd(uint16_t var, uint16_t count, uint16_t out[9]) 
{
    uint16_t* vars = get_vars();
    int8_t idx = 0;
    for(; idx < count; idx++)
    {
        if (idx >= 9 || (var + idx) >= VARS_CNT)
        {
            break;
        }   
        out[idx] = vars[var+idx];
    }
    
    return idx > 0 ? idx : CMD_ADDR_NOT_FOUND;
}

static uint8_t blob[256] = {0};

int8_t on_int_wr(uint16_t var, uint16_t val)
{
    uint16_t* vars = get_vars();
    if (var >= VARS_CNT)
    {
        return CMD_ADDR_NOT_FOUND;
    }
    
    vars[var] = val;
    return CMD_OK;
}


void on_blob_rd(uint32_t addr, uint8_t buf[16])
{//TODO read from flash
 //shift is not needed	; 32 Mbit
	uint8_t m[16];
	uint32_t as;
    if(addr < sizeof(blob))
    {
        memcpy(buf, &blob[addr], 16);
    }
//	  user_spi_flash_init(SPI_FLASH_GPIO_MAP);
//    spi_flash_read_data(m, addr+SPI_FLASH_ADDR_START_RECORD_ADC,
//                           16, &as);	
//		memcpy(buf, m, 16);
}

int8_t on_blob_wr(uint32_t addr, uint8_t buf[16])
{
    if(addr < sizeof(blob))
    {
        memcpy(&blob[addr], buf, 16);
    }

    return 0;
}
