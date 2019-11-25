#include "pod.h"
#include "pins.h"
#include "ws2812.h"

#define MIN_DELAY 5

xQueueHandle podevent_queue = nullptr;
xSemaphoreHandle podevent_semaphore = nullptr;

struct pod_state {
  uint8_t number;
  pod_status expected;
  pod_status current;
};

const uint8_t pod_count = 7;
pod_state pod_states[pod_count];

/**
 * This will scan all pods to find out their status and emit
 * appropriate messages on `podevent_queue`.
 */
void podevent_scanner_task(void* arg) {
  gpio_pad_select_gpio(POD_LID_GPIO);
  gpio_set_direction(POD_LID_GPIO, GPIO_MODE_INPUT);
  gpio_set_pull_mode(POD_LID_GPIO, GPIO_PULLUP_ONLY);

  gpio_pad_select_gpio(POD_IR_GPIO);
  gpio_set_pull_mode(POD_IR_GPIO, GPIO_PULLUP_ONLY);
  gpio_set_direction(POD_IR_GPIO, GPIO_MODE_INPUT);

  while (true) {
    // TODO: set pod input mode
    if (xSemaphoreTake(podevent_semaphore, portMAX_DELAY)) {
      pod_event event;
      printf("Scanning...\n");
      if (gpio_stable_get(POD_LID_GPIO) == 1) {
        event.number = 0;
        event.status = opened;
        printf("lid open\n");
        xQueueSendToBack(podevent_queue, &event, 50);
      } else {
        event.number = 0;
        event.status = closed;
        printf("lid closed\n");
        xQueueSendToBack(podevent_queue, &event, 50);
      }
      if (gpio_stable_get(POD_IR_GPIO) == 0) {
        event.number = 0;
        event.status = empty;
        printf("empty\n");
        xQueueSendToBack(podevent_queue, &event, 50);
      }
    }
  }
}

/**
 * Something happened with the pods: this triggers the pod
 * scanning code.
 */
void IRAM_ATTR podevent_isr_handler(void* arg) {
  static auto last_time = 0;
  auto now = xTaskGetTickCountFromISR();
  if (now - last_time > MIN_DELAY) {
    auto ignored = pdFALSE;
    xSemaphoreGiveFromISR(podevent_semaphore, &ignored);
    last_time = now;
  }
}

void podevent_view_task(void* pvParameters) {
  ws2812_init(LED_STRIP_GPIO);

  const auto pixel_count = 28;
  const auto green = makeRGBVal(0, 255, 0);
  const auto red = makeRGBVal(255, 0, 0);
  const auto blue = makeRGBVal(0, 0, 255);
  const auto white = makeRGBVal(10, 10, 10);
  const auto none = makeRGBVal(0, 0, 0);
  auto pixels = new rgb_t[pixel_count];
  for (uint8_t i = 0; i < pixel_count; i++) pixels[i] = none;
  ws2812_set(pixel_count, pixels);

  long current_ts = 0;
  long event_ts = 0;
  pod_event event;
  while (true) {
    current_ts++;
    if (current_ts - event_ts < 1000) {
      // display event
      for (uint8_t i = 0; i < pixel_count; i++) {
        uint8_t pod_number = i / 4;
        if (event.number == pod_number) {
          switch (event.status) {
            case opened:
              pixels[i] = green;
              break;
            case closed:
              pixels[i] = blue;
              break;
            case empty:
              pixels[i] = red;
              break;
          }
        } else {
          pixels[i] = none;
        }
      }
      ws2812_set(pixel_count, pixels);
      if (xQueueReceive(podevent_queue, &event, pdMS_TO_TICKS(20))) {
        event_ts = current_ts;
        printf("Has event!\n");
      }
    } else {
      for (uint8_t i = 0; i < pixel_count; i++) {
        for (uint8_t j = 0; j < pixel_count; j++) {
          if (i == j)
            pixels[j] = white;
          else
            pixels[j] = none;
        }
        ws2812_set(pixel_count, pixels);

        if (xQueueReceive(podevent_queue, &event, pdMS_TO_TICKS(20))) {
          event_ts = current_ts;
          printf("Has event!\n");
          break;
        }
      }
    }
  }
}

void start_pod_tasks() {
  podevent_semaphore = xSemaphoreCreateBinary();
  podevent_queue = xQueueCreate(10, sizeof(pod_event));

  gpio_pad_select_gpio(POD_LID_GPIO);
  gpio_set_direction(POD_LID_GPIO, GPIO_MODE_INPUT);
  gpio_set_pull_mode(POD_LID_GPIO, GPIO_PULLUP_ONLY);
  gpio_set_intr_type(POD_LID_GPIO, GPIO_INTR_ANYEDGE);
  gpio_install_isr_service(ESP_INTR_FLAG_LEVEL1);
  gpio_isr_handler_add(POD_LID_GPIO, podevent_isr_handler, nullptr);

  gpio_pad_select_gpio(POD_IR_GPIO);
  gpio_set_direction(POD_IR_GPIO, GPIO_MODE_INPUT);
  gpio_set_pull_mode(POD_IR_GPIO, GPIO_PULLUP_ONLY);
  gpio_set_intr_type(POD_IR_GPIO, GPIO_INTR_ANYEDGE);
  gpio_install_isr_service(ESP_INTR_FLAG_LEVEL1);
  gpio_isr_handler_add(POD_IR_GPIO, podevent_isr_handler, nullptr);

  xTaskCreate(podevent_view_task, "podevent_view", 4096, nullptr, 10, nullptr);
  xTaskCreate(podevent_scanner_task, "podevent_scanner", 4096, nullptr, 10,
              nullptr);
}
