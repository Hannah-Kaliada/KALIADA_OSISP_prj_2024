#ifndef GAME_H
#define GAME_H

#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define DELAY 150000
#define MAX_FOOD 5
#define MIN_FOOD_LIFETIME 20
#define MAX_FOOD_LIFETIME 200
#define MAX_POISON_FOOD 2
#define MAX_OBSTACLES 10

typedef struct SnakeSegment {
    int x, y;
    struct SnakeSegment *next;
} SnakeSegment;

typedef struct {
    int x, y;
    int lifetime;
    char symbol;
} Food;

typedef struct {
    int x, y;
    char symbol;
} Obstacle;

void init_game(WINDOW *win, int *max_x, int *max_y, SnakeSegment **snake, Food food[], Food poison_food[], Obstacle obstacles[]);
void draw_snake(WINDOW *win, SnakeSegment *snake);
void draw_food(WINDOW *win, Food food[]);
void draw_poison_food(WINDOW *win, Food poison_food[]);
void draw_obstacles(WINDOW *win, Obstacle obstacles[]);
void move_snake(SnakeSegment **snake, int direction);
int check_collision(SnakeSegment *snake, int max_x, int max_y, Obstacle obstacles[]);
int eat_food(SnakeSegment *snake, Food food[], Food poison_food[], int *score);
void grow_snake(SnakeSegment *snake);
void spawn_food(Food food[], int max_x, int max_y);
void spawn_poison_food(Food poison_food[], int max_x, int max_y);
void update_food(Food food[]);
void update_poison_food(Food poison_food[]);
void draw_statistics(WINDOW *win, int score, time_t start_time);
int game_over_screen(int score, int best_score);
int start_game();

#endif // GAME_H
