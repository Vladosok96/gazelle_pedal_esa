class ECU {
public:
    static void init();
    static void set_throttle(float throttle);
    static void set_relay(bool state);
};