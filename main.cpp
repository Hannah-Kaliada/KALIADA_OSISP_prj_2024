#include <dirent.h>
#include <limits.h>
#include <locale.h>
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "config.h"


#define isDir(mode) (S_ISDIR(mode))

// Global variables
struct stat file_stats;
WINDOW *current_win, *info_win, *path_win;
int selection, maxx, maxy, len = 0, start = 0;
size_t total_files = 0;
directory_t *current_directory_ = NULL;

// Function prototypes
void init_curses();
void init();
void init_windows();
void refreshWindows();
int get_no_files_in_directory(char *directory);
int get_files(char *directory, char *target[]);
void scroll_up();
void scroll_down();
void sort(char *files_[], int n);
int check_text(char *path);
void read_(char *path);
void readd(char *path);
void rename_file(char *files[]);
void delete_(char *files[]);
void delete_file(char *files[]);
void copy_files(char *files[]);
void move_file(char *files[]);
void handle_enter(char *files[]);
float get_recursive_size_directory(char *path);
void show_file_info(char *files[]);
char *get_parent_directory(char *cwd);

int main() {
  setlocale(LC_ALL, "ru_RU.UTF-8");
  init();
  init_curses();
  getcwd(current_directory_->cwd, sizeof(current_directory_->cwd));
  strcat(current_directory_->cwd, "/");
  current_directory_->parent_dir =
      strdup(get_parent_directory(current_directory_->cwd));
  int ch;

  do {
    len = get_no_files_in_directory(current_directory_->cwd);
    if (len < 0) {
      perror("Error getting file count in directory");
      break;
    }

    len = len <= 0 ? 1 : len;
    char *files[len];
    if (get_files(current_directory_->cwd, files) < 0) {
      perror("Error getting files in directory");
      break;
    }
    if (selection > len - 1) {
      selection = len - 1;
    }

    getmaxyx(stdscr, maxy, maxx);
    maxy -= 2;
    int t = 0;
    init_windows();

    for (int i = start; i < len; i++) {
      if (t == maxy - 1) break;
      int size = snprintf(NULL, 0, "%s%s", current_directory_->cwd, files[i]);
      if (i == selection) {
        wattron(current_win, A_STANDOUT);
      } else {
        wattroff(current_win, A_STANDOUT);
      }

      char *temp_dir = (char *)malloc(size + 1);
      snprintf(temp_dir, size + 1, "%s%s", current_directory_->cwd, files[i]);

      stat(temp_dir, &file_stats);
      isDir(file_stats.st_mode) ? wattron(current_win, COLOR_PAIR(1))
                                : wattroff(current_win, COLOR_PAIR(1));
      wmove(current_win, t + 1, 2);
      wprintw(current_win, "%.*s\n", maxx, files[i]);
      free(temp_dir);
      t++;
    }
    wmove(path_win, 1, 0);
    wprintw(path_win, " %s", current_directory_->cwd);
    show_file_info(files);
    refreshWindows();

    switch ((ch = wgetch(current_win))) {
      case KEY_UP:
      case KEY_NAVUP:
        scroll_up();
        break;
      case KEY_DOWN:
      case KEY_NAVDOWN:
        scroll_down();
        break;
      case KEY_ENTER:
        handle_enter(files);
        break;
      case 'r':
      case 'R':
        rename_file(files);
        break;
      case 'c':
      case 'C':
        copy_files(files);
        break;
      case 'm':
      case 'M':
        move_file(files);
        break;
      case 'd':
      case 'D':
        delete_file(files);
        break;
    }
    for (int i = 0; i < len; i++) {
      free(files[i]);
    }
  } while (ch != 'q');
  endwin();
  return 0;
}

void init_curses() {
  initscr();
  noecho();
  curs_set(0);
  start_color();
  init_pair(1, DIR_COLOR, 0);
  init_pair(3, STATUS_SELECTED_COLOR, 0);
}

void init() {
  current_directory_ = (directory_t *)malloc(sizeof(directory_t));
  if (current_directory_ == NULL) {
    printf("Error Occured.\n");
    exit(0);
  }
}

void init_windows() {
  current_win = newwin(maxy, maxx / 2, 0, 0);
  refresh();
  path_win = newwin(2, maxx, maxy, 0);
  refresh();
  info_win = newwin(maxy, maxx / 2, 0, maxx / 2);
  refresh();
  keypad(current_win, TRUE);
}

void refreshWindows() {
  box(current_win, '|', '-');
  box(info_win, '|', '-');
  wrefresh(current_win);
  wrefresh(path_win);
  wrefresh(info_win);
}

int get_no_files_in_directory(char *directory) {
  int len = 0;
  DIR *dir_;
  struct dirent *dir_entry;

  dir_ = opendir(directory);
  if (dir_ == NULL) {
    return -1;
  }

  while ((dir_entry = readdir(dir_)) != NULL) {
    if (strcmp(dir_entry->d_name, ".") != 0) {
      len++;
    }
  }
  closedir(dir_);
  return len;
}

