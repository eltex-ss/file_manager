#include <termios.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <stdlib.h>
#include <curses.h>
#include <ctype.h>

#include <string.h>

#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <dirent.h>

#include "list.h"

/*  Constants. */
#define BORDER_WINDOW_HEIGHT  21
#define BORDER_WINDOW_WIDTH   40
#define DIR_WINDOW_HEIGHT     19
#define DIR_WINDOW_WIDTH      38
#define MENU_WINDOW_HEIGHT    1
#define MENU_WINDOW_WIDTH     78
#define MENU_BORDER_HEIGHT    3
#define MENU_BORDER_WIDTH     80
#define PATH_LENGTH           400
#define HIGHLIGHTED_COLOR     1
#define NON_HIGHLIGHTED_COLOR 2

/*  My structs. */
/*  File manager window. */
struct FMWindow {
  char        active_path[PATH_LENGTH]; /*  Path to active directory. */
  struct List *files;                   /*  List of files. */
  WINDOW      *nwindow;                 /*  Pointer to ncurses window. */
};

/*  Struct represents file manager's active window. */
struct ActiveWindow {
  int left;                /*  If active winow is left, this = 1 else = 0. */
  int current_line;        /*  Highlighted line. */
  int current_file;        /*  File num in directory. */
  int lines_count;         /*  Files count in this directory. */
  struct FMWindow *window; /*  Pointer to left or right FMWindow. */
};

/*  Global variables. */
/*  Ncurses windows. */
static WINDOW *left_border_window,
              *left_dir_window,
              *right_border_window,
              *right_dir_window,
              *menu_border_window,
              *menu_text_window;

/*  My windows. */
static struct FMWindow left_fmwindow,
                       right_fmwindow;
static struct ActiveWindow active_window;
static char *path_to_text_editor = "/home/Alexander/Projects/SummerSchool/"\
                                   "file_manager/bin/text_editor";

/* ========================== */
/*                            */
/*  Interface.                */
/*                            */

/*  Lookup functions. */
void SigWinch(int signo);
void InitializeNcurses(void);
void InitializeApp(void);
void CreateWindows(void);
void Finalize(void);

/*  File manager functional. */
void OpenActiveDir(void);
void ChangeActiveWindow(void);
void OpenFile(void);
void CopyFile(void);

/*  Helpfull functions. */
void PrintFileList(struct FMWindow *fmwindow);
void ColorLine(int color_num);
void PickOutLine(int delta);
void GetPrevState(void);
void ReadCurrentLine(char *file_name);
void FmScroll(int step);
void HandleKeyPress(void);

/* ========================== */
/*                            */
/*  Definitions.              */
/*                            */

/*  Resize window when signal emited. */
void SigWinch(int signo)
{
  struct winsize size;
  ioctl(stdout, TIOCGWINSZ, (char *)&size);
  resizeterm(size.ws_row, size.ws_col);
  refresh();
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
  init_pair(3, COLOR_BLACK, COLOR_BLUE);
  init_pair(4, COLOR_BLACK, COLOR_CYAN);
}

/*  Initialize app structures and print launch app. */
void InitializeApp(void)
{
  active_window.left = 0;
  active_window.current_line = 0;
  active_window.current_file = 0;
  active_window.window = &right_fmwindow;
 
  getcwd(left_fmwindow.active_path, PATH_LENGTH);
  getcwd(right_fmwindow.active_path, PATH_LENGTH);

  OpenActiveDir();
  wrefresh(right_dir_window);

  ChangeActiveWindow();  
  OpenActiveDir();
  wrefresh(left_dir_window);
  ColorLine(HIGHLIGHTED_COLOR);
}

