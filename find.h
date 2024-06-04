#ifndef FIND_H
#define FIND_H

#include "globals.h"
#include <ctype.h>
#include <dirent.h>
#include <grp.h>
#include <libgen.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <limits.h>
#include <locale.h>
#include <ncurses.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/types.h>
#include <unistd.h>

void find (char *files[]);

void find_files_recursive (const char *dir_path, const char *regex_str,
                           char ***file_paths, int *num_matches,
                           regex_t *regex);
char **find_files_by_regex (const char *root_dir, const char *regex_str,
                            int *num_matches);
                            
char *get_parent_directory (char *cwd);

#endif // GLOBALS_H
