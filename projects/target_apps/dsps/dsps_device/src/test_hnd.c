#include "test_hnd.h"
#include "cmd_hnd.h"

#define VARS_CNT 32

static uint16_t vars[VARS_CNT] = {0};
static uint8_t blob[256] = {0};

int8_t on_int_rd(uint16_t var, uint16_t count, uint16_t out[9]) 
{
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


int8_t on_int_wr(uint16_t var, uint16_t val)
{
    if (var >= VARS_CNT)
    {
        return CMD_ADDR_NOT_FOUND;
    }
    
    vars[var] = val;
    return CMD_OK;
}


void on_blob_rd(uint32_t addr, uint8_t buf[16])
{
    if(addr < sizeof(blob))
    {
        memcpy(buf, &blob[addr], 16);
    }
}

int8_t on_blob_wr(uint32_t addr, uint8_t buf[16])
{
    if(addr < sizeof(blob))
    {
        memcpy(&blob[addr], buf, 16);
    }
    
    return 0;
}

