#include "gui.h"

/**
 * Инициализирует библиотеку ncurses для работы с
 * графическим пользовательским интерфейсом в терминале.
 */
void
init_curses ()
{
  initscr (); // Инициализация экрана ncurses
  noecho (); // Отключение отображения вводимых символов
  curs_set (0);   // Скрытие курсора
  start_color (); // Включение поддержки цветов
  init_pair (2, COLOR_YELLOW, COLOR_BLACK);
  init_pair (1, DIR_COLOR,
             0); // Инициализация цвета для каталогов
  init_pair (3, STATUS_SELECTED_COLOR,
             0); // Инициализация цвета для выделенного
                 // элемента статуса
}

/**
 * Выделяет память и инициализирует структуру
 * `current_directory_` для хранения информации о текущем
 * каталоге. Если выделение памяти не удалось, функция
 * выводит сообщение об ошибке и завершает программу.
 */
void
init ()
{
  current_directory_
      = (directory_t *)malloc (sizeof (directory_t)); // Выделение памяти под
                                                      // структуру directory_t
  if (current_directory_ == NULL)
    {
      printf ("Error Occurred. Memory allocation "
              "failed.\n");
      exit (1); // Выход из программы с кодом ошибки
    }
}

/**
 * Инициализирует окна для отображения интерфейса.
 * Создает три окна:
 * - `current_win`: Окно для отображения списка файлов
 * текущего каталога.
 * - `path_win`: Окно для отображения текущего пути.
 * - `info_win`: Окно для отображения информации о выбранном
 * файле или статусе. Устанавливает клавиатурный режим для
 * `current_win`, чтобы обрабатывать специальные клавиши.
 */
void
init_windows ()
{
  // Создание окон с помощью функции newwin()
  current_win = newwin (maxy, maxx / 2, 0,
                        0); // Окно для списка файлов (левая часть экрана)
  path_win = newwin (2, maxx, maxy,
                     0); // Окно для отображения текущего
                         // пути (нижняя часть экрана)
  info_win = newwin (maxy, maxx / 2, 0,
                     maxx / 2); // Окно для отображения
                                // информации о выбранном
  // файле (правая часть экрана)

  refresh (); // Обновление стандартного экрана (stdscr)

  // Установка режима обработки клавиш для окна
  // current_win
  keypad (current_win,
          TRUE); // Разрешает обработку специальных клавиш
                 // (например, стрелок) в current_win
}

/**
 * Обновляет содержимое всех окон интерфейса и отображает
 * рамки вокруг некоторых окон. Рамки вокруг окон создаются
 * с использованием функции box(). После этого происходит
 * обновление каждого окна с помощью функции wrefresh().
 */
void
refreshWindows ()
{
  // Рисует рамку вокруг окна current_win с символами '|'
  // (вертикальная линия) и '-' (горизонтальная линия)
  box (current_win, '|', '-');
  // Рисует рамку вокруг окна info_win с символами '|' и
  // '-'
  box (info_win, '|', '-');

  // Обновляет содержимое окон с помощью функции
  // wrefresh()
  wrefresh (current_win); // Обновляет окно current_win
  wrefresh (path_win);    // Обновляет окно path_win
  wrefresh (info_win);    // Обновляет окно info_win
}

/**
 * Прокручивает список файлов вверх (уменьшает индекс выбора
 * `selection`). Если `selection` становится меньше 0, он
 * остается равным 0 (не позволяет выбрать отрицательный
 * индекс). Если количество файлов (`len`) больше или равно
 * высоте окна (`maxy - 1`), то происходит проверка: Если
 * текущий `selection` находится в верхней половине
 * отображаемой области (`start + maxy / 2`), то происходит
 * смещение начальной позиции (`start`) вверх. Если `start`
 * уже равен 0, то окно `current_win` очищается (`wclear`),
 * чтобы убрать старое содержимое. В противном случае
 * `start` уменьшается на 1, и окно `current_win` очищается
 * для обновления содержимого.
 */
