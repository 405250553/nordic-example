/**
 * Copyright (c) 2015 - 2019, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
/**
 * @brief Blinky Sample Application main file.
 *
 * This file contains the source code for a sample server application using the LED Button service.
 */

#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "app_error.h"
#include "ble.h"
#include "ble_gap.h"
#include "ble_err.h"
#include "ble_hci.h"
#include "ble_srv_common.h"
#include "nrf_sdh_soc.h"
#include "ble_db_discovery.h"
#include "ble_advdata.h"
#include "ble_conn_params.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "boards.h"
#include "app_timer.h"
#include "app_button.h"
#include "ble_lbs.h"
#include "ble_nus_c.h"
#include "nrf_ble_gatt.h"
#include "nrf_ble_qwr.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_ble_scan.h"

#include "nrf_drv_clock.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"

#include "fds.h"

#include "nrf_cli.h"
#include "nrf_cli_rtt.h"
#include "nrf_cli_types.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "nrf_log_backend_flash.h"

#include "nrf_fstorage_nvmc.h"
#include "nrf_fstorage_sd.h"

#include "nrf_mpu_lib.h"
#include "nrf_stack_guard.h"

#if defined(APP_USBD_ENABLED) && APP_USBD_ENABLED
#define CLI_OVER_USB_CDC_ACM 1
#else
#define CLI_OVER_USB_CDC_ACM 0
#endif

#if CLI_OVER_USB_CDC_ACM
#include "nrf_cli_cdc_acm.h"
#include "nrf_drv_usbd.h"
#include "app_usbd_core.h"
#include "app_usbd.h"
#include "app_usbd_string_desc.h"
#include "app_usbd_cdc_acm.h"
#endif //CLI_OVER_USB_CDC_ACM

#if 0//defined(TX_PIN_NUMBER) && defined(RX_PIN_NUMBER)
#define CLI_OVER_UART 1
#else
#define CLI_OVER_UART 0
#endif

#if CLI_OVER_UART
#include "nrf_cli_uart.h"
#endif

/* If enabled then CYCCNT (high resolution) timestamp is used for the logger. */
#define USE_CYCCNT_TIMESTAMP_FOR_LOG 1

#define ADVERTISING_LED                 BSP_BOARD_LED_0                         /**< Is on when device is advertising. */
#define CONNECTED_LED                   BSP_BOARD_LED_1                         /**< Is on when device has connected. */
#define LEDBUTTON_LED                   BSP_BOARD_LED_2                         /**< LED to be toggled with the help of the LED Button Service. */
#define LEDBUTTON_BUTTON                BSP_BUTTON_0                            /**< Button that will trigger the notification event with the LED Button Service */


#define APP_BLE_OBSERVER_PRIO           3                                       /**< Application's BLE observer priority. You shouldn't need to modify this value. */
#define APP_BLE_CONN_CFG_TAG            1                                       /**< A tag identifying the SoftDevice BLE configuration. */

#define APP_ADV_INTERVAL                64                                      /**< The advertising interval (in units of 0.625 ms; this value corresponds to 40 ms). */
#define APP_ADV_DURATION                BLE_GAP_ADV_TIMEOUT_GENERAL_UNLIMITED   /**< The advertising time-out (in units of seconds). When set to 0, we will never time out. */


#define FIRST_CONN_PARAMS_UPDATE_DELAY  APP_TIMER_TICKS(20000)                  /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (15 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(5000)                   /**< Time between each call to sd_ble_gap_conn_param_update after the first call (5 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT    3                                       /**< Number of attempts before giving up the connection parameter negotiation. */

#define BUTTON_DETECTION_DELAY          APP_TIMER_TICKS(50)                     /**< Delay from a GPIOTE event until a button is reported as pushed (in number of timer ticks). */

#define DEAD_BEEF                       0xDEADBEEF                              /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */
#define NUS_SERVICE_UUID_TYPE   BLE_UUID_TYPE_VENDOR_BEGIN              /**< UUID type for the Nordic UART Service (vendor specific). */

//BLE_LBS_DEF(m_lbs);                                                             /**< LED Button Service instance. */
NRF_BLE_GATT_DEF(m_gatt);                                                       /**< GATT module instance. */
NRF_BLE_QWR_DEF(m_qwr);                                                         /**< Context for the Queued Write module.*/
BLE_NUS_C_DEF(m_ble_nus_c);                                             /**< BLE Nordic UART Service (NUS) client instanm_ble_nus_c*/
NRF_BLE_SCAN_DEF(m_ble_scan);                                               /**< Scanning Module instance. */
BLE_DB_DISCOVERY_DEF(m_db_disc);                                        /**< Database discovery module instance. */

