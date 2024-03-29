#include <esp_task_wdt.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <cmath>
#include "esp_log.h"
#include "pins.h"
#include "sdkconfig.h"
#include "ws2812.h"
#include "uartx.h"

static xSemaphoreHandle semaphore_handle = nullptr;

[[noreturn]]
void led_task(void *) {
    bool x = false;
    while (true) {
        xSemaphoreTake(semaphore_handle, portMAX_DELAY);
        x = !x;
        for (int i = 0; i < 20; i++) {
            gpio_set_level(BLINK_GPIO, i % 2 == 0);
            vTaskDelay(pdMS_TO_TICKS(50));
        }
        gpio_set_level(BLINK_GPIO, x);
    }
}

[[noreturn]]
void compute_stuff(void *) {
    while (true) {
        auto start = xTaskGetTickCount();
        double x = 0;
        for (int i = 0; i < 500000; i++) x += sin(i);
        if (x != -120) {
            ESP_LOGI("MAIN", "Took %d ticks\n", xTaskGetTickCount() - start);
            vTaskDelay(1);
            // xSemaphoreGive(semaphore_handle);
        }
    }
}

void IRAM_ATTR button_isr_handler(void *arg) {
    static auto last_time = 0;
    auto now = xTaskGetTickCountFromISR();
    if (now - last_time > 5) {
        auto ignored = pdFALSE;
        xSemaphoreGiveFromISR(semaphore_handle, &ignored);
        last_time = now;
    }
}

[[noreturn]]
void led_strip_task(void *) {
    esp_task_wdt_init(portMAX_DELAY, false);
//#define NAIVE
#ifdef NAIVE
    gpio_pad_select_gpio(LED_STRIP_GPIO);
    gpio_set_direction(LED_STRIP_GPIO, GPIO_MODE_OUTPUT);
#else
    //ws2812_rmt ws2812(LED_STRIP_GPIO);
    ws2812_rmt_init(LED_STRIP_GPIO);
#endif
    const auto pixel_count = 8;
    rgb_t pixels[pixel_count];
    rgb_t color = makeRGBVal(200, 0, 0);
    uint8_t step = 0;

    while (true) {
        switch (step) {
            case 0:
                color.r++;
                if (color.r == 255) step = 1;
                break;
            case 1:
                color.g++;
                if (color.g == 255) step = 2;
                break;
            case 2:
                color.b++;
                if (color.b == 255) step = 3;
                break;
            case 3:
                color.g--;
                if (color.g == 0) step = 4;
                break;
            case 4:
                color.r--;
                if (color.r == 0) step = 5;
                break;
            case 5:
                color.b--;
                if (color.b == 0) step = 0;
                break;
        }
        for (auto &pixel : pixels) pixel = color;

#ifdef NAIVE
        ws2812_naive_set(pixels, pixel_count);
#else
        //ws2812 << pixels;
        ws2812_rmt_set(pixels, pixel_count);
#endif
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

extern "C" void app_main(void) {
    gpio_pad_select_gpio(BUTTON_GPIO);
    gpio_set_direction(BUTTON_GPIO, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_GPIO, GPIO_PULLUP_ONLY);
    gpio_set_intr_type(BUTTON_GPIO, GPIO_INTR_NEGEDGE);

    gpio_pad_select_gpio(BLINK_GPIO);
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);

    semaphore_handle = xSemaphoreCreateBinary();
    xTaskCreate(led_task, "led_task", 1048, nullptr, 10, nullptr);
    gpio_install_isr_service(ESP_INTR_FLAG_LEVEL1);
    gpio_isr_handler_add(BUTTON_GPIO, button_isr_handler, nullptr);

    xTaskCreate(led_strip_task, "led strip", 8192, nullptr, 10, nullptr);
    xTaskCreate(sms_task, "SMS task", 2048, nullptr, 10, nullptr);
//    xTaskCreate(compute_stuff, "compute stuff 1", 2048, nullptr, 10, nullptr);
//    xTaskCreate(compute_stuff, "compute stuff 2", 2048, nullptr, 10, nullptr);
//    xTaskCreate(compute_stuff, "compute stuff 3", 2048, nullptr, 10, nullptr);
}
