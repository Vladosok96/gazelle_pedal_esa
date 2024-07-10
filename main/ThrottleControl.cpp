#include "ThrottleControl.h"

#include "ECU.h"
#include "esp_log.h"
#include "nvstorage.h"
#include <RPMCounter.h>


static const char* TAG = "Throttle";

void ThrottleControl::loop(void* args) {    
    while (true) {
        if (ThrottleControl::target_rpm < 1000) {
            ECU::set_throttle(0);
        }
        else if (ThrottleControl::target_rpm < 6000) {
            ECU::set_throttle(20);
        }
        // if (RPMCounter::getRPM() > 100 && ThrottleControl::target_rpm > 900) {
        //     int rpm_deviation = ThrottleControl::target_rpm - RPMCounter::getRPM();
        //     float pid_result = pid(rpm_deviation);
        //     // ESP_LOGI(TAG, "value %f %f %d", RPMCounter::getRPM(), pid_result, ThrottleControl::target_rpm);
        //     ECU::set_throttle(pid_result);
        // }
        // else {
        //     ECU::set_throttle(0);
        // }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void ThrottleControl::start() {
    loadPedalParams();
    ThrottleControl::target_rpm = 1500;
    ThrottleControl::set_params(pedalParams.position_P, pedalParams.position_I, pedalParams.position_D, pedalParams.ramp, pedalParams.limit);   
    xTaskCreate(loop, "throttle_control", 4096, NULL, 4, NULL);
}

void ThrottleControl::set_target(int32_t rpm) { ThrottleControl::target_rpm = rpm; }

void ThrottleControl::set_params(float position_P, float position_I, float position_D, float ramp, float limit) {
    pid.P = position_P;
    pid.I = position_I;
    pid.D = position_D;
    pid.output_ramp = ramp;
    pid.limit = limit;
    pid.reset();
}