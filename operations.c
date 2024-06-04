#include "operations.h"
#include "globals.h"
/**
 * @brief Функция для сравнения строк.
 *
 * Эта функция используется вместе с функцией `qsort` для
 * сортировки массива строк в алфавитном порядке.
 *
 * @param a Указатель на первую строку для сравнения.
 * @param b Указатель на вторую строку для сравнения.
 * @return Значение меньше нуля, если первая строка меньше
 * второй; значение больше нуля, если первая строка больше
 * второй; ноль, если строки равны.
 */
int
compare_strings (const void *a, const void *b)
{
  // Приводим указатели к типу const char** и сравниваем
  // строки
  return strcmp (*(const char **)a, *(const char **)b);
}

/**
 * @brief Функция для сортировки массива строк.
 *
 * Эта функция использует стандартную функцию сортировки
 * `qsort` для сортировки массива строк в алфавитном
 * порядке.
 *
 * @param files Массив строк для сортировки.
 * @param n Количество элементов в массиве.
 */
void
sort_ (char *files[], int n)
{
  // Используем функцию qsort для сортировки массива строк
  // с помощью кастомного компаратора
  qsort (files, n, sizeof (char *), compare_strings);
}
/**
 * Удаляет файл из текущего каталога.
 * @param files Массив строк, содержащий имена файлов.
 *              files[selection] представляет выбранный файл
 * для удаления.
 */
void
delete_ (char *files[])
{
  char curr_path[1000];

  // Формируем полный путь к файлу для удаления
  snprintf (curr_path, sizeof (curr_path), "%s%s", current_directory_->cwd,
            files[selection]);

  // Удаляем файл с указанным путем
  remove (curr_path);
}

/**
 * Запрашивает подтверждение перед удалением файла.
 * @param files Массив строк, содержащий имена файлов.
 *              files[selection] представляет выбранный файл
 * для удаления.
 */
void
delete_file (char *files[])
{
  int c;

  // Очищаем окно path_win и выводим запрос на
  // подтверждение удаления
  wclear (path_win);
  wmove (path_win, 1, 0);
  wprintw (path_win, "Are you sure to delete? (y/n)");

LOOP_:
  // Получаем ответ пользователя
  c = wgetch (path_win);

  // Очищаем окно path_win и выводим запрос с ответом
  // пользователя
  wclear (path_win);
  wmove (path_win, 1, 0);
  wprintw (path_win, "Are you sure to delete? (y/n) %c", c);

  switch (c)
    {
    case 'y':
    case 'Y':
      delete_ (files); // Вызываем функцию delete_ для
                       // удаления файла
      break;
    case 'n':
    case 'N':
      break; // Ничего не делаем, пользователь
             // отказался от удаления
    default:
      goto LOOP_; // Повторяем цикл, если введенный
                  // символ некорректен
      break;
    }
}

/**
 * Копирует выбранный файл в новый путь, запрашиваемый у
 * пользователя.
 * @param files Массив строк, содержащий имена файлов.
 *              files[selection] представляет выбранный файл
 * для копирования.
 */
void
copy_files (char *files[])
{
  char new_path[1000];
  int i = 0, c;

  // Очищаем окно path_win и запрашиваем новый путь у
  // пользователя
  wclear (path_win);
  wmove (path_win, 1, 0);
  while ((c = wgetch (path_win)) != '\n')
    {
      if (c == 127 || c == 8)
        {
          new_path[--i] = '\0';
          i = i < 0 ? 0 : i;
        }
      else
        {
          new_path[i++] = c;
          new_path[i] = '\0';
        }
      wclear (path_win);
      wmove (path_win, 1, 0);
      wprintw (path_win, "%s", new_path);
    }

  // Формируем полный путь до нового каталога/файла
  char target_path[1000];
  snprintf (target_path, sizeof (target_path), "%s/%s", new_path,
            files[selection]);

  // Открываем старый файл для чтения и новый файл для
  // записи
  FILE *old_file, *new_file;
  char curr_path[1000];
  snprintf (curr_path, sizeof (curr_path), "%s/%s", current_directory_->cwd,
            files[selection]);

  old_file = fopen (curr_path, "rb");
  if (old_file == NULL)
    {
      perror ("Error opening source file");
      return;
    }

  new_file = fopen (target_path, "wb");
  if (new_file == NULL)
    {
      perror ("Error opening destination file");
      fclose (old_file);
      return;
    }

  // Копируем содержимое старого файла в новый файл
  int ch;
  while ((ch = fgetc (old_file)) != EOF)
    {
      fputc (ch, new_file);
    }

  // Закрываем файлы
  fclose (old_file);
  fclose (new_file);

  // Очищаем окно current_win и выводим информацию об
  // успешном копировании
  wclear (current_win);
  wmove (current_win, 10, 10);
  wprintw (current_win, "File copied successfully from\n%s\nto\n%s\n",
           curr_path, target_path);
  wrefresh (current_win);
}

