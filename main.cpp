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
#include <termios.h>
#include <unistd.h>
#include <util.h>

#include "config.h"

#define isDir(mode) (S_ISDIR(mode))

struct stat file_stats;
WINDOW *current_win, *info_win, *path_win;
int selection, maxx, maxy, len = 0, start = 0;
size_t total_files = 0;
directory_t* current_directory_ = NULL;

void init_curses();
void init();
void init_windows();
void refreshWindows();
int get_number_of_files_in_directory(char* directory);
int get_files(char* directory, char* target[]);
void scroll_up();
void scroll_down();
void sort(char* files_[], int n);
int check_text(char* path);
void read_(char* path);
void readInNano(char* path);
void rename_file(char* files[]);
void delete_(char* files[]);
void delete_file(char* files[]);
void copy_files(char* files[]);
void move_file(char* files[]);
void handle_enter(char* files[]);
float get_recursive_size_directory(char* path);
void show_file_info(char* files[]);
char* get_parent_directory(char* cwd);

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
        len = get_number_of_files_in_directory(current_directory_->cwd);
        if (len < 0) {
            perror("Error getting file count in directory");
            break;
        }

        len = len <= 0 ? 1 : len;
        char* files[len];
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
            if (t == maxy - 1)
                break;
            int size =
                snprintf(NULL, 0, "%s%s", current_directory_->cwd, files[i]);
            if (i == selection) {
                wattron(current_win, A_STANDOUT);
            } else {
                wattroff(current_win, A_STANDOUT);
            }

            char* temp_dir = (char*)malloc(size + 1);
            snprintf(temp_dir, size + 1, "%s%s", current_directory_->cwd,
                     files[i]);

            stat(temp_dir, &file_stats);
            isDir(file_stats.st_mode) ? wattron(current_win, COLOR_PAIR(1))
                                      : wattroff(current_win, COLOR_PAIR(2));
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

/**
 * Инициализирует библиотеку ncurses для работы с графическим пользовательским
 * интерфейсом в терминале.
 */
void init_curses() {
    initscr();  // Инициализация экрана ncurses
    noecho();  // Отключение отображения вводимых символов
    curs_set(0);    // Скрытие курсора
    start_color();  // Включение поддержки цветов
    init_pair(1, DIR_COLOR, 0);  // Инициализация цвета для каталогов
    init_pair(3, STATUS_SELECTED_COLOR,
              0);  // Инициализация цвета для выделенного элемента статуса
}

/**
 * Выделяет память и инициализирует структуру `current_directory_` для хранения
 * информации о текущем каталоге. Если выделение памяти не удалось, функция
 * выводит сообщение об ошибке и завершает программу.
 */
void init() {
    current_directory_ = (directory_t*)malloc(
        sizeof(directory_t));  // Выделение памяти под структуру directory_t
    if (current_directory_ == NULL) {
        printf("Error Occurred. Memory allocation failed.\n");
        exit(1);  // Выход из программы с кодом ошибки
    }
}

/**
 * Инициализирует окна для отображения интерфейса.
 * Создает три окна:
 * - `current_win`: Окно для отображения списка файлов текущего каталога.
 * - `path_win`: Окно для отображения текущего пути.
 * - `info_win`: Окно для отображения информации о выбранном файле или статусе.
 * Устанавливает клавиатурный режим для `current_win`, чтобы обрабатывать
 * специальные клавиши.
 */
void init_windows() {
    // Создание окон с помощью функции newwin()
    current_win = newwin(maxy, maxx / 2, 0,
                         0);  // Окно для списка файлов (левая часть экрана)
    path_win =
        newwin(2, maxx, maxy,
               0);  // Окно для отображения текущего пути (нижняя часть экрана)
    info_win = newwin(maxy, maxx / 2, 0,
                      maxx / 2);  // Окно для отображения информации о выбранном
                                  // файле (правая часть экрана)

    // Обновление экрана с помощью функции refresh()
    refresh();  // Обновление стандартного экрана (stdscr)

    // Установка режима обработки клавиш для окна current_win
    keypad(current_win, TRUE);  // Разрешает обработку специальных клавиш
                                // (например, стрелок) в current_win
}

/**
 * Обновляет содержимое всех окон интерфейса и отображает рамки вокруг некоторых
 * окон. Рамки вокруг окон создаются с использованием функции box(). После этого
 * происходит обновление каждого окна с помощью функции wrefresh().
 */
