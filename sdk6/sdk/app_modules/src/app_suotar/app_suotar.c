/**
 ****************************************************************************************
 *
 * @file app_suotar.c
 *
 * @brief SUOTA Reporter Application entry point
 *
 * Copyright (C) 2013-2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup APP
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwble_config.h"

#if (BLE_SUOTA_RECEIVER)

#include <stdint.h>
#include "app_api.h"                 // application task definitions
#include "suotar_task.h"             // SUOTA functions
#include "app_suotar.h"
#include "gpio.h"
#if (!SUOTAR_SPI_DISABLE)
#include "spi.h"
#include "spi_flash.h"
#endif // (SUOTAR_SPI_DISABLE)
#if (!SUOTAR_I2C_DISABLE)
#include "i2c_eeprom.h"
#endif

#include "user_periph_setup.h"

#if (BLE_APP_KEYBOARD)
#include "app_kbd_fsm.h"
#endif

#include "user_callback_config.h"
#include "app_prf_perm_types.h"

#include "user_config_storage.h"
#ifdef CFG_CONFIG_STORAGE
#include "user_config_common.h"
#endif
/*
 * GLOBAL VARIABLES
 ****************************************************************************************
 */

//application SUOTA state structure
app_suota_state suota_state __SECTION_ZERO("retention_mem_area0"); //@RETENTION MEMORY

__ALIGNED(4) uint8_t suota_all_pd[SUOTA_OVERALL_PD_SIZE] __SECTION_ZERO("retention_mem_area0"); // word aligned buffer to read the patch to before patch execute

/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
*/
#if (!SUOTAR_I2C_DISABLE)
void app_suotar_i2c_config(i2c_gpio_config_t *i2c_conf);
#endif

int app_read_ext_mem( uint8_t *, uint32_t, uint32_t );
int app_read_image_headers(uint8_t, uint8_t*, uint32_t );
uint8_t app_set_image_valid_flag(void);
uint8_t app_get_image_id(uint8_t);
uint8_t app_find_old_img(uint8_t, uint8_t);

extern void platform_reset(uint32_t error);
 /**
 ****************************************************************************************
 * SUOTAR Application Functions
 ****************************************************************************************
 */

void app_suotar_init(void)
{
    //suota_state.status_ind_func = app_suotar_status;
    suota_state.suota_pd_idx = 0;
    suota_state.suota_block_idx = 0;
    suota_state.suota_img_idx = 0;
    suota_state.new_patch_len = 0;
    suota_state.crc_calc = 0;
    memset( suota_all_pd, 0x00, sizeof(uint32_t)); // Set first WORD to 0x00
    suota_state.mem_dev = SUOTAR_MEM_INVAL_DEV;
}

void app_suotar_reset(void)
{
    suota_state.suota_pd_idx = 0;
    suota_state.suota_block_idx = 0;
    suota_state.new_patch_len = 0;
}

void app_suotar_create_db(void)
{
    struct suotar_db_cfg* db_cfg;

    struct gapm_profile_task_add_cmd *req = KE_MSG_ALLOC_DYN(GAPM_PROFILE_TASK_ADD_CMD,
                                                             TASK_GAPM,
                                                             TASK_APP,
                                                             gapm_profile_task_add_cmd,
                                                             sizeof(struct suotar_db_cfg));

    // Fill message
    req->operation = GAPM_PROFILE_TASK_ADD;
    req->sec_lvl = get_user_prf_srv_perm(TASK_ID_SUOTAR);
    req->prf_task_id = TASK_ID_SUOTAR;
    req->app_task = TASK_APP;
    req->start_hdl = 0;

    // Set parameters
    db_cfg = (struct suotar_db_cfg*) req->param;
    db_cfg->features = 0;

    // Send the message
    ke_msg_send(req);
}

