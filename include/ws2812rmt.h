#pragma once
#include <vector>
#include "freertos/FreeRTOS.h"

typedef union {
  struct __attribute__((packed)) {
    uint8_t g, r, b;
  };
  uint32_t num : 24;
} rgb_t;

void rmt_task(void *);
void ws2812_rmt_init(const gpio_num_t gpio);
void ws2812_rmt_set(const std::vector<rgb_t> items);
