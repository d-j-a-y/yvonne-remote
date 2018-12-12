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

#ifndef __YVONNEREMOTEUI__
#define __YVONNEREMOTEUI__

#include <stdarg.h>
#include <ncurses.h>

#define YRC_MENU_WIDTH      30
#define YRC_MENU_HEIGHT     10

#define YRC_MENU_ENTRY_NO   -1

#define YRC_UI_FOOTER_H     (4)

#define YRC_UI_INDEX_X      (COLS-12)
#define YRC_UI_INDEX_TEXT_L (8)

typedef enum
{
  YVONNE_MSG_ERROR,
  YVONNE_MSG_WARNING,
  YVONNE_MSG_INFO,
  YVONNE_MSG_UI,
  YVONNE_MSG_VIDEO_BANNER
} YvonneMsgType;

int yrc_uiSetup (void);
int yrc_uiRestore (void);

int yrc_menuOpen (void);
int yrc_menuClose (void);
void yrc_menuPrint (int highlight);
int yrc_menuCheckEntry (WINDOW* win, int *highlight);

int yrc_errorOpen (void);
int yrc_errorClose (void);

void yrc_stateMachineLocal (volatile sig_atomic_t *yrc_stateField);

void yrc_coloredPrintf (YvonneMsgType msgType, char* message, ...);

void yrc_uiPrintHelp (void);
void yrc_uiPrintLayout (void);
void yrc_uiPrintMediaIndex (int currentPhoto, int currentVideo);
void yrc_uiPrintMessage (YvonneMsgType msgType, char* errorMessage, ...);

#endif //__YVONNEREMOTEUI__

