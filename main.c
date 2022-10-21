/******************************************************************************
* File Name:   main.c
*
* Description: This is the source code for the AnyCloud: Multi Beacon Example
*              for ModusToolbox.
*
* Related Document: See README.md
*
*
*******************************************************************************
* Copyright 2020-2022, Cypress Semiconductor Corporation (an Infineon company) or
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
#include "cybsp_bt_config.h"
#include "cycfg_bt_settings.h"
#include "wiced_bt_stack.h"
#include "cybsp.h"
#include "cy_retarget_io.h"
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <string.h>
#include "cybt_platform_trace.h"
#include "wiced_memory.h"
#include "stdio.h"
#include "beacon_utils.h"
#include "beacon_utils.h"
#include "wiced_bt_ble.h"


/*******************************************************************************
*        Macro Definitions
*******************************************************************************/
/* Allocate the multi-advertising instance numbers */
#define BEACON_EDDYSTONE_URL        (1)
#define BEACON_IBEACON_URL          (2)

/* This one byte will insert .com at the end of a URL in a URL frame. */
#define DOT_COM (0x07)

/* Minimum and maximum ADV interval */
#define ADVERT_INTERVAL_MIN 0x00A0 /* This is a requirement for BLE version 4.2 */
#define ADVERT_INTERVAL_MAX BTM_BLE_ADVERT_INTERVAL_MAX
/* adv parameter boundary values */
#define BTM_BLE_ADVERT_INTERVAL_MIN     0x0020
#define BTM_BLE_ADVERT_INTERVAL_MAX     0x4000
#define BLE_ADDR_PUBLIC                 0x00
/*******************************************************************************
*        Variable Definitions
*******************************************************************************/
wiced_bt_ble_multi_adv_params_t adv_parameters =
{
    .adv_int_min = ADVERT_INTERVAL_MIN,
    .adv_int_max = ADVERT_INTERVAL_MAX,
    .adv_type = MULTI_ADVERT_NONCONNECTABLE_EVENT,
    .channel_map = BTM_BLE_ADVERT_CHNL_37 | BTM_BLE_ADVERT_CHNL_38 | BTM_BLE_ADVERT_CHNL_39,
    .adv_filter_policy = BTM_BLE_ADV_POLICY_ACCEPT_CONN_AND_SCAN,
    .adv_tx_power = MULTI_ADV_TX_POWER_MAX_INDEX,
    .peer_bd_addr = {0},
    .peer_addr_type = BLE_ADDR_PUBLIC,
    .own_bd_addr = {0},
    .own_addr_type = BLE_ADDR_PUBLIC
};

/* User defined UUID for iBeacon */
#define UUID_IBEACON     0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f

/* This enables RTOS aware debugging. */
volatile int uxTopUsedPriority;

/*******************************************************************************
*        Function Prototypes
*******************************************************************************/
static void             ble_app_set_advertisement_data (void);
static void             ble_address_print              (wiced_bt_device_address_t bdadr);

/* Callback function for Bluetooth stack management type events */
static wiced_bt_dev_status_t  app_bt_management_callback (wiced_bt_management_evt_t event,
                                                          wiced_bt_management_evt_data_t *p_event_data);

/******************************************************************************
 *                          Function Definitions
 ******************************************************************************/
/*
 *  Entry point to the application. Set device configuration and start BT
 *  stack initialization.  The actual application initialization will happen
 *  when stack reports that BT device is ready.
 */
int main()
{
    cy_rslt_t result ;

    /* This enables RTOS aware debugging in OpenOCD. */
    uxTopUsedPriority = configMAX_PRIORITIES - 1;

    /* Initialize the board support package */
    if(CY_RSLT_SUCCESS != cybsp_init())
    {
        CY_ASSERT(0);
    }

    /* Enable global interrupts */
    __enable_irq();

    /* Initialize retarget-io to use the debug UART port */
    cy_retarget_io_init(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX,\
                        CY_RETARGET_IO_BAUDRATE);

    cybt_platform_config_init(&cybsp_bt_platform_cfg);

    printf("***********AnyCloud Example***********\n");
    printf("****Multi Beacon Application Start****\n");
    printf("**************************************\n\n");

    /* Register call back and configuration with stack */
    result=wiced_bt_stack_init (app_bt_management_callback, &wiced_bt_cfg_settings);

    /* Check if stack initialization was successful */
    if( WICED_BT_SUCCESS == result)
    {
        printf("Bluetooth Stack Initialization Successful \n");
    }
    else
    {
        printf("Bluetooth Stack Initialization failed!! \n");
        CY_ASSERT(0);
    }

    /* Start the FreeRTOS scheduler */
    vTaskStartScheduler() ;

    /* Should never get here */
    CY_ASSERT(0) ;
}
/********************************************************************************
* Function Name: app_bt_management_callback
*********************************************************************************
* Summary:
*   This is a Bluetooth stack event handler function to receive management events from
*   the BLE stack and process as per the application.
*
* Parameters:
*   wiced_bt_management_evt_t event             : BLE event code of one byte length
*   wiced_bt_management_evt_data_t *p_event_data: Pointer to BLE management event structures
*
* Return:
*  wiced_result_t: Error code from WICED_RESULT_LIST or BT_RESULT_LIST
*
*********************************************************************************/
wiced_result_t app_bt_management_callback(wiced_bt_management_evt_t event,
                                         wiced_bt_management_evt_data_t *p_event_data)
{
    wiced_result_t status = WICED_BT_SUCCESS;
    wiced_bt_device_address_t bda = { 0 };
    wiced_bt_multi_adv_opcodes_t multi_adv_resp_opcode;
    uint8_t multi_adv_resp_status = 0;

    switch (event)
    {
    case BTM_ENABLED_EVT:
        if( WICED_BT_SUCCESS == p_event_data->enabled.status )
        {
            printf("Bluetooth Enabled\r\n");

            wiced_bt_dev_read_local_addr(bda);
                    printf("Local Bluetooth Address: ");
                    ble_address_print(bda);

            /* Create the packet and begin advertising */
            ble_app_set_advertisement_data();
        }
        break;

    case BTM_MULTI_ADVERT_RESP_EVENT:

        /* Multi ADV Response */
        multi_adv_resp_opcode = p_event_data->ble_multi_adv_response_event.opcode;
        multi_adv_resp_status = p_event_data->ble_multi_adv_response_event.status;

        if (SET_ADVT_PARAM_MULTI == multi_adv_resp_opcode)
        {
            if(WICED_SUCCESS == multi_adv_resp_status)
            {
                printf("Multi ADV Set Param Event Status: SUCCESS\n");
            }
            else
            {
                printf("Multi ADV Set Param Event Status: FAILED\n");
            }
        }
        else if (SET_ADVT_DATA_MULTI == multi_adv_resp_opcode)
        {
            if(WICED_SUCCESS == multi_adv_resp_status)
            {
                printf("Multi ADV Set Data Event Status: SUCCESS\n");
            }
            else
            {
                printf("Multi ADV Set Data Event Status: FAILED\n");
            }
        }
        else if (SET_ADVT_ENABLE_MULTI == multi_adv_resp_opcode)
        {
            if(WICED_SUCCESS == multi_adv_resp_status)
            {
                printf("Multi ADV Start Event Status: SUCCESS\n");
            }
            else
            {
                printf("Multi ADV Start Event Status: FAILED\n");
            }
        }
        break;

    default:
        break;
    }

    return status;
}

