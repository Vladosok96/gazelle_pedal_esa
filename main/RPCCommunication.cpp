#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "RPCClient.h"
#include "RPCCommunication.h"
#include "Can.h"
#include "RPMCounter.h"

static const char *TAG = "RPCCOMMUNICATION";

void RPCCommunication::rpc_communication_task(void *args) {
    RPCClient::init();

    struct sockaddr_in dest_addr_main;
    dest_addr_main.sin_addr.s_addr = inet_addr("192.168.1.101");
    dest_addr_main.sin_family = AF_INET;
    dest_addr_main.sin_port = htons(90);

    /*

    struct sockaddr_in dest_addr_accelerator;
    dest_addr_accelerator.sin_addr.s_addr = inet_addr("192.168.X.X");
    dest_addr_accelerator.sin_family = AF_INET;
    dest_addr_accelerator.sin_port = htons(90);

    struct sockaddr_in dest_addr_relay;
    dest_addr_relay.sin_addr.s_addr = inet_addr("192.168.X.X");
    dest_addr_relay.sin_family = AF_INET;
    dest_addr_relay.sin_port = htons(90);

    */

    while (true) {
        RPCClient::send_request("set_rpm_speed", (int32_t)RPMCounter::getRPM(), (int32_t)CAN::speed, dest_addr_main);
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void RPCCommunication::init() {
    xTaskCreate(rpc_communication_task, "rpc com task", 8192, NULL, 24, NULL);
}
