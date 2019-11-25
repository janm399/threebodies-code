#include <driver/gpio.h>

#define BUTTON_GPIO GPIO_NUM_17
#define BLINK_GPIO GPIO_NUM_5
#define CLK_GPIO GPIO_NUM_15

#define POD_IR_GPIO GPIO_NUM_22
#define POD_LID_GPIO GPIO_NUM_23
#define LED_STRIP_GPIO GPIO_NUM_18

bool gpio_stable_check(gpio_num_t pin, int expected, int iterations = 20,
                       int threshold = 5);
int gpio_stable_get(gpio_num_t pin, int iterations = 20, int theshold = 5);