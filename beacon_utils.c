/******************************************************************************
* File Name: beacon_utils.c
*
* Description:This is the source code for the eddystone utility
* functions
*
*******************************************************************************
* Copyright 2020-2021, Cypress Semiconductor Corporation (an Infineon company) or
* an affiliate of Cypress Semiconductor Corporation.  All rights reserved.
*
* This software, including source code, documentation and related
* materials ("Software") is owned by Cypress Semiconductor Corporation
* or one of its affiliates ("Cypress") and is protected by and subject to
* worldwide patent protection (United States and foreign),
* United States copyright laws and international treaty provisions.
* Therefore, you may use this Software only as provided in the license
* agreement accompanying the software package from which you
* obtained this Software ("EULA").
* If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
* non-transferable license to copy, modify, and compile the Software
* source code solely for use in connection with Cypress's
* integrated circuit products.  Any reproduction, modification, translation,
* compilation, or representation of this Software except as specified
* above is prohibited without the express written permission of Cypress.
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
* including Cypress's product in a High Risk Product, the manufacturer
* of such system or application assumes all risk of such use and in doing
* so agrees to indemnify Cypress against all liability.
*******************************************************************************/

/*******************************************************************************
*        Header Files
*******************************************************************************/

#include "wiced_bt_stack.h"
#include "beacon_utils.h"

/* local data used by methods */
static beacon_ble_advert_elem_t ibeacon_adv_elem[IBEACON_ELEM_NUM];
const uint8_t ibeacon_type[ LEN_UUID_16 ] = { IBEACON_PROXIMITY };
const uint8_t ibeacon_company_id[ LEN_UUID_16 ] = { IBEACON_COMPANY_ID_APPLE };

/******************************************************************************
* Function Name:ibeacon_set_adv_data
***************************************************************************//**
*
* \brief Creates Apple iBeacon format advertising data
* \details The application calls the function to populate advertisement data
*          for Apple iBeacon advertisement format. It returns the adv data buffer and length.
*
* @param[in]   ibeacon_uuid                 Customer beacon UUID, see specification
*                                           for details for input parameters.
* @param[in]   ibeacon_major_number         Beacon major number
* @param[in]   ibeacon_minor_number         Beacon minor number
* @param[in]   tx_power_lcl                 measured power
* @param[out]  adv_data                     Buffer of advertisement data
* @param[out]  adv_len                      Length of advertisement data
*
* @return     None.
*
******************************************************************************/


void ibeacon_set_adv_data (uint8_t ibeacon_uuid[LEN_UUID_128],uint16_t ibeacon_major_number,
                           uint16_t ibeacon_minor_number,uint8_t tx_power_lcl,
                           uint8_t adv_data[BEACON_ADV_DATA_MAX],uint8_t *adv_len)

{
    uint8_t flag = BTM_BLE_GENERAL_DISCOVERABLE_FLAG|BTM_BLE_BREDR_NOT_SUPPORTED;
    uint8_t ibeacon_data[IBEACON_DATA_LENGTH];

    /* first adv element Byte 0: Length :  0x02 */
    ibeacon_adv_elem[IBEACON_ADV_INDEX0].len          = ADV_PKT_FLAG_LENGTH;
    ibeacon_adv_elem[IBEACON_ADV_INDEX0].advert_type  = BTM_BLE_ADVERT_TYPE_FLAG;
    ibeacon_adv_elem[IBEACON_ADV_INDEX0].data[IBEACON_ADV_DATA0] = flag;

    /* Second adv element */
    ibeacon_adv_elem[IBEACON_ADV_INDEX1].len          = IBEACON_ADV_PKT_LENGTH;
    ibeacon_adv_elem[IBEACON_ADV_INDEX1].advert_type  = BTM_BLE_ADVERT_TYPE_MANUFACTURER;

    /* Setting Company Identifier */
    ibeacon_data[IBEACON_DATA_INDEX0] = ibeacon_company_id[IBEACON_DATA_COMPANY_ID_INDEX0];
    ibeacon_data[IBEACON_DATA_INDEX1] = ibeacon_company_id[IBEACON_DATA_COMPANY_ID_INDEX1];

    /* Setting beacon type */
    ibeacon_data[IBEACON_DATA_INDEX2] = ibeacon_type[IBEACON_DATA_TYPE_INDEX0];
    ibeacon_data[IBEACON_DATA_INDEX3] = ibeacon_type[IBEACON_DATA_TYPE_INDEX1];

    /* Setting the ibeacon UUID in the manufacturer data */
    memcpy( &ibeacon_data[IBEACON_DATA_INDEX4], ibeacon_uuid, LEN_UUID_128 );

    /* Setting the Major field */
    ibeacon_data[IBEACON_DATA_INDEX20] = ibeacon_major_number & 0xff;
    /* shifting by 8 to move major value 1 */
    ibeacon_data[IBEACON_DATA_INDEX21] = (ibeacon_major_number >> 8) & 0xff;

    /* Setting the Minor field */
    ibeacon_data[IBEACON_DATA_INDEX22] = ibeacon_minor_number & 0xff;
    /* shifting by 8 to move minor value 2 */
    ibeacon_data[IBEACON_DATA_INDEX23] = (ibeacon_minor_number >> 8) & 0xff;

    /* Measured power */
    ibeacon_data[IBEACON_TX_POWER_INDEX] = tx_power_lcl;

    memcpy(ibeacon_adv_elem[IBEACON_ADV_INDEX1].data, ibeacon_data,
           IBEACON_DATA_LENGTH);

    /* Copy the adv data to output buffer */
    beacon_set_adv_data(ibeacon_adv_elem, IBEACON_ELEM_NUM, adv_data, adv_len);
}

