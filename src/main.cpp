#include <esp_task_wdt.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <math.h>
#include "pins.h"
#include "sdkconfig.h"
#include "ws2812.h"

#define delay_a() \
  { __asm__ __volatile__("nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;"); };

#define delay_b() \
  {               \
    delay_a();    \
    delay_a();    \
    delay_a();    \
    delay_a();    \
    delay_a();    \
    delay_a();    \
    delay_a();    \
    delay_a();    \
    delay_a();    \
    delay_a();    \
    delay_a();    \
    delay_a();    \
  };

static xSemaphoreHandle semaphore_handle = nullptr;

void led_task(void* arg) {
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

void compute_stuff(void*) {
  while (true) {
    auto start = xTaskGetTickCount();
    double x = 0;
    for (int i = 0; i < 500000; i++) x += sin(i);
    if (x != -120) {
      printf("Took %d ticks\n", xTaskGetTickCount() - start);
      vTaskDelay(1);
      // xSemaphoreGive(semaphore_handle);
    }
  }
}

void IRAM_ATTR button_isr_handler(void* arg) {
  static auto last_time = 0;
  auto now = xTaskGetTickCountFromISR();
  if (now - last_time > 5) {
    auto ignored = pdFALSE;
    xSemaphoreGiveFromISR(semaphore_handle, &ignored);
    last_time = now;
  }
}

void ws2812_naive_set(const int pixel_count, const rgb_t* pixels) {
  gpio_set_level(LED_STRIP_GPIO, 0);
  ets_delay_us(52);
  for (int i = 0; i < pixel_count; i++) {
    const auto pixel = pixels[i];
    bool x[24] = {0};
    for (uint8_t j = 0; j < 8; j++) x[7 - j] = ((pixel.g >> j) & 1) == 0;
    for (uint8_t j = 0; j < 8; j++) x[15 - j] = ((pixel.r >> j) & 1) == 0;
    for (uint8_t j = 0; j < 8; j++) x[23 - j] = ((pixel.b >> j) & 1) == 0;
    for (uint8_t j = 0; j < 24; j++) {
      if (!x[j]) {
        GPIO.out_w1ts = 1 << LED_STRIP_GPIO;
        delay_b();
        GPIO.out_w1tc = 1 << LED_STRIP_GPIO;
        delay_a();
      }
      if (x[j]) {
        GPIO.out_w1ts = 1 << LED_STRIP_GPIO;
        delay_a();
        GPIO.out_w1tc = 1 << LED_STRIP_GPIO;
        delay_b();
      }
    }
  }
}

void led_strip_task(void*) {
  esp_task_wdt_init(portMAX_DELAY, false);
  bool naive = false;
  if (naive) {
    gpio_pad_select_gpio(LED_STRIP_GPIO);
    gpio_set_direction(LED_STRIP_GPIO, GPIO_MODE_OUTPUT);
  } else {
    ws2812_init(LED_STRIP_GPIO);
  }
  const auto pixel_count = 8;
  auto pixels = new rgb_t[pixel_count];
  auto color = makeRGBVal(200, 0, 0);
  uint8_t step = 0;

  while (1) {
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
    for (int i = 0; i < pixel_count; i++) pixels[i] = color;

    if (naive) {
      ws2812_naive_set(pixel_count, pixels);
      vTaskDelay(pdMS_TO_TICKS(50));
    } else
      ws2812_set(pixel_count, pixels);
  }
}

extern "C" void app_main(void) {
  // gpio_pad_select_gpio(CLK_GPIO);
  // gpio_set_direction(CLK_GPIO, GPIO_MODE_OUTPUT);

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

  xTaskCreate(led_strip_task, "led strip", 1024, nullptr, 10, nullptr);
  xTaskCreate(compute_stuff, "compute stuff 1", 2048, nullptr, 10, nullptr);
  xTaskCreate(compute_stuff, "compute stuff 2", 2048, nullptr, 10, nullptr);
  xTaskCreate(compute_stuff, "compute stuff 2", 2048, nullptr, 10, nullptr);
}
