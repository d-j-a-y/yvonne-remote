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

char *yrc_msgTypePrefix[] = { "E: ",
                              "W: ",
                              "I: ",
                              "", //UI
                              "", //Video banner
                        };

static WINDOW *menu_win = NULL;
static WINDOW *msg_win = NULL;

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

int yrc_uiSetup (void)
{
    initscr();
    clear();
    noecho();
    cbreak();   /* Line buffering disabled. pass on everything */

    start_color(); /* Start color*/
    init_pair(1, COLOR_BLUE, COLOR_BLACK);
    //~ int row, col;
    //~ getmaxyx(stdscr,row,col); /* FIXME get the number of rows and columns */

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
int yrc_uiRestore (void)
{
    clrtoeol();
    refresh();
    endwin();

    return ERROR_NO;
}

/**
 *  yrc_menuOpen
 *
 *  @return Error Code
 *
 *  Setup the menu window, caller is responsible of cleaning the memory (see yrc_menuClose)
 */
int yrc_menuOpen (void)
{

    int startx = (COLS - YRC_MENU_WIDTH) / 2;
    int starty = (LINES - YRC_MENU_HEIGHT) / 2;

    menu_win = newwin(YRC_MENU_HEIGHT, YRC_MENU_WIDTH, starty, startx);
    if(menu_win !=NULL){
        wtimeout (menu_win, 300);
        keypad(menu_win, TRUE);
        return ERROR_NO;
    }

    return ERROR_GENERIC;
}

/**
 *  yrc_menuClose
 *
 *  @return Error Code
 *
 *  Clean menu memory
 */
int yrc_menuClose (void)
{
    if (menu_win) delwin(menu_win);
    return ERROR_NO;
}

/**
 *  yrc_errorOpen
 *
 *  @return Error Code
 *
 *  Setup the error window, caller is responsible of cleaning the memory (see yrc_errorClose)
 */
int yrc_errorOpen (void)
{
    msg_win = newwin(YRC_UI_FOOTER_H, YRC_UI_INDEX_X-1, LINES-YRC_UI_FOOTER_H, 0);
    if (msg_win != NULL) {
        scrollok(msg_win, TRUE);
        return ERROR_NO;
    }

    return ERROR_GENERIC;
}

/**
 *  yrc_errorClose
 *
 *  @return Error Code
 *
 *  Clean menu memory
 */
int yrc_errorClose (void)
{
    if(msg_win) delwin(msg_win);
    return ERROR_NO;
}

/**
 *  yrc_menuPrint
 *  @param A one based index for current highlighed menu entry
 *
 *  Build and display the menu with current highlighed menu entry
 */
void yrc_menuPrint (int highlight)
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
 *  Check (no block has set @see yrc_menuOpen) for key pressed.
 *  Adjust menu selection and if new event occurs, return user choice.
 */
int yrc_menuCheckEntry (WINDOW *menu_win, int *highlight)
{
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
    yrc_menuPrint(*highlight);

    return choice;
}

/**
 *  yrc_stateMachineLocal
 *  @param pointer to int for bit field
 *
 *  Update the state from local stupid UI. @see yrc_menuCheckEntry
 */

void yrc_stateMachineLocal (volatile sig_atomic_t *yrc_stateField)
{
    static int highlight = 1;

    static int yy = -1;
    static char tictac = '*';

    if(yy == -1) yy = YRC_UI_INDEX_X ;
    mvaddch(LINES-YRC_UI_FOOTER_H-1,yy++,tictac);
    refresh();
    if (yy >= COLS-1 ){ yy = YRC_UI_INDEX_X; tictac = ~tictac; }

    int menu_choice;
    if ((menu_choice = yrc_menuCheckEntry (menu_win, &highlight)) != YRC_MENU_ENTRY_NO) {
        switch(menu_choice) {
            case 'p':
                (*yrc_stateField) &= ~YRC_STATE_PHOTO;
            break;
            case 's':
                (*yrc_stateField) |= YRC_STATE_PHOTO;
            break;
            case 'v':
                (*yrc_stateField) |= YRC_STATE_VIDEO;
            break;
            case 'q':
                (*yrc_stateField) |= YRC_STATE_QUIT;
            break;
        }
    }
}

/**
 *  yrc_coloredPrintf
 *  @param msgType Type of message has YvonneMsgType
 *  @param char* message
 *  @param ...
 *
 *  Basic message printing
 *  @warning Do not use with ncurses support aka failback
 */
void yrc_coloredPrintf (YvonneMsgType msgType, char* message, ...)
{
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
  fprintf( stdout, "%s%s%s%s\n",yrc_msgTypePrefix[msgType], errorColor, buf, ANSI_COLOR_RESET );
  va_end(args);
}

/**
 *  yrc_uiPrintHelp
 *
 *  Display the help message in the upper left window corner
 */
void yrc_uiPrintHelp(void)
{
    mvprintw(0, 0, "Use shortcuts or arrow keys up and down and Enter to select a choice.");
    refresh();
}

/**
 *  yrc_uiPrintLayout
 *
 *  Print the immobile stuf
 */
void yrc_uiPrintLayout(void)
{
    mvprintw(LINES-YRC_UI_FOOTER_H,     YRC_UI_INDEX_X, "photo : ");
    mvprintw(LINES-YRC_UI_FOOTER_H+1,   YRC_UI_INDEX_X, "video : ");
    refresh();
}

/**
 *  yrc_uiPrintMediaIndex
 *
 *  Refresh the current photo and video index
 */
void yrc_uiPrintMediaIndex(int currentPhoto, int currentVideo)
{
    mvprintw(LINES-YRC_UI_FOOTER_H,     YRC_UI_INDEX_X+YRC_UI_INDEX_TEXT_L, "%d", currentPhoto); //strlen current photo
    mvprintw(LINES-YRC_UI_FOOTER_H+1,   YRC_UI_INDEX_X+YRC_UI_INDEX_TEXT_L, "%d", currentVideo);
    refresh();
}

/**
 *  yrc_uiPrintMessage
 *  @param msgType Type of message has YvonneMsgType
 *  @param char* message
 *  @param ...
 *
 *
 *  Type based (error/warning/info...) messages printing.
 */
void yrc_uiPrintMessage(YvonneMsgType msgType, char* errorMessage, ...)
{
    va_list args;
    int textAttrib;
    // parse arguments
    va_start(args, errorMessage);

    switch(msgType) {
        case YVONNE_MSG_ERROR:
            textAttrib = A_STANDOUT;
        break;
        case YVONNE_MSG_VIDEO_BANNER:
            textAttrib = COLOR_PAIR(1);
        break;
        default:
            textAttrib = A_NORMAL;
        break;
    }
    wattron(msg_win,textAttrib);
    vwprintw(msg_win, errorMessage, args); //strlen current photo
    wattroff(msg_win,textAttrib);
    wrefresh(msg_win);
    va_end(args);
}