int get_files(char *directory, char *target[]) {
  int i = 0;
  DIR *dir_;
  struct dirent *dir_entry;

  dir_ = opendir(directory);
  if (dir_ == NULL) {
    return -1;
  }

  while ((dir_entry = readdir(dir_)) != NULL) {
    if (strcmp(dir_entry->d_name, ".") != 0) {
      target[i++] = strdup(dir_entry->d_name);
    }
  }
  closedir(dir_);
  return 1;
}

void scroll_up() {
  selection--;
  selection = (selection < 0) ? 0 : selection;
  if (len >= maxy - 1)
    if (selection <= start + maxy / 2) {
      if (start == 0)
        wclear(current_win);
      else {
        start--;
        wclear(current_win);
      }
    }
}

void scroll_down() {
  selection++;
  selection = (selection > len - 1) ? len - 1 : selection;
  if (len >= maxy - 1)
    if (selection - 1 > maxy / 2) {
      if (start + maxy - 2 != len) {
        start++;
        wclear(current_win);
      }
    }
}

void sort(char *files_[], int n) {
  char temp[1000];
  for (int i = 0; i < n - 1; i++) {
    for (int j = i + 1; j < n; j++) {
      if (strcmp(files_[i], files_[j]) > 0) {
        strcpy(temp, files_[i]);
        strcpy(files_[i], files_[j]);
        strcpy(files_[j], temp);
      }
    }
  }
}

int check_text(char *path) {
  FILE *ptr;
  ptr = fopen(path, "r");
  if (ptr == NULL) {
    perror("Error opening file");
    return 0;
  }
  int c;
  while (!feof(ptr)) {
    c = fgetc(ptr);
    if (c == EOF) {
      if (ferror(ptr)) {
        perror("Error reading file");
        fclose(ptr);
        return 0;
      }
      break;
    }
    if (c < 0 || c > 127) {
      fclose(ptr);
      return 0;
    }
  }
  fclose(ptr);
  return 1;
}

void read_(char *path) {
  char buffer[256];
  int ch;
  wclear(current_win);
  wclear(info_win);
  wresize(current_win, maxy, maxx);
  FILE *ptr;
  ptr = fopen(path, "rb");
  if (ptr == NULL) {
    perror("Error");
  }
  int t = 2, pos = 0, count;

  do {
    wmove(current_win, 1, 2);
    wprintw(current_win, "Press \"E\" to Exit (Caps Lock off)");

    if (check_text(path)) {
      count = 0;
      while (fgets(buffer, sizeof(buffer), ptr)) {
        if (count < pos) {
          count++;
          continue;
        }
        wmove(current_win, ++t, 1);
        wprintw(current_win, "%.*s", maxx - 2, buffer);
      }
    } else {
      while (!feof(ptr)) {
        fread(&buffer, sizeof(unsigned char), maxx - 2, ptr);
        wmove(current_win, ++t, 1);
        for (int i = 0; i < maxx - 2; i += 2) {
          wprintw(current_win, "%02x%02x ", (unsigned int)buffer[i],
                  (unsigned int)buffer[i + 1]);
        }
      }
    }
    box(current_win, '|', '-');
    wrefresh(current_win);
    ch = wgetch(current_win);
    wclear(current_win);
    switch (ch) {
      case 259:
        pos = pos == 0 ? 0 : pos - 1;
        break;
      case 258:
        pos++;
        break;
    }
  } while (ch != 'e');

  endwin();
}

// todo: not working also nees to return to working window
// не передает путь???
void readd(char *path) {
    pid_t pid = fork();

    if (pid == 0) {
        execlp("nano", "nano", path, NULL);
        perror("exec");
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("fork");
    } else {
        int status;
        waitpid(pid, &status, 0);
    }
}
void rename_file(char *files[]) {
  char new_name[100];
  int i = 0, c;
  wclear(path_win);
  wmove(path_win, 1, 0);
  while ((c = wgetch(path_win)) != '\n') {
    if (c == 127 || c == 8) {
      new_name[--i] = '\0';
    } else {
      new_name[i++] = c;
      new_name[i] = '\0';
    }
    wclear(path_win);
    wmove(path_win, 1, 0);
    wprintw(path_win, "%s", new_name);
  }
  c = ' ';
LOOP:
  wclear(path_win);
  wmove(path_win, 1, 0);
  wprintw(path_win, "%s (y/n) %c", new_name, c);
  c = wgetch(path_win);
  char *a, *b;
  switch (c) {
    case 'y':
    case 'Y':
      a = strdup(current_directory_->cwd);
      b = strdup(current_directory_->cwd);
      strcat(a, files[selection]);
      strcat(b, new_name);
      rename(a, b);
      free(a);
      free(b);
      break;
    case 'n':
    case 'N':
      break;
    default:
      goto LOOP;
      break;
  }
}

char *get_parent_directory(char *cwd) {
  char *a;
  a = strdup(cwd);
  int i = (int)(strlen(a) - 1);
  while (a[--i] != '/')
    ;
  a[++i] = '\0';
  return a;
}