void app_suotar_read_mem(uint32_t mem_dev, uint32_t* mem_info)
{
    // *mem_info = 0;

    uint8_t mem_dev_cmd = (uint8_t) ((mem_dev & SUOTAR_READ_MEM_DEV_TYPE) >> 24);

    // If valid memory device, store memory device type and base address
    if( mem_dev_cmd < SUOTAR_MEM_INVAL_DEV )
    {
        suota_state.mem_dev = (uint8_t) ((mem_dev & SUOTAR_READ_MEM_DEV_TYPE) >> 24);
        suota_state.mem_base_add = mem_dev & SUOTAR_READ_MEM_BASE_ADD;

        // Read memory device and set memory info only for internal RAM deviced. External devices need
        // GPIO configuration first before the memory info can be returned
        if( (suota_state.mem_dev == SUOTAR_MEM_INT_SYSRAM) ||
            (suota_state.mem_dev == SUOTAR_MEM_INT_RETRAM) )
        {
            // SYSRAM patch base address is the addres of the last index of the suota_all_pd array
            suota_state.mem_base_add = (uint32_t)(suota_all_pd + (SUOTA_OVERALL_PD_SIZE - sizeof(uint32_t)));
        }

        // Store requested image bank
        if( (suota_state.mem_dev == SUOTAR_IMG_I2C_EEPROM) ||
            (suota_state.mem_dev == SUOTAR_IMG_SPI_FLASH) )
        {
            if(suota_state.mem_base_add <= 2)
            {
                suota_state.suota_image_bank = suota_state.mem_base_add;
            }
            else
            {
                // Invalid image bank
                suotar_send_status_update_req((uint8_t) SUOTAR_INVAL_IMG_BANK);
                return;
            }
        }
    }

    switch( mem_dev_cmd )
    {
        case SUOTAR_IMG_I2C_EEPROM:
        case SUOTAR_IMG_SPI_FLASH:
            // Initialise SUOTA Receiver app
            app_suotar_start();
            break;

        case SUOTAR_IMG_INT_SYSRAM:
        case SUOTAR_IMG_INT_RETRAM:
            // No support for SUOTA for SYSRAM/RETRAM
            suotar_send_status_update_req((uint8_t) SUOTAR_INVAL_MEM_TYPE);
            break;

        case SUOTAR_MEM_SERVICE_EXIT:
            app_suotar_stop();
            app_suotar_reset();
            // Initiator requested to exit service. Send notification to initiator
            suotar_send_status_update_req((uint8_t) SUOTAR_SRV_EXIT);
            break;

        case SUOTAR_IMG_END:
        {
            uint8_t ret;
            // Initiator requested to exit service. Calculate CRC if succesfull, send notification to initiator
            if(  suota_state.crc_calc != 0 )
            {
                suotar_send_status_update_req((uint8_t) SUOTAR_CRC_ERR);
            }
            else
            {
                ret = app_set_image_valid_flag();
                suotar_send_status_update_req((uint8_t) ret);
            }
            app_suotar_stop();
            app_suotar_reset();
            break;
        }

        case SUOTAR_REBOOT:
        {
            if (app_env[0].conidx == GAP_INVALID_CONIDX)
            {
                // Reboot now since peer has already disconnected
                platform_reset(RESET_AFTER_SUOTA_UPDATE);
            } else {
                // Reboot is requested
                suota_state.reboot_requested = 1;

                // Send disconnect command
                app_easy_gap_disconnect(app_env[0].conidx);
            }

            // It's up to user's code do a reset and boot the new image when the initiator
            // disconnects. User's code in the 'user_app_on_disconnect' callback can
            // test the 'suota_state.reboot_requested' flag and act accordingly.
            break;
        }

        default:
            suotar_send_status_update_req((uint8_t) SUOTAR_INVAL_MEM_TYPE);
            suota_state.mem_dev = SUOTAR_MEM_INVAL_DEV;
            *mem_info = 0;
            break;
    }
}

void on_suotar_status_change(const uint8_t suotar_event)
{
#if (!SUOTAR_SPI_DISABLE)
#if 1

    if (suotar_event == SUOTAR_END)
    {
        // Power down SPI Flash
        spi_flash_power_down();
    }
    user_spi_flash_init(SPI_FLASH_GPIO_MAP);

#else
    int8_t man_dev_id = 0;

    man_dev_id = spi_flash_enable(SPI_EN_GPIO_PORT, SPI_EN_GPIO_PIN);
    if (man_dev_id == SPI_FLASH_AUTO_DETECT_NOT_DETECTED)
    {
        // The device was not identified. The default parameters are used.
        // Alternatively, an error can be asserted here.
        spi_flash_init(SPI_FLASH_DEFAULT_SIZE, SPI_FLASH_DEFAULT_PAGE);
    }

    if( suotar_event == SUOTAR_END )
    {
        // Power down SPI Flash
        spi_flash_power_down();
    }

#endif
#endif
}

