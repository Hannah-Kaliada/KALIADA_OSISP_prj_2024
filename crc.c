#include "crc.h"

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
      checked_value[j] = ((checked_value[j] == gen_poly[j]) ? '0' : '1');
    }
}

int
is_binary_string (const char *str)
{
  while (*str)
    {
      if (*str != '0' && *str != '1')
        {
          return 0;
        }
      str++;
    }
  return 1;
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
  memset (checked_value, 0, sizeof (checked_value));
  memset (check_value, 0, sizeof (check_value));
  memset (gen_poly, 0, sizeof (gen_poly));
  char selected_path[PATH_MAX];
  snprintf (selected_path, PATH_MAX, "%s/%s", current_directory_->cwd,
            files[selection]);

  FILE *file = fopen (selected_path, "r");
  if (file == NULL)
    {
      wclear (info_win);
      box (info_win, '|', '-');
      wmove (info_win, 1, 1);
      wrefresh (info_win);
      wprintw (info_win,
               " Error opening file. Insufficient file permissions.\n");
      wprintw (info_win, "  Press Enter to return...\n");
      box (info_win, '|', '-');
      wrefresh (info_win);
      char ch;
      while ((ch = getch ()) != KEY_ENTER && ch != '\n')
        ;
      return;
    }

  int i = 0, c;

  // Ввод генерирующего полинома
  do
    {
      wclear (info_win);
      wmove (info_win, 1, 1);
      wrefresh (path_win);
      wprintw (info_win, " Enter the Generating polynomial below: ");
      wprintw (info_win, " Enter the Generating polynomial: ");
      wprintw (info_win, "\n\n  Main types of CRC and their generating "
                         "polynomials (in binary):\n");
      wprintw (info_win, "  CRC-8   (0x%X): 100000111\n", 0x07);
      wprintw (info_win, "  CRC-16-CCITT (0x%X): 11000000000000101\n", 0x1021);
      wprintw (info_win, "  CRC-16/ARC (0x%X): 11000000000000101\n", 0x1021);
      wprintw (info_win, "  CRC-16/DECT-R (0x%X): 10000011000001001\n", 0x0589);
      wprintw (info_win, "  CRC-16/TELEDISK (0x%X): 11000000000000101\n",
               0xA097);
      wprintw (info_win, "  CRC-16/XMODEM (0x%X): 11000000000000101\n", 0x1021);
      wprintw (info_win, "  CRC-32 (0x%X): 100000100110000010001110110110111\n",
               0x04C11DB7);
      wprintw (info_win,
               "  CRC-32C (0x%X): 100000100110000010001110110110111\n",
               0x1EDC6F41);

      wclear (path_win);
      wmove (path_win, 1, 0);
      box (info_win, '|', '-');
      wrefresh (info_win);
      i = 0;
      while ((c = wgetch (path_win)) != '\n')
        {
          if (c == 127 || c == 8)
            {
              if (i > 0)
                {
                  gen_poly[--i] = '\0';
                }
            }
          else if (c == '0' || c == '1')
            {
              gen_poly[i++] = c;
              gen_poly[i] = '\0';
            }
          wclear (path_win);
          wmove (path_win, 1, 0);
          wprintw (path_win, "%s", gen_poly);
          wrefresh (path_win);
        }

      if (strlen (gen_poly) == 0)
        {
          wclear (info_win);
          wmove (info_win, 1, 1);
          wprintw (info_win, " Generating polynomial cannot be empty.\n");
          wprintw (info_win, "  Press Enter to return...\n");
          box (info_win, '|', '-');
          wrefresh (info_win);
          while ((c = getch ()) != '\n')
            ;
        }
      else if (!is_binary_string (gen_poly))
        {
          wclear (info_win);
          wmove (info_win, 1, 1);
          wprintw (
              info_win,
              " Invalid polynomial entered. Must be binary (0s and 1s).\n");
          wprintw (info_win, "  Press Enter to return...\n");
          box (info_win, '|', '-');
          wrefresh (info_win);
          while ((c = getch ()) != '\n')
            ;
        }
    }
  while (strlen (gen_poly) == 0 || !is_binary_string (gen_poly));

  // Ввод суммы для проверки
  do
    {
      wclear (info_win);
      wmove (info_win, 1, 1);
      wprintw (info_win, " Enter the checksum to verify:");
      box (info_win, '|', '-');
      wrefresh (info_win);

      i = 0;
      wclear (path_win);
      wrefresh (path_win);
      while ((c = wgetch (path_win)) != '\n')
        {
          if (c == 127 || c == 8)
            {
              if (i > 0)
                {
                  check_value[--i] = '\0';
                }
            }
          else if (c == '0' || c == '1')
            {
              check_value[i++] = c;
              check_value[i] = '\0';
            }
          wclear (path_win);
          wmove (path_win, 1, 0);
          wprintw (path_win, "%s", check_value);
          wrefresh (path_win);
        }

      if (strlen (check_value) == 0)
        {
          wclear (info_win);
          wmove (info_win, 1, 1);
          wprintw (info_win, " Checksum cannot be empty.\n");
          wprintw (info_win, "  Press Enter to return...\n");
          box (info_win, '|', '-');
          wrefresh (info_win);
          while ((c = getch ()) != '\n')
            ;
        }
    }
  while (strlen (check_value) == 0);

  int j;
  char ch;

  // Инициализация проверяемых данных N битами данных из файла
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
        {
          XOR ();
        }

      for (j = 0; j < N - 1; j++)
        {
          checked_value[j] = checked_value[j + 1];
        }

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
  wprintw (info_win, " CRC or Check value is: %s\n", checked_value);
  if (strcmp (check_value, checked_value) == 0)
    wprintw (info_win, "  Data downloaded successfully\n");
  else
    wprintw (info_win, "  Data downloaded with mistakes\n");
  wprintw (info_win, "  Press Enter to return...\n");
  box (info_win, '|', '-');
  wrefresh (info_win);
  while ((ch = getch ()) != KEY_ENTER && ch != '\n')
    ;
}
