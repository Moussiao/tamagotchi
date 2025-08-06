#include <esp_lcd_panel_ops.h>
#include <esp_lcd_panel_vendor.h>
#include <esp_log.h>

#include "displays/st7789.hpp"
#include "fonts.hpp"

static const char *TAG = "ST7789";

ST7789Display::ST7789Display(const Config &config) : config_(config) {}

ST7789Display::~ST7789Display() {
  if (panel_handle_) {
    esp_lcd_panel_del(panel_handle_);
  }
  if (io_handle_) {
    esp_lcd_panel_io_del(io_handle_);
  }
}

void ST7789Display::initialize() {
  // 1. Настройка SPI для панели
  spi_bus_config_t buscfg = {
      .mosi_io_num = config_.pin_mosi,
      .miso_io_num = config_.pin_miso,
      .sclk_io_num = config_.pin_sclk,
      .quadwp_io_num = -1,
      .quadhd_io_num = -1,
      .data4_io_num = -1,
      .data5_io_num = -1,
      .data6_io_num = -1,
      .data7_io_num = -1,
      .max_transfer_sz = static_cast<int>(config_.width * sizeof(uint16_t)),
      .flags = SPICOMMON_BUSFLAG_MASTER | SPICOMMON_BUSFLAG_SCLK,
      .isr_cpu_id = ESP_INTR_CPU_AFFINITY_AUTO,
      .intr_flags = 0,
  };
  ESP_ERROR_CHECK(spi_bus_initialize(config_.spi_host, &buscfg, SPI_DMA_CH_AUTO));

  // Конфигурация панели
  esp_lcd_panel_io_spi_config_t io_config = {
      .cs_gpio_num = config_.pin_cs,
      .dc_gpio_num = config_.pin_dc,
      .spi_mode = 0,
      .pclk_hz = config_.spi_freq_hz,
      .trans_queue_depth = 10,
      .on_color_trans_done = NULL,
      .user_ctx = NULL,
      .lcd_cmd_bits = 8,
      .lcd_param_bits = 8,
      .cs_ena_pretrans = 1,
      .cs_ena_posttrans = 1,
      .flags =
          {
              .dc_high_on_cmd = 0,
              .dc_low_on_data = 0,
              .dc_low_on_param = false,
              .octal_mode = false,
          },
  };
  ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)config_.spi_host, &io_config,
                                           &io_handle_));
  // Создание панели ST7789
  esp_lcd_panel_dev_config_t panel_config = {
      .reset_gpio_num = config_.pin_rst,
      .color_space = ESP_LCD_COLOR_SPACE_RGB,
      .data_endian = LCD_RGB_DATA_ENDIAN_LITTLE,
      .bits_per_pixel = 16,
  };
  ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(io_handle_, &panel_config, &panel_handle_));

  // Сброс дисплея
  esp_lcd_panel_reset(panel_handle_);

  // Инициализация дисплея
  esp_lcd_panel_init(panel_handle_);

  // Установка 16-битного режима
  uint8_t colmod = 0x55;
  esp_lcd_panel_io_tx_param(io_handle_, 0x3A, &colmod, 1);

  // Альбомная RGB - 0х60
  // Портретная RGB - 0x00
  uint8_t madctl = 0x60;
  esp_lcd_panel_io_tx_param(io_handle_, 0x36, &madctl, 1);

  // Настройка цветов
  esp_lcd_panel_invert_color(panel_handle_, true); // Инвертировать цвета

  // Установка смещения (если необходимо)
  esp_lcd_panel_set_gap(panel_handle_, config_.offset_x, config_.offset_y);

  // Настройка подсветки
  if (config_.pin_bckl >= 0) {
    gpio_reset_pin((gpio_num_t)config_.pin_bckl);
    gpio_set_direction((gpio_num_t)config_.pin_bckl, GPIO_MODE_OUTPUT);
    set_backlight(true);
  }

  // Включение дисплея
  esp_lcd_panel_disp_on_off(panel_handle_, true);

  frame_buffer_ = create_frame_buffer_();
  ESP_LOGI(TAG, "Display initialized");
}

void ST7789Display::set_backlight(bool on) {
  if (config_.pin_bckl >= 0) {
    gpio_set_level((gpio_num_t)config_.pin_bckl,
                   on ? (config_.bckl_active ? 1 : 0) : (config_.bckl_active ? 0 : 1));
  }
}

void ST7789Display::clear(uint16_t color) { draw_rect(0, 0, config_.width, config_.height, color); }

void ST7789Display::draw_pixel(int x, int y, uint16_t color) {}

void ST7789Display::draw_rect(int x, int y, int width, int height, uint16_t color) {
  if (x > config_.width || y > config_.height || width < 0 || height < 0)
    return;

  if (x < 0) {
    width += x;
    x = 0;
  }
  if (y < 0) {
    height += y;
    y = 0;
  }
  if (x + width > config_.width)
    width = config_.width - x;
  if (y + height > config_.height)
    height = config_.height - y;

  for (int row = y; row < y + height; row++) {
    uint16_t *line = &frame_buffer_[row * config_.width + x];
    for (int col = 0; col < width; col++) {
      line[col] = color;
    }
  }
}

void ST7789Display::draw_char(uint16_t x, uint16_t y, unsigned char c, uint16_t color,
                              uint16_t bg_color, uint8_t scale) {
  const uint8_t *glyph = get_char_glyph(c);

  for (int col = 0; col < CHAR_WIDTH; col++) {
    uint8_t col_data = glyph[col];
    for (int row = 0; row < CHAR_HEIGHT; row++) {
      uint16_t pixel_color = (col_data & (1 << row)) ? color : bg_color;

      // Масштабирование
      for (int sy = 0; sy < scale; sy++) {
        for (int sx = 0; sx < scale; sx++) {
          uint16_t px = x + col * scale + sx;
          uint16_t py = y + row * scale + sy;

          if (px < config_.width && py < config_.height) {
            frame_buffer_[py * config_.width + px] = pixel_color;
          }
        }
      }
    }
  }

  // Межсимвольный пробел
  for (int row = 0; row < CHAR_HEIGHT * scale; row++) {
    for (int s = 0; s < CHAR_SPACING * scale; s++) {
      uint16_t px = x + CHAR_WIDTH * scale + s;
      uint16_t py = y + row;

      if (px < config_.width && py < config_.height) {
        frame_buffer_[py * config_.width + px] = bg_color;
      }
    }
  }
}
void ST7789Display::draw_string(uint16_t x, uint16_t y, const char *str, uint16_t color,
                                uint16_t bg_color, uint8_t scale) {
  while (*str) {
    unsigned char c = utf8_to_cp1251(&str);
    draw_char(x, y, c, color, bg_color, scale);
    x += (CHAR_WIDTH + CHAR_SPACING) * scale;
  }
}

void ST7789Display::sumbit_frame() {
  esp_lcd_panel_draw_bitmap(panel_handle_, 0, 0, config_.width, config_.height, frame_buffer_);
}

uint16_t *ST7789Display::create_frame_buffer_() {
  ssize_t buffer_size = config_.width * config_.height * sizeof(uint16_t);
  // Пытаемся выделить в PSRAM, если доступна
  uint16_t *frame_buffer = (uint16_t *)heap_caps_malloc(buffer_size, MALLOC_CAP_SPIRAM);
  if (!frame_buffer) {
    ESP_LOGW(TAG, "PSRAM not available, using internal RAM");
    frame_buffer = (uint16_t *)heap_caps_malloc(buffer_size, MALLOC_CAP_DMA);
  }

  return frame_buffer;
}