static uint16_t m_conn_handle = BLE_CONN_HANDLE_INVALID;                        /**< Handle of the current connection. */

static uint8_t m_adv_handle = BLE_GAP_ADV_SET_HANDLE_NOT_SET;                   /**< Advertising handle used to identify an advertising set. */
static uint8_t m_enc_advdata[BLE_GAP_ADV_SET_DATA_SIZE_MAX];                    /**< Buffer for storing an encoded advertising set. */
static uint8_t m_enc_scan_response_data[BLE_GAP_ADV_SET_DATA_SIZE_MAX];         /**< Buffer for storing an encoded scan data. */

static uint16_t m_ble_nus_max_data_len = BLE_GATT_ATT_MTU_DEFAULT - OPCODE_LENGTH - HANDLE_LENGTH; /**< Maximum length of data (in bytes) that can be transmitted to the peer by the Nordic UART service module. */

/**@brief NUS UUID. */
static ble_uuid_t const m_nus_uuid =
{
    .uuid = BLE_UUID_NUS_SERVICE,
    .type = NUS_SERVICE_UUID_TYPE
};

#if NRF_LOG_BACKEND_FLASHLOG_ENABLED
NRF_LOG_BACKEND_FLASHLOG_DEF(m_flash_log_backend);
#endif

#if NRF_LOG_BACKEND_CRASHLOG_ENABLED
NRF_LOG_BACKEND_CRASHLOG_DEF(m_crash_log_backend);
#endif

/* Counter timer. */
APP_TIMER_DEF(m_timer_0);

/* Declared in demo_cli.c */
extern uint32_t m_counter;
extern bool m_counter_active;

#if CLI_OVER_USB_CDC_ACM

/**
 * @brief Enable power USB detection
 *
 * Configure if example supports USB port connection
 */
#ifndef USBD_POWER_DETECTION
#define USBD_POWER_DETECTION true
#endif

 int i=0;

static void usbd_user_ev_handler(app_usbd_event_type_t event)
{
        switch (event)
        {
        case APP_USBD_EVT_STOPPED:
                app_usbd_disable();
                break;
        case APP_USBD_EVT_POWER_DETECTED:
                if (!nrf_drv_usbd_is_enabled())
                {
                        app_usbd_enable();
                }
                break;
        case APP_USBD_EVT_POWER_REMOVED:
                app_usbd_stop();
                break;
        case APP_USBD_EVT_POWER_READY:
                app_usbd_start();
                break;
        default:
                break;
        }
}

#endif //CLI_OVER_USB_CDC_ACM


/**
 * @brief Command line interface instance
 * */
#define CLI_EXAMPLE_LOG_QUEUE_SIZE  (16)//4)

#if CLI_OVER_USB_CDC_ACM
NRF_CLI_CDC_ACM_DEF(m_cli_cdc_acm_transport);
NRF_CLI_DEF(m_cli_cdc_acm,
            "usb_cli:~$ ",
            &m_cli_cdc_acm_transport.transport,
            '\r',
            CLI_EXAMPLE_LOG_QUEUE_SIZE);
#endif //CLI_OVER_USB_CDC_ACM

#if CLI_OVER_UART
NRF_CLI_UART_DEF(m_cli_uart_transport, 0, 64, 16);
NRF_CLI_DEF(m_cli_uart,
            "uart_cli:~$ ",
            &m_cli_uart_transport.transport,
            '\r',
            CLI_EXAMPLE_LOG_QUEUE_SIZE);
#endif

NRF_CLI_RTT_DEF(m_cli_rtt_transport);
NRF_CLI_DEF(m_cli_rtt,
            "rtt_cli:~$ ",
            &m_cli_rtt_transport.transport,
            '\n',
            CLI_EXAMPLE_LOG_QUEUE_SIZE);


