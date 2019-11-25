#pragma once
#include <driver/gpio.h>
#include <esp_task_wdt.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <math.h>
#include "sdkconfig.h"

extern xQueueHandle podevent_queue;

enum pod_status { closed, opened, empty };

struct pod_event {
  uint8_t number;
  pod_status status;
};

void start_pod_tasks();

// void end_pod_tasks();