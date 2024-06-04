#ifndef CRC_H
#define CRC_H

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

void crc (char *files[]);


#endif // GLOBALS_H