//
//static void CDC_ACM_1_user_ev_handler(app_usbd_class_inst_t const * p_inst,
//                                    app_usbd_cdc_acm_user_event_t event);
//
//#define READ_SIZE 1
//
//static char m_rx_buffer[READ_SIZE];
//static char m_tx_buffer[NRF_DRV_USBD_EPSIZE];
//static bool m_send_flag = 0;
//
//#define CDC_ACM_1_COMM_INTERFACE  2//0
//#define CDC_ACM_1_COMM_EPIN       NRF_DRV_USBD_EPIN4//NRF_DRV_USBD_EPIN2
//
//#define CDC_ACM_1_DATA_INTERFACE  3//1
//#define CDC_ACM_1_DATA_EPIN       NRF_DRV_USBD_EPIN3//NRF_DRV_USBD_EPIN1
//#define CDC_ACM_1_DATA_EPOUT      NRF_DRV_USBD_EPOUT2//NRF_DRV_USBD_EPOUT1
//
//
///**
// * @brief CDC_ACM class instance
// * */
//APP_USBD_CDC_ACM_GLOBAL_DEF(m_app_cdc_acm_1,
//                            CDC_ACM_1_user_ev_handler,
//                            CDC_ACM_1_COMM_INTERFACE,
//                            CDC_ACM_1_DATA_INTERFACE,
//                            CDC_ACM_1_COMM_EPIN,
//                            CDC_ACM_1_DATA_EPIN,
//                            CDC_ACM_1_DATA_EPOUT,
//                            APP_USBD_CDC_COMM_PROTOCOL_AT_V250
//);
//
///**
// * @brief User event handler @ref app_usbd_cdc_acm_user_ev_handler_t (headphones)
// * */
//static void CDC_ACM_1_user_ev_handler(app_usbd_class_inst_t const * p_inst,
//                                    app_usbd_cdc_acm_user_event_t event)
//{
//    app_usbd_cdc_acm_t const * p_cdc_acm = app_usbd_cdc_acm_class_get(p_inst);
//
//    switch (event)
//    {
//        case APP_USBD_CDC_ACM_USER_EVT_PORT_OPEN:
//        {
//
//            /*Setup first transfer*/
//            ret_code_t ret = app_usbd_cdc_acm_read(&m_app_cdc_acm_1,
//                                                   m_rx_buffer,
//                                                   READ_SIZE);
//            UNUSED_VARIABLE(ret);
//            break;
//        }
//        case APP_USBD_CDC_ACM_USER_EVT_PORT_CLOSE:
//            break;
//        case APP_USBD_CDC_ACM_USER_EVT_TX_DONE:
//            break;
//        case APP_USBD_CDC_ACM_USER_EVT_RX_DONE:
//        {
//            ret_code_t ret;
//            NRF_LOG_INFO("Bytes waiting: %d", app_usbd_cdc_acm_bytes_stored(p_cdc_acm));
//            do
//            {
//                /*Get amount of data transfered*/
//                size_t size = app_usbd_cdc_acm_rx_size(p_cdc_acm);
//                NRF_LOG_INFO("RX: size: %lu char: %c", size, m_rx_buffer[0]);
//
//                /* Fetch data until internal buffer is empty */
//                ret = app_usbd_cdc_acm_read(&m_app_cdc_acm_1,
//                                            m_rx_buffer,
//                                            READ_SIZE);
//            } while (ret == NRF_SUCCESS);
//
//            break;
//        }
//        default:
//            break;
//    }
//}

static void timer_handle(void * p_context)
{
        UNUSED_PARAMETER(p_context);

        if (m_counter_active)
        {
                m_counter++;
                NRF_LOG_RAW_INFO("counter = %d\n", m_counter);
        }
}

static void cli_start(void)
{
        ret_code_t ret;

#if CLI_OVER_USB_CDC_ACM
        ret = nrf_cli_start(&m_cli_cdc_acm);
        APP_ERROR_CHECK(ret);
#endif

#if CLI_OVER_UART
        ret = nrf_cli_start(&m_cli_uart);
        APP_ERROR_CHECK(ret);
#endif

        ret = nrf_cli_start(&m_cli_rtt);
        APP_ERROR_CHECK(ret);
}

static void cli_init(void)
{
        ret_code_t ret;

#if CLI_OVER_USB_CDC_ACM
        ret = nrf_cli_init(&m_cli_cdc_acm, NULL, true, true, NRF_LOG_SEVERITY_INFO);
        APP_ERROR_CHECK(ret);
#endif

#if CLI_OVER_UART
        nrf_drv_uart_config_t uart_config = NRF_DRV_UART_DEFAULT_CONFIG;
        uart_config.pseltxd = TX_PIN_NUMBER;
        uart_config.pselrxd = RX_PIN_NUMBER;
        uart_config.hwfc    = NRF_UART_HWFC_DISABLED;
        ret = nrf_cli_init(&m_cli_uart, &uart_config, true, true, NRF_LOG_SEVERITY_INFO);
        APP_ERROR_CHECK(ret);
#endif

        ret = nrf_cli_init(&m_cli_rtt, NULL, true, true, NRF_LOG_SEVERITY_INFO);
        APP_ERROR_CHECK(ret);
}

static void usbd_init(void)
{
#if CLI_OVER_USB_CDC_ACM
        ret_code_t ret;
        static const app_usbd_config_t usbd_config = {
                .ev_handler = app_usbd_event_execute,
                .ev_state_proc = usbd_user_ev_handler
        };
        ret = app_usbd_init(&usbd_config);
        APP_ERROR_CHECK(ret);

        app_usbd_class_inst_t const * class_cdc_acm =
                app_usbd_cdc_acm_class_inst_get(&nrf_cli_cdc_acm);
        ret = app_usbd_class_append(class_cdc_acm);
        APP_ERROR_CHECK(ret);
#endif 
}

