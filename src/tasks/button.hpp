#pragma once
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>

typedef enum {
  BTN_STATE_RELEASED,
  BTN_STATE_PRESS_DETECTED,
  BTN_STATE_PRESSED,
  BTN_STATE_RELEASE_DETECTED
} ButtonState;

typedef struct {
  gpio_num_t button_pin;
  uint8_t debounce_ms;
  SemaphoreHandle_t pressed_sem;
} button_task_params_t;

void button_task(void *arg);
