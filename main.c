/**
 * @file main.c
 * @brief Главный файл проекта.
 *
 * Этот файл содержит основную функцию и точку входа в
 * программу.
 *
 * Полный исходный текст проекта доступен на GitHub:
 * https://github.com/Hannah-Kaliada/KALIADA_OSISP_prj_2024.git
 */
#include <ctype.h>
#include <dirent.h>
#include <grp.h>
#include <libgen.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <limits.h>
#include <locale.h>
#include <ncurses.h>
#include <pthread.h>
#include <pwd.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#include "game.h"
#include "gui.h"
#include "config.h"
#include "globals.h"
#include "operations.h"
#include "find.h"
#include "dir_size.h"
#include "crc.h"

struct stat file_stats;
WINDOW *current_win, *info_win, *path_win;
int selection, maxx, maxy, len = 0, start = 0;
directory_t *current_directory_ = NULL;
char gen_poly[64];       // Генерирующий полином
char check_value[128];   // Сумма для проверки CRC
char checked_value[128]; // Проверяемые данные из файла
int sortFlag = 0;

int get_number_of_files_in_directory (char *directory);
int get_files (char *directory, char *target[]);


void show_file_info (char *files[]);




int
main ()
{
  setlocale (LC_ALL, "ru_RU.UTF-8");
  init ();
  init_curses ();
  strcat (current_directory_->cwd, "/");
  current_directory_->parent_dir
      = strdup (get_parent_directory (current_directory_->cwd));
  int ch;

  do
    {
      len = get_number_of_files_in_directory (current_directory_->cwd);
      if (len < 0)
        {
          perror ("Error getting file count in "
                  "directory");
          break;
        }

      len = len <= 0 ? 1 : len;
      char *files[len];
      if (get_files (current_directory_->cwd, files) < 0)
        {
          perror ("Error getting files in directory");
          break;
        }
      if (sortFlag > 0)
        {
          sort_ (files, len);
        }
      if (selection > len - 1)
        {
          selection = len - 1;
        }
      getmaxyx (stdscr, maxy, maxx);
      maxy -= 2;
      int t = 0;
      init_windows ();
      for (int i = start; i < len; i++)
        {
          if (t == maxy - 1)
            break;
          int size
              = snprintf (NULL, 0, "%s%s", current_directory_->cwd, files[i]);
          if (i == selection)
            {
              wattron (current_win, A_STANDOUT);
            }
          else
            {
              wattroff (current_win, A_STANDOUT);
            }

          char *temp_dir = (char *)malloc (size + 1);
          snprintf (temp_dir, size + 1, "%s%s", current_directory_->cwd,
                    files[i]);

          stat (temp_dir, &file_stats);
          isDir (file_stats.st_mode) ? wattron (current_win, COLOR_PAIR (1))
                                     : wattroff (current_win, COLOR_PAIR (2));
          wmove (current_win, t + 1, 2);
          wprintw (current_win, "%.*s\n", maxx, files[i]);
          free (temp_dir);
          t++;
        }
      wmove (path_win, 1, 0);
      wprintw (path_win, " %s", current_directory_->cwd);
      show_file_info (files);
      refreshWindows ();

      switch ((ch = wgetch (current_win)))
        {
        case KEY_UP:
        case KEY_NAVUP:
          scroll_up ();
          break;
        case KEY_DOWN:
        case KEY_NAVDOWN:
          scroll_down ();
          break;
        case KEY_ENTER:
          handle_enter (files);
          break;
        case KEY_RENAME:
          rename_file (files);
          break;
         case 'g':
          start_game();
          break; 
        case KEY_COPY:
          copy_files (files);
          break;
        case KEY_MOVE:
          move_file (files);
          break;
        case KEY_DELETE:
          delete_file (files);
          break;
        case KEY_СRC:
          crc (files);
          break;
        case KEY_DIR_SIZE:
          {
            int exceeded = 0;
            off_t dir_size
                = get_dir_size (current_directory_->cwd, SIZE_LIMIT, &exceeded);
            if (exceeded)
              {
                if (!open_dialog_window (
                        "Directory size is too big. The sizing process may "
                        "take too long. Do you want to continue?"))
                  {
                    break;
                  }
                else
                  {
                    open_print_window (current_directory_->cwd);
                  }
              } else open_print_window(current_directory_->cwd);
          }
          break;
        case KEY_SORT:
          {
            if (sortFlag > 0)
              sortFlag = 0;
            else
              sortFlag = 1;
            break;
          }
        case KEY_FIND:
          find (files);
          break;
        case KEY_HELP:
          {
            wclear (info_win);
            wmove (info_win, 1, 1);
            wprintw (info_win, " Press \"k\" to "
                               "scroll up\n");
            wprintw (info_win, "  Press \"j\" to "
                               "scroll down\n");
            wprintw (info_win, "  Press \"r\" to "
                               "rename file\n");
            wprintw (info_win, "  Press \"c\" to "
                               "copy file\n");
            wprintw (info_win, "  Press \"m\" to "
                               "move file\n");
            wprintw (info_win, "  Press \"d\" to "
                               "delete file\n");
            wprintw (info_win, "  Press \"a\" to "
                               "calculate CRC\n");
            wprintw (info_win, "  Press \"s\" to "
                               "sort files\n");
            wprintw (info_win, "  Press \"f\" to "
                               "find files\n");
            wprintw (info_win, "  Press \"h\" to "
                               "show help\n");
            wprintw (info_win, "  Press \"q\" to "
                               "exit\n");
            wprintw (info_win, "  Press \"z\" to "
                               "calculate directory size\n");       
            wprintw (info_win, "  Press Enter to "
                               "continue...\n");
            box (info_win, '|', '-');
            wrefresh (info_win);
            while (getch () != '\n')
              ;
            break;
          }
        }

      for (int i = 0; i < len; i++)
        {
          free (files[i]);
        }
    }
  while (ch != 'q');
  endwin ();
  return 0;
}

