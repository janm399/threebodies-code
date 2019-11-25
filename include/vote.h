#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

class vote {
 private:
 public:
  QueueHandle_t begin();
  void vote_now();
};
