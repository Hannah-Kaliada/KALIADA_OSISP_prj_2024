#ifndef OPERATIONS_H
#define OPERATIONS_H
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

void sort_ (char *files[], int n);
void sort (char *files_[], int n);
int check_text (char *path);
void read_ (char *path);
void rename_file (char *files[]);
void delete_ (char *files[]);
void delete_file (char *files[]);
void copy_files (char *files[]);
void move_file (char *files[]);

#endif