/**
 ****************************************************************************************
 * @brief Starts SUOTAR service and disables sleep.
 *
 * @param[in]   void
 *
 * @return      void
 ****************************************************************************************
 */
void app_suotar_start(void)
{
    //app_force_active_mode();
    CALLBACK_ARGS_1(user_app_suotar_cb.on_suotar_status_change, SUOTAR_START);

    if( SUOTAR_IS_FOR_IMAGE( suota_state.mem_dev ) )
    {
        suotar_send_status_update_req((uint8_t) SUOTAR_IMG_STARTED);
    }
    else
    {
        suotar_send_status_update_req((uint8_t) SUOTAR_INVAL_MEM_TYPE);
    }

    // Initialise indexes
    suota_state.suota_pd_idx = 0;
    suota_state.suota_block_idx = 0;
    suota_state.suota_img_idx = 0;
    suota_state.new_patch_len = 0;
    suota_state.crc_calc = 0;
}

void app_suotar_stop(void)
{
#if (!SUOTAR_I2C_DISABLE)
    if( suota_state.mem_dev == SUOTAR_MEM_I2C_EEPROM ){
        i2c_eeprom_release();
    }
#endif
#if (!SUOTAR_SPI_DISABLE)
    if( suota_state.mem_dev == SUOTAR_MEM_SPI_FLASH ) {
        spi_release();
    }
#endif
    // Set memory device to invalid type so that service will not
    // start until the memory device is explicitly set upon service start
    suota_state.mem_dev = SUOTAR_MEM_INVAL_DEV;

    CALLBACK_ARGS_1(user_app_suotar_cb.on_suotar_status_change, SUOTAR_END);
    //app_restore_sleep_mode();
}

void suotar_send_status_update_req( uint8_t status )
{
    // Inform SUOTAR task.
    struct suotar_status_update_req *req = KE_MSG_ALLOC(SUOTAR_STATUS_UPDATE_REQ,
                      prf_get_task_from_id(TASK_ID_SUOTAR), TASK_APP,
                      suotar_status_update_req);

    // req->conhdl = active_conhdl;
    req->status = status;

    // Send the message
    ke_msg_send(req);
}

void suotar_send_mem_info_update_req(uint32_t mem_info)
{
    // Inform SUOTAR task.
    struct suotar_patch_mem_info_update_req *req = KE_MSG_ALLOC(SUOTAR_PATCH_MEM_INFO_UPDATE_REQ,
                      prf_get_task_from_id(TASK_ID_SUOTAR), TASK_APP,
                      suotar_patch_mem_info_update_req);

    req->mem_info = mem_info;

    // Send the message
    ke_msg_send(req);
}

