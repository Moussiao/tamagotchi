#include "tasks/button.hpp"

void button_task(void *arg) {
  button_task_params_t *params = (button_task_params_t *)arg;

  // Инициализация GPIO
  gpio_config_t io_conf = {
      // .pin_bit_mask = (1ULL << params->button_pin),
      .pin_bit_mask = (1ULL << 13),     .mode = GPIO_MODE_INPUT,
      .pull_up_en = GPIO_PULLUP_ENABLE, .pull_down_en = GPIO_PULLDOWN_DISABLE,
      .intr_type = GPIO_INTR_DISABLE,
  };
  gpio_config(&io_conf);

  ButtonState state = BTN_STATE_RELEASED;
  TickType_t last_change_time = 0;

  while (1) {
    bool current_state = gpio_get_level(GPIO_NUM_13);
    // bool current_state = gpio_get_level(params->button_pin);
    TickType_t now = xTaskGetTickCount();

    switch (state) {
    case BTN_STATE_RELEASED:
      if (!current_state) { // Обнаружено нажатие (LOW)
        state = BTN_STATE_PRESS_DETECTED;
        last_change_time = now;
      }
      break;

    case BTN_STATE_PRESS_DETECTED:
      if (!current_state) {
        if ((now - last_change_time) * portTICK_PERIOD_MS > 25) {
          // if ((now - last_change_time) * portTICK_PERIOD_MS > params->debounce_ms) {
          state = BTN_STATE_PRESSED;
          xSemaphoreGive(params->pressed_sem);
          // xSemaphoreGive(params->pressed_sem);
        }
      } else {
        state = BTN_STATE_RELEASED; // Дребезг, игнорируем
      }
      break;

    case BTN_STATE_PRESSED:
      if (current_state) { // Обнаружено отпускание (HIGH)
        state = BTN_STATE_RELEASE_DETECTED;
        last_change_time = now;
      }
      break;

    case BTN_STATE_RELEASE_DETECTED:
      if (current_state) {
        if ((now - last_change_time) * portTICK_PERIOD_MS > 25) {
          // if ((now - last_change_time) * portTICK_PERIOD_MS > params->debounce_ms) {
          state = BTN_STATE_RELEASED;
        }
      } else {
        state = BTN_STATE_PRESSED; // Дребезг, игнорируем
      }
      break;
    }

    vTaskDelay(10 / portTICK_PERIOD_MS); // Опрос каждые 10 мс
  }
}
