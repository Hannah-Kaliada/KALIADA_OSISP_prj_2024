#ifndef DIR_SIZE_H
#define DIR_SIZE_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <ncurses.h>
#include <ctype.h>
#include <pthread.h>
#include "globals.h"

float get_recursive_size_directory (char *path);
off_t get_size (const char *path);
off_t get_dir_size (const char *dir_path, off_t limit, int *exceeded);
void
display_buttons (WINDOW *dialog_win, int yes_pos, int no_pos,
                 int current_selection, char *message);
int
open_dialog_window (char *message);
void
format_size (float size, char *formatted_size);
void *
compute_directory_size (void *arg);
void
open_print_window (char *path);


#endif
