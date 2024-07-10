#pragma once

class RPCCommunication {
public:
    static void set_gearbox_state(int32_t is_ready, int32_t gear);
    static void init();
private:
    static void rpc_communication_task(void *args);
    static inline int accelerator = 0;
    static inline int accelerator_relay = 0;
    static inline bool left_turn_lights = false;
    static inline bool right_turn_lights = false;
    static inline bool alarm = false;
    static inline bool signal = false;
};
