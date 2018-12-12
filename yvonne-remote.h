/*****************************************************
* This file is part of yvonne-remote.
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


#ifndef __YVONNEREMOTE__
#define __YVONNEREMOTE__

#include "ansi-colors.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

//mkdir
#include <sys/stat.h>
#include <sys/types.h>
//teminal control
#include <sys/select.h>
#include <sys/time.h>

#include <signal.h>

extern volatile sig_atomic_t exec_status;

#define ERROR_NO      0
#define ERROR_GENERIC 1

#define _POSIX_SOURCE 1 /* POSIX compliant source FIXME */

#define LINE_BUFFER                 255
#define TEXTMAX_PHOTO               255

#define SERIAL_PORT_MAXLENGHT       80
#define SCENE_NAME_MAXLENGHT        20
#define LOG_MAXLENGHT               255

#define FFMPEG_STATIC_BUILD_INSTALL   "/usr/local/bin/"

#define SCENE_DEFAULT_NAME            "yvonne"
#define OLD_LOWQUALITY_RESOLUTION     "640x480"
#define LOWQUALITY_RESOLUTION         "1024x576"
#define LOWQUALITY_RESOLUTION_W       1024
#define LOWQUALITY_RESOLUTION_H       576
#define LOWQUALITY_REPEAT             5

#define CAMERA_INIT_ATTEMPT_MAX       30

#define SHOOTING_DEFAULT_DELAY        5
#define LOWQUALITY_DIRECTORY          "./lowk"
#define VIDEO_DIRECTORY               "./video"
#define TMP_DIRECTORY                 "./temp"

#define ARDUINO_57600_BAUDRATE        57600
#define ARDUINO_MEDA2560_ACM0         "/dev/ttyACM0"
#define ARDUINO_DEFAULT_PORT          ARDUINO_MEDA2560_ACM0

#define YRC_STATE_VIDEO               (1 << 0)  /* 00000001 */
#define YRC_STATE_PHOTO               (1 << 1)  /* 00000010 */
#define YRC_STATE_QUIT                (1 << 2)  /* 00000100 */
#define YRC_STATE_VIDEO_PROGRESS      (1 << 3)  /* 00001000 */

#define JPG                           0
#define PNG                           1

#define RENDER_OK                     0
#define RENDER_KO                     1

#endif //  __YVONNEREMOTE__
