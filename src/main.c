#include <termios.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <stdlib.h>
#include <curses.h>
#include <ctype.h>

#include <string.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <dirent.h>

#define BORDER_WINDOW_HEIGHT 21
#define BORDER_WINDOW_WIDTH 40
#define DIR_WINDOW_HEIGHT 19
#define DIR_WINDOW_WIDTH 38
#define MENU_WINDOW_HEIGHT 1
#define MENU_WINDOW_WIDTH 78
#define MENU_BORDER_HEIGHT 3
#define MENU_BORDER_WIDTH 80

static WINDOW *left_border_window,
              *left_dir_window,
              *right_border_window,
              *right_dir_window,
              *active_window,
              *menu_border_window,
              *menu_text_window;

void SigWinch(int signo)
{
  struct winsize size;
  ioctl(stdout, TIOCGWINSZ, (char *)&size);
  resizeterm(size.ws_row, size.ws_col);
  refresh();
}

void CreateWindows(void)
{
  /*  Creating the windows */
  /*  Left part */
  left_border_window = newwin(BORDER_WINDOW_HEIGHT, BORDER_WINDOW_WIDTH,
                              0, 0);
  left_dir_window = derwin(left_border_window, DIR_WINDOW_HEIGHT,
                           DIR_WINDOW_WIDTH, 1, 1);
  box(left_border_window, 0, 0);
  keypad(left_dir_window, 1);
  wrefresh(left_border_window);

  /*  Right part */
  right_border_window = newwin(BORDER_WINDOW_HEIGHT, BORDER_WINDOW_WIDTH,
                               0, BORDER_WINDOW_WIDTH);
  right_dir_window = derwin(right_border_window, DIR_WINDOW_HEIGHT,
                            DIR_WINDOW_WIDTH, 1, 1);
  keypad(right_dir_window, 1);
  box(right_border_window, 0, 0);
  wrefresh(right_border_window);

  /*  Menu part */
  menu_border_window = newwin(MENU_BORDER_HEIGHT, MENU_BORDER_WIDTH,
                              BORDER_WINDOW_HEIGHT, 0);
  menu_text_window = derwin(menu_border_window, MENU_WINDOW_HEIGHT,
                            MENU_WINDOW_WIDTH, 1, 1);
  box(menu_border_window, 0, 0);
  wrefresh(menu_border_window);
}

void InitializeNcurses(void)
{
  initscr();
  signal(SIGWINCH, SigWinch);
  cbreak();
  noecho();
  curs_set(0);
  keypad(stdscr, 1);
  refresh();
  start_color();

  init_pair(1, COLOR_BLACK, COLOR_WHITE);
  init_pair(2, COLOR_WHITE, COLOR_BLACK);
}

void OpenActiveDir(const char *active_path, size_t size)
{
  DIR *dir;

  dir = opendir(active_path);
  if (dir == NULL) {
    /*  Some errors occur */
  } else {
    struct dirent *current_file = NULL;
    int current_y = 0;
    
    while ((current_file = readdir(dir)) != NULL) {
      if (current_file->d_name[0] == '.' &&
          current_file->d_name[1] != '.')
        continue;

      mvwprintw(active_window, current_y++, 0, "%s", current_file->d_name);
    }
    closedir(dir);
  }
}

void Finalize(void)
{
  delwin(right_dir_window);
  delwin(right_border_window);
  delwin(left_dir_window);
  delwin(left_border_window);
  delwin(menu_text_window);
  delwin(menu_border_window);
  endwin();
}

void ColorLine(int y, int color_num)
{
  char line[38];

  mvwinnstr(active_window, y, 0, line, 38);
  wmove(active_window, y, 0);
  wclrtoeol(active_window);
  wattron(active_window, COLOR_PAIR(color_num));
  wprintw(active_window, "%s", line);
  wattroff(active_window, COLOR_PAIR(color_num)); 
  redrawwin(active_window);
}

void PickOutLine(int *y, int delta)
{
  ColorLine(*y, 2);
  (*y) += delta;
  ColorLine(*y, 1);
}

int ChangeCurrentDirectory(int active_line, char *active_path)
{
  char line[38];
  char path[200];
  struct stat file_info;

  mvwinnstr(active_window, active_line, 0, line, 38);
  for (int i = 36; i > -1; --i) {
    if (line[i] != ' ') {
      line[i + 1] = '\0';
      break;
    }
  }
  sprintf(path, "%s/%s", active_path, line);
  if (stat(path, &file_info) == -1) {
    /*  Can't get info about this file */
    return -1;
  }
  if (S_ISDIR(file_info.st_mode)) {
    if (chdir(path) == -1) {
      return -1;
    }
    wclear(active_window);
    getcwd(active_path, 100);
    OpenActiveDir(active_path, 100);
  } else {
    return -1;
  }

  return 0;
}

void ChangeActiveWindow(void)
{
  if (active_window == left_dir_window)
    active_window = right_dir_window;
  else
    active_window = left_dir_window;
}

int main(int argc, char **argv)
{
  char active_path[100];
  int symbol;
  int active_line = 0,
      prev_line = 0;
  
  if (argc > 1) {
    strcpy(active_path, argv[1]);
  } else {
    getcwd(active_path, 100);
  }
  InitializeNcurses();
  CreateWindows();
  active_window = left_dir_window; 
  wmove(active_window, 0, 0);

  OpenActiveDir(active_path, 100);
  wrefresh(left_dir_window);
  active_window = right_dir_window;
  OpenActiveDir(active_path, 100);
  wrefresh(right_dir_window);
  active_window = left_dir_window;
  
  PickOutLine(&active_line, 1);
  while (1) {
    symbol = wgetch(active_window);
    if (symbol == 27)
      break;
    switch (symbol) {
      case KEY_UP:
        if (active_line != 0) {
          PickOutLine(&active_line, -1);
        }
        break;
      case KEY_DOWN:
        if (active_line != DIR_WINDOW_HEIGHT - 1) {
          PickOutLine(&active_line, 1);
        }
        break;
      case KEY_LEFT:
        if (active_window == right_dir_window) {
          ColorLine(active_line, 2);
          ChangeActiveWindow();
          ColorLine(active_line, 1);
          wrefresh(right_dir_window);
        }
        break;
      case KEY_RIGHT:
        if (active_window == left_dir_window) {
          ColorLine(active_line, 2);
          ChangeActiveWindow();
          ColorLine(active_line, 1);
          wrefresh(left_dir_window);
        }
        break;
      case '\n':
        if (ChangeCurrentDirectory(active_line, active_path) == 0) {
          active_line = 0;
          ColorLine(active_line, 1);
        }
        break;
    }
  }
 
  Finalize();
  return 0;
}