/**
 * Перемещает выбранный файл в новый путь, запрашиваемый у
 * пользователя. Эта операция выполняется путем копирования
 * файла в новый путь и затем удаления оригинала.
 * @param files Массив строк, содержащий имена файлов.
 *              files[selection] представляет выбранный файл
 * для перемещения.
 */
void
move_file (char *files[])
{
  // Копируем выбранный файл в новый путь
  copy_files (files);

  // Удаляем оригинал файла
  delete_ (files);
}


/**
 * Сортирует массив имен файлов в алфавитном порядке с
 * использованием сортировки пузырьком (bubble sort).
 * @param files_ Массив указателей на строки (имена файлов),
 * который нужно отсортировать.
 * @param n Количество элементов в массиве `files_`.
 */
void
sort (char *files_[], int n)
{
  char temp[1000]; // Временный буфер для обмена значениями

  // Перебор всех элементов массива для сортировки
  for (int i = 0; i < n - 1; i++)
    {
      for (int j = i + 1; j < n; j++)
        {
          // Сравниваем имена файлов и меняем их
          // местами, если необходимо
          if (strcmp (files_[i], files_[j]) > 0)
            {
              strcpy (temp,
                      files_[i]); // Копируем
                                  // текущий
                                  // элемент во
                                  // временный
                                  // буфер
              strcpy (files_[i],
                      files_[j]); // Заменяем
                                  // текущий
                                  // элемент на
                                  // следующий
              strcpy (files_[j],
                      temp); // Заменяем следующий
                             // элемент на
              // текущий из временного буфера
            }
        }
    }
}
// Список наиболее часто встречающихся текстовых расширений
const char *text_extensions[] = {
    ".txt", ".c", ".h", ".cpp", ".hpp", ".py", ".java", ".html", ".xml", ".csv", ".json", ".md", ".sh", ".bat"
};

// Функция для проверки расширения файла
int has_text_extension(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if (!dot) return 0; // Нет расширения

    for (int i = 0; i < sizeof(text_extensions) / sizeof(text_extensions[0]); i++) {
        if (strcmp(dot, text_extensions[i]) == 0) {
            return 1; // Найдено совпадение с текстовым расширением
        }
    }
    return 0; // Расширение не найдено в списке текстовых
}

// Функция для проверки содержания файла
int check_text_content(const char *path) {
    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        return 0; // Файл не найден или не может быть открыт
    }

    int printable_count = 0;
    int total_count = 0;
    int c;

    while ((c = fgetc(file)) != EOF) {
        total_count++;
        if (isprint(c) || isspace(c)) {
            printable_count++;
        }
    }

    fclose(file);

    // Проверяем долю печатных символов
    if (total_count == 0) {
        return 0; // Пустой файл не считается текстовым
    }

    double printable_ratio = (double)printable_count / total_count;
    return printable_ratio > 0.9; // Считаем файл текстовым, если более 90% символов печатные
}

// Основная функция для проверки, является ли файл текстовым
int check_text(char *path) {
    // Сначала проверяем расширение
    if (has_text_extension(path)) {
        return 1; // Файл имеет текстовое расширение
    }

    // Если расширение не является текстовым, проверяем содержимое файла
    return check_text_content(path);
}


/**
 * Читает содержимое файла и выводит его на экран в окне
 * ncurses.
 * @param path Путь к файлу, который нужно прочитать.
 */
