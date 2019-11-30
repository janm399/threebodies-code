#include "ws2812rmt.h"
#include "driver/rmt.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "pins.h"

#define RMT_TX_CHANNEL RMT_CHANNEL_0

void ws2812_rmt_init(const gpio_num_t gpio) {
  rmt_config_t config;
  config.rmt_mode = RMT_MODE_TX;
  config.channel = RMT_TX_CHANNEL;
  config.gpio_num = gpio;
  config.mem_block_num = 1;
  config.tx_config.loop_en = false;
  config.tx_config.carrier_en = false;
  config.tx_config.idle_output_en = true;
  config.tx_config.idle_level = RMT_IDLE_LEVEL_LOW;
  config.clk_div = 4;

  ESP_ERROR_CHECK(rmt_config(&config));
  ESP_ERROR_CHECK(rmt_driver_install(config.channel, 0, 0));
}

const uint32_t ds = 4;
const uint32_t dl = 20;                         // 1us
const uint32_t rs = dl * 50;                    // 50us
const rmt_item32_t reset = {{{rs, 0, rs, 0}}};  // reset

inline void colour_to_rmt(const uint8_t colour,
                          std::vector<rmt_item32_t> &items) {
  const rmt_item32_t bit0 = {{{ds, 1, dl, 0}}};  // 0
  const rmt_item32_t bit1 = {{{dl, 1, ds, 0}}};  // 1
  for (int i = 7; i >= 0; i--) {
    if (colour & (1 << i)) {
      items.emplace_back(bit1);
    } else {
      items.emplace_back(bit0);
    }
  }
}

void ws2812_rmt_set(const std::vector<rgb_t> items) {
  std::vector<rmt_item32_t> rmt_items;
  rmt_items.emplace_back(reset);
  for (const auto &item : items) {
    colour_to_rmt(item.g, rmt_items);
    colour_to_rmt(item.r, rmt_items);
    colour_to_rmt(item.b, rmt_items);
  }
  ESP_ERROR_CHECK(rmt_write_items(RMT_TX_CHANNEL, rmt_items.data(),
                                  rmt_items.size(), true));
}