/******************************************************************************
* File Name: beacon_utils.c
*
* Description:This is the source code for the eddystone utility
* functions
*
*******************************************************************************/
/*******************************************************************************
* (c) 2020, Cypress Semiconductor Corporation. All rights reserved.
*******************************************************************************
* This software, including source code, documentation and related materials
* ("Software"), is owned by Cypress Semiconductor Corporation or one of its
* subsidiaries ("Cypress") and is protected by and subject to worldwide patent
* protection (United States and foreign), United States copyright laws and
* international treaty provisions. Therefore, you may use this Software only
* as provided in the license agreement accompanying the software package from
* which you obtained this Software ("EULA").
*
* If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
* non-transferable license to copy, modify, and compile the Software source
* code solely for use in connection with Cypress's integrated circuit products.
* Any reproduction, modification, translation, compilation, or representation
* of this Software except as specified above is prohibited without the express
* written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
* reserves the right to make changes to the Software without notice. Cypress
* does not assume any liability arising out of the application or use of the
* Software or any product or circuit described in the Software. Cypress does
* not authorize its products for use in any products where a malfunction or
* failure of the Cypress product may reasonably be expected to result in
* significant property damage, injury or death ("High Risk Product"). By
* including Cypress's product in a High Risk Product, the manufacturer of such
* system or application assumes all risk of such use and in doing so agrees to
* indemnify Cypress against all liability.
*******************************************************************************/

#include "wiced_bt_stack.h"
#include "beacon_utils.h"


/**************************************************************************************************
* Function Name: eddystone_set_data_for_uid()
***************************************************************************************************
* Summary:
*   This function creates Google Eddystone UID format advertising data
*
* Parameters:
*   eddystone_ranging_data:        Calibrated TX power
*   eddystone_namespace:       UID namespace
*   eddystone_instance:     Instance
*   adv_data:        Buffer for advertisement data out
*   adv_len:         Length of advertisement data
*
* Return:
*   None
*
**************************************************************************************************/
void eddystone_set_data_for_uid(uint8_t eddystone_ranging_data,
                                uint8_t eddystone_namespace[EDDYSTONE_UID_NAMESPACE_LEN],
                                uint8_t eddystone_instance[EDDYSTONE_UID_INSTANCE_ID_LEN],
                                uint8_t adv_data[BEACON_ADV_DATA_MAX], uint8_t *adv_len)
{
    uint8_t frame_data[EDDYSTONE_UID_FRAME_LEN];
    beacon_ble_advert_elem_t eddystone_adv_elem[EDDYSTONE_ELEM_NUM];

    /* Set common portion of the adv data */
    eddystone_set_data_common(eddystone_adv_elem, EDDYSTONE_FRAME_TYPE_UID, EDDYSTONE_UID_FRAME_LEN);

    /* Set frame data */
    frame_data[0] = EDDYSTONE_FRAME_TYPE_UID;
    frame_data[1] = eddystone_ranging_data;
    memcpy( &frame_data[2], eddystone_namespace, EDDYSTONE_UID_NAMESPACE_LEN );
    memcpy( &frame_data[12], eddystone_instance, EDDYSTONE_UID_INSTANCE_ID_LEN );

    memcpy(&(eddystone_adv_elem[2].data[2]), frame_data, EDDYSTONE_UID_FRAME_LEN);

    /* Copy the adv data to output buffer */
    beacon_set_adv_data(eddystone_adv_elem, EDDYSTONE_ELEM_NUM, adv_data, adv_len);

}