void app_suotar_img_hdlr(void)
{
    uint32_t mem_info;
    uint16_t status = SUOTAR_CMP_OK;
#if (!SUOTAR_I2C_DISABLE)
    i2c_gpio_config_t i2c_conf;
#endif

    int32_t    ret;
    uint32_t     i;

    // Update CRC
    for(i=0;i<suota_state.suota_block_idx;i++){
        suota_state.crc_calc ^= suota_all_pd[i];
    }

    // Check mem dev.
    switch (suota_state.mem_dev)
    {
        case SUOTAR_IMG_I2C_EEPROM:
        {
#if (!SUOTAR_I2C_DISABLE)
            app_suotar_i2c_config(&i2c_conf);

            // Update address from received message
            i2c_eeprom_update_slave_address(i2c_conf.slave_addr);

            // Initialize I2C EEPROM
            i2c_eeprom_initialize();

            // When the first block is received, read image header first
            if( suota_state.suota_block_idx != 0 && suota_state.suota_img_idx == 0 )
            {
                // Read image headers and determine active image.
                ret = app_read_image_headers( suota_state.suota_image_bank, suota_all_pd, suota_state.suota_block_idx );//RDD i2c
                if( ret != IMAGE_HEADER_OK )
                {
                    status = ret;
                }
                else
                {
                    // Update block index
                    suota_state.suota_img_idx += suota_state.suota_block_idx;
                }
            }
            else
            {
                //check file size
                if (( suota_state.suota_image_len+ADDITINAL_CRC_SIZE ) >= ( suota_state.suota_img_idx + suota_state.suota_block_idx ))
                {
                    if (suota_state.suota_image_len < (suota_state.suota_img_idx + suota_state.suota_block_idx))
                        suota_state.suota_block_idx = suota_state.suota_image_len - suota_state.suota_img_idx;

                    i2c_eeprom_write_data(suota_all_pd, (suota_state.mem_base_add + suota_state.suota_img_idx), suota_state.suota_block_idx, &ret);
                    if( ret !=  suota_state.suota_block_idx){
                        status = SUOTAR_EXT_MEM_WRITE_ERR;//rdd i2c
                    }
                    else
                    {
                        // Update block index
                        suota_state.suota_img_idx += suota_state.suota_block_idx;
                    }
                }
                else
                {
                    status = SUOTAR_EXT_MEM_WRITE_ERR;//rdd i2c
                }
            }
            suota_state.suota_block_idx = 0;
            mem_info = suota_state.suota_img_idx;
            suotar_send_mem_info_update_req(mem_info);
#else
            status = SUOTAR_INVAL_MEM_TYPE;
#endif // (!SUOTAR_I2C_DISABLE)
            break;
        }
        case SUOTAR_IMG_SPI_FLASH:
        {
#if (!SUOTAR_SPI_DISABLE)
            user_spi_flash_init(suota_state.gpio_map);

            // When the first block is received, read image header first
            if( suota_state.suota_block_idx != 0 && suota_state.suota_img_idx == 0 )
            {
                // Read image headers and determine active image.
                ret = app_read_image_headers( suota_state.suota_image_bank, suota_all_pd, suota_state.suota_block_idx );//rdd SPI3 
                if( ret != IMAGE_HEADER_OK )
                {
                    status = ret;
                }
                else
                {
                    // Update block index
                    suota_state.suota_img_idx += suota_state.suota_block_idx;
                }
            }
            else
            {
                //check file size
                if (( suota_state.suota_image_len+ADDITINAL_CRC_SIZE ) >= ( suota_state.suota_img_idx + suota_state.suota_block_idx ))
                {
                    if (suota_state.suota_image_len < (suota_state.suota_img_idx + suota_state.suota_block_idx))
                        suota_state.suota_block_idx = suota_state.suota_image_len - suota_state.suota_img_idx;

                    ret = user_flash_write_data (suota_all_pd, (suota_state.mem_base_add + suota_state.suota_img_idx), suota_state.suota_block_idx);
                    if(ret < 0){
                        status = SUOTAR_EXT_MEM_WRITE_ERR;//rdd spi 1.0
                    }
                    else
                    {
                        // Update block index
                        suota_state.suota_img_idx += suota_state.suota_block_idx;
                    }
                }
                else
                {
                    status = SUOTAR_EXT_MEM_WRITE_ERR;//rdd spi 1.1
                }
            }
            suota_state.suota_block_idx = 0;
            mem_info = suota_state.suota_img_idx;
            suotar_send_mem_info_update_req(mem_info);
#else
            status = SUOTAR_INVAL_MEM_TYPE;
#endif //(!SUOTAR_SPI_DISABLE)
            break;
        }
        default:
        {
            status = SUOTAR_INVAL_MEM_TYPE;
            break;
        }
    }

    // SUOTA finished successfully. Send Indication to initiator
    suotar_send_status_update_req((uint8_t) status);
}

#if (!SUOTAR_I2C_DISABLE)
/**
 ****************************************************************************************
 * @brief Reserves I2C GPIO pins, reads the GPIO map set by initiator, and configures the the I2C pins.
 *
 * @param[in]   i2c_conf:  pointer to port/pin pad structure.
 ****************************************************************************************
 */
