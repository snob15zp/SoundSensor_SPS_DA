#include "cmd_mux.h"

#define CMD_RSP_BIT 0x80

typedef struct __packed
{
    uint8_t nb;
    uint8_t data[19];
} cmd_t;

extern const cmd_list_t _cmd_list[];

static void cmd_err(cmd_t *cmd, size_t *sz, int8_t ret)
{
    cmd->data[0] = (uint8_t)(-ret);
    cmd->nb |= CMD_RSP_BIT;
    *sz = 2;
}

void cmd_mux(uint8_t *data, size_t *sz)
{
    cmd_t *cmd = (cmd_t *)data;
    for (size_t idx = 0; idx < 255; idx++)
    {
        if (_cmd_list[idx].nb == cmd->nb)
        {
            if (_cmd_list[idx].hnd)
            {
                int16_t ret = _cmd_list[idx].hnd(cmd->data, *sz - 1);
                *sz = ret;
                if (ret > 0)
                {
                    *sz = *sz > (sizeof(cmd->data)) ? sizeof(cmd->data) : *sz;
                    *sz += sizeof(cmd->nb);
                }
                else if (ret < 0)
                {
                    cmd_err(cmd, sz, ret);
                }
            }
            else
            {
                *sz = 0;
            }
            return;
        }

        if (_cmd_list[idx].nb == 0xff && _cmd_list[idx].hnd == NULL)
        {
            cmd_err(cmd, sz, -1);
            return;
        }
    }
}


