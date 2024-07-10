#include <stdio.h>
#include "Can.h"
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_log.h"
#include "driver/twai.h"


static const char* TAG = "CAN";

/* --------------------- Definitions and static variables ------------------ */

//Configurations
#define NO_OF_MSGS              100
#define TX_GPIO_NUM             GPIO_NUM_41
#define RX_GPIO_NUM             GPIO_NUM_42

//PIDs
#define PID_TACHO               0x0C
#define PID_SPEED               0x0D
#define PID_CAN_ID              0x7DF

static const twai_filter_config_t f_config = {.acceptance_code = ((uint32_t)0x7E8 << 21),
                                             .acceptance_mask = ~((uint32_t)0x7FF << 21),
                                             .single_filter = true};

static const twai_timing_config_t t_config = TWAI_TIMING_CONFIG_250KBITS();
//static const twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

void CAN::receive_message() {
    twai_message_t rx_message;
    esp_err_t result = twai_receive(&rx_message, portMAX_DELAY);
    if (result == ESP_OK) {
        // ESP_LOGI(TAG, "can received %x %x", (int)rx_message.identifier, (int)rx_message.data[2]);
        switch (rx_message.data[2]) {
        case PID_SPEED:
            // ESP_LOGI(TAG, "speed received");
            CAN::speed = rx_message.data[3];
            break;
        case PID_TACHO:
            CAN::tacho = ((256. * rx_message.data[3]) + rx_message.data[4]) / 4.;
            ESP_LOGI(TAG, "rpm received: %f", CAN::tacho);
            break;
        default:
            break;
        }
    }
}

void CAN::send_message(int pid) {
    twai_message_t tx_message = {};
    tx_message.identifier = PID_CAN_ID;
    tx_message.data_length_code = 8;
    tx_message.extd = 0;
    tx_message.data[0] = 0x02;
    tx_message.data[1] = 0x01;
    tx_message.data[2] = pid;
    tx_message.data[3] = 0xAA;    // Best to use 0xAA (0b10101010) instead of 0
	tx_message.data[4] = 0xAA;    // CAN works better this way as it needs
	tx_message.data[5] = 0xAA;    // to avoid bit-stuffing
	tx_message.data[6] = 0xAA;
	tx_message.data[7] = 0xAA;
    twai_transmit(&tx_message, 10 / portTICK_PERIOD_MS);
}

void CAN::TWAI_tx_thread(void* args) {
    while (true) {
        send_message(PID_SPEED);

        vTaskDelay(50 / portTICK_PERIOD_MS);
        
        send_message(PID_TACHO);

        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

void CAN::TWAI_rx_thread(void* args) {
    while (true) {
        receive_message();
    }
}

void CAN::init() {
    twai_general_config_t g_config = {};
    g_config.mode = TWAI_MODE_NORMAL;
    g_config.tx_io = TX_GPIO_NUM;
    g_config.rx_io = RX_GPIO_NUM;
    g_config.clkout_io = TWAI_IO_UNUSED;
    g_config.bus_off_io = TWAI_IO_UNUSED;
    g_config.tx_queue_len = 5;
    g_config.rx_queue_len = 5;
    g_config.alerts_enabled = TWAI_ALERT_NONE;
    g_config.clkout_divider = 0;
    g_config.intr_flags = ESP_INTR_FLAG_LEVEL2;

    twai_driver_install(&g_config, &t_config, &f_config);
    ESP_LOGI(TAG, "CAN driver installed");

    ESP_ERROR_CHECK(twai_start());
    ESP_LOGI(TAG, "CAN driver started");

    CAN::speed = -1;
    CAN::tacho = -1;

    xTaskCreatePinnedToCore(TWAI_rx_thread, "CAN_rx", 4096, NULL, 4, NULL, 1);
    xTaskCreatePinnedToCore(TWAI_tx_thread, "CAN_tx", 4096, NULL, 4, NULL, 1);
}