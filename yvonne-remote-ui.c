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

/**
 *  yrc_uiPrint
 *  @param msgType Type of message has YvonneMsgType
 *  @param char* message
 *  @param ...
 *
 *  Basic message printing
 */
void yrc_uiPrint(YvonneMsgType msgType, char* message, ...) {
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
