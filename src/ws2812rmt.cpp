#include "ws2812rmt.h"
#include "driver/rmt.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "pins.h"

static constexpr const uint32_t ds = 10;                         // 125ns
static constexpr const uint32_t dl = 80;                         // 1000ns
static constexpr const uint32_t rs = dl * 50;                    // 50us
static constexpr const rmt_item32_t reset = {{{rs, 0, rs, 0}}};  // reset
static constexpr const rmt_item32_t bit0 = {{{ds, 1, dl, 0}}};   // 0
static constexpr const rmt_item32_t bit1 = {{{dl, 1, ds, 0}}};   // 1

static inline void colour_to_rmt(const uint8_t colour,
                                 std::vector<rmt_item32_t> &items) {
  for (int i = 7; i >= 0; i--) {
    if (colour & (1 << i)) {
      items.emplace_back(bit1);
    } else {
      items.emplace_back(bit0);
    }
  }
}

ws2812_rmt::ws2812_rmt(const gpio_num_t gpio, const rmt_channel_t channel,
                       const uint8_t mem_block_num) {
  this->channel = channel;
  rmt_config_t config;
  config.rmt_mode = RMT_MODE_TX;
  config.channel = channel;
  config.gpio_num = gpio;
  config.mem_block_num = mem_block_num;
  config.tx_config.loop_en = false;
  config.tx_config.carrier_en = false;
  config.tx_config.idle_output_en = true;
  config.tx_config.idle_level = RMT_IDLE_LEVEL_LOW;
  config.clk_div = 1;

  ESP_ERROR_CHECK(rmt_config(&config));
  ESP_ERROR_CHECK(rmt_driver_install(config.channel, 0, 0));
}

void ws2812_rmt::operator<<(const std::vector<rgb_t> &items) const {
  std::vector<rmt_item32_t> rmt_items;
  rmt_items.emplace_back(reset);
  for (const auto &item : items) {
    colour_to_rmt(item.g, rmt_items);
    colour_to_rmt(item.r, rmt_items);
    colour_to_rmt(item.b, rmt_items);
  }
  ESP_ERROR_CHECK(
      rmt_write_items(this->channel, rmt_items.data(), rmt_items.size(), true));
}