/*  Create ncurses windows and initialize FMWindows. */
void CreateWindows(void)
{
  /*  Creating the windows. */
  /*  Left part. */
  left_border_window = newwin(BORDER_WINDOW_HEIGHT, BORDER_WINDOW_WIDTH,
                              0, 0);
  left_dir_window = derwin(left_border_window, DIR_WINDOW_HEIGHT,
                           DIR_WINDOW_WIDTH, 1, 1);
  left_fmwindow.nwindow = left_dir_window;
  left_fmwindow.files = CreateList();
  box(left_border_window, 0, 0);
  keypad(left_dir_window, 1);
  wrefresh(left_border_window);

  /*  Right part. */
  right_border_window = newwin(BORDER_WINDOW_HEIGHT, BORDER_WINDOW_WIDTH,
                               0, BORDER_WINDOW_WIDTH);
  right_dir_window = derwin(right_border_window, DIR_WINDOW_HEIGHT,
                            DIR_WINDOW_WIDTH, 1, 1);
  right_fmwindow.nwindow = right_dir_window;
  right_fmwindow.files = CreateList();
  keypad(right_dir_window, 1);
  box(right_border_window, 0, 0);
  wrefresh(right_border_window);

  menu_border_window = newwin(MENU_BORDER_HEIGHT, MENU_BORDER_WIDTH,
                              BORDER_WINDOW_HEIGHT, 0);
  menu_text_window = derwin(menu_border_window, MENU_WINDOW_HEIGHT,
                            MENU_WINDOW_WIDTH, 1, 1);
  box(menu_border_window, 0, 0);
  wrefresh(menu_border_window);
  mvwprintw(menu_text_window, 0, 0, "F2)Copy Esc)Exit");
  wrefresh(menu_text_window);
}

void Finalize(void)
{
  RemoveList(left_fmwindow.files);
  RemoveList(right_fmwindow.files);

  delwin(right_dir_window);
  delwin(right_border_window);
  delwin(left_dir_window);
  delwin(left_border_window);
  endwin();
}

/*  Just open the directory and print all files. */
void OpenActiveDir(void)
{
  DIR *dir;

  dir = opendir(active_window.window->active_path);
  if (dir == NULL) {
    /*  Some errors occur. */
  } else {
    struct stat file_info;
    struct dirent **name_list;
    struct List **head;
    char file_path[PATH_LENGTH];
    char dir_prefix[2];
    char file_name[100];
    int current_y = 0;
    int files_count;
    
    dir_prefix[1] = '\0';
    files_count = scandir(".", &name_list, NULL, alphasort);
    if (files_count < 0) {
      /*  Error here */
    }
    head = &active_window.window->files;
    ClearList(*head); 

    while (files_count--) {
      if (name_list[files_count]->d_name[0] == '.' &&
          name_list[files_count]->d_name[1] != '.')
        continue;

      sprintf(file_path, "%s/%s",active_window.window->active_path,
              name_list[files_count]->d_name);
      stat(file_path, &file_info);
      if (S_ISDIR(file_info.st_mode))
        dir_prefix[0] = '/';
      else
        dir_prefix[0] = '\0';
      sprintf(file_name, "%s%s", dir_prefix, name_list[files_count]->d_name);
      
      AddLeaf(file_name, *head);
      free(name_list[files_count]);
      ++current_y;
    }
    active_window.lines_count = current_y;
    current_y = 0;
    free(name_list);
    PrintFileList(active_window.window);
    closedir(dir);
  }
}

/*  Change active window and highlight first file in directory. */
void ChangeActiveWindow(void)
{
  WINDOW *prev_nwindow = NULL;
  struct List *head = NULL;
  int current_y = 0;

  ColorLine(NON_HIGHLIGHTED_COLOR);
  if (active_window.left) {
    active_window.window = &right_fmwindow;
    prev_nwindow = left_dir_window;
    active_window.left = 0;
  }
  else {
    active_window.window = &left_fmwindow;
    prev_nwindow = right_dir_window;
    active_window.left = 1;
  }
  active_window.current_line = 0;
  active_window.current_file = 0;
  active_window.lines_count = ListSize(active_window.window->files);
  wclear(active_window.window->nwindow);
  head = active_window.window->files->next;
  while (current_y < DIR_WINDOW_HEIGHT && head) {
    mvwprintw(active_window.window->nwindow, current_y, 0, "%s",
              head->file_name);
    ++current_y;
    head = head->next;
  }
  
  ColorLine(HIGHLIGHTED_COLOR);

  redrawwin(prev_nwindow);
  redrawwin(active_window.window->nwindow);
}

