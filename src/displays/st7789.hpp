#pragma once
#include <driver/gpio.h>
#include <esp_lcd_panel_io.h>

#define CHAR_WIDTH 5
#define CHAR_HEIGHT 7
#define CHAR_SPACING 1

class ST7789Display {
public:
  struct Config {
    spi_host_device_t spi_host = SPI2_HOST;
    gpio_num_t pin_mosi;
    gpio_num_t pin_miso;
    gpio_num_t pin_sclk;
    gpio_num_t pin_cs;
    gpio_num_t pin_dc;
    gpio_num_t pin_rst;
    gpio_num_t pin_bckl;
    int width = 240;
    int height = 240;
    int offset_x = 0;
    int offset_y = 0;
    uint8_t bckl_active = 1;
    uint32_t spi_freq_hz = 40 * 1000 * 1000; // 40 MHz
  };

  ST7789Display(const Config &config);
  ~ST7789Display();

  // Инициализация дисплея
  void initialize();

  // Управление подсветкой
  void set_backlight(bool on);

  // Основные операции
  void clear(uint16_t color);
  void draw_pixel(int x, int y, uint16_t color);
  void draw_rect(int x, int y, int width, int height, uint16_t color);
  void draw_char(uint16_t x, uint16_t y, unsigned char c, uint16_t color, uint16_t bg_color,
                 uint8_t scale);
  void draw_string(uint16_t x, uint16_t y, const char *str, uint16_t color, uint16_t bg_color,
                   uint8_t scale);
  void sumbit_frame();

  // Размеры дисплея
  int width() const { return config_.width; }
  int height() const { return config_.height; }

private:
  Config config_;
  esp_lcd_panel_io_handle_t io_handle_ = nullptr;
  esp_lcd_panel_handle_t panel_handle_ = nullptr;
  uint16_t *frame_buffer_;
  uint16_t *create_frame_buffer_();
};
