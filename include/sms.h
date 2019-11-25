#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <memory>

using phone_number = std::string;

struct command {
  char code;
  uint8_t param;
};

union event {
  using error_code = uint8_t;
  struct error_event {
    error_code code;
  };
  struct drug_event {};
};

class transport {
 private:
  QueueHandle_t queue_handle;

 public:
  QueueHandle_t begin();
  void send(const phone_number phone_number, const event event);
};