/*  If current file is directory then open the directory. Otherwise if
 *  file is marked as executable then execute it, else open file
 *  in text editor. */
void OpenFile(void)
{
  char line[DIR_WINDOW_WIDTH];
  char path[PATH_LENGTH];
  char print_pattern[6];
  struct stat file_info;
  int is_dir;

  ReadCurrentLine(line);
  is_dir = line[0] == '/';
  if (is_dir)
    sprintf(print_pattern, "%s", "%s%s");
  else
    sprintf(print_pattern, "%s", "%s/%s");
  sprintf(path, print_pattern, active_window.window->active_path, line);
  if (stat(path, &file_info) == -1) {
    /*  Can't get info about this file. */
    return;
  }
  if (is_dir) {
    /*  Change current directory. */
    if (chdir(path) == -1) {
      return;
    }
    wclear(active_window.window->nwindow);
    getcwd(active_window.window->active_path, PATH_LENGTH);
    OpenActiveDir();
  } else if (file_info.st_mode & S_IXUSR ||
             file_info.st_mode & S_IXGRP || 
             file_info.st_mode & S_IXOTH) {
    if (fork() == 0) {
      /*  Execute file. */
      Finalize();
      wclear(stdscr);
      execlp(path, path, (char *) NULL);
    } else {
      int status;
      
      wait(&status);
      GetPrevState();
    }
  }
  else if (file_info.st_mode & S_IRUSR ||
           file_info.st_mode & S_IRGRP || 
           file_info.st_mode & S_IROTH) {
    if (fork() == 0) {
      /*  Open file in text editor. */
      Finalize();
      execlp(path_to_text_editor, "text_editor", line, (char *) NULL);
    }
    else {
      int status;

      wait(&status);
      GetPrevState();
    }
  } else {
    return;
  }

  ColorLine(NON_HIGHLIGHTED_COLOR);
  active_window.current_line = 0;
  active_window.current_file = 0;
  ColorLine(HIGHLIGHTED_COLOR); 
}

/*  If current file is not a dir, then copy it in neighboor tab. */
void CopyFile(void)
{
  WINDOW *copy_main_window,
         *copy_inside_window,
         *copy_progress_bar_bkgd,
         *copy_progress_bar_frgd,
         *copy_text_window;
  char file_name[100];

  copy_main_window = newwin(6, 14, 10, 10);
  copy_inside_window = derwin(copy_main_window, 4, 12, 1, 1);
  copy_progress_bar_bkgd = derwin(copy_inside_window, 1, 10, 1, 1);
  wbkgd(copy_progress_bar_bkgd, COLOR_PAIR(3));
  copy_progress_bar_frgd = NULL;
  wbkgd(copy_progress_bar_frgd, COLOR_PAIR(4));
  copy_text_window = derwin(copy_inside_window, 1, 10, 2, 1);
  box(copy_main_window, 0, 0);
  wrefresh(copy_main_window);

  wrefresh(copy_progress_bar_bkgd);
  mvwprintw(copy_text_window, 0, 1, "Copying");
  wrefresh(copy_text_window);

  mvwinnstr(active_window.window->nwindow, active_window.current_line, 0,
            file_name, DIR_WINDOW_WIDTH);

  getch();
  /*  Finalize. */
  delwin(copy_text_window);
  delwin(copy_progress_bar_bkgd);
  delwin(copy_inside_window);
  delwin(copy_main_window);

  redrawwin(left_dir_window);
}

void PrintFileList(struct FMWindow *fmwindow)
{
  struct List *head = fmwindow->files->next;
  int current_y = 0;

  while (head) {
    mvwprintw(fmwindow->nwindow, current_y, 0, "%s", head->file_name);
    head = head->next;
    ++current_y;
  }
  wrefresh(fmwindow->nwindow);
}

