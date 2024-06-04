#include "dir_size.h"


/**
 * Рекурсивно вычисляет размер указанного каталога и всех
 * его подкаталогов в байтах.
 * @param path Путь к каталогу, размер которого требуется
 * вычислить.
 * @return Общий размер каталога и его содержимого в байтах.
 */
float
get_recursive_size_directory (char *path)
{
  float directory_size = 0;
  DIR *dir_ = opendir (path);
  if (dir_ == NULL)
    {
      return 0;
    }

  struct dirent *dir_entry;
  struct stat file_stat;
  char temp_path[1000];

  // Перебираем все элементы в указанном каталоге
  while ((dir_entry = readdir (dir_)) != NULL)
    {
      // Игнорируем ссылки на текущий и родительский
      // каталоги
      if (strcmp (dir_entry->d_name, ".") != 0
          && strcmp (dir_entry->d_name, "..") != 0)
        {
          // Формируем полный путь к текущему
          // элементу
          snprintf (temp_path, sizeof (temp_path), "%s/%s", path,
                    dir_entry->d_name);

          // Получаем информацию о текущем
          // элементе
          if (lstat (temp_path, &file_stat) == -1)
            {
              continue; // Пропускаем элемент
                        // в случае ошибки
            }

          // Если текущий элемент является
          // каталогом, рекурсивно вычисляем его
          // размер
          if (S_ISDIR (file_stat.st_mode))
            {
              directory_size += get_recursive_size_directory (temp_path);
            }
          else
            {
              // Если текущий элемент не
              // является каталогом, добавляем
              // его размер к общему размеру
              directory_size += file_stat.st_size;
            }
        }
    }

  closedir (dir_);
  return directory_size;
}


// Функция для получения размера файла или директории
off_t
get_size (const char *path)
{
  struct stat statbuf;
  if (stat (path, &statbuf) == -1)
    {
      perror ("stat");
      return 0;
    }
  if (S_ISDIR (statbuf.st_mode))
    {
      return 0;
    }
  return statbuf.st_size;
}

// Рекурсивная функция для получения размера директории с проверкой лимита
off_t
get_dir_size (const char *dir_path, off_t limit, int *exceeded)
{
  DIR *dir;
  struct dirent *entry;
  char path[1024];
  off_t total_size = 0;

  if ((dir = opendir (dir_path)) == NULL)
    {
      perror ("opendir");
      return 0;
    }

  while ((entry = readdir (dir)) != NULL)
    {
      if (strcmp (entry->d_name, ".") == 0 || strcmp (entry->d_name, "..") == 0)
        {
          continue;
        }
      snprintf (path, sizeof (path), "%s/%s", dir_path, entry->d_name);
      if (entry->d_type == DT_DIR)
        {
          total_size += get_dir_size (path, limit - total_size, exceeded);
        }
      else
        {
          total_size += get_size (path);
        }

      if (total_size > limit)
        {
          *exceeded = 1;
          break;
        }
    }
  closedir (dir);
  return total_size;
}

void
display_buttons (WINDOW *dialog_win, int yes_pos, int no_pos,
                 int current_selection, char *message)
{
  wclear (dialog_win);
  box (dialog_win, 0, 0);
  wmove (dialog_win, 1, 1);
  wprintw (dialog_win, "%s", message);
  wmove (dialog_win, 7, yes_pos);
  if (current_selection == yes_pos)
    wattron (dialog_win, A_REVERSE);
  wprintw (dialog_win, "[ Yes ]");
  wattroff (dialog_win, A_REVERSE);

  wmove (dialog_win, 7, no_pos);
  if (current_selection == no_pos)
    wattron (dialog_win, A_REVERSE);
  wprintw (dialog_win, "[ No ]");
  wattroff (dialog_win, A_REVERSE);
  box (dialog_win, 0, 0);
  wrefresh (dialog_win);
}