void
scroll_up ()
{
  selection--; // Уменьшаем индекс выбора на один
  selection = (selection < 0) ? 0 : selection; // Если индекс выбора стал
                                               // отрицательным,
  // устанавливаем его в 0

  // Проверяем, достаточно ли много файлов для прокрутки
  if (len >= maxy - 1)
    {
      // Проверяем, если текущий выбор находится в
      // верхней половине отображаемой области
      if (selection <= start + maxy / 2)
        {
          // Если начальная позиция (start) уже
          // равна 0, очищаем окно current_win
          if (start == 0)
            {
              wclear (current_win); // Очищаем
                                    // окно
                                    // current_win
                                    // (удаляем
                                    // старое содержимое)
            }
          else
            {
              start--;              // Уменьшаем начальную
                                    // позицию (сдвигаем
                                    // список вверх)
              wclear (current_win); // Очищаем
                                    // окно
                                    // current_win
                                    // для
                                    // обновления
                                    // содержимого
            }
        }
    }
}

/**
 * Прокручивает список файлов вниз (увеличивает индекс
 * выбора `selection`). Если `selection` становится больше
 * или равно количеству файлов (`len - 1`), он остается
 * равным `len - 1` (не позволяет выбрать индекс за
 * пределами списка файлов). Если количество файлов (`len`)
 * больше или равно высоте окна (`maxy - 1`), то происходит
 * проверка: Если текущий `selection` находится в нижней
 * половине отображаемой области (`selection - 1 > maxy /
 * 2`), и начальная позиция (`start + maxy - 2`) не достигла
 * конца списка (`len`), то происходит смещение начальной
 * позиции (`start`) вниз. Если начальная позиция (`start`)
 * достигла конца списка, окно `current_win` очищается для
 * обновления содержимого.
 */
void
scroll_down ()
{
  selection++; // Увеличиваем индекс выбора на один
  selection
      = (selection > len - 1) ? len - 1 : selection; // Проверяем, не превысил
                                                     // ли индекс выбора
  // количество файлов - 1

  // Проверяем, достаточно ли много файлов для прокрутки
  if (len >= maxy - 1)
    {
      // Проверяем, если текущий выбор находится в
      // нижней половине отображаемой области
      if (selection - 1 > maxy / 2)
        {
          // Проверяем, достигла ли начальная
          // позиция конца списка
          if (start + maxy - 2 != len)
            {
              start++; // Увеличиваем
                       // начальную позицию
                       // (сдвигаем список
              // вниз)
              wclear (current_win); // Очищаем
                                    // окно
                                    // current_win
                                    // для
                                    // обновления
                                    // содержимого
            }
        }
    }
}


/**
 * Обрабатывает нажатие клавиши Enter для выбора действия в
 * текущем каталоге. При выборе каталога он переходит в этот
 * каталог, а при выборе файла открывает его для чтения.
 * @param files Массив строк, содержащий имена файлов и
 * подкаталогов текущего каталога. files[selection]
 * представляет выбранный элемент для обработки.
 */
void
handle_enter (char *files[])
{
  char selected_path[PATH_MAX];
  char *original_cwd = strdup (current_directory_->cwd);

  // Закрываем ncurses окно перед обработкой нажатия Enter
  endwin ();

  // Формируем полный путь к выбранному файлу или каталогу
  snprintf (selected_path, PATH_MAX, "%s/%s", current_directory_->cwd,
            files[selection]);

  struct stat file_stats;

  if (strcmp (files[selection], "..") == 0)
    {
      // Переход на уровень выше (если выбран "..")
      start = 0;
      selection = 0;
      strcpy (current_directory_->cwd, current_directory_->parent_dir);
      free (current_directory_->parent_dir);
      current_directory_->parent_dir
          = strdup (get_parent_directory (current_directory_->cwd));
    }
  else
    {
      // Переход в выбранный каталог или открытие
      // выбранного файла
      strcat (current_directory_->cwd, files[selection]);
      if (stat (selected_path, &file_stats) == -1)
        {
          perror ("Error getting file status");
          free (original_cwd);
          return;
        }

      if (S_ISDIR (file_stats.st_mode))
        {
          // Если выбран каталог, переходим в него
          start = 0;
          selection = 0;
          free (current_directory_->parent_dir);
          current_directory_->parent_dir = strdup (original_cwd);
          strcat (current_directory_->cwd, "/");
        }
      else
        {
          // Если выбран файл, открываем его для
          // чтения
          read_ (selected_path);

          // Добавляем "/" к текущему каталогу,
          // если это необходимо
          size_t original_len = strlen (original_cwd);
          if (original_len > 0 && original_cwd[original_len - 1] != '/')
            {
              strcat (current_directory_->cwd, "/");
            }
          strcpy (current_directory_->cwd, original_cwd);
        }
    }

  // Обновляем ncurses окно после обработки нажатия Enter
  refresh ();
  free (original_cwd);
}

