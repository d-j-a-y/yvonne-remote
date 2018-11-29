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
                        };

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

    int row, col;

    getmaxyx(stdscr,row,col); /* get the number of rows and columns */
    startx = (80 - YRC_MENU_WIDTH) / 2;
    starty = (24 - YRC_MENU_HEIGHT) / 2;

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
 *  yrc_menuGetEntry
 *  @param A pointer to a ncurses WINDOW pointer
 *  @param A pointer to the current highlighted menu entry
 *
 *  @return Error Code
 *
 *  Restore the initial terminal state
 */
int yrc_menuGetEntry (WINDOW *menu_win, int *highlight) {
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
            mvprintw(24, 0, "Charcter pressed is = %3d Hopefully it can be printed as '%c'", c, c);
            refresh();
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
            //~ mvprintw(24, 0, "Charcter pressed is = %3d Hopefully it can be printed as '%c'", c, c);
            //~ refresh();
            break;
    }
    yrc_menuPrint(menu_win, (*highlight));

    return choice;
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
