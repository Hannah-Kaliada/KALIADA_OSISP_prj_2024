#include "find.h"

/**
 * @brief Функция для поиска файлов по регулярному выражению
 * и обработки результата.
 *
 * Функция позволяет пользователю вводить регулярное
 * выражение для поиска файлов в текущей директории и ее
 * поддиректориях. После завершения поиска и вывода
 * результатов, пользователь может вернуться к родительской
 * директории или выйти из программы.
 *
 * @param files Массив строк с путями к файлам.
 */
 void find(char *files[]) {
    char root_dir[MAX_PATH_LENGTH];
    if (current_directory_ == NULL) {
        return;
    }

    // Проверяем и корректируем путь к корневой директории
    if (current_directory_->cwd[strlen(current_directory_->cwd) - 1] == '/') {
        snprintf(root_dir, sizeof(root_dir), "%.*s",
                 (int)(strlen(current_directory_->cwd) - 1),
                 current_directory_->cwd);
    } else {
        strncpy(root_dir, current_directory_->cwd, sizeof(root_dir) - 1);
        root_dir[sizeof(root_dir) - 1] = '\0'; // Ensure null-termination
    }

    int i = 0, c;
    int ch;

    wclear(current_win);
    wclear(info_win);

    wresize(current_win, maxy, maxx);

    wmove(info_win, 1, 1);
    wprintw(info_win, "Enter the regex for searching: ");

    // Основные правила для создания регулярного выражения
    wprintw(info_win, "\n\n Main rules for composing a regex:\n");
    wprintw(info_win, "  - Use '.' to match any single character\n");
    wprintw(info_win, "  - Use '*' to match zero or more "
                     "occurrences of the "
                     "previous character\n");
    wprintw(info_win, "  - Use '|' for alternation (OR operation)\n");
    wprintw(info_win, "  - Use '()' for grouping\n");
    wprintw(info_win, "  - Use '^' to match the start of a string\n");
    wprintw(info_win, "  - Use '$' to match the end of a string\n");
    wprintw(info_win, "  - Use '\\\\' to escape special "
                     "characters if needed\n");
    wprintw(info_win, "  - Use '[]' for character classes\n");
    wprintw(info_win, "  - Use '{}' for specifying repetition\n");
    wprintw(info_win, "  - Use '\\' to escape special "
                     "characters, if needed\n");

    wrefresh(info_win);

    // Выделяем память для регулярного выражения
    char *regexp = (char *)malloc(256 * sizeof(char));
    if (regexp == NULL) {
        perror("Memory allocation failed");
        return;
    }
    regexp[0] = '\0'; // Ensure null-termination

    // Чтение регулярного выражения от пользователя
    while ((c = wgetch(info_win)) != '\n') {
        if (c == 127) { // Assuming 127 is the backspace key
            if (i > 0) {
                regexp[--i] = '\0';
            }
        } else {
            if (i < 255) { // Prevent buffer overflow
                regexp[i++] = c;
                regexp[i] = '\0';
            }
        }
        wclear(info_win);
        wmove(info_win, 1, 1);
        wprintw(info_win, "Enter the regex for searching: %s", regexp);
        wrefresh(info_win);
    }

    int num_matches;
    // Ищем файлы с использованием регулярного выражения
    char **file_paths = find_files_by_regex(root_dir, regexp, &num_matches);
    if (file_paths == NULL || num_matches == 0) {
        wclear(current_win);
        box(current_win, '|', '-');
        wmove(current_win, 1, 1);
        wprintw(current_win, "No matching files found");
        wrefresh(current_win);
        free(regexp);
        wgetch(current_win);
        return;
    }

    wclear(info_win);
    wmove(info_win, 1, 1);
    wprintw(info_win, "Found %d files matching the regular expression '%s'\n",
           num_matches, regexp);
    wmove(info_win, 2, 2);
    wprintw(info_win,
           "Press Enter to return to parent directory or 'q' to exit...\n");
    wrefresh(info_win);
    // Вывод всех файлов совпадающих с регулярным выражением
    int selection = 0;
    do {
        wclear(current_win);
        box(current_win, '|', '-');
        wmove(current_win, 1, 1);
        for (int i = 0; i < num_matches; ++i) {
            wmove(current_win, i + 2, 2);
            if (i == selection) {
                wattron(current_win, A_STANDOUT);
            }
            wprintw(current_win, "%s\n", file_paths[i]);
            if (i == selection) {
                wattroff(current_win, A_STANDOUT);
            }
        }
        wrefresh(current_win);
        ch = wgetch(current_win);
        switch (ch) {
            case KEY_UP:
                if (selection > 0) {
                    selection--;
                }
                break;
            case KEY_DOWN:
                if (selection < num_matches - 1) {
                    selection++;
                }
                break;
        }
    } while (ch != KEY_ENTER && ch != '\n' && ch != 'q');
    if (ch == KEY_ENTER || ch == '\n') {
          strcpy (current_directory_->cwd, dirname (file_paths[selection]));
          strcat(current_directory_->cwd, "/");
          start = 0;
          selection = 0;
          free (current_directory_->parent_dir);
          current_directory_->parent_dir
          = strdup (get_parent_directory (current_directory_->cwd));
          refresh ();
    }
} 



