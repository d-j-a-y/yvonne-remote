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

#include <wand/MagickWand.h>
#include <gphoto2/gphoto2-camera.h>

#define ARDUINO_TEXTMAX                     255

typedef struct _YvonneCamera {
    GPContext *ctx;
    Camera *cam;
}YvonneCamera;

int YvonneArduinoInit (int iFileDescriptor, int baudrate, struct termios* oldtio);
void YvonneArduinoClose (int fd, struct termios* oldtio);
int YvonneArduinoOpen (char* arduinoPort);

void yrc_stateMachineArduino (int* yrc_stateField, int fdArduinoModem);

int YvonneTerminalInit (struct termios * ttysave);
int YvonneTerminalRestore (struct termios ttysave);

char* YvonneGetSceneName();

int YvonneExecute (char* srtCommandName, char* strCommandLine);
int YvonneExecuteForked (char* srtCommandName, char* strCommandLine);

char* strstr_last (const char* str1, const char* str2);

int YvonnePhotoCaptureInit (YvonneCamera* cam);
int YvonnePhotoCaptureUnref (YvonneCamera* cam);
int YvonnePhotoCapture (YvonneCamera* cam, const char *filename);
void YvonnePhotoCaptureError (GPContext *context, const char *format, void *data);
void YvonnePhotoCaptureMessage (GPContext *context, const char *format, void *data);

int YvonnePhotoResize (char* filesource, char* filetarget, long width, long height);

#endif //__YVONNEREMOTELIB__

