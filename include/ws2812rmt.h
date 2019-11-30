#pragma once
#include <vector>
#include "driver/rmt.h"
#include "freertos/FreeRTOS.h"

typedef union {
  struct __attribute__((packed)) {
    uint8_t g, r, b;
  };
  uint32_t num : 24;
} rgb_t;

inline rgb_t makeRGBVal(uint8_t r, uint8_t g, uint8_t b) {
  rgb_t v;

  v.r = r;
  v.g = g;
  v.b = b;
  return v;
}

class ws2812_rmt {
 private:
  rmt_channel_t channel;

 public:
  ws2812_rmt(const gpio_num_t gpio, const rmt_channel_t channel = RMT_CHANNEL_0,
             const uint8_t mem_block_num = 1);
  void operator<<(const std::vector<rgb_t> &items) const;
};