/**
 * @brief Функция для рекурсивного поиска файлов в
 * директории, соответствующих регулярному выражению.
 *
 * Функция открывает указанную директорию и рекурсивно ищет
 * файлы и поддиректории, соответствующие заданному
 * регулярному выражению. При успешном завершении функции,
 * найденные файлы добавляются в массив путей к файлам.
 *
 * @param dir_path Путь к директории, в которой нужно
 * выполнить поиск.
 * @param regex_str Регулярное выражение для поиска файлов.
 * @param file_paths Указатель на указатель на массив строк
 * с путями к файлам.
 * @param num_matches Указатель на переменную для хранения
 * количества найденных совпадений.
 * @param regex Указатель на скомпилированное регулярное
 * выражение.
 */
void
find_files_recursive (const char *dir_path,
                      const char *regex_str,
                      char ***file_paths, int *num_matches,
                      regex_t *regex)
{
  // Открываем директорию
  DIR *dir = opendir (dir_path);
  if (dir == NULL)
    {
      return;
    }

  // Перебираем файлы и поддиректории в директории
  struct dirent *entry;
  while ((entry = readdir (dir)) != NULL)
    {
      // Формируем полный путь к элементу
      char full_path[MAX_PATH_LENGTH];
      snprintf (full_path, MAX_PATH_LENGTH, "%s/%s",
                dir_path, entry->d_name);

      // Получаем информацию о файле (с учетом
      // символических ссылок)
      struct stat statbuf;
      if (lstat (full_path, &statbuf) == -1)
        {
          continue;
        }

      // Проверяем, является ли элемент директорией
      if (S_ISDIR (statbuf.st_mode))
        {
          // Пропускаем специальные директории "."
          // и ".."
          if (strcmp (entry->d_name, ".") == 0
              || strcmp (entry->d_name, "..") == 0)
            continue;

          // Рекурсивно ищем файлы в поддиректории
          find_files_recursive (full_path, regex_str,
                                file_paths, num_matches,
                                regex);
        }
      else if (S_ISREG (statbuf.st_mode))
        {
          // Проверяем, соответствует ли имя файла
          // регулярному выражению
          if (regexec (regex, entry->d_name, 0, NULL, 0)
              == 0)
            {
              // Добавляем путь к файлу в
              // массив
              char *file_path = (char *)malloc (
                  MAX_PATH_LENGTH * sizeof (char));
              strncpy (file_path, full_path,
                       MAX_PATH_LENGTH);
              (*file_paths)[*num_matches] = file_path;
              (*num_matches)++;

              // Проверка на
              // максимальное количество
              // совпадений
              if (*num_matches >= MAX_MATCHES)
                {
                  closedir (dir);
                  return;
                }
            }
        }
    }

  // Закрываем директорию
  closedir (dir);
}

/**
 * @brief Поиск файлов в файловой системе по регулярному
 * выражению.
 *
 * Функция ищет файлы во всей файловой системе,
 * соответствующие заданному регулярному выражению. При
 * успешном завершении функции возвращает массив строк,
 * содержащий пути к найденным файлам.
 *
 * @param root_dir Корневая директория, в которой начинается
 * поиск.
 * @param regex_str Регулярное выражение для поиска файлов.
 * @param num_matches Указатель на переменную для хранения
 * количества найденных совпадений.
 * @return Указатель на массив строк с путями к найденным
 * файлам. В случае ошибки возвращает NULL.
 */
char **
find_files_by_regex (const char *root_dir,
                     const char *regex_str,
                     int *num_matches)
{
  // Компилируем регулярное выражение
  // REG_EXTENDED - используем расширенный синтаксис
  // REG_NOSUB - не сохраняем подстроки
  regex_t regex;
  if (regcomp (&regex, regex_str, REG_EXTENDED | REG_NOSUB)
      != 0)
    {
      return NULL;
    }

  // Выделяем память под массив строк для путей к файлам
  char **file_paths
      = (char **)malloc (MAX_MATCHES * sizeof (char *));
  if (file_paths == NULL)
    {
      exit (EXIT_FAILURE);
    }

  // Инициализирование счетчика совпадений
  *num_matches = 0;

  // Рекурсивный поиск файлов в корневой директории
  find_files_recursive (root_dir, regex_str, &file_paths,
                        num_matches, &regex);

  // Освобождение ресурсов
  regfree (&regex);

  return file_paths;
}

/**
 * Возвращает родительский каталог для заданного пути.
 * @param cwd Строка с текущим рабочим каталогом (полный
 * путь).
 * @return Строка с родительским каталогом заданного пути.
 *         Память для возвращаемой строки выделяется
 * динамически и должна быть освобождена вызывающей
 * стороной.
 */
char *
get_parent_directory (char *cwd)
{
  char *a;
  a = strdup (cwd); // Создание копии текущего рабочего каталога
  int i = (int)(strlen (a) - 1);

  // Поиск последнего символа '/' в пути
  while (a[--i] != '/')
    ;

  a[++i] = '\0'; // Установка завершающего нуля для
                 // формирования строки
  // родительского каталога
  return a;
}