void app_suotar_i2c_config(i2c_gpio_config_t *i2c_conf)
{
    i2c_conf->sda.port      = ((GPIO_PORT)((suota_state.gpio_map & 0x000000f0) >>  4));
    i2c_conf->sda.pin       = ((GPIO_PIN) ((suota_state.gpio_map & 0x0000000f)));
    i2c_conf->scl.port      = ((GPIO_PORT)((suota_state.gpio_map & 0x0000f000) >> 12));
    i2c_conf->scl.pin       = ((GPIO_PIN) ((suota_state.gpio_map & 0x00000f00) >> 8));
    i2c_conf->slave_addr    = ((suota_state.gpio_map & 0xffff0000) >> 16);
#if DEVELOPMENT_DEBUG
#if (!BLE_APP_KEYBOARD)
    RESERVE_GPIO( I2C_SCL, i2c_conf->scl.port, i2c_conf->scl.pin, PID_I2C_SCL);
    RESERVE_GPIO( I2C_SDA, i2c_conf->sda.port, i2c_conf->sda.pin, PID_I2C_SDA);
#endif
#endif

    GPIO_ConfigurePin(i2c_conf->scl.port, i2c_conf->scl.pin, INPUT, PID_I2C_SCL, false);
    GPIO_ConfigurePin(i2c_conf->sda.port, i2c_conf->sda.pin, INPUT, PID_I2C_SDA, false);
}
#endif //(!SUOTAR_I2C_DISABLE)

/**
 ****************************************************************************************
 * @brief This function is called when the first SUOTA block is received.
 *        Firstly, the image header is extracted from the first block, then the external memmory
 *        is checked to determin where to write this new image and finaly the header and the first
 *        image data are written to external memory.
 *
 * @param[in]   bank:      User can select bank=1 or bank=2 to force where to write the image
 *                         bypassing the check to update the oldest image in the ext memory
 *              data:      Points to the first data block received over SUOTAR protocol
 *              data_len:  Length of the data block
 *
 * @return      0 for success, otherwise error codes
 *
 ****************************************************************************************
 */
