#include "tasks/button.hpp"
#include "tasks/game.hpp"

// Общая конфигурация
#define LCD_HOST SPI2_HOST
#define LCD_HZ 80 * 1000 * 1000
#define LCD_WIDTH 320
#define LCD_HEIGHT 240
#define Y_OFFSET 0
#define DEBOUNCE_TIME_MS 25

// Конфигурация пинов
#define PIN_NUM_BUTTON GPIO_NUM_13
#define PIN_NUM_MOSI GPIO_NUM_23
#define PIN_NUM_SCLK GPIO_NUM_18
#define PIN_NUM_CS GPIO_NUM_5
#define PIN_NUM_DC GPIO_NUM_19
#define PIN_NUM_RST GPIO_NUM_4
#define PIN_NUM_BCKL GPIO_NUM_2
#define PIN_NUM_MISO GPIO_NUM_NC

extern "C" void app_main() {
  ST7789Display::Config display_config = {
      .spi_host = LCD_HOST,
      .pin_mosi = PIN_NUM_MOSI,
      .pin_miso = PIN_NUM_MISO,
      .pin_sclk = PIN_NUM_SCLK,
      .pin_cs = PIN_NUM_CS,
      .pin_dc = PIN_NUM_DC,
      .pin_rst = PIN_NUM_RST,
      .pin_bckl = PIN_NUM_BCKL,
      .width = LCD_WIDTH,
      .height = LCD_HEIGHT,
      .offset_x = 0,
      .offset_y = 0,
      .bckl_active = 1,
      .spi_freq_hz = LCD_HZ,
  };
  SemaphoreHandle_t button_sem = xSemaphoreCreateBinary();

  static button_task_params_t button_task_params = {
      .button_pin = PIN_NUM_BUTTON,
      .debounce_ms = DEBOUNCE_TIME_MS,
      .pressed_sem = button_sem,
  };
  static game_task_params_t game_task_params = {
      .display_config = display_config,
      .button_sem = button_sem,
  };

  xTaskCreate(game_task, "game_task", 4096, &game_task_params, 5, NULL);
  xTaskCreate(button_task, "button_task", 4096, &button_task_params, 10, NULL);
}
