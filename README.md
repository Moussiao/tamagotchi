# Tamagotchi (Sattelite) - любительская портативная приставка

## Функционал
1. Простая реализация игры Flappy Bird

## Используемые модули и детали

1. ESP-WROOM-32 (Микроконтроллер)
2. Waveshare 2inch - ST7789 (Дисплей 240x320)
3. Тактовая кнопка 

## Дополнительная информация
| ESP32 | ST7789 | Цвет провода |
|-------|--------|--------------|
| 3.3V  | VCC    | Фиолетовый   |
| GND   | GND    | Белый        |
| GPI023| DIN    | Зеленый      |
| GPI018| CLK    | Оранжевый    |
| GPIO05| CS     | Желтый       |
| GPIO19| DC     | Синий        |
| GPIO04| RST    | Коричневый   |
| GPIO02| BL     | Серый        |

## Установка ESP-IDF и инстурументов для сборки

### ESP-IDS ([Подробное руководство](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/linux-macos-setup.html))

1. Устанавливаем инстуременты для сборки (Debian):

   ```bash
    sudo apt install git wget flex bison gperf python3 python3-pip cmake ninja-build ccache libffi-dev libssl-dev dfu-util
   ```

2. Копируем репозиторий:

   ```bash
   mkdir -p ~/esp
   cd ~/esp
   git clone --recursive https://github.com/espressif/esp-idf.git
   ```

3. Устанавливаем из исходников:

   ```bash
   cd ~/esp/esp-idf
   ./install.sh esp32
   ```

4. Настраимваем переменные окружения:

   ```bash
   . $HOME/esp/esp-idf/export.sh
   ```

### PlatformIO ([Подробное руководство](https://docs.platformio.org/en/latest/core/installation/methods/installer-script.html))

1. Установка (macOS/Linux)

   ```bash
   curl -fsSL -o get-platformio.py https://raw.githubusercontent.com/platformio/platformio-core-installer/master/get-platformio.py
   python3 get-platformio.py
   ```

2. Настраиваем переменные окружения

   Добавьте `export PATH=$PATH:$HOME/.local/bin` в `~/.zshrc`

3. Создать символические ссылки на бинарники platformio

   ```bash
   ln -s ~/.platformio/penv/bin/platformio ~/.local/bin/platformio
   ln -s ~/.platformio/penv/bin/pio ~/.local/bin/pio
   ln -s ~/.platformio/penv/bin/piodebuggdb ~/.local/bin/piodebuggdb
   ```

#### Команды для использования:

1. Сборка проекта

   ```bash
   pio run
   ```

2. Прошивка устройства

   ```bash
   pio run -t upload
   ```

3. Мониторинг порта

   ```bash
   pio device monitor
   ```

4. Очистка проекта

   ```bash
   pio run -t clean
   ```

5. Очистка проекта

   ```bash
   pio run -t compiledb
   ```

## Полезные матерьялы

1. Программирование ESP32 с ESP-IDF в среде platformio:
   https://habr.com/ru/articles/919362/
