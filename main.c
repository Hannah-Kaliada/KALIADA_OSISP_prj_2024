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
#include <dirent.h>
#include <libgen.h>
#include <limits.h>
#include <locale.h>
#include <ncurses.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#include "config.h"

#define N strlen (gen_poly) // Длина генерирующего полинома
#define MAX_PATH_LENGTH 1024
#define MAX_MATCHES 100

struct stat file_stats;
WINDOW *current_win, *info_win, *path_win;
int selection, maxx, maxy, len = 0, start = 0;
directory_t *current_directory_ = NULL;
char gen_poly[64]; // Генерирующий полином
char check_value[128]; // Сумма для проверки CRC
char checked_value[128]; // Проверяемые данные из файла
int sortFlag = 0;

void init_curses ();
void init ();
void init_windows ();
void refreshWindows ();
int get_number_of_files_in_directory (char *directory);
int get_files (char *directory, char *target[]);
void scroll_up ();
void scroll_down ();
void sort (char *files_[], int n);
int check_text (char *path);
void read_ (char *path);
void readInNano (char *path);
void rename_file (char *files[]);
void delete_ (char *files[]);
void delete_file (char *files[]);
void copy_files (char *files[]);
void move_file (char *files[]);
void handle_enter (char *files[]);
float get_recursive_size_directory (char *path);
void show_file_info (char *files[]);
char *get_parent_directory (char *cwd);
void crc (char *files[]);
int compare_strings (const void *a, const void *b);
void sort_ (char *files[], int n);
void find (char *files[]);
void find_files_recursive (const char *dir_path,
                           const char *regex_str,
                           char ***file_paths,
                           int *num_matches,
                           regex_t *regex);
char **find_files_by_regex (const char *root_dir,
                            const char *regex_str,
                            int *num_matches);

