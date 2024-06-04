#ifndef GUI_H
#define GUI_H

#include <ncurses.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "config.h"
#include "globals.h"
#include <sys/stat.h>
#include <limits.h>
#include "find.h"
#include "operations.h"

void init_curses ();
void init ();
void init_windows ();
void refreshWindows ();
void scroll_up ();
void scroll_down ();
void handle_enter (char *files[]);

#endif 