/*  Hightlight the file with number color_num. */
void ColorLine(int color_num)
{
  char line[38];
  int y = active_window.current_line;

  mvwinnstr(active_window.window->nwindow, y, 0, line, 38);
  wmove(active_window.window->nwindow, y, 0);
  wclrtoeol(active_window.window->nwindow);
  wattron(active_window.window->nwindow, COLOR_PAIR(color_num));
  wprintw(active_window.window->nwindow, "%s", line);
  wattroff(active_window.window->nwindow, COLOR_PAIR(color_num)); 
  redrawwin(active_window.window->nwindow);
}

/*  Remove previous file highlighting and
 *  highlight current file. */
void PickOutLine(int delta)
{
  ColorLine(2);
  active_window.current_line += delta;
  ColorLine(1);
}

/*  Return program to previous state that was
 *  before executing another program. */
void GetPrevState(void)
{
  Finalize();
  InitializeNcurses();
  CreateWindows();
  InitializeApp();
  
  wclear(stdscr);
  box(left_border_window, 0, 0);
  box(right_border_window, 0, 0);
  wrefresh(left_border_window);
  wrefresh(right_border_window);

  PrintFileList(&left_fmwindow);
  PrintFileList(&right_fmwindow);
  wmove(left_dir_window, 2, 0);
  curs_set(0);
  redrawwin(stdscr);
  redrawwin(left_border_window);
  redrawwin(left_dir_window);
  redrawwin(right_dir_window);
}

/*  Read current line from active window in file_name. */
void ReadCurrentLine(char *file_name)
{
  int i;

  mvwinnstr(active_window.window->nwindow, active_window.current_line, 0,
            file_name, DIR_WINDOW_WIDTH);
  for (i = 36; i > -1; --i) {
    if (file_name[i] != ' ') {
      file_name[i + 1] = '\0';
      break;
    }
  }
}

/*  Scroll window by step. */
void FmScroll(int step)
{
  int current_pos = 0;
  int current_y = 0;
  int end;
  struct List *head;

  head = active_window.window->files->next;
  wclear(active_window.window->nwindow);
  active_window.current_file += step;
  if (step > 0) {
    end = active_window.current_file - DIR_WINDOW_HEIGHT + 1;
  } else {
    end = active_window.current_file;
  }
  while (current_pos != end) {
    head = head->next;
    ++current_pos;
  }

  while (current_pos < active_window.lines_count &&
         head) {
    mvwprintw(active_window.window->nwindow, current_y, 0, "%s",
              head->file_name);
    ++current_pos;
    ++current_y;
    head = head->next;
  }
  wrefresh(active_window.window->nwindow);
  ColorLine(1);
}

void HandleKeyPress(void)
{
  unsigned int symbol;

  while (1) {
    symbol = wgetch(active_window.window->nwindow);
    if (symbol == 27)
      break;
    switch (symbol) {
      case KEY_F(2):
        CopyFile();

        break;
      case KEY_UP:
        if (active_window.current_line != 0) {
          PickOutLine(-1);
          --active_window.current_file;
        } else if (active_window.current_file != 0) {
          FmScroll(-1);
        }
        break;
      case KEY_DOWN:
        if (active_window.current_line != DIR_WINDOW_HEIGHT - 1 &&
            active_window.current_file != active_window.lines_count - 1) {
          PickOutLine(1);
          ++active_window.current_file;
        } else if (active_window.current_file !=
                   active_window.lines_count - 1) {
          FmScroll(1);
        }
        break;
      case KEY_LEFT:
        if (!active_window.left) {
          ChangeActiveWindow();
          wrefresh(right_dir_window);
        }
        break;
      case KEY_RIGHT:
        if (active_window.left) {
          ChangeActiveWindow();
          wrefresh(left_dir_window);
        }
        break;
      case '\n':
        OpenFile();
        break;
    }
  }
}

int main(int argc, char **argv)
{
  InitializeNcurses();
  CreateWindows();
  InitializeApp();
  if (argc > 1) {
    strcpy(left_fmwindow.active_path, argv[1]);
    strcpy(right_fmwindow.active_path, argv[1]);
  }
  HandleKeyPress();
  Finalize();
  return 0;
}
