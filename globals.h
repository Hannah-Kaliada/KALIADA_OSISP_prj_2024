#ifndef GLOBALS_H
#define GLOBALS_H

#include <ncurses.h>
#include "config.h"

extern int selection, maxx, maxy, len, start;
extern directory_t *current_directory_;
extern char gen_poly[64];
extern char check_value[128];
extern char checked_value[128];
extern int sortFlag;
extern WINDOW *current_win, *info_win, *path_win;


#endif // GLOBALS_H
