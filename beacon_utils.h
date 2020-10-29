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

#ifndef __BEACON_UTILS_H__
#define __BEACON_UTILS_H__

/******************************************************************************
 *                                Constants
 ******************************************************************************/
/* Type of eddystone frame
   https://github.com/google/eddystone/blob/master/protocol-specification.md */
#define EDDYSTONE_FRAME_TYPE_UID          (0x00)
#define EDDYSTONE_FRAME_TYPE_URL          (0x10)
#define EDDYSTONE_FRAME_TYPE_TLM          (0x20)
#define EDDYSTONE_FRAME_TYPE_EID          (0x30)

/* Definitions for UID frame format */
#define EDDYSTONE_UID_FRAME_LEN           (20)
#define EDDYSTONE_UID_NAMESPACE_LEN       (10)
#define EDDYSTONE_UID_INSTANCE_ID_LEN     (6)

/******************************************************************************
* URL Scheme Prefix for Google Eddystone
* Decimal Hex   Expansion
*   0      0x00   http://www.
*   1      0x01   https://www.
*   2      0x02   http://
*   3      0x03   https://
*
******************************************************************************/
#define EDDYSTONE_URL_SCHEME_0           (0x00)
#define EDDYSTONE_URL_SCHEME_1           (0x01)
#define EDDYSTONE_URL_SCHEME_2           (0x02)
#define EDDYSTONE_URL_SCHEME_3           (0x03)

/* Definitions for URL frame format */
#define EDDYSTONE_URL_FRAME_LEN           (20)
#define EDDYSTONE_URL_VALUE_MAX_LEN       (17)

/* Eddystone UUID*/
#define EDDYSTONE_UUID16                  (0xFEAA)

/* Number of advertisement elements for Eddystone */
#define EDDYSTONE_ELEM_NUM                (3)

/* Max ADV data length */
#define BEACON_ADV_DATA_MAX (31)

/******************************************************************************
 *                                Structures
 ******************************************************************************/
/* Structure to hold advertisement element data */
typedef struct
{
    uint8_t data[BEACON_ADV_DATA_MAX];     /* Advertisement  data */
    uint8_t len;                                    /* Advertisement length */
    wiced_bt_ble_advert_type_t advert_type;         /* Advertisement data type */
}beacon_ble_advert_elem_t;

/* Structure to hold eddystone URL parameters */
typedef struct __attribute__((packed, aligned(1)))
{
    uint8_t tx_power;                                      /* ADV Tx Power */
    uint8_t urlscheme;                                       /* URL Scheme */
    uint8_t encoded_url[EDDYSTONE_URL_VALUE_MAX_LEN];               /* URL */
}eddystone_url_t;

/* Structure to hold eddystone UID parameters */
typedef struct __attribute__((packed, aligned(1)))
{
    uint8_t eddystone_ranging_data;                            /* Calibrated TX power */
    uint8_t eddystone_namespace[EDDYSTONE_UID_NAMESPACE_LEN];  /* UID namespace */
    uint8_t eddystone_instance[EDDYSTONE_UID_INSTANCE_ID_LEN]; /* Instance */
}eddystone_uid_t;

/****************************************************************************
 *                              FUNCTION DECLARATIONS
 ***************************************************************************/
void eddystone_set_data_for_uid(eddystone_uid_t uid_data,
                                uint8_t adv_data[BEACON_ADV_DATA_MAX], uint8_t *adv_len);

void eddystone_set_data_for_url(eddystone_url_t url_data,
                                uint8_t adv_data[BEACON_ADV_DATA_MAX], uint8_t *adv_len);

void eddystone_set_data_common(beacon_ble_advert_elem_t *eddystone_adv_elem,
                                                       uint8_t frame_type, uint8_t frame_len);

void beacon_set_adv_data(beacon_ble_advert_elem_t *beacon_adv_elem, uint8_t num_elem,
                                  uint8_t adv_data[BEACON_ADV_DATA_MAX], uint8_t *adv_len);

#endif      /* __BEACON_UTILS_H__ */


/* [] END OF FILE */
