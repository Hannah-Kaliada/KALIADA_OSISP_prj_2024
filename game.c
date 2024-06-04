#include "game.h"

void init_game(WINDOW *win, int *max_x, int *max_y, SnakeSegment **snake, Food food[], Food poison_food[], Obstacle obstacles[]) {
    getmaxyx(win, *max_y, *max_x);
    *max_y -= 2;
    *max_x -= 2;

    *snake = (SnakeSegment *)malloc(sizeof(SnakeSegment));
    (*snake)->x = *max_x / 2;
    (*snake)->y = *max_y / 2;
    (*snake)->next = NULL;

    for (int i = 0; i < MAX_FOOD; i++) {
        food[i].x = rand() % (*max_x - 4) + 2;
        food[i].y = rand() % (*max_y - 4) + 2;
        food[i].lifetime = rand() % (MAX_FOOD_LIFETIME - MIN_FOOD_LIFETIME + 1) + MIN_FOOD_LIFETIME;
        food[i].symbol = '@';
    }

    for (int i = 0; i < MAX_POISON_FOOD; i++) {
        poison_food[i].x = rand() % (*max_x - 4) + 2;
        poison_food[i].y = rand() % (*max_y - 4) + 2;
        poison_food[i].lifetime = rand() % (MAX_FOOD_LIFETIME - MIN_FOOD_LIFETIME + 1) + MIN_FOOD_LIFETIME;
        poison_food[i].symbol = '*';
    }

    for (int i = 0; i < MAX_OBSTACLES; i++) {
        obstacles[i].x = rand() % (*max_x - 4) + 2;
        obstacles[i].y = rand() % (*max_y - 4) + 2;
        obstacles[i].symbol = (rand() % 2) ? '|' : '-';
    }
}

void draw_snake(WINDOW *win, SnakeSegment *snake) {
    SnakeSegment *current = snake;
    while (current != NULL) {
        mvwprintw(win, current->y, current->x, "O");
        current = current->next;
    }
}

void draw_food(WINDOW *win, Food food[]) {
    for (int i = 0; i < MAX_FOOD; i++) {
        if (food[i].lifetime > 0) {
            mvwprintw(win, food[i].y, food[i].x, "%c", food[i].symbol);
        }
    }
}

void draw_poison_food(WINDOW *win, Food poison_food[]) {
    for (int i = 0; i < MAX_POISON_FOOD; i++) {
        if (poison_food[i].lifetime > 0) {
            mvwprintw(win, poison_food[i].y, poison_food[i].x, "%c", poison_food[i].symbol);
        }
    }
}

void draw_obstacles(WINDOW *win, Obstacle obstacles[]) {
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        mvwprintw(win, obstacles[i].y, obstacles[i].x, "%c", obstacles[i].symbol);
    }
}

void move_snake(SnakeSegment **snake, int direction) {
    SnakeSegment *new_head = (SnakeSegment *)malloc(sizeof(SnakeSegment));
    new_head->x = (*snake)->x;
    new_head->y = (*snake)->y;
    new_head->next = *snake;

    switch (direction) {
        case KEY_UP:
            new_head->y -= 1;
            break;
        case KEY_DOWN:
            new_head->y += 1;
            break;
        case KEY_LEFT:
            new_head->x -= 1;
            break;
        case KEY_RIGHT:
            new_head->x += 1;
            break;
    }

    *snake = new_head;

    SnakeSegment *current = *snake;
    while (current->next->next != NULL) {
        current = current->next;
    }

    free(current->next);
    current->next = NULL;
}

int check_collision(SnakeSegment *snake, int max_x, int max_y, Obstacle obstacles[]) {
    SnakeSegment *head = snake;

    if (head->x <= 0 || head->x >= max_x + 1 || head->y <= 0 || head->y >= max_y + 1) {
        return 1;
    }

    for (int i = 0; i < MAX_OBSTACLES; i++) {
        if (head->x == obstacles[i].x && head->y == obstacles[i].y) {
            return 1;
        }
    }

    SnakeSegment *current = head->next;
    while (current != NULL) {
        if (current->x == head->x && current->y == head->y) {
            return 1;
        }
        current = current->next;
    }

    return 0;
}

int eat_food(SnakeSegment *snake, Food food[], Food poison_food[], int *score) {
    for (int i = 0; i < MAX_FOOD; i++) {
        if (food[i].lifetime > 0 && snake->x == food[i].x && snake->y == food[i].y) {
            food[i].lifetime = 0;
            *score += 10;
            return 1;
        }
    }
    for (int i = 0; i < MAX_POISON_FOOD; i++) {
        if (poison_food[i].lifetime > 0 && snake->x == poison_food[i].x && snake->y == poison_food[i].y) {
            poison_food[i].lifetime = 0;
            *score -= 5;
            return -1;
        }
    }
    return 0;
}

