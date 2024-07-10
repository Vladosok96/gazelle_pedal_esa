#include <RPMCounter.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"

#define IGNITION_COIL_PIN       GPIO_NUM_1
#define PULSE_MIN_DURATIN_US    10
#define COUNTER_LIMIT           3               // Pulses to calculate RPM


static const char* TAG = "RPM_Counter";

bool RPMCounter::pcnt_on_reach(pcnt_unit_handle_t unit, const pcnt_watch_event_data_t *edata, void *user_ctx) {
    int64_t timestamp = esp_timer_get_time();
    rpm = (60000000.0 * COUNTER_LIMIT) / (double)(timestamp - lastReachedTime);
    lastReachedTime = timestamp;
    return (pdTRUE);
}

void RPMCounter::init() {
    ESP_LOGI(TAG, "Initializing PCNT");

    pcnt_unit_config_t unit_config  = {
        .low_limit = -1,
        .high_limit = COUNTER_LIMIT,
    };

    pcnt_unit_handle_t pcnt_unit = NULL;
    ESP_ERROR_CHECK(pcnt_new_unit(&unit_config, &pcnt_unit));

    pcnt_glitch_filter_config_t filter_config = {
        .max_glitch_ns = PULSE_MIN_DURATIN_US * 1000,
    };
    ESP_ERROR_CHECK(pcnt_unit_set_glitch_filter(pcnt_unit, &filter_config));

    pcnt_chan_config_t chan_config = {
        .edge_gpio_num = IGNITION_COIL_PIN,
        .level_gpio_num = GPIO_NUM_NC,
    };
    pcnt_channel_handle_t pcnt_chan = NULL;
    ESP_ERROR_CHECK(pcnt_new_channel(pcnt_unit, &chan_config, &pcnt_chan));

    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(pcnt_chan, PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_HOLD));

    ESP_ERROR_CHECK(pcnt_unit_add_watch_point(pcnt_unit, COUNTER_LIMIT));

    pcnt_event_callbacks_t cbs = {
        .on_reach = pcnt_on_reach,
    };

    ESP_ERROR_CHECK(pcnt_unit_register_event_callbacks(pcnt_unit, &cbs, NULL));

    ESP_ERROR_CHECK(pcnt_unit_enable(pcnt_unit));
    
    ESP_ERROR_CHECK(pcnt_unit_clear_count(pcnt_unit));
    
    ESP_ERROR_CHECK(pcnt_unit_start(pcnt_unit));

    xTaskCreate(RPMCounter::task, "RPM_watchdog", 4096, NULL, 5, NULL);

    ESP_LOGI(TAG, "PCNT initialized");
}

float RPMCounter::getRPM() {
    return rpm;
}

void RPMCounter::task(void* args) {
    while (true) {
        if (esp_timer_get_time() - lastReachedTime > 500000)
            rpm = 0;
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}