int
main ()
{
    setlocale (LC_ALL, "ru_RU.UTF-8");
    init ();
    init_curses ();
    strcat (current_directory_->cwd, "/");
    current_directory_->parent_dir = strdup (
        get_parent_directory (current_directory_->cwd));
    int ch;

    do
        {
            len = get_number_of_files_in_directory (
                current_directory_->cwd);
            if (len < 0)
                {
                    perror ("Error getting file count in "
                            "directory");
                    break;
                }

            len = len <= 0 ? 1 : len;
            char *files[len];
            if (get_files (current_directory_->cwd, files)
                < 0)
                {
                    perror (
                        "Error getting files in directory");
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
                    int size = snprintf (
                        NULL, 0, "%s%s",
                        current_directory_->cwd, files[i]);
                    if (i == selection)
                        {
                            wattron (current_win,
                                     A_STANDOUT);
                        }
                    else
                        {
                            wattroff (current_win,
                                      A_STANDOUT);
                        }

                    char *temp_dir
                        = (char *)malloc (size + 1);
                    snprintf (temp_dir, size + 1, "%s%s",
                              current_directory_->cwd,
                              files[i]);

                    stat (temp_dir, &file_stats);
                    isDir (file_stats.st_mode)
                        ? wattron (current_win,
                                   COLOR_PAIR (1))
                        : wattroff (current_win,
                                    COLOR_PAIR (2));
                    wmove (current_win, t + 1, 2);
                    wprintw (current_win, "%.*s\n", maxx,
                             files[i]);
                    free (temp_dir);
                    t++;
                }
            wmove (path_win, 1, 0);
            wprintw (path_win, " %s",
                     current_directory_->cwd);
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
                        wprintw (info_win,
                                 " Press \"k\" to "
                                 "scroll up\n");
                        wprintw (info_win,
                                 " Press \"j\" to "
                                 "scroll down\n");
                        wprintw (info_win,
                                 " Press \"r\" to "
                                 "rename file\n");
                        wprintw (info_win,
                                 " Press \"c\" to "
                                 "copy file\n");
                        wprintw (info_win,
                                 " Press \"m\" to "
                                 "move file\n");
                        wprintw (info_win,
                                 " Press \"d\" to "
                                 "delete file\n");
                        wprintw (info_win,
                                 " Press \"a\" to "
                                 "calculate CRC\n");
                        wprintw (info_win,
                                 " Press \"s\" to "
                                 "sort files\n");
                        wprintw (info_win,
                                 " Press \"f\" to "
                                 "find files\n");
                        wprintw (info_win, "Press \"h\" to "
                                           "show help\n");
                        wprintw (info_win,
                                 " Press \"q\" to "
                                 "exit\n");
                        wprintw (info_win,
                                 " Press Enter to "
                                 "continue...\n");
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
 * Инициализирует библиотеку ncurses для работы с
 * графическим пользовательским интерфейсом в терминале.
 */
void
init_curses ()
{
    initscr (); // Инициализация экрана ncurses
    noecho (); // Отключение отображения вводимых символов
    curs_set (0); // Скрытие курсора
    start_color (); // Включение поддержки цветов
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
    current_directory_ = (directory_t *)malloc (
        sizeof (directory_t)); // Выделение памяти под
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
    current_win = newwin (
        maxy, maxx / 2, 0,
        0); // Окно для списка файлов (левая часть экрана)
    path_win = newwin (2, maxx, maxy,
                       0); // Окно для отображения текущего
                           // пути (нижняя часть экрана)
    info_win = newwin (maxy, maxx / 2, 0,
                       maxx / 2); // Окно для отображения
                                  // информации о выбранном
    // файле (правая часть экрана)

    // Обновление экрана с помощью функции refresh()
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
    wrefresh (path_win); // Обновляет окно path_win
    wrefresh (info_win); // Обновляет окно info_win
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
                    target[i++]
                        = strdup (dir_entry->d_name);
                }
        }
    closedir (dir_);
    return 1;
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
    selection = (selection < 0)
                    ? 0
                    : selection; // Если индекс выбора стал
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
                            wclear (
                                current_win); // Очищаем
                                              // окно
                                              // current_win
                                              // (удаляем
                            // старое содержимое)
                        }
                    else
                        {
                            start--; // Уменьшаем начальную
                                     // позицию (сдвигаем
                                     // список вверх)
                            wclear (
                                current_win); // Очищаем
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
    selection = (selection > len - 1)
                    ? len - 1
                    : selection; // Проверяем, не превысил
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
                            wclear (
                                current_win); // Очищаем
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
 * Сортирует массив имен файлов в алфавитном порядке с
 * использованием сортировки пузырьком (bubble sort).
 * @param files_ Массив указателей на строки (имена файлов),
 * который нужно отсортировать.
 * @param n Количество элементов в массиве `files_`.
 */
void
sort (char *files_[], int n)
{
    char
        temp[1000]; // Временный буфер для обмена значениями

    // Перебор всех элементов массива для сортировки
    for (int i = 0; i < n - 1; i++)
        {
            for (int j = i + 1; j < n; j++)
                {
                    // Сравниваем имена файлов и меняем их
                    // местами, если необходимо
                    if (strcmp (files_[i], files_[j]) > 0)
                        {
                            strcpy (
                                temp,
                                files_[i]); // Копируем
                                            // текущий
                                            // элемент во
                                            // временный
                                            // буфер
                            strcpy (
                                files_[i],
                                files_[j]); // Заменяем
                                            // текущий
                                            // элемент на
                                            // следующий
                            strcpy (
                                files_[j],
                                temp); // Заменяем следующий
                                       // элемент на
                            // текущий из временного буфера
                        }
                }
        }
}

/**
 * Проверяет текстовый файл на наличие непечатаемых
 * символов.
 * @param path Путь к файлу, который нужно проверить.
 * @return Возвращает 1, если файл содержит только
 * печатаемые символы ASCII (от 0x20 до 0x7E), иначе
 * возвращает 0.
 */
int
check_text (char *path)
{
    FILE *ptr;
    ptr = fopen (path, "r"); // Открываем файл для чтения
    if (ptr == NULL)
        {
            perror (
                "Error opening file"); // Выводим сообщение
                                       // об ошибке, если не
            // удалось открыть файл
            return 0; // Возвращаем 0, чтобы указать на
                      // ошибку
        }

    int c;
    while ((c = fgetc (ptr)) != EOF)
        { // Читаем файл посимвольно, пока не достигнем
          // конца
            // файла (EOF)
            if (c < 0 || c > 127)
                { // Проверяем, является ли символ
                  // непечатаемым
                    // (вне диапазона ASCII)
                    fclose (ptr); // Закрываем файл
                    return 0; // Возвращаем 0, чтобы указать
                              // на наличие непечатаемого
                    // символа
                }
        }

    fclose (ptr); // Закрываем файл после окончания чтения
    return 1; // Возвращаем 1, если файл содержит только
              // печатаемые символы
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
    wclear (
        info_win); // Очищаем информационное окно ncurses
    wresize (current_win, maxy,
             maxx); // Изменяем размер текущего окна до
                    // максимального

    FILE *ptr;
    ptr = fopen (path,
                 "rb"); // Открываем файл для чтения в
                        // бинарном режиме
    if (ptr == NULL)
        {
            perror ("Error"); // Выводим сообщение об
                              // ошибке, если не удалось
            // открыть файл
            return;
        }

    int t = 2, pos = 0, count;
    int is_text = check_text (
        path); // Проверяем, является ли файл текстовым

    do
        {
            wmove (current_win, 1, 2);
            wprintw (current_win, "Press \"q\" to Exit");

            if (is_text)
                {
                    count = 0;
                    while (fgets (buffer, sizeof (buffer),
                                  ptr))
                        { // Читаем файл построчно
                            if (count < pos)
                                {
                                    count++;
                                    continue;
                                }
                            wmove (current_win, ++t,
                                   1); // Перемещаем курсор
                                       // в следующую строку
                            wprintw (current_win, "%.*s",
                                     maxx - 2,
                                     buffer); // Выводим
                                              // содержимое
                                              // строки
                        }
                }
            else
                {
                    fseek (
                        ptr, pos * (maxx - 2),
                        SEEK_SET); // Перемещаем указатель в
                                   // нужное место файла
                    size_t bytesRead = fread (
                        buffer, sizeof (unsigned char),
                        maxx - 2,
                        ptr); // Читаем данные из файла
                    if (bytesRead > 0)
                        {
                            wmove (current_win, ++t,
                                   1); // Перемещаем курсор
                                       // в следующую строку
                            wprintw (current_win, "%.*s",
                                     bytesRead,
                                     buffer); // Выводим
                                              // прочитанные
                                              // данные
                        }
                }

            box (current_win, '|',
                 '-'); // Отображаем рамку вокруг текущего
                       // окна
            wrefresh (current_win); // Обновляем текущее
                                    // окно ncurses
            ch = wgetch (
                current_win); // Получаем нажатую клавишу
            wclear (current_win); // Очищаем текущее окно
                                  // перед обновлением

            switch (ch)
                {
                case KEY_UP:
                    pos = pos == 0
                              ? 0
                              : pos - 1; // Перемещаемся
                                         // вверх по файлу
                    // (уменьшаем позицию)
                    break;
                case KEY_DOWN:
                    pos++; // Перемещаемся вниз по файлу
                           // (увеличиваем позицию)
                    break;
                }
        }
    while (ch != 'q'); // Выполняем цикл, пока не нажата
                       // клавиша "e" или "E"

    fclose (ptr); // Закрываем файл после окончания чтения
    endwin (); // Завершаем работу с библиотекой ncurses
}

/**
 * Открывает указанный файл в текстовом редакторе nano для
 * редактирования.
 * @param path Путь к файлу, который нужно открыть.
 */
void
readInNano (char *path)
{
    struct termios original_termios;
    tcgetattr (STDIN_FILENO,
               &original_termios); // Получаем текущие
                                   // настройки терминала

    pid_t pid
        = fork (); // Создаем новый процесс с помощью fork()

    if (pid == 0)
        { // Дочерний процесс
            execlp ("nano", "nano", path,
                    NULL); // Запускаем программу nano для
                           // открытия файла
            perror ("exec"); // В случае ошибки выполнения
                             // exec выводим сообщение об
            // ошибке
            exit (EXIT_FAILURE); // Завершаем дочерний
                                 // процесс с ошибкой
        }
    else if (pid < 0)
        { // Ошибка при вызове fork()
            perror ("fork"); // Выводим сообщение об ошибке
        }
    else
        { // Родительский процесс
            int status;
            pid_t wpid = waitpid (
                pid, &status,
                0); // Ждем завершения дочернего процесса

            if (wpid == -1)
                { // Ошибка при ожидании дочернего процесса
                    perror ("waitpid"); // Выводим сообщение
                                        // об ошибке
                }
            else
                {
                    if (WIFEXITED (status))
                        { // Проверяем, завершился ли
                          // дочерний
                            // процесс нормально
                            tcsetattr (
                                STDIN_FILENO, TCSANOW,
                                &original_termios); // Восстанавливаем
                                                    // настройки
                                                    // терминала
                        }
                    else if (WIFSIGNALED (status))
                        { // Проверяем, завершился ли
                            // дочерний процесс из-за
                            // сигнала
                            printf (
                                "nano editor terminated by "
                                "signal: %d\n",
                                WTERMSIG (
                                    status)); // Выводим
                                              // информацию
                                              // о сигнале
                        }
                }
        }
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
    a = strdup (
        cwd); // Создание копии текущего рабочего каталога
    int i = (int)(strlen (a) - 1);

    // Поиск последнего символа '/' в пути
    while (a[--i] != '/')
        ;

    a[++i] = '\0'; // Установка завершающего нуля для
                   // формирования строки
    // родительского каталога
    return a;
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
    snprintf (curr_path, sizeof (curr_path), "%s%s",
              current_directory_->cwd, files[selection]);

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
    wprintw (path_win, "Are you sure to delete? (y/n) %c",
             c);

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
    snprintf (target_path, sizeof (target_path), "%s/%s",
              new_path, files[selection]);

    // Открываем старый файл для чтения и новый файл для
    // записи
    FILE *old_file, *new_file;
    char curr_path[1000];
    snprintf (curr_path, sizeof (curr_path), "%s/%s",
              current_directory_->cwd, files[selection]);

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
    wprintw (current_win,
             "File copied successfully from\n%s\nto\n%s\n",
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
    snprintf (selected_path, PATH_MAX, "%s/%s",
              current_directory_->cwd, files[selection]);

    struct stat file_stats;

    if (strcmp (files[selection], "..") == 0)
        {
            // Переход на уровень выше (если выбран "..")
            start = 0;
            selection = 0;
            strcpy (current_directory_->cwd,
                    current_directory_->parent_dir);
            free (current_directory_->parent_dir);
            current_directory_->parent_dir
                = strdup (get_parent_directory (
                    current_directory_->cwd));
        }
    else
        {
            // Переход в выбранный каталог или открытие
            // выбранного файла
            strcat (current_directory_->cwd,
                    files[selection]);
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
                    current_directory_->parent_dir
                        = strdup (original_cwd);
                    strcat (current_directory_->cwd, "/");
                }
            else
                {
                    // Если выбран файл, открываем его для
                    // чтения
                    read_ (selected_path);

                    // Добавляем "/" к текущему каталогу,
                    // если это необходимо
                    size_t original_len
                        = strlen (original_cwd);
                    if (original_len > 0
                        && original_cwd[original_len - 1]
                               != '/')
                        {
                            strcat (current_directory_->cwd,
                                    "/");
                        }
                    strcpy (current_directory_->cwd,
                            original_cwd);
                }
        }

    // Обновляем ncurses окно после обработки нажатия Enter
    refresh ();
    free (original_cwd);
}

/**
 * Рекурсивно вычисляет размер указанного каталога и всех
 * его подкаталогов в килобайтах (KB).
 * @param path Путь к каталогу, размер которого требуется
 * вычислить.
 * @return Общий размер каталога и его содержимого в KB.
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
                    snprintf (temp_path, sizeof (temp_path),
                              "%s/%s", path,
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
                            directory_size
                                += get_recursive_size_directory (
                                    temp_path);
                        }
                    else
                        {
                            // Если текущий элемент не
                            // является каталогом, добавляем
                            // его размер к общему размеру
                            directory_size
                                += (float)(file_stat
                                               .st_size)
                                   / (float)1024; // Преобразуем
                                                  // размер
                                                  // в KB
                        }
                }
        }

    closedir (dir_);
    return directory_size;
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
            snprintf (temp_address, sizeof (temp_address),
                      "%s%s", current_directory_->cwd,
                      files[selection]);

            struct stat file_stats;

            // Получаем информацию о выбранном элементе с
            // помощью lstat
            if (lstat (temp_address, &file_stats) == 0)
                {
                    wprintw (info_win, "Name: %s\n",
                             files[selection]);

                    // Определяем тип файла
                    wprintw (info_win, " Type:");
                    switch (file_stats.st_mode & S_IFMT)
                        {
                        case S_IFREG:
                            wprintw (info_win,
                                     " Regular File\n");
                            break;
                        case S_IFDIR:
                            wprintw (info_win,
                                     " Directory\n");
                            break;
                        case S_IFLNK:
                            wprintw (info_win,
                                     " Symbolic Link\n");
                            break;
                        case S_IFIFO:
                            wprintw (info_win,
                                     " FIFO/Named Pipe\n");
                            break;
                        case S_IFCHR:
                            wprintw (info_win,
                                     " Character Device\n");
                            break;
                        case S_IFBLK:
                            wprintw (info_win,
                                     " Block Device\n");
                            break;
                        case S_IFSOCK:
                            wprintw (info_win, " Socket\n");
                            break;
                        default:
                            wprintw (info_win,
                                     " Unknown\n");
                            break;
                        }
                    wprintw (info_win, " Permissions: ");
                    wprintw (info_win,
                             (file_stats.st_mode & S_IRUSR)
                                 ? "r"
                                 : "-");
                    wprintw (info_win,
                             (file_stats.st_mode & S_IWUSR)
                                 ? "w"
                                 : "-");
                    wprintw (info_win,
                             (file_stats.st_mode & S_IXUSR)
                                 ? "x"
                                 : "-");
                    wprintw (info_win,
                             (file_stats.st_mode & S_IRGRP)
                                 ? "r"
                                 : "-");
                    wprintw (info_win,
                             (file_stats.st_mode & S_IWGRP)
                                 ? "w"
                                 : "-");
                    wprintw (info_win,
                             (file_stats.st_mode & S_IXGRP)
                                 ? "x"
                                 : "-");
                    wprintw (info_win,
                             (file_stats.st_mode & S_IROTH)
                                 ? "r"
                                 : "-");
                    wprintw (info_win,
                             (file_stats.st_mode & S_IWOTH)
                                 ? "w"
                                 : "-");
                    wprintw (info_win,
                             (file_stats.st_mode & S_IXOTH)
                                 ? "x"
                                 : "-");
                    wprintw (info_win, "\n");
                    // Выводим дополнительную информацию
                    wprintw (
                        info_win, " Device ID: [%lx,%lx]\n",
                        (long)major (file_stats.st_dev),
                        (long)minor (file_stats.st_dev));
                    wprintw (info_win,
                             " Number of Links: %ld\n",
                             (long)file_stats.st_nlink);
                    if (!S_ISDIR (file_stats.st_mode))
                        {
                            long long size
                                = file_stats.st_size;
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
                            else if (size < 1024LL * 1024LL
                                                * 1024LL)
                                {
                                    size /= (1024LL
                                             * 1024LL);
                                    unit = "MB";
                                }
                            else
                                {
                                    size /= (1024LL * 1024LL
                                             * 1024LL);
                                    unit = "GB";
                                }

                            wprintw (info_win,
                                     " Size: %lld %s\n",
                                     size, unit);
                        }
                    wprintw (info_win,
                             " Last Status Change:\n %s",
                             ctime (&file_stats.st_ctime));
                    wprintw (info_win, " Last Access:\n %s",
                             ctime (&file_stats.st_atime));
                    wprintw (info_win,
                             " Last Modification:\n %s",
                             ctime (&file_stats.st_mtime));
                }
            else
                {
                    // Если возникла ошибка при получении
                    // информации о файле
                    perror ("Error getting file status");
                }
        }
    else
        {
            if (strcmp (current_directory_->cwd, "/") == 0)
                {
                    wprintw (info_win, " Root directory\n");
                    wprintw (info_win,
                             "\n Possible commands:\n");
                    wprintw (info_win,
                             " r: Rename a file\n");
                    wprintw (info_win, " c: Copy files\n");
                    wprintw (info_win,
                             " d: Delete files\n");
                    wprintw (info_win, " m: Move files\n");
                    wprintw (info_win,
                             " a: Calculate CRC (Cyclic "
                             "Redundancy Check)\n");
                    wprintw (info_win, " s: Sort files\n");
                    wprintw (info_win,
                             " f: Find files by regular "
                             "expression\n");
                    wprintw (info_win,
                             " r: Rename files\n");
                    wprintw (info_win,
                             " h: Show all commands\n");
                }
            else
                {
                    wprintw (
                        info_win, "Return to %s\n",
                        current_directory_->parent_dir);
                }
        }

    // Обновляем окно info_win, чтобы отобразить все
    // изменения
    wrefresh (info_win);
}

/**
 * @brief Выполняет операцию XOR между checked_value и
 * gen_poly.
 *
 * Эта функция выполняет операцию XOR между элементами
 * массивов `checked_value` и `gen_poly` для вычисления CRC
 * (циклического избыточного кода). Каждый элемент
 * `checked_value[j]` сравнивается с `gen_poly[j]`, и если
 * они равны, результатом является символ '0', в противном
 * случае — символ '1'.
 *
 * @note Функция предполагает, что массивы `checked_value` и
 * `gen_poly` корректно инициализированы перед вызовом
 * данной функции. Эта функция используется в процессе
 * вычисления CRC.
 */
void
XOR ()
{
    for (int j = 1; j < N; j++)
        {
            checked_value[j]
                = ((checked_value[j] == gen_poly[j]) ? '0'
                                                     : '1');
        }
}

/**
 * @brief Функция для вычисления CRC и проверки суммы.
 *
 * Эта функция выполняет вычисление CRC (циклического
 * избыточного кода) для файла, используя заданный
 * генерирующий полином, и сравнивает результат с
 * предоставленной суммой для проверки. Результат и статус
 * скачивания выводятся в окно информации.
 *
 * @param files Массив строк с именами файлов для обработки.
 */
void
crc (char *files[])
{
    char selected_path[PATH_MAX];
    // Формирование пути к выбранному файлу
    snprintf (selected_path, PATH_MAX, "%s/%s",
              current_directory_->cwd, files[selection]);
    // Открытие файла для чтения
    FILE *file = fopen (selected_path, "r");
    if (file == NULL)
        {
            // Вывод сообщения об ошибке в окно информации
            wclear (info_win);
            box (info_win, '|', '-');
            wmove (info_win, 1, 1);
            wrefresh (info_win);
            wprintw (info_win, "Error opening file.\n");
            wprintw (info_win,
                     "Press Enter to return...\n");
            box (info_win, '|', '-');
            wrefresh (info_win);
            // Ожидание нажатия клавиши Enter или Return для
            // возврата
            char ch;
            while ((ch = getch ()) != KEY_ENTER
                   && ch != '\n')
                ;
            return;
        }
    int i = 0, c;
    // Ввод генерирующего полинома
    wclear (info_win);
    wmove (info_win, 1, 1);
    wrefresh (path_win);
    wprintw (info_win,
             "Enter the Generating polynomial below: ");
    wprintw (info_win, "Enter the Generating polynomial: ");
    wprintw (info_win,
             "\n\n Main types of CRC and their generating "
             "polynomials (in binary):\n");
    wprintw (info_win, "CRC-8   (0x%X): 100000111\n", 0x07);
    wprintw (info_win,
             " CRC-16-CCITT (0x%X): 11000000000000101\n",
             0x1021);
    wprintw (info_win,
             " CRC-16/ARC (0x%X): 11000000000000101\n",
             0x1021);
    wprintw (info_win,
             " CRC-16/DECT-R (0x%X): 10000011000001001\n",
             0x0589);
    wprintw (info_win,
             " CRC-16/TELEDISK (0x%X): 11000000000000101\n",
             0xA097);
    wprintw (info_win,
             " CRC-16/XMODEM (0x%X): 11000000000000101\n",
             0x1021);
    wprintw (info_win,
             " CRC-32 (0x%X): "
             "100000100110000010001110110110111\n",
             0x04C11DB7);
    wprintw (info_win,
             " CRC-32C (0x%X): "
             "100000100110000010001110110110111\n",
             0x1EDC6F41);

    wclear (path_win);
    wmove (path_win, 1, 0);
    wrefresh (info_win);
    while ((c = wgetch (path_win)) != '\n')
        {
            if (c == 127 || c == 8)
                {
                    gen_poly[--i] = '\0';
                }
            else
                {
                    gen_poly[i++] = c;
                    gen_poly[i] = '\0';
                }
            wclear (path_win);
            wmove (path_win, 1, 0);
            wprintw (path_win, "%s", gen_poly);
        }
    wclear (info_win);
    wmove (info_win, 1, 1);
    wprintw (info_win,
             "Enter the summary before downloading:");
    wrefresh (info_win);

    // Ввод суммы для проверки
    i = 0; // Сброс счетчика для нового ввода
    wclear (path_win);
    wrefresh (path_win);

    while ((c = wgetch (path_win)) != '\n')
        {
            if (c == 127 || c == 8)
                {
                    check_value[--i] = '\0';
                }
            else
                {
                    check_value[i++] = c;
                    check_value[i] = '\0';
                }
            wclear (path_win);
            wmove (path_win, 1, 0);
            wprintw (path_win, "%s", check_value);
            wrefresh (path_win);
        }

    int j;
    char ch;

    // Инициализация проверяемых данных N битами данных из
    // файла
    for (i = 0; i < N; i++)
        {
            if ((ch = fgetc (file)) == EOF)
                {
                    break;
                }
            checked_value[i] = ch;
        }
    // Вычисление CRC и проверка суммы
    do
        {
            if (checked_value[0] == '1')
                XOR ();

            for (j = 0; j < N - 1; j++)
                checked_value[j] = checked_value[j + 1];

            if ((ch = fgetc (file)) != EOF)
                {
                    checked_value[j] = ch;
                    i++;
                }
        }
    while (ch != EOF);

    fclose (file);
    wclear (info_win);
    wmove (info_win, 1, 1);
    // Вывод результата CRC и проверка скачивания
    wprintw (info_win, "CRC or Check value is: %s\n",
             checked_value);
    if (strcmp (check_value, checked_value) == 0)
        wprintw (info_win,
                 "  Data downloaded succesfully\n");
    else
        wprintw (info_win,
                 "  Data downloaded with mistakes\n");
    wprintw (info_win, "  Press Enter to return...\n");
    wrefresh (info_win);
    while ((ch = getch ()) != KEY_ENTER && ch != '\n')
        {
        }
}

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
void
find (char *files[])
{
    // Получаем путь к корневой директории
    char root_dir[MAX_PATH_LENGTH];

    if (current_directory_
            ->cwd[strlen (current_directory_->cwd) - 1]
        == '/')
        {
            sprintf (
                root_dir, "%.*s",
                (int)(strlen (current_directory_->cwd) - 1),
                current_directory_->cwd);
        }
    else
        {
            strcpy (root_dir, current_directory_->cwd);
        }
    char regex_str[MAX_PATH_LENGTH];
    int i = 0, c;
    int ch;
    wclear (current_win);
    wclear (info_win);

    wresize (current_win, maxy, maxx);

    wmove (info_win, 1, 1);
    wprintw (info_win, "Enter the regex for searching: ");

    // Основные правила для создания регулярного выражения
    wprintw (info_win,
             "\n\n Main rules for composing a regex:\n");
    wprintw (info_win,
             "  - Use '.' to match any single character\n");
    wprintw (info_win, "  - Use '*' to match zero or more "
                       "occurrences of the "
                       "previous character\n");
    wprintw (
        info_win,
        "  - Use '|' for alternation (OR operation)\n");
    wprintw (info_win, "  - Use '()' for grouping\n");
    wprintw (
        info_win,
        "  - Use '^' to match the start of a string\n");
    wprintw (info_win,
             "  - Use '$' to match the end of a string\n");
    wprintw (info_win, "  - Use '\\\\' to escape special "
                       "characters if needed\n");
    wprintw (info_win,
             "  - Use '[]' for character classes\n");
    wprintw (info_win,
             "  - Use '{}' for specifying repetition\n");
    wprintw (info_win, "  - Use '\\' to escape special "
                       "characters, if needed\n");

    wrefresh (info_win);

    char *regexp = (char *)malloc (256 * sizeof (char));
    if (regexp == NULL)
        {
            perror ("Memory allocation failed");
            return;
        }

    while ((c = wgetch (info_win)) != '\n')
        {
            if (c == 127 || c == 8)
                {
                    if (i > 0)
                        {
                            regexp[--i] = '\0';
                        }
                }
            else
                {
                    regexp[i++] = c;
                    regexp[i] = '\0';
                }
            wclear (info_win);
            wmove (info_win, 1, 1);
            wprintw (info_win,
                     "Enter the regex for searching: %s",
                     regexp);
            wrefresh (info_win);
        }
    int num_matches;
    // Ищем файлы с использованием регулярного выражения
    char **file_paths = find_files_by_regex (
        root_dir, regexp, &num_matches);
    if (file_paths == NULL || num_matches == 0)
        {
            wclear (current_win);
            box (current_win, '|', '-');
            wmove (current_win, 1, 1);
            wprintw (current_win,
                     "No matching files found");
            wrefresh (current_win);
            free (regexp);
            wgetch (current_win);
            return;
        }

    wclear (info_win);
    wmove (info_win, 1, 1);
    wprintw (
        info_win,
        "Found %d files matching the regular expression "
        "'%s'\n",
        num_matches, regexp);
    wmove (info_win, 2, 2);
    wprintw (info_win, "Press Enter to return to parent "
                       "directory or 'q' to exit...\n");
    wrefresh (info_win);

    // Вывод всех файлов совпадающих с регулярным выражением
    int selection = 0;
    do
        {
            wclear (current_win);
            box (current_win, '|', '-');
            wmove (current_win, 1, 1);
            for (int i = 0; i < num_matches; ++i)
                {
                    wmove (current_win, i + 2, 2);
                    if (i == selection)
                        {
                            wattron (current_win,
                                     A_STANDOUT);
                        }
                    wprintw (current_win, "%s\n",
                             file_paths[i]);
                    if (i == selection)
                        {
                            wattroff (current_win,
                                      A_STANDOUT);
                        }
                }
            wrefresh (current_win);
            ch = wgetch (current_win);
            switch (ch)
                {
                case KEY_UP:
                    if (selection > 0)
                        {
                            selection--;
                        }
                    break;
                case KEY_DOWN:
                    if (selection < num_matches - 1)
                        {
                            selection++;
                        }
                    break;
                }
        }
    while (ch != KEY_ENTER && ch != '\n' && ch != 'q');
    //Обработка перехода к родительскому каталогу выбранного
    //файла
    if (ch == KEY_ENTER || ch == '\n')
        {
            char *parent_dir;
            char *file_path = file_paths[selection];
            // Получаем копию пути к файлу
            char *file_path_copy = strdup (file_path);
            if (file_path_copy == NULL)
                {
                    perror ("Error copying file path");
                    return;
                }
            // Извлекаем путь к родительскому каталогу
            char *directory = dirname (file_path_copy);
            // Выделяем память для parent_dir
            parent_dir = (char *)malloc (
                strlen (directory)
                + 2); // +2 для строки и символа '/'!!!
            if (parent_dir == NULL)
                {
                    perror ("Error allocating memory for "
                            "parent "
                            "directory path");
                    free (file_path_copy);
                    return;
                }
            // Копируем путь к родительскому каталогу в
            // parent_dir
            strcpy (parent_dir, directory);
            strcat (
                parent_dir,
                "/"); // Добавляем символ '/' в конец строки

            strcpy (current_directory_->cwd, parent_dir);

            // Освобождаем скопированный путь к файлу и
            // выделенную память для parent_dir
            free (file_path_copy);
            free (parent_dir);
        }

    free (regexp);
    for (int i = 0; i < num_matches; ++i)
        {
            free (file_paths[i]);
        }
    free (file_paths);
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
            fprintf (stderr,
                     "Не удалось открыть директорию '%s'\n",
                     dir_path);
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
                    fprintf (
                        stderr,
                        "Ошибка при получении информации о "
                        "файле '%s'\n",
                        full_path);
                    continue;
                }

            // Проверяем, является ли элемент директорией
            if (S_ISDIR (statbuf.st_mode))
                {
                    // Пропускаем специальные директории "."
                    // и ".."
                    if (strcmp (entry->d_name, ".") == 0
                        || strcmp (entry->d_name, "..")
                               == 0)
                        continue;

                    // Рекурсивно ищем файлы в поддиректории
                    find_files_recursive (
                        full_path, regex_str, file_paths,
                        num_matches, regex);
                }
            else if (S_ISREG (statbuf.st_mode))
                {
                    // Проверяем, соответствует ли имя файла
                    // регулярному выражению
                    if (regexec (regex, entry->d_name, 0,
                                 NULL, 0)
                        == 0)
                        {
                            // Добавляем путь к файлу в
                            // массив
                            char *file_path
                                = (char *)malloc (
                                    MAX_PATH_LENGTH
                                    * sizeof (char));
                            strncpy (file_path, full_path,
                                     MAX_PATH_LENGTH);
                            (*file_paths)[*num_matches]
                                = file_path;
                            (*num_matches)++;

                            // Проверяем, не превысили ли
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
    regex_t regex;
    if (regcomp (&regex, regex_str,
                 REG_EXTENDED | REG_NOSUB)
        != 0)
        {
            return NULL;
        }

    // Выделяем память под массив строк для путей к файлам
    char **file_paths
        = (char **)malloc (MAX_MATCHES * sizeof (char *));
    if (file_paths == NULL)
        {
            fprintf (stderr, "Memory failed\n");
            exit (EXIT_FAILURE);
        }

    // Инициализируем счетчик совпадений
    *num_matches = 0;

    // Рекурсивно ищем файлы в корневой директории
    find_files_recursive (root_dir, regex_str, &file_paths,
                          num_matches, &regex);

    // Освобождаем ресурсы, связанные с регулярным
    // выражением
    regfree (&regex);

    return file_paths;
}