static void usbd_enable(void)
{
        ret_code_t ret;
        if (USBD_POWER_DETECTION)
        {
                ret = app_usbd_power_events_enable();
                APP_ERROR_CHECK(ret);
        }
        else
        {
                NRF_LOG_INFO("No USB power detection enabled\nStarting USB now");

                app_usbd_enable();
                app_usbd_start();
        }

        /* Give some time for the host to enumerate and connect to the USB CDC port */
        nrf_delay_ms(500);

//#endif
}

static void cli_process(void)
{
#if CLI_OVER_USB_CDC_ACM
        nrf_cli_process(&m_cli_cdc_acm);
#endif

#if CLI_OVER_UART
        nrf_cli_process(&m_cli_uart);
#endif
        nrf_cli_process(&m_cli_rtt);
}

static void flashlog_init(void)
{
        ret_code_t ret;
        int32_t backend_id;

        ret = nrf_log_backend_flash_init(&nrf_fstorage_sd);
        APP_ERROR_CHECK(ret);

#if NRF_LOG_BACKEND_FLASHLOG_ENABLED
        backend_id = nrf_log_backend_add(&m_flash_log_backend, NRF_LOG_SEVERITY_WARNING);
        APP_ERROR_CHECK_BOOL(backend_id >= 0);

        nrf_log_backend_enable(&m_flash_log_backend);
#endif

#if NRF_LOG_BACKEND_CRASHLOG_ENABLED
        backend_id = nrf_log_backend_add(&m_crash_log_backend, NRF_LOG_SEVERITY_INFO);
        APP_ERROR_CHECK_BOOL(backend_id >= 0);

        nrf_log_backend_enable(&m_crash_log_backend);
#endif
}

static inline void stack_guard_init(void)
{
        APP_ERROR_CHECK(nrf_mpu_lib_init());
        APP_ERROR_CHECK(nrf_stack_guard_init());
}

uint32_t cyccnt_get(void)
{
        return DWT->CYCCNT;
}


/**@brief Function for assert macro callback.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in] line_num    Line number of the failing ASSERT call.
 * @param[in] p_file_name File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
        app_error_handler(DEAD_BEEF, line_num, p_file_name);
}


/**@brief Function for the LEDs initialization.
 *
 * @details Initializes all LEDs used by the application.
 */
static void leds_init(void)
{
        bsp_board_init(BSP_INIT_LEDS);
}


/**@brief Function for the Timer initialization.
 *
 * @details Initializes the timer module.
 */
static void timers_init(void)
{
        // Initialize timer module, making it use the scheduler
        ret_code_t err_code = app_timer_init();
        APP_ERROR_CHECK(err_code);
}


/**@brief Function for the GAP initialization.
 *
 * @details This function sets up all the necessary GAP (Generic Access Profile) parameters of the
 *          device including the device name, appearance, and the preferred connection parameters.
 */
 /*
static void gap_params_init(void)
{
        ret_code_t err_code;
        ble_gap_conn_params_t gap_conn_params;
        ble_gap_conn_sec_mode_t sec_mode;

        BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

        err_code = sd_ble_gap_device_name_set(&sec_mode,
                                              (const uint8_t *)DEVICE_NAME,
                                              strlen(DEVICE_NAME));
        APP_ERROR_CHECK(err_code);

        memset(&gap_conn_params, 0, sizeof(gap_conn_params));

        gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
        gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
        gap_conn_params.slave_latency     = SLAVE_LATENCY;
        gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

        err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
        APP_ERROR_CHECK(err_code);
}

/**@brief Function for handling events from the GATT library. */
void gatt_evt_handler(nrf_ble_gatt_t * p_gatt, nrf_ble_gatt_evt_t const * p_evt)
{
    if (p_evt->evt_id == NRF_BLE_GATT_EVT_ATT_MTU_UPDATED)
    {
        //NRF_LOG_INFO("ATT MTU exchange completed.");

        m_ble_nus_max_data_len = p_evt->params.att_mtu_effective - OPCODE_LENGTH - HANDLE_LENGTH;
        //NRF_LOG_INFO("Ble NUS max data length set to 0x%X(%d)", m_ble_nus_max_data_len, m_ble_nus_max_data_len);
    }
}


/**@brief Function for initializing the GATT module.
 */
 