/**
 * Подсчитывает количество файлов в указанном каталоге (не
 * включая ссылки "." и
 * "..").
 * @param directory Путь к каталогу, для которого нужно
 * подсчитать файлы.
 * @return Возвращает количество файлов в каталоге или -1 в
 * случае ошибки.
 */
int
get_number_of_files_in_directory (char *directory)
{
  int len = 0;
  DIR *dir_;
  struct dirent *dir_entry;

  dir_ = opendir (directory);
  if (dir_ == NULL)
    {
      return -1;
    }

  while ((dir_entry = readdir (dir_)) != NULL)
    {
      if (strcmp (dir_entry->d_name, ".") != 0)
        {
          len++;
        }
    }
  closedir (dir_);
  return len;
}

/**
 * Получает имена всех файлов в указанном каталоге (исключая
 * ссылки "." и "..") и сохраняет их в массив строк. Каждое
 * имя файла копируется в элемент массива `target`.
 * @param directory Путь к каталогу, из которого нужно
 * получить имена файлов.
 * @param target Массив строк, в который будут сохранены
 * имена файлов.
 * @return Возвращает 1 в случае успешного получения имен
 * файлов или -1 в случае ошибки.
 */
int
get_files (char *directory, char *target[])
{
  int i = 0;
  DIR *dir_;
  struct dirent *dir_entry;

  dir_ = opendir (directory);
  if (dir_ == NULL)
    {
      return -1;
    }

  while ((dir_entry = readdir (dir_)) != NULL)
    {
      if (strcmp (dir_entry->d_name, ".") != 0)
        {
          target[i++] = strdup (dir_entry->d_name);
        }
    }
  closedir (dir_);
  return 1;
}

/**
 * Определяет тип файловой системы на основе идентификатора fsid.
 *
 * @param fsid Идентификатор файловой системы.
 * @return Строка, представляющая тип файловой системы.
 *         "ext4" для ext4,
 *         "ntfs" для ntfs,
 *         "APFS" для APFS,
 *         "unknown" для неизвестных файловых систем.
 */
