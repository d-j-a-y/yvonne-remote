/*****************************************************
* This file is part of yvonne-remote.
* ( ncurses terminal interface )
*
* 2018, Jérôme Blanchi aka d.j.a.y
* http://github.com/d-j-a-y/yvonne-remote
*
*    yvonne-remote is free software: you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation, either version 3 of the License, or
*    any later version.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*****************************************************/

#include "yvonne-remote.h"
#include "yvonne-remote-ui.h"

int startx = 0;
int starty = 0;

char *yrc_menuEntry[] = { "Start - 's'",
                          "Pause - 'p'",
                          "Video - 'v'",
                          "Quit  - 'q'",
                        }; // Help ?

char yrc_menuEntryCode[] = { 's',
                             'p',
                             'v',
                             'q',
                            };

int yrc_menuEntries = sizeof(yrc_menuEntry) / sizeof(char *);

/**
 *  yrc_uiX
 *  @param ...
 *
 *  @return
 *
 *  Do x
 */

/**
 *  yrc_uiSetup
 *  @param 
 *
 *  @return Error Code
 *
 *  Setup the ncurse user interface
 */

int yrc_uiSetup () {
    initscr();
    clear();
    noecho();
    cbreak();   /* Line buffering disabled. pass on everything */

    return ERROR_NO;
}

/**
 *  yrc_uiRestore
 *  @param ...
 *
 *  @return Error Code
 *
 *  Restore the initial terminal state
 */
int yrc_uiRestore () {
    clrtoeol();
    refresh();
    endwin();

    return ERROR_NO;
}

/**
 *  yrc_menuOpen
 *  @param A pointer to a ncurses WINDOW pointer
 *
 *  @return Error Code
 *
 *  Setup the menu window, caller is responsible of cleaning the memory (see yrc_menuClose)
 */
int yrc_menuOpen (WINDOW **menu_win) {

    //~ int row, col;
    //~ getmaxyx(stdscr,row,col); /* FIXME get the number of rows and columns */
    startx = (COLS - YRC_MENU_WIDTH) / 2;
    starty = (LINES - YRC_MENU_HEIGHT) / 2;

    (*menu_win) = newwin(YRC_MENU_HEIGHT, YRC_MENU_WIDTH, starty, startx);
    wtimeout ((*menu_win), 300);
    keypad((*menu_win), TRUE);

    return ERROR_NO;
}

/**
 *  yrc_menuClose
 *  @param A ncurses WINDOW pointer
 *
 *  @return Error Code
 *
 *  Clean menu memory
 */
int yrc_menuClose (WINDOW *menu_win) {
    delwin(menu_win);
    return ERROR_NO;
}

/**
 *  yrc_menuPrint
 *  @param A pointer to a ncurses WINDOW pointer
 *  @param A one based index for current highlighed menu entry
 *
 *  Build and display the menu with current highlighed menu entry
 */
void yrc_menuPrint (WINDOW *menu_win, int highlight)
{
    int x, y, i;

    x = 2;
    y = 2;
    box(menu_win, 0, 0);
    for(i = 0; i < yrc_menuEntries; ++i)
    {	if(highlight == i + 1) /* High light the present choice */
        {	wattron(menu_win, A_REVERSE); 
            mvwprintw(menu_win, y, x, "%s", yrc_menuEntry[i]);
            wattroff(menu_win, A_REVERSE);
        }
        else
            mvwprintw(menu_win, y, x, "%s", yrc_menuEntry[i]);
        ++y;
    }
    wrefresh(menu_win);
}

/**
 *  yrc_menuCheckEntry
 *  @param A pointer to a ncurses WINDOW pointer
 *  @param A pointer to the current highlighted menu entry
 *
 *  @return Menu entry selection has letter or YRC_MENU_ENTRY_NO
 *
 *  Check (no block has set) for key pressed.
 *  Adjust menu selection and status accordingly.
 */