static void gatt_init(void)
{
        ret_code_t err_code = nrf_ble_gatt_init(&m_gatt, gatt_evt_handler);
        APP_ERROR_CHECK(err_code);

        err_code = nrf_ble_gatt_att_mtu_central_set(&m_gatt, NRF_SDH_BLE_GATT_MAX_MTU_SIZE);
        APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing the Advertising functionality.
 *
 * @details Encodes the required advertising data and passes it to the stack.
 *          Also builds a structure to be passed to the stack when starting advertising.
 */
 /*
static void advertising_init(void)
{
        ret_code_t err_code;
        ble_advdata_t advdata;
        ble_advdata_t srdata;

        ble_uuid_t adv_uuids[] = {{LBS_UUID_SERVICE, m_lbs.uuid_type}};

        // Build and set advertising data.
        memset(&advdata, 0, sizeof(advdata));

        advdata.name_type          = BLE_ADVDATA_FULL_NAME;
        advdata.include_appearance = true;
        advdata.flags              = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;


        memset(&srdata, 0, sizeof(srdata));
        srdata.uuids_complete.uuid_cnt = sizeof(adv_uuids) / sizeof(adv_uuids[0]);
        srdata.uuids_complete.p_uuids  = adv_uuids;

        err_code = ble_advdata_encode(&advdata, m_adv_data.adv_data.p_data, &m_adv_data.adv_data.len);
        APP_ERROR_CHECK(err_code);

        err_code = ble_advdata_encode(&srdata, m_adv_data.scan_rsp_data.p_data, &m_adv_data.scan_rsp_data.len);
        APP_ERROR_CHECK(err_code);

        ble_gap_adv_params_t adv_params;

        // Set advertising parameters.
        memset(&adv_params, 0, sizeof(adv_params));

        adv_params.primary_phy     = BLE_GAP_PHY_1MBPS;
        adv_params.duration        = APP_ADV_DURATION;
        adv_params.properties.type = BLE_GAP_ADV_TYPE_CONNECTABLE_SCANNABLE_UNDIRECTED;
        adv_params.p_peer_addr     = NULL;
        adv_params.filter_policy   = BLE_GAP_ADV_FP_ANY;
        adv_params.interval        = APP_ADV_INTERVAL;

        err_code = sd_ble_gap_adv_set_configure(&m_adv_handle, &m_adv_data, &adv_params);
        APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling Queued Write Module errors.
 *
 * @details A pointer to this function will be passed to each service which may need to inform the
 *          application about an error.
 *
 * @param[in]   nrf_error   Error code containing information about what went wrong.
 */
static void nrf_qwr_error_handler(uint32_t nrf_error)
{
        APP_ERROR_HANDLER(nrf_error);
}


/**@brief Function for handling write events to the LED characteristic.
 *
 * @param[in] p_lbs     Instance of LED Button Service to which the write applies.
 * @param[in] led_state Written/desired state of the LED.
 */
static void led_write_handler(uint16_t conn_handle, ble_lbs_t * p_lbs, uint8_t led_state)
{
   // static int i=0;

        ret_code_t err_code;
        if (led_state)
        {
                bsp_board_led_on(LEDBUTTON_LED);
                NRF_LOG_RAW_INFO("led on time:%d\n",i++);
                NRF_LOG_RAW_INFO("Received LED ON!\n");
//                {
//                    size_t size = sprintf(m_tx_buffer, "Received LED ON!\n");
//                    err_code = app_usbd_cdc_acm_write(&m_app_cdc_acm_1, m_tx_buffer, size);
//                    if (err_code == NRF_SUCCESS)
//                    {
//                    }
//                }
        }
        else
        {
                bsp_board_led_off(LEDBUTTON_LED);
                NRF_LOG_INFO("Received LED OFF!");
//                {
//                    size_t size = sprintf(m_tx_buffer, "Received LED OFF!\n");
//                    err_code = app_usbd_cdc_acm_write(&m_app_cdc_acm_1, m_tx_buffer, size);
//                    if (err_code == NRF_SUCCESS)
//                    {
//                    }
//                }

        }
}



/**@brief Function for handling BLE events.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 * @param[in]   p_context   Unused.
 */
static void ble_evt_handler(ble_evt_t const * p_ble_evt, void * p_context)
{
        ret_code_t err_code;
        ble_gap_evt_t const * p_gap_evt = &p_ble_evt->evt.gap_evt;

        switch (p_ble_evt->header.evt_id)
        {
        case BLE_GAP_EVT_CONNECTED:
                NRF_LOG_INFO("Connected");
                bsp_board_led_on(CONNECTED_LED);
                bsp_board_led_off(ADVERTISING_LED);
                /*
                m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
                err_code = nrf_ble_qwr_conn_handle_assign(&m_qwr, m_conn_handle);
                APP_ERROR_CHECK(err_code);
                */
                err_code = ble_nus_c_handles_assign(&m_ble_nus_c, p_ble_evt->evt.gap_evt.conn_handle, NULL);
                APP_ERROR_CHECK(err_code);

                // start discovery of services. The NUS Client waits for a discovery result
                err_code = ble_db_discovery_start(&m_db_disc, p_ble_evt->evt.gap_evt.conn_handle);
                APP_ERROR_CHECK(err_code);

                err_code = app_button_enable();
                APP_ERROR_CHECK(err_code);
                
                break;

        case BLE_GAP_EVT_DISCONNECTED:
                //NRF_LOG_INFO("Disconnected");
                bsp_board_led_off(CONNECTED_LED);
                //m_conn_handle = BLE_CONN_HANDLE_INVALID;
                err_code = app_button_disable();
                APP_ERROR_CHECK(err_code);
                //advertising_start();
                
                break;

        case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
                // Pairing not supported
                /*
                err_code = sd_ble_gap_sec_params_reply(m_conn_handle,
                                                       BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP,
                                                       NULL,
                                                       NULL);
                APP_ERROR_CHECK(err_code);
                */
                // Pairing not supported.
                err_code = sd_ble_gap_sec_params_reply(p_ble_evt->evt.gap_evt.conn_handle, BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP, NULL, NULL);
                APP_ERROR_CHECK(err_code);
                break;

        case BLE_GAP_EVT_CONN_PARAM_UPDATE_REQUEST:
            // Accepting parameters requested by peer.
            err_code = sd_ble_gap_conn_param_update(p_gap_evt->conn_handle,
                                                    &p_gap_evt->params.conn_param_update_request.conn_params);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GAP_EVT_PHY_UPDATE_REQUEST:
        {
                NRF_LOG_DEBUG("PHY update request.");
                ble_gap_phys_t const phys =
                {
                        .rx_phys = BLE_GAP_PHY_AUTO,
                        .tx_phys = BLE_GAP_PHY_AUTO,
                };
                err_code = sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle, &phys);
                APP_ERROR_CHECK(err_code);
        } break;
/*
        case BLE_GATTS_EVT_SYS_ATTR_MISSING:
                // No system attributes have been stored.
                err_code = sd_ble_gatts_sys_attr_set(m_conn_handle, NULL, 0, 0);
                APP_ERROR_CHECK(err_code);
                break;
                */

        case BLE_GATTC_EVT_TIMEOUT:
                // Disconnect on GATT Client timeout event.
                NRF_LOG_DEBUG("GATT Client Timeout.");
                err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                                 BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
                APP_ERROR_CHECK(err_code);
                break;

        case BLE_GATTS_EVT_TIMEOUT:
                // Disconnect on GATT Server timeout event.
                NRF_LOG_DEBUG("GATT Server Timeout.");
                err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                                 BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
                APP_ERROR_CHECK(err_code);
                break;

        default:
                // No implementation needed.
                break;
        }
}


