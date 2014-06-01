/*****************************************************
* This file is part of yvonne-remote.
* (Arduino connection lib + pipe to command line)
*
* 2014, Jérôme Blanchi aka d.j.a.y
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

#ifndef __YVONNEREMOTELIB__
#define __YVONNEREMOTELIB__

#include <termios.h>
#include <sys/ioctl.h>
#include <fcntl.h>

int InitArduinoConnection (int iFileDescriptor, int baudrate, struct termios* oldtio);
void CloseArduinoConnection (int fd, struct termios* oldtio);
int OpenArduinoConnection (char* arduinoPort);

int ExecuteCommandLine(char* srtCommandName, char* strCommandLine);

char* strstr_last(const char* str1, const char* str2);

#endif //__YVONNEREMOTELIB__