int
open_dialog_window (char *message)
{
  // Создание и инициализация диалогового окна
  WINDOW *dialog_win = newwin (10, 50, (maxy - 10) / 2, (maxx - 50) / 2);
  keypad (dialog_win, TRUE);
  box (dialog_win, 0, 0);

  // Позиции кнопок
  int yes_pos = 10;
  int no_pos = 30;
  int current_selection = yes_pos;

  display_buttons (dialog_win, yes_pos, no_pos, current_selection, message);

  // Обработка нажатий клавиш
  int ch;
  int fl;
  while ((ch = wgetch (dialog_win)) != '\n')
    {
      switch (ch)
        {
        case KEY_LEFT:
          current_selection = yes_pos;
          break;
        case KEY_RIGHT:
          current_selection = no_pos;
          break;
        default:
          break;
        }
      display_buttons (dialog_win, yes_pos, no_pos, current_selection, message);
      if (ch == '\n')
        {
          break;
        }
    }

  // Обработка выбора кнопок
  if (current_selection == yes_pos)
    {
      wclear (dialog_win);
      wrefresh (dialog_win);
      fl = 1;
    }
  else
    {
      fl = 0;
    }

  // Удаление диалогового окна
  delwin (dialog_win);
  return fl;
}

typedef struct
{
  char *path;
  float *result;
  pthread_mutex_t *mutex;
} ThreadArgs;

void
format_size (float size, char *formatted_size)
{
  const char *units[] = { "bytes", "KB", "MB", "GB" };
  int index = 0;
  while (size >= 1024 && index < 3)
    {
      size /= 1024;
      index++;
    }
  sprintf (formatted_size, "%.2f %s", size, units[index]);
}

void *
compute_directory_size (void *arg)
{
  ThreadArgs *args = (ThreadArgs *)arg;
  pthread_mutex_lock (args->mutex);
  *(args->result) = get_recursive_size_directory (args->path);
  pthread_mutex_unlock (args->mutex);
  return NULL;
}

void
open_print_window (char *path)
{
  // Инициализация библиотеки ncurses
  initscr ();
  cbreak ();
  noecho ();
  curs_set (0);

  int maxy, maxx;
  getmaxyx (stdscr, maxy, maxx);

  // Создание и инициализация диалогового окна
  WINDOW *dialog_win = newwin (10, 50, (maxy - 10) / 2, (maxx - 50) / 2);
  box (dialog_win, 0, 0);
  wmove (dialog_win, 1, 1);
  wprintw (dialog_win, "Directory size: calculating...");
  wrefresh (dialog_win);

  // Создание потока для вычисления размера директории
  pthread_t tid;
  float size = -1; // Инициализация размера как -1
  pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
  ThreadArgs args = { path, &size, &mutex };
  pthread_create (&tid, NULL, compute_directory_size, (void *)&args);

  // Устанавливаем неблокирующее чтение клавиш
  nodelay (stdscr, TRUE);

  // Имитация полосы загрузки
  int progress = 0;
  while (size == -1)
    {
      if (progress == 10)
        {
          // Очистка полосы загрузки при переполнении
          mvwprintw (dialog_win, 2, 11, "           ");
          wrefresh (dialog_win);
          progress = 0;
        }
      mvwprintw (dialog_win, 2, 10, "[");
      for (int i = 0; i < progress; ++i)
        {
          wprintw (dialog_win, ".");
        }
      wprintw (dialog_win, "]");
      wrefresh (dialog_win);

      // Добавляем небольшую задержку между обновлениями
      usleep (500000); // 500000 микросекунд = 0.5 секунды

      progress++;
    }

  // Ожидаем завершения потока
  pthread_join (tid, NULL);

  char formatted_size[20];
  format_size (size, formatted_size);
  wclear (dialog_win);
  // Вывод размера директории
  wmove (dialog_win, 1, 1);
  wprintw (dialog_win, "Directory size: %s", formatted_size);
  box (dialog_win, 0, 0);
  wrefresh (dialog_win);

  // Ожидание нажатия Enter для закрытия диалогового окна
  wmove (dialog_win, 2, 1);
  wprintw (dialog_win, "Press Enter to close...");

  wrefresh (dialog_win);
  while (getch () != '\n')
    ;

  // Удаление диалогового окна
  delwin (dialog_win);
}