/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 */
static void ble_stack_init(void)
{
        ret_code_t err_code;


        err_code = nrf_sdh_enable_request();
        APP_ERROR_CHECK(err_code);


        // Configure the BLE stack using the default settings.
        // Fetch the start address of the application RAM.
        uint32_t ram_start = 0;
        err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
        APP_ERROR_CHECK(err_code);



        // Enable BLE stack.
        err_code = nrf_sdh_ble_enable(&ram_start);
        APP_ERROR_CHECK(err_code);


        // Register a handler for BLE events.
        NRF_SDH_BLE_OBSERVER(m_ble_observer, APP_BLE_OBSERVER_PRIO, ble_evt_handler, NULL);
}


/**@brief Function for handling events from the button handler module.
 *
 * @param[in] pin_no        The pin that the event applies to.
 * @param[in] button_action The button action (press/release).
 */
 /*
static void button_event_handler(uint8_t pin_no, uint8_t button_action)
{
        ret_code_t err_code;

        switch (pin_no)
        {
        case LEDBUTTON_BUTTON:
                //NRF_LOG_INFO("Send button state change.");
                err_code = ble_lbs_on_button_change(m_conn_handle, &m_lbs, button_action);
                if (err_code != NRF_SUCCESS &&
                    err_code != BLE_ERROR_INVALID_CONN_HANDLE &&
                    err_code != NRF_ERROR_INVALID_STATE &&
                    err_code != BLE_ERROR_GATTS_SYS_ATTR_MISSING)
                {
                        APP_ERROR_CHECK(err_code);
                }
                break;

        default:
                APP_ERROR_HANDLER(pin_no);
                break;
        }
}


/**@brief Function for initializing the button handler module.
 */
 /*
static void buttons_init(void)
{
        ret_code_t err_code;

        //The array must be static because a pointer to it will be saved in the button handler module.
        static app_button_cfg_t buttons[] =
        {
                {LEDBUTTON_BUTTON, false, BUTTON_PULL, button_event_handler}
        };

        err_code = app_button_init(buttons, ARRAY_SIZE(buttons),
                                   BUTTON_DETECTION_DELAY);
        APP_ERROR_CHECK(err_code);
}


static void log_init(void)
{
        ret_code_t err_code = NRF_LOG_INIT(NULL);
        APP_ERROR_CHECK(err_code);

        NRF_LOG_DEFAULT_BACKENDS_INIT();
}


/**@brief Function for initializing power management.
 */