void refreshWindows() {
    // Рисует рамку вокруг окна current_win с символами '|' (вертикальная линия)
    // и '-' (горизонтальная линия)
    box(current_win, '|', '-');
    // Рисует рамку вокруг окна info_win с символами '|' и '-'
    box(info_win, '|', '-');

    // Обновляет содержимое окон с помощью функции wrefresh()
    wrefresh(current_win);  // Обновляет окно current_win
    wrefresh(path_win);     // Обновляет окно path_win
    wrefresh(info_win);     // Обновляет окно info_win
}

/**
 * Подсчитывает количество файлов в указанном каталоге (не включая ссылки "." и
 * "..").
 * @param directory Путь к каталогу, для которого нужно подсчитать файлы.
 * @return Возвращает количество файлов в каталоге или -1 в случае ошибки.
 */
int get_number_of_files_in_directory(char* directory) {
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

/**
 * Получает имена всех файлов в указанном каталоге (исключая ссылки "." и "..")
 * и сохраняет их в массив строк. Каждое имя файла копируется в элемент массива
 * `target`.
 * @param directory Путь к каталогу, из которого нужно получить имена файлов.
 * @param target Массив строк, в который будут сохранены имена файлов.
 * @return Возвращает 1 в случае успешного получения имен файлов или -1 в случае
 * ошибки.
 */
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

/**
 * Прокручивает список файлов вверх (уменьшает индекс выбора `selection`).
 * Если `selection` становится меньше 0, он остается равным 0 (не позволяет
 * выбрать отрицательный индекс). Если количество файлов (`len`) больше или
 * равно высоте окна (`maxy - 1`), то происходит проверка: Если текущий
 * `selection` находится в верхней половине отображаемой области (`start + maxy
 * / 2`), то происходит смещение начальной позиции (`start`) вверх. Если `start`
 * уже равен 0, то окно `current_win` очищается (`wclear`), чтобы убрать старое
 * содержимое. В противном случае `start` уменьшается на 1, и окно `current_win`
 * очищается для обновления содержимого.
 */
void scroll_up() {
    selection--;  // Уменьшаем индекс выбора на один
    selection = (selection < 0)
                    ? 0
                    : selection;  // Если индекс выбора стал отрицательным,
                                  // устанавливаем его в 0

    // Проверяем, достаточно ли много файлов для прокрутки
    if (len >= maxy - 1) {
        // Проверяем, если текущий выбор находится в верхней половине
        // отображаемой области
        if (selection <= start + maxy / 2) {
            // Если начальная позиция (start) уже равна 0, очищаем окно
            // current_win
            if (start == 0) {
                wclear(current_win);  // Очищаем окно current_win (удаляем
                                      // старое содержимое)
            } else {
                start--;  // Уменьшаем начальную позицию (сдвигаем список вверх)
                wclear(current_win);  // Очищаем окно current_win для обновления
                                      // содержимого
            }
        }
    }
}

/**
 * Прокручивает список файлов вниз (увеличивает индекс выбора `selection`).
 * Если `selection` становится больше или равно количеству файлов (`len - 1`),
 * он остается равным `len - 1` (не позволяет выбрать индекс за пределами списка
 * файлов). Если количество файлов (`len`) больше или равно высоте окна (`maxy -
 * 1`), то происходит проверка: Если текущий `selection` находится в нижней
 * половине отображаемой области (`selection - 1 > maxy / 2`), и начальная
 * позиция (`start + maxy - 2`) не достигла конца списка (`len`), то происходит
 * смещение начальной позиции (`start`) вниз. Если начальная позиция (`start`)
 * достигла конца списка, окно `current_win` очищается для обновления
 * содержимого.
 */
void scroll_down() {
    selection++;  // Увеличиваем индекс выбора на один
    selection = (selection > len - 1)
                    ? len - 1
                    : selection;  // Проверяем, не превысил ли индекс выбора
                                  // количество файлов - 1

    // Проверяем, достаточно ли много файлов для прокрутки
    if (len >= maxy - 1) {
        // Проверяем, если текущий выбор находится в нижней половине
        // отображаемой области
        if (selection - 1 > maxy / 2) {
            // Проверяем, достигла ли начальная позиция конца списка
            if (start + maxy - 2 != len) {
                start++;  // Увеличиваем начальную позицию (сдвигаем список
                          // вниз)
                wclear(current_win);  // Очищаем окно current_win для обновления
                                      // содержимого
            }
        }
    }
}

/**
 * Сортирует массив имен файлов в алфавитном порядке с использованием сортировки
 * пузырьком (bubble sort).
 * @param files_ Массив указателей на строки (имена файлов), который нужно
 * отсортировать.
 * @param n Количество элементов в массиве `files_`.
 */
void sort(char* files_[], int n) {
    char temp[1000];  // Временный буфер для обмена значениями

    // Перебор всех элементов массива для сортировки
    for (int i = 0; i < n - 1; i++) {
        for (int j = i + 1; j < n; j++) {
            // Сравниваем имена файлов и меняем их местами, если необходимо
            if (strcmp(files_[i], files_[j]) > 0) {
                strcpy(
                    temp,
                    files_[i]);  // Копируем текущий элемент во временный буфер
                strcpy(files_[i],
                       files_[j]);  // Заменяем текущий элемент на следующий
                strcpy(files_[j], temp);  // Заменяем следующий элемент на
                                          // текущий из временного буфера
            }
        }
    }
}

/**
 * Проверяет текстовый файл на наличие непечатаемых символов.
 * @param path Путь к файлу, который нужно проверить.
 * @return Возвращает 1, если файл содержит только печатаемые символы ASCII (от
 * 0x20 до 0x7E), иначе возвращает 0.
 */
int check_text(char* path) {
    FILE* ptr;
    ptr = fopen(path, "r");  // Открываем файл для чтения
    if (ptr == NULL) {
        perror("Error opening file");  // Выводим сообщение об ошибке, если не
                                       // удалось открыть файл
        return 0;  // Возвращаем 0, чтобы указать на ошибку
    }

    int c;
    while (
        (c = fgetc(ptr)) !=
        EOF) {  // Читаем файл посимвольно, пока не достигнем конца файла (EOF)
        if (c < 0 || c > 127) {  // Проверяем, является ли символ непечатаемым
                                 // (вне диапазона ASCII)
            fclose(ptr);         // Закрываем файл
            return 0;  // Возвращаем 0, чтобы указать на наличие непечатаемого
                       // символа
        }
    }

    fclose(ptr);  // Закрываем файл после окончания чтения
    return 1;  // Возвращаем 1, если файл содержит только печатаемые символы
}

/**
 * Читает содержимое файла и выводит его на экран в окне ncurses.
 * @param path Путь к файлу, который нужно прочитать.
 */
void read_(char* path) {
    char buffer[256];  // Буфер для чтения данных из файла
    int ch;  // Переменная для хранения нажатой клавиши
    wclear(current_win);  // Очищаем текущее окно ncurses
    wclear(info_win);  // Очищаем информационное окно ncurses
    wresize(current_win, maxy,
            maxx);  // Изменяем размер текущего окна до максимального

    FILE* ptr;
    ptr = fopen(path, "rb");  // Открываем файл для чтения в бинарном режиме
    if (ptr == NULL) {
        perror("Error");  // Выводим сообщение об ошибке, если не удалось
                          // открыть файл
        return;
    }

    int t = 2, pos = 0, count;
    int is_text = check_text(path);  // Проверяем, является ли файл текстовым

    do {
        wmove(current_win, 1, 2);
        wprintw(current_win, "Press \"q\" to Exit");

        if (is_text) {
            count = 0;
            while (
                fgets(buffer, sizeof(buffer), ptr)) {  // Читаем файл построчно
                if (count < pos) {
                    count++;
                    continue;
                }
                wmove(current_win, ++t,
                      1);  // Перемещаем курсор в следующую строку
                wprintw(current_win, "%.*s", maxx - 2,
                        buffer);  // Выводим содержимое строки
            }
        } else {
            fseek(ptr, pos * (maxx - 2),
                  SEEK_SET);  // Перемещаем указатель в нужное место файла
            size_t bytesRead = fread(buffer, sizeof(unsigned char), maxx - 2,
                                  ptr);  // Читаем данные из файла
            if (bytesRead > 0) {
                wmove(current_win, ++t,
                      1);  // Перемещаем курсор в следующую строку
                wprintw(current_win, "%.*s", bytesRead,
                        buffer);  // Выводим прочитанные данные
            }
        }

        box(current_win, '|', '-');  // Отображаем рамку вокруг текущего окна
        wrefresh(current_win);  // Обновляем текущее окно ncurses
        ch = wgetch(current_win);  // Получаем нажатую клавишу
        wclear(current_win);  // Очищаем текущее окно перед обновлением

        switch (ch) {
            case KEY_UP:
                pos = pos == 0 ? 0 : pos - 1;  // Перемещаемся вверх по файлу
                                               // (уменьшаем позицию)
                break;
            case KEY_DOWN:
                pos++;  // Перемещаемся вниз по файлу (увеличиваем позицию)
                break;
        }
    } while (ch != 'q');  // Выполняем цикл, пока не нажата клавиша "e" или "E"

    fclose(ptr);  // Закрываем файл после окончания чтения
    endwin();  // Завершаем работу с библиотекой ncurses
}

/**
 * Открывает указанный файл в текстовом редакторе nano для редактирования.
 * @param path Путь к файлу, который нужно открыть.
 */
void readInNano(char* path) {
    struct termios original_termios;
    tcgetattr(STDIN_FILENO,
              &original_termios);  // Получаем текущие настройки терминала

    pid_t pid = fork();  // Создаем новый процесс с помощью fork()

    if (pid == 0) {  // Дочерний процесс
        execlp("nano", "nano", path,
               NULL);  // Запускаем программу nano для открытия файла
        perror("exec");  // В случае ошибки выполнения exec выводим сообщение об
                         // ошибке
        exit(EXIT_FAILURE);  // Завершаем дочерний процесс с ошибкой
    } else if (pid < 0) {  // Ошибка при вызове fork()
        perror("fork");  // Выводим сообщение об ошибке
    } else {             // Родительский процесс
        int status;
        pid_t wpid =
            waitpid(pid, &status, 0);  // Ждем завершения дочернего процесса

        if (wpid == -1) {  // Ошибка при ожидании дочернего процесса
            perror("waitpid");  // Выводим сообщение об ошибке
        } else {
            if (WIFEXITED(status)) {  // Проверяем, завершился ли дочерний
                                      // процесс нормально
                tcsetattr(
                    STDIN_FILENO, TCSANOW,
                    &original_termios);  // Восстанавливаем настройки терминала
            } else if (WIFSIGNALED(status)) {  // Проверяем, завершился ли
                                               // дочерний процесс из-за сигнала
                printf("nano editor terminated by signal: %d\n",
                       WTERMSIG(status));  // Выводим информацию о сигнале
            }
        }
    }
}

/**
 * Переименовывает выбранный файл из списка `files`.
 * Функция запрашивает новое имя файла у пользователя через ncurses.
 * После ввода нового имени, пользователю предлагается подтвердить
 * переименование.
 * @param files Массив строк с именами файлов.
 */
void rename_file(char* files[]) {
    char new_name[100];
    int i = 0, c;

    // Очистка окна path_win и установка курсора для ввода нового имени файла
    wclear(path_win);
    wmove(path_win, 1, 0);

    // Чтение нового имени файла с клавиатуры
    while ((c = wgetch(path_win)) != '\n') {
        if (c == 127 || c == 8) {
            // Обработка удаления символа (Backspace)
            new_name[--i] = '\0';
        } else {
            // Добавление введенного символа к новому имени файла
            new_name[i++] = c;
            new_name[i] = '\0';
        }
        wclear(path_win);
        wmove(path_win, 1, 0);
        wprintw(path_win, "%s", new_name);
    }

    c = ' ';

LOOP:
    // Очистка окна path_win и вывод нового имени файла с запросом подтверждения
    wclear(path_win);
    wmove(path_win, 1, 0);
    wprintw(path_win, "%s (y/n) %c", new_name, c);
    c = wgetch(path_win);

    char *a, *b;
    switch (c) {
        case 'y':
        case 'Y':
            // Формирование полного пути и переименование файла
            a = strdup(current_directory_->cwd);
            b = strdup(current_directory_->cwd);
            strcat(a, files[selection]);
            strcat(b, new_name);
            rename(a, b);  // Переименование файла
            free(a);
            free(b);
            break;
        case 'n':
        case 'N':
            // Отмена операции переименования
            break;
        default:
            // Обработка некорректного ввода (повторный ввод подтверждения)
            goto LOOP;
            break;
    }
}

/**
 * Возвращает родительский каталог для заданного пути.
 * @param cwd Строка с текущим рабочим каталогом (полный путь).
 * @return Строка с родительским каталогом заданного пути.
 *         Память для возвращаемой строки выделяется динамически и должна быть
 * освобождена вызывающей стороной.
 */
char* get_parent_directory(char* cwd) {
    char* a;
    a = strdup(cwd);  // Создание копии текущего рабочего каталога
    int i = (int)(strlen(a) - 1);

    // Поиск последнего символа '/' в пути
    while (a[--i] != '/')
        ;

    a[++i] = '\0';  // Установка завершающего нуля для формирования строки
                    // родительского каталога
    return a;
}

/**
 * Удаляет файл из текущего каталога.
 * @param files Массив строк, содержащий имена файлов.
 *              files[selection] представляет выбранный файл для удаления.
 */
void delete_(char* files[]) {
    char curr_path[1000];

    // Формируем полный путь к файлу для удаления
    snprintf(curr_path, sizeof(curr_path), "%s%s", current_directory_->cwd,
             files[selection]);

    // Удаляем файл с указанным путем
    remove(curr_path);
}

/**
 * Запрашивает подтверждение перед удалением файла.
 * @param files Массив строк, содержащий имена файлов.
 *              files[selection] представляет выбранный файл для удаления.
 */
void delete_file(char* files[]) {
    int c;

    // Очищаем окно path_win и выводим запрос на подтверждение удаления
    wclear(path_win);
    wmove(path_win, 1, 0);
    wprintw(path_win, "Are you sure to delete? (y/n)");

LOOP_:
    // Получаем ответ пользователя
    c = wgetch(path_win);

    // Очищаем окно path_win и выводим запрос с ответом пользователя
    wclear(path_win);
    wmove(path_win, 1, 0);
    wprintw(path_win, "Are you sure to delete? (y/n) %c", c);

    switch (c) {
        case 'y':
        case 'Y':
            delete_(files);  // Вызываем функцию delete_ для удаления файла
            break;
        case 'n':
        case 'N':
            break;  // Ничего не делаем, пользователь отказался от удаления
        default:
            goto LOOP_;  // Повторяем цикл, если введенный символ некорректен
            break;
    }
}

/**
 * Копирует выбранный файл в новый путь, запрашиваемый у пользователя.
 * @param files Массив строк, содержащий имена файлов.
 *              files[selection] представляет выбранный файл для копирования.
 */
void copy_files(char* files[]) {
    char new_path[1000];
    int i = 0, c;

    // Очищаем окно path_win и запрашиваем новый путь у пользователя
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

    // Формируем полный путь до нового каталога/файла
    char target_path[1000];
    snprintf(target_path, sizeof(target_path), "%s/%s", new_path, files[selection]);

    // Открываем старый файл для чтения и новый файл для записи
    FILE *old_file, *new_file;
    char curr_path[1000];
    snprintf(curr_path, sizeof(curr_path), "%s/%s", current_directory_->cwd, files[selection]);

    old_file = fopen(curr_path, "rb");
    if (old_file == NULL) {
        perror("Error opening source file");
        return;
    }

    new_file = fopen(target_path, "wb");
    if (new_file == NULL) {
        perror("Error opening destination file");
        fclose(old_file);
        return;
    }

    // Копируем содержимое старого файла в новый файл
    int ch;
    while ((ch = fgetc(old_file)) != EOF) {
        fputc(ch, new_file);
    }

    // Закрываем файлы
    fclose(old_file);
    fclose(new_file);

    // Очищаем окно current_win и выводим информацию об успешном копировании
    wclear(current_win);
    wmove(current_win, 10, 10);
    wprintw(current_win, "File copied successfully from\n%s\nto\n%s\n", curr_path, target_path);
    wrefresh(current_win);
}

/**
 * Перемещает выбранный файл в новый путь, запрашиваемый у пользователя.
 * Эта операция выполняется путем копирования файла в новый путь и затем
 * удаления оригинала.
 * @param files Массив строк, содержащий имена файлов.
 *              files[selection] представляет выбранный файл для перемещения.
 */
void move_file(char* files[]) {
    // Копируем выбранный файл в новый путь
    copy_files(files);

    // Удаляем оригинал файла
    delete_(files);
}
/**
 * Обрабатывает нажатие клавиши Enter для выбора действия в текущем каталоге.
 * При выборе каталога он переходит в этот каталог, а при выборе файла открывает
 * его для чтения.
 * @param files Массив строк, содержащий имена файлов и подкаталогов текущего
 * каталога. files[selection] представляет выбранный элемент для обработки.
 */
void handle_enter(char* files[]) {
    char selected_path[PATH_MAX];
    char* original_cwd = strdup(current_directory_->cwd);

    // Закрываем ncurses окно перед обработкой нажатия Enter
    endwin();

    // Формируем полный путь к выбранному файлу или каталогу
    snprintf(selected_path, PATH_MAX, "%s/%s", current_directory_->cwd,
             files[selection]);

    struct stat file_stats;

    if (strcmp(files[selection], "..") == 0) {
        // Переход на уровень выше (если выбран "..")
        start = 0;
        selection = 0;
        strcpy(current_directory_->cwd, current_directory_->parent_dir);
        free(current_directory_->parent_dir);
        current_directory_->parent_dir =
            strdup(get_parent_directory(current_directory_->cwd));
    } else {
        // Переход в выбранный каталог или открытие выбранного файла
        strcat(current_directory_->cwd, files[selection]);
        if (stat(selected_path, &file_stats) == -1) {
            perror("Error getting file status");
            free(original_cwd);
            return;
        }

        if (S_ISDIR(file_stats.st_mode)) {
            // Если выбран каталог, переходим в него
            start = 0;
            selection = 0;
            free(current_directory_->parent_dir);
            current_directory_->parent_dir = strdup(original_cwd);
            strcat(current_directory_->cwd, "/");
        } else {
            // Если выбран файл, открываем его для чтения
            read_(selected_path);

            // Добавляем "/" к текущему каталогу, если это необходимо
            size_t original_len = strlen(original_cwd);
            if (original_len > 0 && original_cwd[original_len - 1] != '/') {
                strcat(current_directory_->cwd, "/");
            }
            strcpy(current_directory_->cwd, original_cwd);
        }
    }

    // Обновляем ncurses окно после обработки нажатия Enter
    refresh();
    free(original_cwd);
}

/**
 * Рекурсивно вычисляет размер указанного каталога и всех его подкаталогов в
 * килобайтах (KB).
 * @param path Путь к каталогу, размер которого требуется вычислить.
 * @return Общий размер каталога и его содержимого в KB.
 */
float get_recursive_size_directory(char* path) {
    float directory_size = 0;
    DIR* dir_ = opendir(path);
    if (dir_ == NULL) {
        return 0;
    }

    struct dirent* dir_entry;
    struct stat file_stat;
    char temp_path[1000];

    // Перебираем все элементы в указанном каталоге
    while ((dir_entry = readdir(dir_)) != NULL) {
        // Игнорируем ссылки на текущий и родительский каталоги
        if (strcmp(dir_entry->d_name, ".") != 0 &&
            strcmp(dir_entry->d_name, "..") != 0) {
            // Формируем полный путь к текущему элементу
            snprintf(temp_path, sizeof(temp_path), "%s/%s", path,
                     dir_entry->d_name);

            // Получаем информацию о текущем элементе
            if (lstat(temp_path, &file_stat) == -1) {
                continue;  // Пропускаем элемент в случае ошибки
            }

            // Если текущий элемент является каталогом, рекурсивно вычисляем его
            // размер
            if (S_ISDIR(file_stat.st_mode)) {
                directory_size += get_recursive_size_directory(temp_path);
            } else {
                // Если текущий элемент не является каталогом, добавляем его
                // размер к общему размеру
                directory_size += (float)(file_stat.st_size) /
                                  (float)1024;  // Преобразуем размер в KB
            }
        }
    }

    closedir(dir_);
    return directory_size;
}

/**
 * Отображает информацию о выбранном файле или каталоге в окне info_win.
 * @param files Массив имен файлов.
 */
void show_file_info(char* files[]) {
    wmove(info_win, 1, 1);

    // Проверяем, что выбранный элемент не является ссылкой на родительский
    // каталог
    if (strcmp(files[selection], "..") != 0) {
        char temp_address[1000];
        snprintf(temp_address, sizeof(temp_address), "%s%s",
                 current_directory_->cwd, files[selection]);

        struct stat file_stats;
        // Получаем информацию о выбранном элементе с помощью lstat
        if (lstat(temp_address, &file_stats) == 0) {
            wprintw(info_win, "Name: %s\n", files[selection]);
            // Определяем тип элемента (каталог, файл или ссылка)
            if (S_ISLNK(file_stats.st_mode)) {
                wprintw(info_win, " Type: Symbolic Link\n");
            } else if (S_ISDIR(file_stats.st_mode)) {
                wprintw(info_win, " Type: Directory\n");
            } else if (S_ISREG(file_stats.st_mode)) {
                wprintw(info_win, " Type: Regular File\n");
                // Если элемент - файл, выводим его размер в KB
                wprintw(info_win, " Size: %.2f KB\n",
                        (float)file_stats.st_size / (float)1024);
            } else {
                wprintw(info_win, " Type: Unknown\n");
            }
        } else {
            // Если возникла ошибка при получении информации о файле
            perror("Error getting file status");
        }
    } else {
        wprintw(info_win, "Press Enter to go back\n");
    }
}
