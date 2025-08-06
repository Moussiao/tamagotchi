#include "fonts.hpp"

#define DEFAULT_FONT_INDEX 63 - 32 * 5 // ?

const uint8_t *get_char_glyph(unsigned char c) {
  uint16_t font_index = DEFAULT_FONT_INDEX;

  if (c <= 126) {
    // ASCII символы
    font_index = (c - 32) * 5;
  } else if (c >= 192) {
    // Кириллица (CP1251)
    font_index = (95 + (c - 192)) * 5;
  }

  return &font_5x7[font_index];
};

unsigned char utf8_to_cp1251(const char **str) {
  unsigned char c = (unsigned char)(*str)[0];

  // Для символов ASCII
  if (c < 0x80) {
    (*str)++;
    return c;
  }

  // Для двухбайтовых символов UTF-8
  if ((c & 0xE0) == 0xC0) {
    unsigned char c2 = (unsigned char)(*str)[1];
    (*str) += 2;

    // Упрощенное преобразование для кириллицы
    if (c == 0xD0) {
      if (c2 >= 0x90 && c2 <= 0xBF)
        return 0xC0 + (c2 - 0x90); // А-п
    } else if (c == 0xD1) {
      if (c2 >= 0x80 && c2 <= 0x8F)
        return 0xE0 + (c2 - 0x80); // р-я
      else if (c2 == 0x91)
        return 0xA8; // Ё
      else if (c2 == 0x81)
        return 0xB8; // ё
    }
  }

  return '?'; // Неподдерживаемый символ
}