/********************************************************************************
* Function Name: eddystone_set_data_for_url
*********************************************************************************
* Summary:
*   This function creates Google Eddystone URL format advertising data
*
* Parameters:
*   url_data: See structure eddystone_url_t
*
* Return:
*   None
*
********************************************************************************/
void eddystone_set_data_for_url(eddystone_url_t url_data,
                                uint8_t adv_data[BEACON_ADV_DATA_MAX],
                                uint8_t *adv_len)
{
    uint8_t len = strlen((char *)url_data.encoded_url);
    beacon_ble_advert_elem_t eddystone_adv_elem[EDDYSTONE_NUM_ELEM_URL];

    /* Set common portion of the adv data */
    eddystone_set_data_common(eddystone_adv_elem, EDDYSTONE_FRAME_TYPE_URL,
    len + EDDYSTONE_URL_COM_LENGTH);

    /* Set frame data */
    eddystone_adv_elem[EDDYSTONE_ADV_INDEX2].data[EDDYSTONE_ADV_DATA_INDEX2] =
    EDDYSTONE_FRAME_TYPE_URL;
    memcpy(&(eddystone_adv_elem[EDDYSTONE_ADV_INDEX2].data[EDDYSTONE_ADV_DATA_INDEX3]),
    &url_data,
    len+EDDYSTONE_URL_COM_LENGTH);

    /* Copy the adv data to output buffer */
    beacon_set_adv_data(eddystone_adv_elem, EDDYSTONE_NUM_ELEM_URL, adv_data, adv_len);
}

/********************************************************************************
* Function Name: eddystone_set_data_common
*********************************************************************************
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
*********************************************************************************/
void eddystone_set_data_common(beacon_ble_advert_elem_t *eddystone_adv_elem,
                                                       uint8_t frame_type,
                                                       uint8_t frame_len)
{
    uint8_t flag = BTM_BLE_GENERAL_DISCOVERABLE_FLAG | BTM_BLE_BREDR_NOT_SUPPORTED;
    uint8_t eddystone_uuid[LEN_UUID_16] = { BIT16_TO_8(EDDYSTONE_UUID16) };

    /* First adv element */
    eddystone_adv_elem[EDDYSTONE_ADV_INDEX0].len         = ADV_PKT_FLAG_LENGTH;
    eddystone_adv_elem[EDDYSTONE_ADV_INDEX0].advert_type = BTM_BLE_ADVERT_TYPE_FLAG;
    eddystone_adv_elem[EDDYSTONE_ADV_INDEX0].data[EDDYSTONE_ADV_DATA_INDEX0] = flag;

    /* Second adv element */
    eddystone_adv_elem[EDDYSTONE_ADV_INDEX1].len = ADV_PKT_16SRV_LENGTH;
    eddystone_adv_elem[EDDYSTONE_ADV_INDEX1].advert_type =
    BTM_BLE_ADVERT_TYPE_16SRV_COMPLETE;
    memcpy(eddystone_adv_elem[EDDYSTONE_ADV_INDEX1].data, eddystone_uuid, UUID_LENGTH);

    /* Third adv element (partial), rest is frame specific
     frame_len + advert_type (1) + uuid (2 )*/
    eddystone_adv_elem[EDDYSTONE_ADV_INDEX2].len         = frame_len + EDDYSTONE_SERVICE_DATA_LENGTH;
    eddystone_adv_elem[EDDYSTONE_ADV_INDEX2].advert_type = BTM_BLE_ADVERT_TYPE_SERVICE_DATA;
    eddystone_adv_elem[EDDYSTONE_ADV_INDEX2].data[EDDYSTONE_ADV_DATA_INDEX0] =
    eddystone_uuid[EDDYSTONE_UUID_INDEX0];
    eddystone_adv_elem[EDDYSTONE_ADV_INDEX2].data[EDDYSTONE_ADV_DATA_INDEX1] =
    eddystone_uuid[EDDYSTONE_UUID_INDEX1];
}

/********************************************************************************
* Function Name: beacon_set_adv_data
*********************************************************************************
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
*********************************************************************************/
void beacon_set_adv_data(beacon_ble_advert_elem_t *beacon_adv_elem, uint8_t num_elem,
                                  uint8_t adv_data[BEACON_ADV_DATA_MAX], uint8_t *adv_len)
{
    uint8_t structure_index = 0, buffer_index = 0;
    for (structure_index = 0; structure_index < num_elem; structure_index++)
    {
        adv_data[buffer_index++] = beacon_adv_elem[structure_index].len;
        adv_data[buffer_index++] = beacon_adv_elem[structure_index].advert_type;
        memcpy(&adv_data[buffer_index], beacon_adv_elem[structure_index].data,
            (beacon_adv_elem[structure_index].len - 1)); /* len - 1 for advert_type */
        buffer_index += beacon_adv_elem[structure_index].len - 1;
    }
    *adv_len = buffer_index;
}

/* [] END OF FILE */
