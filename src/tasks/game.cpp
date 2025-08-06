#include <esp_log.h>
#include <esp_timer.h>

#include "colors.hpp"
#include "tasks/game.hpp"

static const char *TAG = "GAME";

void init_game(int height) {
  game.bird.x = 50;
  game.bird.y = height / 2;
  game.bird.velocity = 0;
  game.pipe_count = 0;
  game.score = 0;
  game.game_over = false;
  game.button_pressed = false;
}

void update_game(ST7789Display *display) {
  if (game.game_over) {
    if (game.button_pressed) {
      // Перезапуск игры
      init_game(display->height());
    }
    game.button_pressed = false;
    return;
  }

  // Физика птицы
  if (game.button_pressed) {
    game.bird.velocity = JUMP_FORCE;
    game.button_pressed = false;
  }

  game.bird.velocity += GRAVITY;
  game.bird.y += game.bird.velocity;

  // Проверка столкновения с землей
  if (game.bird.y + BIRD_SIZE > display->height() - GROUND_HEIGHT) {
    game.bird.y = display->height() - GROUND_HEIGHT - BIRD_SIZE;
    game.game_over = true;
  }

  // Проверка столкновения с потолком
  if (game.bird.y < 0) {
    game.bird.y = 0;
    game.bird.velocity = 0;
  }

  // Обновление труб
  for (int i = 0; i < game.pipe_count; i++) {
    game.pipes[i].x -= PIPE_SPEED;

    // Проверка столкновения с трубой
    if (game.bird.x + BIRD_SIZE > game.pipes[i].x && game.bird.x < game.pipes[i].x + PIPE_WIDTH) {
      if (game.bird.y < game.pipes[i].gap_y - PIPE_GAP / 2 ||
          game.bird.y + BIRD_SIZE > game.pipes[i].gap_y + PIPE_GAP / 2) {
        game.game_over = true;
      }
    }

    // Увеличение счета при прохождении трубы
    if (game.pipes[i].x + PIPE_WIDTH < game.bird.x && !game.pipes[i].passed) {
      game.score++;
      game.pipes[i].passed = true;
    }
  }

  // Удаление труб, вышедших за экран
  while (game.pipe_count > 0 && game.pipes[0].x + PIPE_WIDTH < 0) {
    for (int i = 0; i < game.pipe_count - 1; i++) {
      game.pipes[i] = game.pipes[i + 1];
    }
    game.pipe_count--;
  }

  // Создание новых труб
  static int spawn_counter = 0;
  if (++spawn_counter >= PIPE_SPAWN_RATE) {
    if (game.pipe_count < 3) {
      Pipe new_pipe = {.x = display->width(),
                       .gap_y = 100 + rand() % (display->height() - 200),
                       .passed = false};
      game.pipes[game.pipe_count++] = new_pipe;
    }
    spawn_counter = 0;
  }
}

void draw_game(ST7789Display *dispaly) {
  // Очистка экрана (небо)
  dispaly->draw_rect(0, 0, dispaly->width(), dispaly->height(), color565(0, 191, 255));

  // Отрисовка земли
  dispaly->draw_rect(0, dispaly->height() - GROUND_HEIGHT, dispaly->width(), GROUND_HEIGHT,
                     color565(101, 67, 33));

  // Отрисовка труб
  for (int i = 0; i < game.pipe_count; i++) {
    // Верхняя труба
    dispaly->draw_rect(game.pipes[i].x, 0, PIPE_WIDTH, game.pipes[i].gap_y - PIPE_GAP / 2,
                       color565(0, 255, 0));
    // Нижняя труба
    dispaly->draw_rect(game.pipes[i].x, game.pipes[i].gap_y + PIPE_GAP / 2, PIPE_WIDTH,
                       dispaly->height() - (game.pipes[i].gap_y + PIPE_GAP / 2),
                       color565(0, 255, 0));
  }

  // Отрисовка птицы
  dispaly->draw_rect(game.bird.x, game.bird.y, BIRD_SIZE, BIRD_SIZE, color565(255, 255, 0));

  // Если игра окончена
  if (game.game_over) {
    char game_over_buffer[20];
    snprintf(game_over_buffer, sizeof(game_over_buffer), "СЧЕТ: %d", game.score);
    dispaly->draw_string(dispaly->width() / 2 - 85, dispaly->height() / 2 - 20, game_over_buffer,
                         color565(0, 0, 0), color565(255, 0, 0), 4);

    if (game.max_score < game.score)
      game.max_score = game.score;
  }

  char score_buffer[5];
  snprintf(score_buffer, sizeof(score_buffer), "%d", game.game_over ? game.max_score : game.score);
  dispaly->draw_string(10, 10, score_buffer, color565(0, 0, 0), color565(0, 191, 255), 3);
  dispaly->sumbit_frame();
}

// Задача для игровой логики и отрисовки
void game_task(void *arg) {
  game_task_params_t *params = (game_task_params_t *)arg;

  int64_t frame_start, draw_time;
  int frame_count = 0;

  ST7789Display display(params->display_config);
  display.initialize();

  init_game(display.height());

  int64_t last_time = esp_timer_get_time();

  while (1) {
    if (xSemaphoreTake(params->button_sem, 0)) {
      game.button_pressed = true;
    }

    frame_start = esp_timer_get_time();
    // Расчет дельты времени
    int64_t current_time = esp_timer_get_time();
    float delta = (current_time - last_time) / 1000000.0f;
    last_time = current_time;

    // Обновление игры
    update_game(&display);
    // Отрисовка
    draw_game(&display);
    draw_time = esp_timer_get_time();

    frame_count++;
    if (frame_count % 60 == 0)
      ESP_LOGI(TAG, "Draw: %.1fms", (draw_time - frame_start) / 1000.0);

    // Задержка для стабилизации FPS
    if (delta < 16)
      vTaskDelay(pdMS_TO_TICKS(16 - delta));
  }
}