int app_read_image_headers(uint8_t image_bank, uint8_t* data, uint32_t data_len)
{
    image_header_t *pImageHeader;
    config_product_header_t *pProductHeader;
    image_header_t *pfwHeader;
    uint32_t codesize;
    int32_t ret;
    uint8_t mem_data_buff[64];
    uint32_t imageposition1;
    uint32_t imageposition2;
    uint8_t new_bank = ANY_IMAGE_BANK;
    uint8_t is_invalid_image1 = IMAGE_HEADER_OK;
    uint8_t is_invalid_image2 = IMAGE_HEADER_OK;
    uint8_t imageid  = IMAGE_ID_0;
    uint8_t imageid1 = IMAGE_ID_0;
    uint8_t imageid2 = IMAGE_ID_0;


    if( data_len < sizeof(image_header_t) )
    {
        // block size should be at least image header size
        return SUOTAR_INVAL_IMG_HDR;
    }
    else
    {
        // read header form first data block
        pfwHeader = (image_header_t*)data;
    }

    // read product header
    pProductHeader = (config_product_header_t*) mem_data_buff;

    ret = app_read_ext_mem( (uint8_t*)mem_data_buff, (unsigned long)PRODUCT_HEADER_POSITION, (unsigned long)sizeof(config_product_header_t) );
    if(ret < 0)
        return SUOTAR_EXT_MEM_READ_ERR;

    // verify product header
    if( (pProductHeader->signature[0] != PRODUCT_HEADER_SIGNATURE1) ||
        (pProductHeader->signature[1] != PRODUCT_HEADER_SIGNATURE2)    )
    {
        return SUOTAR_INVAL_PRODUCT_HDR;
    }
#ifdef CFG_CONFIG_STORAGE
    if( (pfwHeader->signature[0] == CONFIGURATION_HEADER_SIGNATURE1) && (pfwHeader->signature[1] == CONFIGURATION_HEADER_SIGNATURE2) &&
        (pfwHeader->validflag == STATUS_VALID_CONFIG) )
    {
        uint32_t len;
        suota_state.suota_image_len = ((user_config_struct_header_t*)pfwHeader)->data_size + sizeof(user_config_struct_header_t);
        if (current_config_struct_index == 1)
            suota_state.mem_base_add = pProductHeader->config_offset2;
        else
            suota_state.mem_base_add = pProductHeader->config_offset1;

        pfwHeader->imageid = current_config_struct_id + 1;
        pfwHeader->validflag = STATUS_INVALID_CONFIG;
        len = (data_len == suota_state.suota_image_len + ADDITINAL_CRC_SIZE) ? suota_state.suota_image_len : data_len;
        ret = user_flash_write_data((uint8_t*)pfwHeader, suota_state.mem_base_add, len);
        if (ret < 0)
            return SUOTAR_EXT_MEM_WRITE_ERR;//rdd spi 2

        current_config_struct_id++;
        current_config_struct_index++;
        if (current_config_struct_index == 3) current_config_struct_index = 1;

        return IMAGE_HEADER_OK;
    }
    else
#endif
    // check firmware header
    if( (pfwHeader->signature[0] != IMAGE_HEADER_SIGNATURE1) || (pfwHeader->signature[1] != IMAGE_HEADER_SIGNATURE2) )
    {
        return SUOTAR_INVAL_IMG_HDR;
    }
    // Get image size
    codesize = pfwHeader->code_size;
    suota_state.suota_image_len = pfwHeader->code_size+sizeof(image_header_t);

    // Store the start addresses of both images positions
    imageposition1 = pProductHeader->offset1;
    imageposition2 = pProductHeader->offset2;

    // verify image positions
    // the code size must be less than the space between images
    if (!codesize || codesize > (imageposition2 - imageposition1 - sizeof(image_header_t)))
    {
        return SUOTAR_INVAL_IMG_SIZE;
    }

    pImageHeader = (image_header_t*)mem_data_buff;

    // read first image header, image id must be stored for the bank selection
    ret = app_read_ext_mem( (uint8_t*)pImageHeader, (unsigned long)imageposition1, (unsigned long)sizeof(image_header_t) );
    if(ret < 0) return SUOTAR_EXT_MEM_READ_ERR;

    imageid1 = pImageHeader->imageid;
    if( (pImageHeader->validflag != STATUS_VALID_IMAGE)         ||
        (pImageHeader->signature[0] != IMAGE_HEADER_SIGNATURE1) ||
        (pImageHeader->signature[1] != IMAGE_HEADER_SIGNATURE2)    )
    {
        is_invalid_image1 = IMAGE_HEADER_INVALID;
        imageid1 = IMAGE_ID_0;
    } else {
        //compare version string and timestamp
        if (!memcmp(pfwHeader->version, pImageHeader->version, IMAGE_HEADER_VERSION_SIZE) && (pfwHeader->timestamp==pImageHeader->timestamp))
        {
            is_invalid_image1 = IMAGE_HEADER_SAME_VERSION;
        }
    }

    // read second image header, image id must be stored for the bank selection
    ret = app_read_ext_mem((uint8_t*)pImageHeader, (unsigned long)imageposition2, (unsigned long)sizeof(image_header_t));
    if(ret < 0) return SUOTAR_EXT_MEM_READ_ERR;

    imageid2=pImageHeader->imageid;
    if( (pImageHeader->validflag != STATUS_VALID_IMAGE)         ||
        (pImageHeader->signature[0] != IMAGE_HEADER_SIGNATURE1) ||
        (pImageHeader->signature[1] != IMAGE_HEADER_SIGNATURE2)    )
    {
        is_invalid_image2 = IMAGE_HEADER_INVALID;
        imageid2 = IMAGE_ID_0;
    } else {
        //compate version string and timestamp
        if (!memcmp(pfwHeader->version, pImageHeader->version, IMAGE_HEADER_VERSION_SIZE) && (pfwHeader->timestamp==pImageHeader->timestamp))
        {
            is_invalid_image2 = IMAGE_HEADER_SAME_VERSION;
        }
    }

    if (image_bank == ANY_IMAGE_BANK ||  image_bank > SECOND_IMAGE_BANK)
    {
        if (is_invalid_image1 == IMAGE_HEADER_INVALID) new_bank = FISRT_IMAGE_BANK;
        else if (is_invalid_image2 == IMAGE_HEADER_INVALID) new_bank = SECOND_IMAGE_BANK;
        else new_bank = app_find_old_img(imageid1, imageid2);
    }
    else
    {
        new_bank = image_bank;
    }

    memset(mem_data_buff, 0xFF, sizeof(mem_data_buff));
    if (new_bank == SECOND_IMAGE_BANK)
    {
        if (is_invalid_image2==IMAGE_HEADER_SAME_VERSION) return SUOTAR_SAME_IMG_ERR;
        suota_state.mem_base_add = imageposition2;

        if (is_invalid_image1 == IMAGE_HEADER_OK || is_invalid_image1 == IMAGE_HEADER_SAME_VERSION)
            imageid = app_get_image_id(imageid1);
        else
            imageid = IMAGE_ID_1;
    }
    else
    {
        if (is_invalid_image1==IMAGE_HEADER_SAME_VERSION) return SUOTAR_SAME_IMG_ERR;
        suota_state.mem_base_add = imageposition1;

        if (is_invalid_image2 == IMAGE_HEADER_OK || is_invalid_image2 == IMAGE_HEADER_SAME_VERSION)
            imageid = app_get_image_id(imageid2);
        else
            imageid = IMAGE_ID_1;
    }

    // Erase header and image
    if( suota_state.mem_dev == SUOTAR_IMG_SPI_FLASH )
    {
#if (!SUOTAR_SPI_DISABLE)

    #if (!SUOTAR_SPI_DELATE_SECTOR_ERASE)
        ret = user_erase_flash_sectors(suota_state.mem_base_add, codesize + CODE_OFFSET);
    #else
        ret = user_erase_flash_sectors(suota_state.mem_base_add, 1);
    #endif

    if( ret != SPI_FLASH_ERR_OK) return SUOTAR_EXT_MEM_WRITE_ERR;//rdd spi 3.0
#else
        return SUOTAR_EXT_MEM_WRITE_ERR; //rdd spi 3.1
#endif
    }
    else
    {
#if (!SUOTAR_I2C_DISABLE)
        uint32_t ret_i2c;
        // For i2c eeprom, no need to delete entire image, just write 0xff to the header to invalidate image
        if (i2c_eeprom_write_data(mem_data_buff, suota_state.mem_base_add, sizeof(mem_data_buff), &ret_i2c) != I2C_NO_ERROR)
        {
            ret = -1;
        }
        if( ret_i2c !=  CODE_OFFSET ) return SUOTAR_EXT_MEM_WRITE_ERR;//rdd spi 3.2
#else
        return SUOTAR_EXT_MEM_WRITE_ERR; //rdd spi 3.3
#endif
    }

    //write header apart from validflag. set validflag when the entire image has been received
    pImageHeader->imageid = imageid;
    pImageHeader->code_size= pfwHeader->code_size;
    pImageHeader->CRC=pfwHeader->CRC;
    memcpy(pImageHeader->version,pfwHeader->version, IMAGE_HEADER_VERSION_SIZE);
    pImageHeader->timestamp=pfwHeader->timestamp;
    pImageHeader->signature[0]=pfwHeader->signature[0];
    pImageHeader->signature[1]=pfwHeader->signature[1];
    pImageHeader->encryption = pfwHeader->encryption;

    // write image and header
    if( suota_state.mem_dev == SUOTAR_IMG_SPI_FLASH )
    {
#if (!SUOTAR_SPI_DISABLE)
        // write header
        ret = user_flash_write_data((uint8_t*)pImageHeader, suota_state.mem_base_add, sizeof(image_header_t));
        if(ret < 0) 
					           return SUOTAR_EXT_MEM_WRITE_ERR; //rdd spi 3.4

        ret = user_flash_write_data((uint8_t*) &data[CODE_OFFSET], suota_state.mem_base_add + CODE_OFFSET, data_len - CODE_OFFSET);
        if(ret < 0) 
					           return SUOTAR_EXT_MEM_WRITE_ERR; // rdd spi 3.5
#else
        return SUOTAR_EXT_MEM_WRITE_ERR; //rdd spi 3.6
#endif
    }
    else
    {
#if (!SUOTAR_I2C_DISABLE)
        uint32_t ret_i2c;
        // write header
        if (i2c_eeprom_write_data((uint8_t*)pImageHeader, suota_state.mem_base_add, sizeof(image_header_t), &ret_i2c) != I2C_NO_ERROR)
        {
            ret = -1;
        }
        if( ret_i2c != sizeof(image_header_t) ) return SUOTAR_EXT_MEM_WRITE_ERR; //rdd i2c

        if (i2c_eeprom_write_data((uint8_t*) &data[CODE_OFFSET], suota_state.mem_base_add + CODE_OFFSET, data_len - CODE_OFFSET, &ret_i2c) != I2C_NO_ERROR)
        {
            ret = -1;
        }
        if( ret_i2c != (data_len - CODE_OFFSET) ) return SUOTAR_EXT_MEM_WRITE_ERR; //rdd i2c
#else
        return SUOTAR_EXT_MEM_WRITE_ERR; //rdd i2c
#endif
    }

    return IMAGE_HEADER_OK;
}


