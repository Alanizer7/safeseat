#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_rom_sys.h"
#include "esp_timer.h"
#include "hcsr04.h"

void ultrasonic_init(void);
float ultrasonic_read_distance_cm(void);

#define DHT11_PIN GPIO_NUM_21 

static inline void delay_us(uint32_t us) {
    esp_rom_delay_us(us);
}

static int32_t wait_for_level(int level, uint32_t timeout_us) {
    int64_t start = esp_timer_get_time();
    while (gpio_get_level(DHT11_PIN) != level) {
        if ((esp_timer_get_time() - start) > timeout_us) {
            return -1;
        }
    }
    return (int32_t)(esp_timer_get_time() - start);
}

static int dht11_read_data(uint8_t* h_int, uint8_t* h_dec,
                           uint8_t* t_int, uint8_t* t_dec) {
    uint8_t data[5] = {0};

    gpio_set_direction(DHT11_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(DHT11_PIN, 0);
    vTaskDelay(pdMS_TO_TICKS(20));
    gpio_set_level(DHT11_PIN, 1);
    delay_us(30);
    gpio_set_direction(DHT11_PIN, GPIO_MODE_INPUT);

    if (wait_for_level(0, 100) < 0) return -1;
    if (wait_for_level(1, 100) < 0) return -1;
    if (wait_for_level(0, 100) < 0) return -1;

    for (int i = 0; i < 40; i++) {
        if (wait_for_level(1, 70) < 0) return -1;
        int64_t start = esp_timer_get_time();
        if (wait_for_level(0, 100) < 0) return -1;
        int32_t pulse_width = (int32_t)(esp_timer_get_time() - start);

        if (pulse_width > 40) {
            data[i / 8] |= (1 << (7 - (i % 8)));
        }
    }

    uint8_t checksum = data[0] + data[1] + data[2] + data[3];
    if (checksum != data[4]) {
        return -1;
    }

    *h_int = data[0];
    *h_dec = data[1];
    *t_int = data[2];
    *t_dec = data[3];
    return 0;
}

void app_main(void) {
    ultrasonic_init();

    vTaskDelay(pdMS_TO_TICKS(2000));

    while (1) {
        uint8_t h_int = 0, h_dec = 0;
        uint8_t t_int = 0, t_dec = 0;
        if (dht11_read_data(&h_int, &h_dec, &t_int, &t_dec) == 0) {
            printf("Humidity: %d.%d%%   ", h_int, h_dec);
            printf("Temperature: %d.%d°C   ", t_int, t_dec);
        } else {
            printf("DHT11 read error or checksum mismatch.\n");
        }

        float distance_cm = ultrasonic_read_distance_cm();
        if (distance_cm < 0.0f) {
            printf("HC-SR04 -> Out of range or sensor error.\n");
        } else {
            printf("Distance: %.2f cm\n", distance_cm);
        }

        vTaskDelay(pdMS_TO_TICKS(1000)); 
    }
}