void grow_snake(SnakeSegment *snake) {
    SnakeSegment *tail = snake;
    while (tail->next != NULL) {
        tail = tail->next;
    }

    SnakeSegment *new_segment = (SnakeSegment *)malloc(sizeof(SnakeSegment));
    new_segment->x = tail->x;
    new_segment->y = tail->y;
    new_segment->next = NULL;

    tail->next = new_segment;
}

void spawn_food(Food food[], int max_x, int max_y) {
    for (int i = 0; i < MAX_FOOD; i++) {
        if (food[i].lifetime == 0) {
            food[i].x = rand() % (max_x - 4) + 2;
            food[i].y = rand() % (max_y - 4) + 2;
            food[i].lifetime = rand() % (MAX_FOOD_LIFETIME - MIN_FOOD_LIFETIME + 1) + MIN_FOOD_LIFETIME;
            food[i].symbol = '@';
        }
    }
}

void spawn_poison_food(Food poison_food[], int max_x, int max_y) {
    for (int i = 0; i < MAX_POISON_FOOD; i++) {
        if (poison_food[i].lifetime == 0) {
            poison_food[i].x = rand() % (max_x - 4) + 2;
            poison_food[i].y = rand() % (max_y - 4) + 2;
            poison_food[i].lifetime = rand() % (MAX_FOOD_LIFETIME - MIN_FOOD_LIFETIME + 1) + MIN_FOOD_LIFETIME;
            poison_food[i].symbol = '*';
        }
    }
}

void update_food(Food food[]) {
    for (int i = 0; i < MAX_FOOD; i++) {
        if (food[i].lifetime > 0) {
            food[i].lifetime--;
        }
    }
}

void update_poison_food(Food poison_food[]) {
    for (int i = 0; i < MAX_POISON_FOOD; i++) {
        if (poison_food[i].lifetime > 0) {
            poison_food[i].lifetime--;
        }
    }
}

void draw_statistics(WINDOW *win, int score, time_t start_time) {
    time_t current_time = time(NULL);
    int elapsed_time = (int)difftime(current_time, start_time);
    mvwprintw(win, 0, 2, "Score: %d", score);
    mvwprintw(win, 0, 20, "Time: %d s", elapsed_time);
}

int game_over_screen(int score, int best_score) {
    clear();
    mvprintw(10, 10, "GAME OVER!");
    mvprintw(12, 10, "Your Score: %d", score);
    mvprintw(13, 10, "Best Score: %d", best_score);
    mvprintw(15, 10, "Press 'c' to continue or 'q' to quit");

    int ch;
    while ((ch = getch()) != 'q') {
        if (ch == 'c') {
            return 1;
        }
    }

    endwin();
    return 0;
 }

int start_game() {
    srand(time(NULL));
    initscr();
    cbreak();
    noecho();
    curs_set(FALSE);
    keypad(stdscr, TRUE);
    timeout(DELAY / 1000);

    int max_x, max_y;
    int best_score = 0;

    while (1) {
        SnakeSegment *snake = NULL;
        Food food[MAX_FOOD];
        Food poison_food[MAX_POISON_FOOD];
        Obstacle obstacles[MAX_OBSTACLES];
        int direction = KEY_RIGHT;
        int score = 0;
        time_t start_time = time(NULL);

        init_game(stdscr, &max_x, &max_y, &snake, food, poison_food, obstacles);

        while (1) {
            clear();
            draw_statistics(stdscr, score, start_time);
            draw_snake(stdscr, snake);
            draw_food(stdscr, food);
            draw_poison_food(stdscr, poison_food);
            draw_obstacles(stdscr, obstacles);
            refresh();

            int ch = getch();
            switch (ch) {
                case KEY_UP:
                case KEY_DOWN:
                case KEY_LEFT:
                case KEY_RIGHT:
                    direction = ch;
                    break;
                case 'q':
                    endwin();
                    return 0;
            }

            move_snake(&snake, direction);

            if (check_collision(snake, max_x, max_y, obstacles)) {
                best_score = (score > best_score) ? score : best_score;
                if (game_over_screen(score, best_score)) {
                    break;}
                    else {
                        endwin();
                        return 0;
                    }
            }

            int eaten = eat_food(snake, food, poison_food, &score);
            if (eaten) {
                if (eaten == 1) {
                    grow_snake(snake);
                    spawn_food(food, max_x, max_y);
                } else if (eaten == -1) {
                    spawn_poison_food(poison_food, max_x, max_y);
                }
            }

            update_food(food);
            update_poison_food(poison_food);

            usleep(DELAY);
        }
    }

    endwin();
    return 0;
}
