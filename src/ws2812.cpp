#include "ws2812.h"
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

void ws2812_naive_init(const gpio_num_t gpio) {
  gpio_pad_select_gpio(gpio);
  gpio_set_direction(gpio, GPIO_MODE_OUTPUT);
}

void ws2812_naive_set(const rgb_t* pixels, const uint count) {
  gpio_set_level(LED_STRIP_GPIO, 0);
  ets_delay_us(52);
  for (uint i = 0; i < count; i++) {
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

void ws2812_rmt_init(const gpio_num_t gpio,
                     const rmt_channel_t channel,
                     const uint8_t mem_block_num) {
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

static inline void colour_to_rmts(const uint8_t colour, rmt_item32_t *items) {
  for (int i = 7; i >= 0; i--, items++) {
    if (colour & (1 << i)) *items = bit1; else *items = bit0;
  }
}

void ws2812_rmt_set(const rgb_t *pixels,
                    const uint count,
                    const rmt_channel_t channel) {
  const uint rmt_items_size = count * 24 + 1;
  rmt_item32_t rmt_items[rmt_items_size];
  rmt_item32_t *rmt_items_head = rmt_items;
  *rmt_items_head++ = reset;
  for (uint i = 0; i < count; i++, rmt_items_head += 24) {
    const auto &px = pixels[i];
    colour_to_rmts(px.g, rmt_items_head);
    colour_to_rmts(px.r, rmt_items_head + 8);
    colour_to_rmts(px.b, rmt_items_head + 16);
  }
  rmt_write_items(channel, rmt_items, rmt_items_size, true);
}

ws2812_rmt::ws2812_rmt(const gpio_num_t gpio, const rmt_channel_t channel,
                       const uint8_t mem_block_num) {
  this->channel = channel;
  ws2812_rmt_init(gpio, channel, mem_block_num);
}

void ws2812_rmt::operator<<(const std::vector<rgb_t> &items) const {
  ws2812_rmt_set(items.data(), items.size(), this->channel);  
}
