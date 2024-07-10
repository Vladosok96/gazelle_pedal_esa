#pragma once

#include "pid.h"

class ThrottleControl {
public:
    static void start();
    static void set_target(int32_t rpm);
    static void set_params(float position_P, float position_I, float position_D, float ramp, float limit);
private:
    static void loop(void* args);
    static inline int target_rpm;
    static inline PIDController pid = PIDController(5, 30, 0, 100000, 70);
};