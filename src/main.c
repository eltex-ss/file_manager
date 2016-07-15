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
  curs_set(1);
  keypad(stdscr, 1);
  refresh();
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

void SigWinch(int signo)
{
  struct winsize size;
  ioctl(stdout, TIOCGWINSZ, (char *)&size);
  resizeterm(size.ws_row, size.ws_col);
  refresh();
}

int main(int argc, char **argv)
{
  char active_path[100];
  int symbol;
  
  if (argc > 1) {
    strcpy(active_path, argv[1]);
  }
  InitializeNcurses();
  CreateWindows();
  active_window = left_dir_window; 
  wmove(active_window, 0, 0);
  while (1) {
    symbol = wgetch(active_window);
    if (symbol == 27)
      break;
  }
 
  Finalize();
  return 0;
}
