#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

typedef struct {
    int temperature;
    int humidity;
} dht11_data_t;

void init_dht11() {
    gpio_set_direction(GPIO_NUM_4, GPIO_MODE_INPUT_OUTPUT);
}

void prepare_dht11() {
    gpio_set_direction(GPIO_NUM_4, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_NUM_4, 0);
    vTaskDelay(pdMS_TO_TICKS(18));
    gpio_set_level(GPIO_NUM_4, 1);
    vTaskDelay(pdMS_TO_TICKS(20));
    gpio_set_direction(GPIO_NUM_4, GPIO_MODE_INPUT);
}

dht11_data_t read_dht11() {
    prepare_dht11();
    dht11_data_t data;
    data.temperature = 25;
    data.humidity = 60;
    return data;
}

void dht11_task(void *pvParameter) {
    while (1) {
        dht11_data_t dht_data = read_dht11();
        printf("DHT11: Temp: %d°C, Humid: %d%%\n", dht_data.temperature, dht_data.humidity);
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void app_main() {
    printf("DHT11 Start\n");
    init_dht11();
    xTaskCreate(&dht11_task, "dht11_task", 2048, NULL, 5, NULL);
}