/**
 ****************************************************************************************
 * @brief This function is called when the entire image has been received successfully
 *        to set the valid flag field on the image header and make the image valid for
 *        the bootloader.
 *
 * @return      SUOTA Status values
 *
 ****************************************************************************************
 */
uint8_t app_set_image_valid_flag(void)
{
    int8_t ret = SUOTAR_EXT_MEM_WRITE_ERR;//rdd spi 4
    uint8_t flag = STATUS_VALID_IMAGE;

    if( suota_state.mem_dev == SUOTAR_IMG_SPI_FLASH )
    {
#if (!SUOTAR_SPI_DISABLE)
        user_spi_flash_init(suota_state.gpio_map);
        ret = user_flash_write_data((uint8_t*) &flag, suota_state.mem_base_add + 2, sizeof(uint8_t));
        if (ret == 0)
            ret = SUOTAR_CMP_OK;
#endif
    }
    else
    {
#if (!SUOTAR_I2C_DISABLE)
        i2c_gpio_config_t i2c_conf;

        app_suotar_i2c_config(&i2c_conf);

        // Update address from received message
        i2c_eeprom_update_slave_address(i2c_conf.slave_addr);

        // Initialize I2C EEPROM
        i2c_eeprom_initialize();

        i2c_eeprom_write_byte( (uint32_t) (suota_state.mem_base_add + 2), flag );
        ret = SUOTAR_CMP_OK;
#endif
    }

    return ret;
}