void delete_(char *files[]) {
  char curr_path[1000];
  snprintf(curr_path, sizeof(curr_path), "%s%s", current_directory_->cwd,
           files[selection]);
  remove(curr_path);
}

void delete_file(char *files[]) {
  int c;
  wclear(path_win);
  wmove(path_win, 1, 0);
  wprintw(path_win, "Are you sure to delete? (y/n)");

LOOP_:
  c = wgetch(path_win);
  wclear(path_win);
  wmove(path_win, 1, 0);
  wprintw(path_win, "Are you sure to delete? (y/n) %c", c);
  switch (c) {
    case 'y':
    case 'Y':
      delete_(files);
      break;
    case 'n':
    case 'N':
      break;
    default:
      goto LOOP_;
      break;
  }
}

void copy_files(char *files[]) {
  char new_path[1000];
  int i = 0, c;
  wclear(path_win);
  wmove(path_win, 1, 0);
  while ((c = wgetch(path_win)) != '\n') {
    if (c == 127 || c == 8) {
      new_path[--i] = '\0';
      i = i < 0 ? 0 : i;
    } else {
      new_path[i++] = c;
      new_path[i] = '\0';
    }
    wclear(path_win);
    wmove(path_win, 1, 0);
    wprintw(path_win, "%s", new_path);
  }
  FILE *new_file, *old_file;
  strcat(new_path, files[selection]);
  char curr_path[1000];
  snprintf(curr_path, sizeof(curr_path), "%s%s", current_directory_->cwd,
           files[selection]);
  old_file = fopen(curr_path, "r");
  new_file = fopen(new_path, "a");
  wmove(current_win, 10, 10);
  wprintw(current_win, "%.*s,%s", maxx, curr_path, new_path);
  printf("%s %s", curr_path, new_path);
  while ((c = fgetc(old_file)) != EOF) {
    fprintf(new_file, "%c", c);
  }
  fclose(old_file);
  fclose(new_file);
}

void move_file(char *files[]) {
  copy_files(files);
  delete_(files);
}

void handle_enter(char *files[]) {
  char *temp;
  char *original_cwd = strdup(current_directory_->cwd);
  endwin();
  if (strcmp(files[selection], "..") == 0) {
    start = 0;
    selection = 0;
    strcpy(current_directory_->cwd, current_directory_->parent_dir);
    free(current_directory_->parent_dir);
    current_directory_->parent_dir =
        strdup(get_parent_directory(current_directory_->cwd));
  } else {
    temp = (char *)malloc(strlen(files[selection]) + 1);
    snprintf(temp, strlen(files[selection]) + 1, "%s", files[selection]);
    strcat(current_directory_->cwd, "/");
    strcat(current_directory_->cwd, temp);
    stat(current_directory_->cwd, &file_stats);
    if (isDir(file_stats.st_mode)) {
      start = 0;
      selection = 0;
      free(current_directory_->parent_dir);
      current_directory_->parent_dir = strdup(original_cwd);
      strcat(current_directory_->cwd, "/");
    } else {
      char temp_[1000];
      snprintf(temp_, sizeof(temp_), "%s%s", original_cwd, files[selection]);
      readd(original_cwd);
    }
    free(temp);
  }
  refresh();
  free(original_cwd);
}

float get_recursive_size_directory(char *path) {
  float directory_size = 0;
  DIR *dir_ = opendir(path);
  if (dir_ == NULL) {
    return 0;
  }

  struct dirent *dir_entry;
  struct stat file_stat;
  char temp_path[1000];

  while ((dir_entry = readdir(dir_)) != NULL) {
    if (strcmp(dir_entry->d_name, ".") != 0 &&
        strcmp(dir_entry->d_name, "..") != 0) {
      snprintf(temp_path, sizeof(temp_path), "%s/%s", path, dir_entry->d_name);

      if (lstat(temp_path, &file_stat) == -1) {
        continue;
      }

      if (S_ISDIR(file_stat.st_mode)) {
        directory_size += get_recursive_size_directory(temp_path);
      } else {
        directory_size += (float)(file_stat.st_size) / (float)1024;
      }
    }
  }

  closedir(dir_);
  return directory_size;
}

void show_file_info(char *files[]) {
  wmove(info_win, 1, 1);
  if (strcmp(files[selection], "..") != 0) {
    char temp_address[1000];
    snprintf(temp_address, sizeof(temp_address), "%s%s",
             current_directory_->cwd, files[selection]);
    stat(temp_address, &file_stats);

    wprintw(info_win, "Name: %s\n Type: %s\n", files[selection],
            isDir(file_stats.st_mode) ? "Folder" : "File");
    if (!isDir(file_stats.st_mode)) {
      wprintw(info_win, " Size: %.2f KB\n",
              (float)file_stats.st_size / (float)1024);
    }
  } else {
    wprintw(info_win, "Press Enter to go back\n");
  }
}