int yrc_menuCheckEntry (WINDOW *menu_win, int *highlight) {
    int c;
    int choice = YRC_MENU_ENTRY_NO;

    c = wgetch(menu_win);
    switch(c) {
        case KEY_UP:
            if((*highlight) == 1)
                (*highlight) = yrc_menuEntries;
            else
                --(*highlight);
            break;
        case KEY_DOWN:
            if((*highlight) == yrc_menuEntries)
                (*highlight) = 1;
            else 
                ++(*highlight);
            break;
        case 10: //RETURN
            choice = (int)yrc_menuEntryCode[(*highlight)-1];
            break;
        case 's':
        case 'S':
            choice = 's';
            break;
        case 'p':
        case 'P':
            choice = 'p';
            break;
        case 'v':
        case 'V':
            choice = 'v';
            break;
        case 'q':
        case 'Q':
            choice = 'q';
            break;
        case EOF:
            break;
        default:
            mvprintw(24, 0, "Unexpected character pressed '%c'.", c);
            refresh();
            break;
    }
    yrc_menuPrint(menu_win, (*highlight));

    return choice;
}

void yrc_stateMachineLocal ( int *yrc_stateField , WINDOW* menu_win) {
    static int highlight = 1;

    static int yy = -1;
    static char tictac = '*';

    if(yy == -1) yy = YRC_UI_INDEX_X ;
    mvaddch(LINES-3,yy++,tictac);
    refresh();
    if (yy >= COLS-1 ){ yy = YRC_UI_INDEX_X; tictac = ~tictac; }

    int menu_choice;
    if ((menu_choice = yrc_menuCheckEntry (menu_win, &highlight)) != YRC_MENU_ENTRY_NO) {
        switch(menu_choice) {
            case 'p':
                (*yrc_stateField) &= ~YRC_STATE_PHOTO;
            break;
            case 's':
                //~ StateStop = 1;
                (*yrc_stateField) |= YRC_STATE_PHOTO;
            break;
            case 'v':
                //~ StateVideo = 1;
                (*yrc_stateField) |= YRC_STATE_VIDEO;
            break;
            case 'q':
                //~ StateQuit = 1;
                (*yrc_stateField) |= YRC_STATE_QUIT;
            break;
        }
    }
}

/**
 *  yrc_uiPrint
 *  @param msgType Type of message has YvonneMsgType
 *  @param char* message
 *  @param ...
 *
 *  Basic message printing
 *  @warning Do not use with ncurses support
 */
void yrc_uiPrint (YvonneMsgType msgType, char* message, ...) {
  char buf[1024];
  va_list args;
  // parse arguments
  va_start(args, message);
  vsnprintf(buf, sizeof(buf) - 1, message, args);
  char *errorColor;
  switch(msgType) {
    case YVONNE_MSG_ERROR:
      errorColor = ANSI_COLOR_RED;
    break;
    case YVONNE_MSG_WARNING:
      errorColor = ANSI_COLOR_YELLOW;
    break;
    case YVONNE_MSG_INFO:
      errorColor = ANSI_COLOR_CYAN;
    break;
    case YVONNE_MSG_UI:
      errorColor = ANSI_COLOR_MAGENTA;
    break;
    case YVONNE_MSG_VIDEO_BANNER:
      errorColor = ANSI_COLOR_GREEN;
    break;
    default:
      errorColor = ANSI_COLOR_RESET;
  }
  fprintf( stdout, "%s%s%s\n", errorColor, buf, ANSI_COLOR_RESET );
  va_end(args);
}

/**
 *  yrc_uiPrintHelp
 *
 *  Display the help message in the upper left window corner
 */
void yrc_uiPrintHelp(){

    mvprintw(0, 0, "Use shortcuts or arrow keys up and down and Enter to select a choice.");
    refresh();
}

/**
 *  yrc_uiPrintLayout
 *
 *  Print the immobile stuf
 */
void yrc_uiPrintLayout(){

    mvprintw(LINES-2, YRC_UI_INDEX_X, "Current photo : ");
    mvprintw(LINES-1, YRC_UI_INDEX_X, "Current video : ");
    refresh();
}

/**
 *  yrc_uiPrintMediaIndex
 *
 *  Print the immobile stuf
 */
void yrc_uiPrintMediaIndex(int currentPhoto, int currentVideo){

    mvprintw(LINES-2, YRC_UI_INDEX_X+16, "%d", currentPhoto); //strlen current photo
    mvprintw(LINES-1, YRC_UI_INDEX_X+16, "%d", currentVideo);
    refresh();
}
