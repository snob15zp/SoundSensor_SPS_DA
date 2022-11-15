#ifndef __CMD_MUX_H
#define __CMD_MUX_H

#include <stddef.h>
#include <stdint.h>


#ifndef __packed
#define __packed __attribute__((packed))
#endif

#define CMD_MUX_MSG_MAX_SZ 20

/**
 * @brief command handler function prototype
 * @param data incoming data from
 * @param sz Incoming size
 * @return <0 - exception code, 0 - no response, >0 response size
 */
typedef int16_t (*cmd_hnd_t)(uint8_t* data, size_t sz);

/**
 * @brief cmd list item
 * Contains command number and command handler for it
 */
typedef struct __packed {
    uint8_t nb; /**< Command number */
    cmd_hnd_t hnd; /**< Command handler, will called once command message has been received */
} cmd_list_t;

/**
 * @brief defines a global command list array
 * @remark should start a command list
 */
#define CMD_LIST_START const cmd_list_t _cmd_list[] = {

/**
 * @brief Register a command inside command list array
 * @remark should goes after @see CMD_LIST_START or CMD_LIST_REG_HND
 */
#define CMD_LIST_REG_HND(N, H) { (uint8_t)N, H },

/**
 * @brief Ends a global command list
 * @remark should goes after @see CMD_LIST_REG_HND or @see CMD_LIST_START (if there are no commands)
 */
#define CMD_LIST_END {0xff, NULL} };

/**
 * @brief Command multiplexer
 *
 * @param data Incoming data from transport level
 * @param sz Data size, on input must contains incoming size, after will contains data size to send
 */
void cmd_mux(uint8_t* data, size_t* sz);


#endif
