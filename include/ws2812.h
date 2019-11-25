/* Created 19 Nov 2016 by Chris Osborn <fozztexx@fozztexx.com>
 * http://insentricity.com
 *
 * This is a driver for the WS2812 RGB LEDs using the RMT peripheral on the
 * ESP32.
 *
 * This code is placed in the public domain (or CC0 licensed, at your option).
 */

#ifndef WS2812_DRIVER_H
#define WS2812_DRIVER_H

#include <stdint.h>

typedef union {
  struct __attribute__((packed)) {
    uint8_t g, r, b;
  };
  uint32_t num : 24;
} rgb_t;

void ws2812_init(int gpioNum);
void ws2812_set(unsigned int length, rgb_t *array);

inline rgb_t makeRGBVal(uint8_t r, uint8_t g, uint8_t b) {
  rgb_t v;

  v.r = r;
  v.g = g;
  v.b = b;
  return v;
}

#endif /* WS2812_DRIVER_H */
