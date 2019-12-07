#pragma once

#include "driver/rmt.h"
#include "freertos/FreeRTOS.h"
#include <vector>

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

void ws2812_naive_init(const gpio_num_t gpio);

void ws2812_naive_set(const rgb_t *pixels, const uint count);

void ws2812_rmt_init(const gpio_num_t gpio,
                     const rmt_channel_t channel = RMT_CHANNEL_0,
                     const uint8_t mem_block_num = 1);

void ws2812_rmt_set(const rgb_t *pixels,
                    const uint count,
                    const rmt_channel_t channel = RMT_CHANNEL_0);

class ws2812_rmt {
private:
    rmt_channel_t channel;

public:
    ws2812_rmt(const gpio_num_t gpio, const rmt_channel_t channel = RMT_CHANNEL_0,
               const uint8_t mem_block_num = 1);

    void operator<<(const std::vector<rgb_t> &items) const;
};
