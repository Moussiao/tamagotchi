#pragma once
#include "displays/st7789.hpp"

#define GRAVITY 0.5
#define JUMP_FORCE -8.0
#define PIPE_WIDTH 50
#define PIPE_GAP 120
#define PIPE_SPEED 3
#define GROUND_HEIGHT 30
#define BIRD_SIZE 20
#define PIPE_SPAWN_RATE 100

typedef struct {
  float x, y;
  float velocity;
} Bird;

typedef struct {
  int x;
  int gap_y;
  bool passed;
} Pipe;

typedef struct {
  Bird bird;
  Pipe pipes[3];
  int pipe_count;
  int score;
  int max_score;
  bool game_over;
  bool button_pressed;
} GameState;

static GameState game;

typedef struct {
  ST7789Display::Config display_config;
  SemaphoreHandle_t button_sem;
} game_task_params_t;

void init_game(int height);
void update_game(ST7789Display *display);
void draw_game(ST7789Display *dispaly);
void game_task(void *arg);
