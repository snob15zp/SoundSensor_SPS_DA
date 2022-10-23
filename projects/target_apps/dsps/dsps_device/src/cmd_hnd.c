#include <string.h>

#include "cmd_hnd.h"
#include "cmd_mux.h"

#define CHECK_SZ(S, T)      \
    do                      \
    {                       \
        if (S != sizeof(T)) \
            return 0;       \
    } while (0)

#ifdef __UVISION_VERSION
    extern int8_t on_int_rd(uint16_t var, uint16_t count, uint16_t out[9]);
    extern int8_t on_int_wr(uint16_t var, uint16_t val);
    extern void on_blob_rd(uint32_t addr, uint8_t buf[16]);
    extern int8_t on_blob_wr(uint32_t addr, uint8_t buf[16]);
#endif

/**
 * @brief device commands
 */
typedef enum
{
    CMD_INT_RD = 0x03,  /**< Integer read */
    CMD_INT_WR = 0x06,  /**< Integer write */
    CMD_BLOB_RD = 0x12, /**< BLOB (binary large object) read */
    CMD_BLOB_WR = 0x13, /**< BLOB (binary large object) write */
} cmds_t;

typedef struct
{
    int8_t err;
} __err_rsp_t;

typedef struct __packed
{
    uint16_t nr;
    uint16_t count;
} __int_rd_cmd_t;

typedef struct __packed
{
    uint8_t count;
    uint16_t val[9];
} __int_rd_rsp_t;

int16_t __on_int_rd(uint8_t *data, size_t sz)
{
    CHECK_SZ(sz, __int_rd_cmd_t);
    uint16_t nr = ntohs(((__int_rd_cmd_t *)data)->nr);
    uint16_t count = ntohs(((__int_rd_cmd_t *)data)->count);
    
    int8_t ret = on_int_rd(nr, count, (uint16_t *)&data[1]);
    if (ret > 9)
    {
        ret = CMD_ADDR_NOT_FOUND;
    }

    if (ret > 0)
    {
        __int_rd_rsp_t* rsp = ((__int_rd_rsp_t *)data);
        for(uint8_t idx = 0; idx<ret; idx++)
        {
           rsp->val[idx] = htons(rsp->val[idx]); 
        }
        
        rsp->count = ret * sizeof(uint16_t);
        return sizeof(uint8_t) + ret * sizeof(uint16_t);
    }

    return ret;
}

// on a normal compilers it works like a charm
#ifdef __GNUC__
__weak int8_t on_int_rd(uint16_t var, uint16_t count, uint16_t out[9]) 
{
    (void)var;
    (void)count;
    (void)out;

    return CMD_FUNC_NOT_FOUND;
}
#endif

typedef struct __packed
{
    uint16_t var;
    uint16_t val;
} __int_wr_cmd_t;

int16_t __on_int_wr(uint8_t *data, size_t sz)
{
    CHECK_SZ(sz, __int_wr_cmd_t);
    uint16_t var = ntohs(((__int_wr_cmd_t *)data)->var);
    uint16_t val = ntohs(((__int_wr_cmd_t *)data)->val);
    int8_t ret = on_int_wr(var, val);
    return ret >= 0 ? sz : ret;
}

#ifdef __GNUC__
__weak int8_t on_int_wr(uint16_t var, uint16_t val)
{
    (void)var;
    (void)val;
    return CMD_FUNC_NOT_FOUND;
}
#endif


typedef struct __packed
{
    uint8_t addr[3];
} __blob_rd_cmd_t;

typedef struct
{
    uint8_t addr[3];
    uint8_t data[16];
} __blob_rd_rsp_t;

int16_t __on_blob_rd(uint8_t *data, size_t sz)
{
    CHECK_SZ(sz, __blob_rd_cmd_t);

    uint32_t addr = ((__blob_rd_cmd_t *)data)->addr[2] | (((__blob_rd_cmd_t *)data)->addr[1] << 8) | (((__blob_rd_cmd_t *)data)->addr[0] << 16);
    addr <<= 4;
    on_blob_rd(addr, ((__blob_rd_rsp_t *)data)->data);
    return sizeof(__blob_rd_rsp_t);
}

#ifdef __GNUC__
__weak void on_blob_rd(uint32_t addr, uint8_t buf[16])
{
    (void)addr;
    memset(buf, 0, 16);
}
#endif

typedef struct __packed
{
    uint8_t addr[3];
    uint8_t data[16];
} __blob_wr_cmd_t;

int16_t __on_blob_wr(uint8_t *data, size_t sz)
{
    CHECK_SZ(sz, __blob_wr_cmd_t);

    uint32_t addr = ((__blob_wr_cmd_t *)data)->addr[2] | (((__blob_wr_cmd_t *)data)->addr[1] << 8) | (((__blob_wr_cmd_t *)data)->addr[0] << 16);
    addr <<= 4;

    int8_t ret = on_blob_wr(addr, ((__blob_wr_cmd_t *)data)->data);
    return ret >= 0 ? CMD_ACK : ret;

}

#ifdef __GNUC__
__weak int8_t on_blob_wr(uint32_t addr, uint8_t buf[16])
{
    (void)addr;
    (void)buf;
    return CMD_FUNC_NOT_FOUND;
}
#endif

CMD_LIST_START
CMD_LIST_REG_HND(CMD_INT_RD, __on_int_rd)
CMD_LIST_REG_HND(CMD_INT_WR, __on_int_wr)
CMD_LIST_REG_HND(CMD_BLOB_RD, __on_blob_rd)
CMD_LIST_REG_HND(CMD_BLOB_WR, __on_blob_wr)
CMD_LIST_END