/********************************************************************************
* Function Name: ble_app_set_advertisement_data
*********************************************************************************
* Summary:
*   This function configures the advertisement packet data
*
* Parameters:
*   None
*
* Return:
*   None
*
*********************************************************************************/
static void ble_app_set_advertisement_data(void)
{
    uint8_t packet_len;

    /* Eddystone URL advertising packet */
    uint8_t url_packet[BEACON_ADV_DATA_MAX];

    /* Set sample values for iBeacon */
    uint8_t adv_data_ibeacon[BEACON_ADV_DATA_MAX];
    uint8_t adv_len_ibeacon = 0;
    uint8_t ibeacon_uuid[LEN_UUID_128] = { UUID_IBEACON };

    /* Eddystone URL data */
    eddystone_url_t url_data = {adv_parameters.adv_tx_power, EDDYSTONE_URL_SCHEME_0,
                               {'i', 'n', 'f', 'i', 'n', 'e', 'o', 'n', DOT_COM, 0x00}};

    /* Set up a URL packet with max power, and implicit "http://www." prefix */
    eddystone_set_data_for_url(url_data, url_packet, &packet_len);

    /* The multi ADV APIs will return pending status now and will give the success/failure
     * status in the BTM_MULTI_ADVERT_RESP_EVENT callback event
     */
    if(WICED_BT_PENDING != wiced_set_multi_advertisement_data(url_packet, packet_len, BEACON_EDDYSTONE_URL))
    {
        printf("Set data for URL ADV failed\n");
        CY_ASSERT(0);
    }

    if(WICED_BT_PENDING != wiced_set_multi_advertisement_params(BEACON_EDDYSTONE_URL, &adv_parameters))
    {
        printf("Set params for URL ADV failed\n");
        CY_ASSERT(0);
    }

    if(WICED_BT_PENDING != wiced_start_multi_advertisements(MULTI_ADVERT_START, BEACON_EDDYSTONE_URL))
    {
        printf("Start ADV for URL ADV failed\n");
        CY_ASSERT(0);
    }

    /* Set up a IBEACON packet  */

   ibeacon_set_adv_data(ibeacon_uuid, IBEACON_MAJOR_NUMER, IBEACON_MINOR_NUMER, TX_POWER_LEVEL,
                adv_data_ibeacon, &adv_len_ibeacon);

    if(WICED_BT_PENDING != wiced_set_multi_advertisement_data(adv_data_ibeacon, adv_len_ibeacon, BEACON_IBEACON_URL))
    {
        printf("Set data for iBeacon ADV failed\n");
        CY_ASSERT(0);
    }

    if(WICED_BT_PENDING != wiced_set_multi_advertisement_params(BEACON_IBEACON_URL, &adv_parameters))
    {
        printf("Set params for IBEACON ADV failed\n");
        CY_ASSERT(0);
    }

    if(WICED_BT_PENDING != wiced_start_multi_advertisements(MULTI_ADVERT_START, BEACON_IBEACON_URL))
    {
        printf("Start ADV for IBEACON ADV failed\n");
        CY_ASSERT(0);
    }

    printf("Multiple ADV started.\n"
            "Use a scanner to scan for ADV packets.\n");
}

/********************************************************************************
* Function Name: ble_address_print
*********************************************************************************
* Summary:
*   This is the utility function that prints the address of the Bluetooth device
*
* Parameters:
*   wiced_bt_device_address_t bdadr                : Bluetooth address
*
* Return:
*  void
*
*********************************************************************************/
static void ble_address_print(wiced_bt_device_address_t bdadr)
{
    for(uint8_t i=0;i<BD_ADDR_LEN-1;i++)
    {
        printf("%2X:",bdadr[i]);
    }
    printf("%2X\n",bdadr[BD_ADDR_LEN-1]);
}


/* [] END OF FILE */
