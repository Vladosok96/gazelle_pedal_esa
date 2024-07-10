#pragma once
#include "driver/pulse_cnt.h"

class RPMCounter {
public:
    static void init();
    static float getRPM();
private:
    static bool pcnt_on_reach(pcnt_unit_handle_t unit, const pcnt_watch_event_data_t *edata, void *user_ctx);
    static void task(void* args);
    static inline float rpm = 0;
    static inline int64_t lastReachedTime = 0;
};