/**
 ****************************************************************************************
 * @brief This function is called to get the correct image id to write to the image header
 *
 * @param[in]   imgid: current latest image id
 *
 * @return      new image id
 *
 ****************************************************************************************
 */
uint8_t app_get_image_id(uint8_t imgid)
{
    uint8_t new_imgid = IMAGE_ID_0;
    if (imgid == 0xff) return new_imgid;
    else {
        new_imgid = imgid + 1;
        return new_imgid;
    }
}

/**
 ****************************************************************************************
 * @brief This function is called to find the older image of the two in the external memmory
 *
 * @param[in]   imgid: current ids of the two images
 *
 * @return      older image id
 *
 ****************************************************************************************
 */
uint8_t app_find_old_img(uint8_t id1, uint8_t id2)
{
    if (id1 == 0xFF && id2 == 0xFF) return IMAGE_ID_1;
    if (id1 == 0xFF && id2 == 0) return IMAGE_ID_1;
    if (id2 == 0xFF && id1 == 0) return IMAGE_ID_2;
    if (id1 > id2) return IMAGE_ID_2;
    else return IMAGE_ID_1;
}

/**
 ****************************************************************************************
 * @brief Read data from a given starting address
 *
 * @param[in] *rd_data_ptr:  Points to the position the read data will be stored
 * @param[in] address:       Starting address of data to be read
 * @param[in] size:          Size of the data to be read
 *
 * @return  Error code
 ****************************************************************************************
 */
int app_read_ext_mem( uint8_t *rd_data_ptr, uint32_t address, uint32_t size )
{
    int ret = -1;

    if( suota_state.mem_dev == SUOTAR_IMG_SPI_FLASH )
    {
#if (!SUOTAR_SPI_DISABLE)
        uint32_t actual_size;
        ret = spi_flash_read_data( (uint8_t*)rd_data_ptr, (unsigned long)address,
             (unsigned long)size, &actual_size);
#endif
    }
    else
    {
#if (!SUOTAR_I2C_DISABLE)
        uint32_t ret_i2c;
        if (i2c_eeprom_read_data((uint8_t*)rd_data_ptr, (uint32_t)address, (uint32_t)size, &ret_i2c) != I2C_NO_ERROR)
        {
            ret = -1;
        }
        else
        {
            ret = ret_i2c;
        }
#endif
    }
    return ret;
}

#endif //BLE_SUOTA_RECEIVER

/// @} APP