/**************************************************************************************************
* Function Name: eddystone_set_data_for_url()
***************************************************************************************************
* Summary:
*   This function creates Google Eddystone URL format advertising data
*
* Parameters:
*   tx_power:        Calibrated TX power
*   urlscheme:       URL scheme
*   encoded_url:     URL
*   adv_data:        Buffer for advertisement data out
*   adv_len:         Length of advertisement data
*
* Return:
*   None
*
**************************************************************************************************/
void eddystone_set_data_for_url(uint8_t tx_power,
                                uint8_t urlscheme,
                                uint8_t encoded_url[EDDYSTONE_URL_VALUE_MAX_LEN],
                                uint8_t adv_data[BEACON_ADV_DATA_MAX], uint8_t *adv_len)
{
    uint8_t frame_data[EDDYSTONE_URL_FRAME_LEN];
    uint8_t len = strlen((char *)encoded_url);
    beacon_ble_advert_elem_t eddystone_adv_elem[EDDYSTONE_ELEM_NUM];

    /* Set common portion of the adv data */
    eddystone_set_data_common(eddystone_adv_elem, EDDYSTONE_FRAME_TYPE_URL, len + 3);

    /* Set frame data */
    frame_data[0] = EDDYSTONE_FRAME_TYPE_URL;
    frame_data[1] = tx_power;
    frame_data[2] = urlscheme;
    memcpy(&frame_data[3], encoded_url, len);

    memcpy(&(eddystone_adv_elem[2].data[2]), frame_data, len+3);

    /* Copy the adv data to output buffer */
    beacon_set_adv_data(eddystone_adv_elem, EDDYSTONE_ELEM_NUM, adv_data, adv_len);
}

/**************************************************************************************************
* Function Name: eddystone_set_data_common()
***************************************************************************************************
* Summary:
*   This function sets up data common for all Eddystone frames
*
* Parameters:
*   eddystone_adv_elem:        Buffer to hold the ADV data
*   frame_type:                Type of frame
*   frame_len:                 Length of frame
*
* Return:
*   None
*
**************************************************************************************************/
void eddystone_set_data_common(beacon_ble_advert_elem_t *eddystone_adv_elem,
                                                       uint8_t frame_type, uint8_t frame_len)
{
    uint8_t flag = BTM_BLE_GENERAL_DISCOVERABLE_FLAG | BTM_BLE_BREDR_NOT_SUPPORTED;
    uint8_t eddyston_uuid[LEN_UUID_16] = { BIT16_TO_8(EDDYSTONE_UUID16) };

    /* First adv element */
    eddystone_adv_elem[0].len = 2;
    eddystone_adv_elem[0].advert_type = BTM_BLE_ADVERT_TYPE_FLAG;
    eddystone_adv_elem[0].data[0] = flag;

    /* Second adv element */
    eddystone_adv_elem[1].len = 3;
    eddystone_adv_elem[1].advert_type = BTM_BLE_ADVERT_TYPE_16SRV_COMPLETE;
    memcpy(eddystone_adv_elem[1].data, eddyston_uuid, 2);

    /* Third adv element (partial), rest is frame specific */
    eddystone_adv_elem[2].len = frame_len + 3;  // frame_len + advert_type (1) + uuid (2)
    eddystone_adv_elem[2].advert_type = BTM_BLE_ADVERT_TYPE_SERVICE_DATA;
    eddystone_adv_elem[2].data[0] = eddyston_uuid[0];
    eddystone_adv_elem[2].data[1] = eddyston_uuid[1];
}

/**************************************************************************************************
* Function Name: beacon_set_adv_data()
***************************************************************************************************
* Summary:
*   This function sets up data common for all Eddystone frames
*
* Parameters:
*   beacon_adv_elem:        Buffer containing ADV data
*   num_elem:               Number of elements in the ADV data structure
*   adv_data:               Output data buffer
*   adv_len:                Length of output data
*
* Return:
*   None
*
**************************************************************************************************/
void beacon_set_adv_data(beacon_ble_advert_elem_t *beacon_adv_elem, uint8_t num_elem,
                                  uint8_t adv_data[BEACON_ADV_DATA_MAX], uint8_t *adv_len)
{
    uint8_t structure_index = 0, buffer_index = 0;
    for (structure_index = 0; structure_index < num_elem; structure_index++)
    {
        adv_data[buffer_index++] = beacon_adv_elem[structure_index].len;
        adv_data[buffer_index++] = beacon_adv_elem[structure_index].advert_type;
        memcpy(&adv_data[buffer_index], beacon_adv_elem[structure_index].data,
            (beacon_adv_elem[structure_index].len - 1)); /* len -1 for advert_type */
        buffer_index += beacon_adv_elem[structure_index].len - 1;
    }
    *adv_len = buffer_index;
}


/* [] END OF FILE */