const char *
filesystem_type (unsigned long fsid)
{
  switch (fsid)
    {
    case 0x0000BAD1:
      return "ext4";
    case 0x0000FACE:
      return "ntfs";
    case 0x1000013:
      return "APFS";
    default:
      return "unknown";
    }
}

/**
 * Отображает информацию о выбранном файле или каталоге в
 * окне info_win.
 * @param files Массив имен файлов.
 */
void
show_file_info (char *files[])
{
  wmove (info_win, 1, 1);

  // Проверяем, что выбранный элемент не является ссылкой
  // на родительский каталог
  if (strcmp (files[selection], "..") != 0)
    {
      char temp_address[1000];
      snprintf (temp_address, sizeof (temp_address), "%s%s",
                current_directory_->cwd, files[selection]);

      struct stat file_stats;

      // Получаем информацию о выбранном элементе с
      // помощью lstat
      if (lstat (temp_address, &file_stats) == 0)
        {
          wprintw (info_win, " Name: %s\n", files[selection]);

          // Определяем тип файла
          wprintw (info_win, "  Type:");
          switch (file_stats.st_mode & S_IFMT)
            {
            case S_IFREG:
              wprintw (info_win, " Regular File\n");
              break;
            case S_IFDIR:
              wprintw (info_win, " Directory\n");
              break;
            case S_IFLNK:
              wprintw (info_win, " Symbolic Link\n");
              break;
            case S_IFIFO:
              wprintw (info_win, " FIFO/Named Pipe\n");
              break;
            case S_IFCHR:
              wprintw (info_win, " Character Device\n");
              break;
            case S_IFBLK:
              wprintw (info_win, " Block Device\n");
              break;
            case S_IFSOCK:
              wprintw (info_win, " Socket\n");
              break;
            default:
              wprintw (info_win, " Unknown\n");
              break;
            }
          wprintw (info_win, "  Permissions: ");
          wprintw (info_win, (file_stats.st_mode & S_IRUSR) ? "r" : "-");
          wprintw (info_win, (file_stats.st_mode & S_IWUSR) ? "w" : "-");
          wprintw (info_win, (file_stats.st_mode & S_IXUSR) ? "x" : "-");
          wprintw (info_win, (file_stats.st_mode & S_IRGRP) ? "r" : "-");
          wprintw (info_win, (file_stats.st_mode & S_IWGRP) ? "w" : "-");
          wprintw (info_win, (file_stats.st_mode & S_IXGRP) ? "x" : "-");
          wprintw (info_win, (file_stats.st_mode & S_IROTH) ? "r" : "-");
          wprintw (info_win, (file_stats.st_mode & S_IWOTH) ? "w" : "-");
          wprintw (info_win, (file_stats.st_mode & S_IXOTH) ? "x" : "-");
          wprintw (info_win, "\n");
          wprintw (info_win, "  Device ID: [%lx,%lx]\n",
                   (long)major (file_stats.st_dev),
                   (long)minor (file_stats.st_dev));
          struct passwd *pw = getpwuid (file_stats.st_uid);
          struct group *gr = getgrgid (file_stats.st_gid);
          if (pw != NULL && gr != NULL)
            {
              wprintw (info_win, "  Owner: %s (%s)\n", pw->pw_name,
                       gr->gr_name);
            }
          struct passwd *pw_process = getpwuid (getuid ());
          if (pw_process != NULL)
            {
              wprintw (info_win, "  Process Owner: %s\n", pw_process->pw_name);
            }
          wprintw (info_win, "  Number of Links: %ld\n",
                   (long)file_stats.st_nlink);
          wprintw (info_win, "  Block Size: %ld bytes, Blocks: %lld\n",
                   (long)file_stats.st_blksize,
                   (long long)file_stats.st_blocks);
          if (!S_ISDIR (file_stats.st_mode))
            {
              long long size = file_stats.st_size;
              const char *unit;

              if (size < 1024LL)
                {
                  unit = "B";
                }
              else if (size < 1024LL * 1024LL)
                {
                  size /= 1024LL;
                  unit = "KB";
                }
              else if (size < 1024LL * 1024LL * 1024LL)
                {
                  size /= (1024LL * 1024LL);
                  unit = "MB";
                }
              else
                {
                  size /= (1024LL * 1024LL * 1024LL);
                  unit = "GB";
                }

              wprintw (info_win, "  Size: %lld %s\n", size, unit);
            }
          wprintw (info_win, "  Last Status Change:\n  %s",
                   ctime (&file_stats.st_ctime));
          wprintw (info_win, "  Last Access:\n  %s",
                   ctime (&file_stats.st_atime));
          wprintw (info_win, "  Last Modification:\n  %s",
                   ctime (&file_stats.st_mtime));
        }
    }
  else
    {
      if (strcmp (current_directory_->cwd, "/") == 0)
        {
          struct statvfs fs_info;
          wattr_on (info_win, A_BOLD, NULL);
          wattron (info_win, COLOR_PAIR (2));
          if (statvfs (current_directory_->cwd, &fs_info) == 0)
            {
              wprintw (info_win, "\n  INFORMATION ABOUT FILESYSTEM\n\n");
              if (statvfs (current_directory_->cwd, &fs_info) == 0)
                {
                  // Вывод информации о файловой системе
                  wprintw (info_win, "  Filesystem Type: %s\n",
                           filesystem_type (fs_info.f_fsid));
                }
              wprintw (info_win, "  Filesystem Block Size: %lu bytes\n",
                       fs_info.f_bsize);
              wprintw (info_win,
                       "  Fundamental File System Block "
                       "Size: %lu bytes\n",
                       fs_info.f_frsize);
              wprintw (info_win,
                       "  Total Number of Blocks on File "
                       "System: %lld\n",
                       (long long)fs_info.f_blocks);
              wprintw (info_win, "  Total Number of Free Blocks: %lld\n",
                       (long long)fs_info.f_bfree);
              wprintw (info_win,
                       "  Number of Free Blocks Available "
                       "to Non-privileged Process: %lld\n",
                       (long long)fs_info.f_bavail);
              wprintw (info_win,
                       "  Total Number of File Serial "
                       "Numbers: %lld\n",
                       (long long)fs_info.f_files);
              wprintw (info_win,
                       "  Total Number of Free File Serial "
                       "Numbers: %lld\n",
                       (long long)fs_info.f_ffree);
              wprintw (info_win,
                       "  Number of File Serial Numbers "
                       "Available to Non-privileged "
                       "Process: %lld\n",
                       (long long)fs_info.f_favail);
              wprintw (info_win, "  File System ID: %lu\n", fs_info.f_fsid);
              wprintw (info_win, "  Bit Mask of f_flag Values: %lu\n",
                       fs_info.f_flag);
              wprintw (info_win, "  Maximum Filename Length: %lu\n",
                       fs_info.f_namemax);
            }
          wattr_off (info_win, A_BOLD, NULL);
          wattroff (info_win, COLOR_PAIR (2));
          wprintw (info_win, "\n\n  Root directory\n");
          wprintw (info_win, "\n  Possible commands:\n");
          wprintw (info_win, "  r: Rename a file\n");
          wprintw (info_win, "  c: Copy files\n");
          wprintw (info_win, "  d: Delete files\n");
          wprintw (info_win, "  m: Move files\n");
          wprintw (info_win, "  a: Calculate CRC (Cyclic "
                             "Redundancy Check)\n");
          wprintw (info_win, "  s: Sort files\n");
          wprintw (info_win, "  f: Find files by regular "
                             "expression\n");
          wprintw (info_win, "  r: Rename files\n");
          wprintw (info_win, "  z: Calculate director\n");
          wprintw (info_win, "  h: Show all commands\n");
        }
      else
        {
          wprintw (info_win, "  Return to %s\n",
                   current_directory_->parent_dir);
        }
    }
  wrefresh (info_win);
}
