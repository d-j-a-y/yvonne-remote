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

#define YRC_MENU_WIDTH 30
#define YRC_MENU_HEIGHT 10

#define YRC_MENU_ENTRY_NO -1

typedef enum
{
  YVONNE_MSG_ERROR,
  YVONNE_MSG_WARNING,
  YVONNE_MSG_INFO,
  YVONNE_MSG_UI,
  YVONNE_MSG_VIDEO_BANNER
} YvonneMsgType;

void yrc_menuPrint(WINDOW *menu_win, int highlight);
int yrc_uiSetup();
int yrc_uiRestore();

int yrc_menuOpen(WINDOW **menu_win);
int yrc_menuClose(WINDOW *menu_win);
int yrc_menuGetEntry (WINDOW *menu_win, int *highlight);


void yrc_uiPrint(YvonneMsgType msgType, char* message, ...);

#endif //__YVONNEREMOTEUI__

