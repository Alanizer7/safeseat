#include <stdio.h>
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_rom_sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define TRIG_PIN GPIO_NUM_18
#define ECHO_PIN GPIO_NUM_19

static inline void delay_us(uint32_t us) {
    esp_rom_delay_us(us);
}

static int32_t wait_for_level(gpio_num_t pin, int level, uint32_t timeout_us) {
    int64_t start = esp_timer_get_time();
    while (gpio_get_level(pin) != level) {
        if ((esp_timer_get_time() - start) > timeout_us) {
            return -1;
        }
    }
    return (int32_t)(esp_timer_get_time() - start);
}

void ultrasonic_init(void) {
    gpio_reset_pin(TRIG_PIN);
    gpio_set_direction(TRIG_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(TRIG_PIN, 0);

    gpio_reset_pin(ECHO_PIN);
    gpio_set_direction(ECHO_PIN, GPIO_MODE_INPUT);
}

float ultrasonic_read_distance_cm(void) {
    gpio_set_level(TRIG_PIN, 1);
    delay_us(10);
    gpio_set_level(TRIG_PIN, 0);

    if (wait_for_level(ECHO_PIN, 1, 1000000) < 0) {
        return -1.0f;
    }
    int64_t start_us = esp_timer_get_time();

    if (wait_for_level(ECHO_PIN, 0, 1000000) < 0) {
        return -1.0f;
    }
    int64_t end_us = esp_timer_get_time();

    float pulse_width_us = (float)(end_us - start_us);
    float distance_cm = pulse_width_us / 58.0f;
    if (distance_cm < 2.0f || distance_cm > 400.0f) {
        return -1.0f;
    }
    return distance_cm;
}