void
read_ (char *path)
{
  char buffer[256]; // Буфер для чтения данных из файла
  int ch; // Переменная для хранения нажатой клавиши
  wclear (current_win); // Очищаем текущее окно ncurses
  wclear (info_win); // Очищаем информационное окно ncurses
  wresize (current_win, maxy,
           maxx); // Изменяем размер текущего окна до максимального

  FILE *ptr = fopen (path, "rb"); // Открываем файл для чтения в бинарном режиме
  if (ptr == NULL)
    {
      perror ("Error"); // Выводим сообщение об ошибке, если не удалось
                        // открыть файл
      return;
    }

  int t = 2, pos = 0, count;
  int is_text = check_text (path); // Проверяем, является ли файл текстовым

  do
    {
      wmove (current_win, 1, 2);
      wprintw (current_win, "Press \"q\" to Exit");

      if (is_text)
        {
          count = 0;
          int line = 2;
          fseek (ptr, 0,
                 SEEK_SET); // Сбрасываем указатель файла в начало
          while (fgets (buffer, sizeof (buffer), ptr))
            { // Читаем файл построчно
              if (count >= pos && line < maxy - 4)
                { // Если достигли текущей позиции и не вышли за
                  // пределы окна
                  wmove (current_win, line++,
                         1); // Перемещаем курсор в следующую
                             // строку
                  wprintw (current_win, "%.*s", maxx - 2,
                           buffer); // Выводим содержимое строки
                }
              count++;
            }
        }
      else
        {
          fseek (ptr, pos * (maxx - 2),
                 SEEK_SET); // Перемещаем указатель в нужное место файла
          size_t bytesRead = fread (buffer, 1, maxx - 2,
                                    ptr); // Читаем данные из файла
          if (bytesRead > 0)
            {
              wmove (current_win, 2,
                     1); // Перемещаем курсор в следующую строку
              for (size_t i = 0; i < bytesRead; ++i)
                {
                  wprintw (current_win, "%02x ",
                           (unsigned char)buffer[i]); // Выводим прочитанные
                                                      // данные в виде
                                                      // шестнадцатеричных
                                                      // значений
                }
            }
        }

      box (current_win, '|',
           '-'); // Отображаем рамку вокруг текущего окна
      wrefresh (current_win); // Обновляем текущее окно ncurses
      ch = wgetch (current_win); // Получаем нажатую клавишу
      wclear (current_win); // Очищаем текущее окно перед обновлением

      switch (ch)
        {
        case KEY_UP:
          pos = pos == 0 ? 0 : pos - 1; // Перемещаемся вверх по файлу
                                        // (уменьшаем позицию)
          break;
        case KEY_DOWN:
          pos++; // Перемещаемся вниз по файлу (увеличиваем позицию)
          break;
        }
    }
  while (ch != 'q'); // Выполняем цикл, пока не нажата клавиша "q"

  fclose (ptr); // Закрываем файл после окончания чтения
  endwin (); // Завершаем работу с библиотекой ncurses
}

/**
 * Переименовывает выбранный файл из списка `files`.
 * Функция запрашивает новое имя файла у пользователя через
 * ncurses. После ввода нового имени, пользователю
 * предлагается подтвердить переименование.
 * @param files Массив строк с именами файлов.
 */
void
rename_file (char *files[])
{
  char new_name[100];
  int i = 0, c;

  // Очистка окна path_win и установка курсора для ввода
  // нового имени файла
  wclear (path_win);
  wmove (path_win, 1, 0);

  // Чтение нового имени файла с клавиатуры
  while ((c = wgetch (path_win)) != '\n')
    {
      if (c == 127 || c == 8)
        {
          // Обработка удаления символа
          // (Backspace)
          new_name[--i] = '\0';
        }
      else
        {
          // Добавление введенного символа к
          // новому имени файла
          new_name[i++] = c;
          new_name[i] = '\0';
        }
      wclear (path_win);
      wmove (path_win, 1, 0);
      wprintw (path_win, "%s", new_name);
    }

  c = ' ';

LOOP:
  // Очистка окна path_win и вывод нового имени файла с
  // запросом подтверждения
  wclear (path_win);
  wmove (path_win, 1, 0);
  wprintw (path_win, "%s (y/n) %c", new_name, c);
  c = wgetch (path_win);

  char *a, *b;
  switch (c)
    {
    case 'y':
    case 'Y':
      // Формирование полного пути и переименование
      // файла
      a = strdup (current_directory_->cwd);
      b = strdup (current_directory_->cwd);
      strcat (a, files[selection]);
      strcat (b, new_name);
      rename (a, b); // Переименование файла
      free (a);
      free (b);
      break;
    case 'n':
    case 'N':
      // Отмена операции переименования
      break;
    default:
      // Обработка некорректного ввода (повторный ввод
      // подтверждения)
      goto LOOP;
      break;
    }
}