static void power_management_init(void)
{
        ret_code_t err_code;
        err_code = nrf_pwr_mgmt_init();
        APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling the idle state (main loop).
 *
 * @details If there is no pending log operation, then sleep until next the next event occurs.
 */
static void idle_state_handle(void)
{
        //if (NRF_LOG_PROCESS() == false)
        {
                nrf_pwr_mgmt_run();
        }
}

/**@brief Function for starting scanning. */
static void scan_start(void)
{
    ret_code_t ret;

    ret = nrf_ble_scan_start(&m_ble_scan);
    APP_ERROR_CHECK(ret);

    //ret = bsp_indication_set(BSP_INDICATE_SCANNING);
    //APP_ERROR_CHECK(ret);
}

void idle_task(void * p_context)
{

        //advertising_start();

        // Start execution.
        scan_start();

        // Enter main loop.
        for (;;)
        {
        
                if (NRF_LOG_PROCESS() == false)
                {
                        nrf_pwr_mgmt_run();
                }
                
                task_yield();
        }
}

//==================================================================================================================================================

/**@brief Function for handling database discovery events.
 *
 * @details This function is a callback function to handle events from the database discovery module.
 *          Depending on the UUIDs that are discovered, this function forwards the events
 *          to their respective services.
 *
 * @param[in] p_event  Pointer to the database discovery event.
 */
static void db_disc_handler(ble_db_discovery_evt_t * p_evt)
{
    ble_nus_c_on_db_disc_evt(&m_ble_nus_c, p_evt);
}


/** @brief Function for initializing the database discovery module. */
static void db_discovery_init(void)
{
    ret_code_t err_code = ble_db_discovery_init(db_disc_handler);
    APP_ERROR_CHECK(err_code);
}


static void ble_nus_chars_received_uart_print(uint8_t * p_data, uint16_t data_len)
{
    ret_code_t ret_val;

    for (uint32_t i = 0; i < data_len; i++)
    {
         NRF_LOG_RAW_INFO("%.2x",p_data[i]);

    }

    NRF_LOG_RAW_INFO("\n");

}

/**@brief Callback handling Nordic UART Service (NUS) client events.
 *
 * @details This function is called to notify the application of NUS client events.
 *
 * @param[in]   p_ble_nus_c   NUS client handle. This identifies the NUS client.
 * @param[in]   p_ble_nus_evt Pointer to the NUS client event.
 */

/**@snippet [Handling events from the ble_nus_c module] */
static void ble_nus_c_evt_handler(ble_nus_c_t * p_ble_nus_c, ble_nus_c_evt_t const * p_ble_nus_evt)
{
    ret_code_t err_code;

    switch (p_ble_nus_evt->evt_type)
    {
        case BLE_NUS_C_EVT_DISCOVERY_COMPLETE:
            //NRF_LOG_INFO("Discovery complete.");
            err_code = ble_nus_c_handles_assign(p_ble_nus_c, p_ble_nus_evt->conn_handle, &p_ble_nus_evt->handles);
            APP_ERROR_CHECK(err_code);

            err_code = ble_nus_c_tx_notif_enable(p_ble_nus_c);
            APP_ERROR_CHECK(err_code);
            //NRF_LOG_INFO("Connected to device with Nordic UART Service.");
            break;

        case BLE_NUS_C_EVT_NUS_TX_EVT:
            ble_nus_chars_received_uart_print(p_ble_nus_evt->p_data, p_ble_nus_evt->data_len);
            break;

        case BLE_NUS_C_EVT_DISCONNECTED:
            //NRF_LOG_INFO("Disconnected.");
            scan_start();
            break;
    }
}

/**@brief Function for initializing the Nordic UART Service (NUS) client. */
static void nus_c_init(void)
{
    ret_code_t       err_code;
    ble_nus_c_init_t init;

    init.evt_handler = ble_nus_c_evt_handler;

    err_code = ble_nus_c_init(&m_ble_nus_c, &init);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for handling Scanning Module events.
 */
static void scan_evt_handler(scan_evt_t const * p_scan_evt)
{
    ret_code_t err_code;

    switch(p_scan_evt->scan_evt_id)
    {
         case NRF_BLE_SCAN_EVT_CONNECTING_ERROR:
         {
              err_code = p_scan_evt->params.connecting_err.err_code;
              APP_ERROR_CHECK(err_code);
         } break;

         case NRF_BLE_SCAN_EVT_CONNECTED:
         {
              ble_gap_evt_connected_t const * p_connected =
                               p_scan_evt->params.connected.p_connected;
             // Scan is automatically stopped by the connection.
             /*
             NRF_LOG_INFO("Connecting to target %02x%02x%02x%02x%02x%02x",
                      p_connected->peer_addr.addr[0],
                      p_connected->peer_addr.addr[1],
                      p_connected->peer_addr.addr[2],
                      p_connected->peer_addr.addr[3],
                      p_connected->peer_addr.addr[4],
                      p_connected->peer_addr.addr[5]
                      );
                      */
         } break;

         case NRF_BLE_SCAN_EVT_SCAN_TIMEOUT:
         {
             //NRF_LOG_INFO("Scan timed out.");
             scan_start();
         } break;

         default:
             break;
    }
}


/**@brief Function for initializing the scanning and setting the filters.
 */
static void scan_init(void)
{
    ret_code_t          err_code;
    nrf_ble_scan_init_t init_scan;

    memset(&init_scan, 0, sizeof(init_scan));

    init_scan.connect_if_match = true;
    init_scan.conn_cfg_tag     = APP_BLE_CONN_CFG_TAG;

    err_code = nrf_ble_scan_init(&m_ble_scan, &init_scan, scan_evt_handler);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_ble_scan_filter_set (&m_ble_scan, SCAN_UUID_FILTER, &m_nus_uuid);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_ble_scan_filters_enable(&m_ble_scan, NRF_BLE_SCAN_UUID_FILTER, false);
    APP_ERROR_CHECK(err_code);
}

void led_test()
{
    nrf_gpio_cfg_output(NRF_GPIO_PIN_MAP(0,12));
    nrf_gpio_pin_clear(NRF_GPIO_PIN_MAP(0,12));

    while (true)
    {
			nrf_gpio_pin_toggle(NRF_GPIO_PIN_MAP(0,12));
			nrf_delay_ms(100);

    }
}

//=====================================================================================================================================================

/**@brief Function for application main entry.
 */
int main(void)
{
/*
        ret_code_t ret;

        if (USE_CYCCNT_TIMESTAMP_FOR_LOG)
        {
                CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
                DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
                DWT->CYCCNT = 0;
                APP_ERROR_CHECK(NRF_LOG_INIT(cyccnt_get, 64000000));
        }
        else
        {
                APP_ERROR_CHECK(NRF_LOG_INIT(app_timer_cnt_get));
        }

        ret = nrf_drv_clock_init();
        APP_ERROR_CHECK(ret);
        nrf_drv_clock_lfclk_request(NULL);

        // Initialize.
        //log_init();
        leds_init();
        timers_init();

        ret = app_timer_create(&m_timer_0, APP_TIMER_MODE_REPEATED, timer_handle);
        APP_ERROR_CHECK(ret);

        ret = app_timer_start(m_timer_0, APP_TIMER_TICKS(1000), NULL);
        APP_ERROR_CHECK(ret);

        stack_guard_init();

        cli_init();
        usbd_init();

        nrf_delay_ms(1000);
        log_init();

        usbd_enable();


        nrf_delay_ms(3000);

        buttons_init();
        db_discovery_init();
        power_management_init();
        ble_stack_init();
        //gap_params_init();
        gatt_init();
        //services_init();
        //advertising_init();
        //conn_params_init();
        nus_c_init();
        scan_init();

        ret = fds_init();
        APP_ERROR_CHECK(ret);

        UNUSED_RETURN_VALUE(nrf_log_config_load());

        //APP_ERROR_CHECK(nrf_cli_task_create(&m_cli));
        APP_ERROR_CHECK(nrf_cli_task_create(&m_cli_rtt));


//        APP_ERROR_CHECK(nrf_cli_task_create(&m_cli_uart));
        APP_ERROR_CHECK(nrf_cli_task_create(&m_cli_cdc_acm));

        //cli_start();

        flashlog_init();

        task_manager_start(idle_task, NULL);
        */


    // Initialize.
    timers_init();
    db_discovery_init();
    power_management_init();
            //led_test();
    ble_stack_init();
    //led_test();
    gatt_init();
    nus_c_init();
    scan_init();

    // Start execution.
    scan_start();

    // Enter main loop.
    for (;;)
    {
        //idle_state_handle();
    }
}


/**
 * @}
 */
