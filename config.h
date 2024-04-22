#ifndef CONFIG
#define CONFIG

typedef struct directory_ {
  char cwd[1000];
  char *parent_dir;
  char *files[];
} directory_t;

#define DIR_COLOR 2

#define STATUS_SELECTED_COLOR 1

#define KEY_NAVUP 'k'

#define KEY_NAVDOWN 'j'

#undef KEY_ENTER

#define KEY_ENTER 10

#define KEY_RENAME 'r'

#define KEY_COPY 'c'

#define KEY_DELETE 'd'

#undef KEY_MOVE

#define KEY_MOVE 'm'

#define KEY_СRC 'a'

#define KEY_SORT 's'

#define KEY_FIND 'f'

#define isDir(mode) (S_ISDIR(mode))

#endif
