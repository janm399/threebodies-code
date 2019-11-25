#include "pins.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

int gpio_stable_get(gpio_num_t pin, int iterations, int threshold) {
  int lows = 0, highs = 0;
  for (int i = 0; i < iterations; i++) {
    if (gpio_get_level(pin) == 0)
      lows++;
    else
      highs++;
    vTaskDelay(1);
  }

  if (lows >= iterations - threshold) return 0;
  if (highs >= iterations - threshold) return 1;

  return -1;
}

bool gpio_stable_check(gpio_num_t pin, int expected, int iterations,
                       int threshold) {
  return gpio_stable_get(pin, iterations, threshold) == expected;
}