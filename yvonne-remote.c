/*****************************************************
* yvonne-remote - 0.1.dev
* 
*    yvonne-remote program is used to control automatic photo shooting
*    and videos generation for a performativ installation name "Yvonne"
*
*    Copyright (C) 2014 - Jérôme Blanchi aka d.j.a.y
*    http://github.com/d-j-a-y/yvonne-remote
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
* Some part inspirated from "Cratyle" 
* Originally created december 2013
* 2013-2014, Xavier Dubourdieu, Jérôme Blanchi (aka d.j.a.y), 
* http://github.com/d-j-a-y/Cratyle
* Under "GNU/GPL v3" Licence
*
* Some part inspirated from "arduino-serial" 
* Originally created 5 December 2006
* 2006-2013, Tod E. Kurt, http://todbot.com/blog/
* Under "The MIT License (MIT)"
*
* Dependecies
* -----------
* - gphoto2 (best experience with 2.5.4 cause "camera can't 
* shoot due to focus lock" i had with 2.4.8)
* - ffmpeg (need -start-number option) //TODO minimal version
* - imagemagick
* - libmagickwand-6.q16-dev
*
*****************************************************/

#include "yvonne-remote.h"
#include "yvonne-remote-lib.h"

#include <getopt.h>
#include <error.h>
#include <stdbool.h>
#include <signal.h>

void usage(void)
{
    printf("Usage: yvonne-remote [OPTIONS]\n"
    "\n"
    "Options:\n"
    "  -q  --quiet                Don't print out as much info\n"
    "  -h, --help                 Print this help message and quit\n"
    "  -n, --name=scenename       Name of the scene (defaut currentdir)\n"
    "  -p, --port=serialport      Serial port the Arduino is connected to (defaut %s)\n"
    "  -f, --fotogap=integer      Gap of scene (photo) numbering for resumption (optional)\n"
    "  -v, --videogap=integer     Gap of video numbering for resumption (optional)\n"
    "  -d  --delay=seconds        Delay between two shoot in seconds (default %d)\n"
    "  -b, --baud=rate            Baudrate (bps) of Arduino (default %d)\n"
    "  -s, --stream=path_to       Path of stream source to useµ. Bypass the camera detection\n"
    "  -R, --remoteno             No remote control\n" //FIXME
    "\n"
    "Dependecies: libgphoto2, ffmpeg, MagickWand\n"
    "\n", ARDUINO_DEFAULT_PORT, SHOOTING_DEFAULT_DELAY, ARDUINO_57600_BAUDRATE);

}

bool volatile keepRunning = true;

void intHandler(int dummy) {
    keepRunning = false;
}

int main(int argc, char** argv)
{
    // assign defautl value
    int baudrate = ARDUINO_57600_BAUDRATE;
    char quiet=0;
    int shootingDelay = SHOOTING_DEFAULT_DELAY;
    int photoIndex = 0;
    int sceneLowQualityIndex = 1;
    int videoIndex = 1;
    bool streamAsSource = false;
    bool remoteControl = true;
    char arduinoDeviceName[SERIAL_PORT_MAXLENGHT];
    struct termios ttysave;
    char *sourceName=NULL;
    char filesource [256];
    char filetarget [256];

    strcpy(arduinoDeviceName, ARDUINO_DEFAULT_PORT);// FIXME strlcpy

    char *sceneName= YvonneGetSceneName();
    if(!(sceneName)) {
      error(0, 0, ANSI_COLOR_RED "Can't get current working dir" ANSI_COLOR_RESET);
      perror("");
      exit(ERROR_GENERIC);
    }
    printf("Scene name set to '%s'\n", sceneName);

    /* parse options */
    int option_index = 0, opt;
    static struct option loptions[] = {
        {"help",      no_argument,       0, 'h'},
        {"port",      required_argument, 0, 'p'},
        {"baud",      required_argument, 0, 'b'},
        {"name",      required_argument, 0, 'n'},
        {"fotogap",   required_argument, 0, 'f'},
        {"videogap",  required_argument, 0, 'v'},
        {"delay",     required_argument, 0, 'd'},
        {"quiet",     no_argument,       0, 'q'},
        {"stream",    required_argument, 0, 's'},
        {"remoteno",  no_argument,       0, 'R'},
        {NULL,        0,                 0,  0}
    };

    while(1) {
        opt = getopt_long (argc, argv, "hqp:b:n:v:f:d:s:R",
                           loptions, &option_index);
        if (opt==-1) break;
        switch (opt) {
            case '0': break;
            case 'q':
                quiet = 1;
                break;
            case 'd':
                shootingDelay = strtol(optarg,NULL,10);
                if( !quiet ) printf("Shooting delay set to %d seconds\n",shootingDelay);
                break;
            case 'h':
                usage();
                exit(EXIT_SUCCESS);
            case 'b':
                baudrate = strtol(optarg,NULL,10);
                if( !quiet ) printf("Baudrate (bps) of Arduino set to %d\n",baudrate);
                break;
            case 'p':
                if(strlen(optarg)+1 <= SERIAL_PORT_MAXLENGHT) {
                  strcpy(arduinoDeviceName, optarg);// FIXME strlcpy
                  if(!quiet) printf("Arduino port set to %s\n",arduinoDeviceName);
                }
                else {
                  error(0, 0, "Can't assign Arduino serial port to %s (buffer lenght reached)\n",optarg);
                  exit(ERROR_GENERIC); //FIXME use nberr++
                }
                break;
            case 'f':
                  photoIndex = strtol(optarg,NULL,10);
                  sceneLowQualityIndex = 1 + (photoIndex * LOWQUALITY_REPEAT);
                  if(!quiet) printf("Scene numbering start from %d\n",photoIndex);
                break;
            case 'v':
                  videoIndex = strtol(optarg,NULL,10);
                  if(!quiet) printf("Video numbering start from %d\n",videoIndex);
                break;
            case 'n':
                  free(sceneName);
                  sceneName = malloc(strlen(optarg)*sizeof(char));
                  strcpy(sceneName, optarg);// FIXME strlcpy
                  if(!quiet) printf("Scene name set to '%s'\n",sceneName);
                break;
            case 's':
                  streamAsSource = true;
                  sourceName = malloc(strlen(optarg)*sizeof(char));
                  strcpy(sourceName, optarg);// FIXME strlcpy
                  if(!quiet) printf("Source set to '%s'\n",sourceName);
                break;
            case 'R':
                  remoteControl = false;
                  if(!quiet) printf("Remote control OFF\n");
                break;
        }
    }

    char logFile[]="yvonne.log";
    char logMessage [LOG_MAXLENGHT];
    int logDescriptor, logLenght = 0;

    if ((logDescriptor=open(logFile, O_WRONLY|O_CREAT|O_EXCL, 0644)) == -1) {
      YvonnePrint(YVONNE_MSG_ERROR, "Can't create logfile \"%s\"", logFile);
      perror("");
      goto CLOSE_SCENE_NAME;
    }
    logLenght = sprintf(logMessage, "BEGIN of %s's log\n", sceneName);//FIXME snprintf
    write(logDescriptor, logMessage, logLenght*sizeof(char));

    signal(SIGINT, intHandler);

    //! establish the connection with the arduino (mega2560) remote
    int fdArduinoModem;
    struct termios oldtio;

    if (remoteControl) {
        if((fdArduinoModem = YvonneArduinoOpen(arduinoDeviceName)) == ERROR_GENERIC) {
          YvonnePrint(YVONNE_MSG_ERROR, "FATAL : Can't connect to remote. Arduino connection failed at %s", arduinoDeviceName);
          perror("");
          goto CLOSE_LOG;
        }
        printf(ANSI_COLOR_CYAN "Remote control open from %s\n" ANSI_COLOR_RESET, arduinoDeviceName);

        if (YvonneArduinoInit(fdArduinoModem, baudrate, &oldtio) != ERROR_NO) {
          YvonnePrint(YVONNE_MSG_ERROR, "FATAL : Can't talk to remote. Arduino initialization failed");
          perror("");
          goto CLOSE_ARDUINO;
        }
    } else {
        if (YvonneTerminalInit(&ttysave) != ERROR_NO) {
          YvonnePrint(YVONNE_MSG_ERROR, "FATAL : Can't setup the terminal.");
          perror("");
          goto CLOSE_LOG;
        }
    }

    YvonneCamera* camera = NULL;
    bool CameraExited = false;
    if (!streamAsSource) {
        YvonneCamera* camera = malloc(sizeof(YvonneCamera));
        if(YvonnePhotoCaptureInit(camera) != ERROR_NO) {
          goto CLOSE_CAMERA;
        }
        //give other access to the camera
        gp_camera_exit (camera->cam,camera->ctx);
        CameraExited = true;
    }

    if(photoIndex == 0) {
      mkdir(LOWQUALITY_DIRECTORY,S_IFDIR|S_IRWXU|S_IRWXG);
      mkdir(VIDEO_DIRECTORY,S_IFDIR|S_IRWXU|S_IRWXG);
      mkdir(TMP_DIRECTORY,S_IFDIR|S_IRWXU|S_IRWXG);
    }

    int charReceived;
    int startSequence = 1;
//TODO malloc?
    char bufArduino[TEXTMAX];
//TODO State Mask
    int StateStop = 1;
    int StateVideo = 0;
    int StateQuit = 0;
    char commandLine[LINE_BUFFER];
    char currentPhoto[TEXTMAX_PHOTO];

    char* stopIndex = 0;
    char* startIndex = 0;

    unsigned int shootFailed = 0;

    //Generate current photo name
    sprintf(currentPhoto , "%s-%05d.jpg", sceneName, photoIndex);// FIXME strlcpy

    //! here start the (interesting) work
  while (!StateQuit && keepRunning) {

    //Listen to the Arduino remote and adjust state
    if(remoteControl) {
        charReceived = read(fdArduinoModem,bufArduino,TEXTMAX);

        if (charReceived == 0 || (charReceived == -1 && errno == EAGAIN)) {
          if(!quiet) printf("Nothing to read from Arduino\n");
        }
        else {
          bufArduino[charReceived]='\0';
          if(!quiet) printf("Arduino string readed : %s - %d\n", bufArduino, charReceived);

          if (charReceived) {
            StateVideo = (strstr(bufArduino, "VIDEO") ? 1 : 0);
            StateQuit = (strstr(bufArduino, "QUIT") ? 1 : 0);
    //TODO          Speed = (strstr(bufArduino, "SPEED1") ? 1 : 0);
            stopIndex = strstr_last(bufArduino, "STOP");
            startIndex = strstr_last(bufArduino, "START");

            if (startIndex || stopIndex)
              StateStop = (startIndex > stopIndex) ? 0 : 1;

            memset(bufArduino, '\0', charReceived);
          }
        }
    } else {
        struct timeval tv;
        // set the time value to 1 second
        tv.tv_sec = 1; // FIXME needed inside the loop ?
        tv.tv_usec = 0;

        fd_set readset;
        FD_ZERO(&readset);
        FD_SET(fileno(stdin), &readset);

        select(fileno(stdin)+1, &readset, NULL, NULL, &tv);
        // the user typed a character so exit
        if(FD_ISSET(fileno(stdin), &readset))
        {
            putchar(fgetc (stdin)); // manually echo the character
            StateQuit = TRUE;
            break;
        }
    }

    //For Video generation
    if (StateVideo)
    {
      //resize the photo
      //TODO change LOWQUALITY_DIRECTORY to command line parameter
      //INFO -vframes n or -frames:v to control the quantity of frames

      //INFO 25 f/s because all images are duplicated N time
      sprintf(commandLine, "ffmpeg -hide_banner -f image2 -start_number %d -r 25 -i \"%s/%s-%%05d.jpg\" -q:v 1 -c:v mjpeg -s %s -aspect 16:9 %s/%s-%.3d.avi", startSequence, LOWQUALITY_DIRECTORY, sceneName, LOWQUALITY_RESOLUTION, VIDEO_DIRECTORY, sceneName, videoIndex);// FIXME snprintf
      if (YvonneExecuteForked ("video generation", commandLine) != ERROR_NO) {
        YvonnePrint(YVONNE_MSG_ERROR, "ERROR during video generation cmd line : %s", commandLine);
      }
      else {
        printf (ANSI_COLOR_GREEN "#### -------------------------------------------- ####\n" ANSI_COLOR_RESET);
        printf (ANSI_COLOR_GREEN "####                                              ####\n" ANSI_COLOR_RESET);
        printf (ANSI_COLOR_GREEN "#### A fresh video will be available to be chewed ####\n" ANSI_COLOR_RESET);
        printf (ANSI_COLOR_GREEN "#### ----> %s-%d.avi                   ####\n" ANSI_COLOR_RESET, sceneName, videoIndex);
        printf (ANSI_COLOR_GREEN "#### -------------------------------------------- ####\n" ANSI_COLOR_RESET);
      }

      logLenght = sprintf(logMessage, "VIDEO %s-%.3d.avi : %d ---> %d (%d)\n", //FIXME snprintf
                          sceneName,
                          videoIndex,
                          startSequence,
                          sceneLowQualityIndex,
                          photoIndex);

      write(logDescriptor, logMessage, logLenght*sizeof(char));

      startSequence = sceneLowQualityIndex;
      videoIndex++;

      StateVideo = 0;
    }

    YvonnePrint(YVONNE_MSG_INFO, "Current photo : %s", currentPhoto);

    //Exit now!
    if(StateQuit || !keepRunning) continue;

    // to command the shooting and resize the image
    if (!StateStop)
    {
        if (!streamAsSource) {
          if(CameraExited) ; //FIXME will solve some ContextError ?
          //take the photo
          if(!quiet) printf("Capturing to file %s\n", currentPhoto);
          if(YvonnePhotoCapture(camera, currentPhoto) == ERROR_NO){
            CameraExited = false;
            shootFailed = 0;
            sprintf(filesource, "%s/%s", TMP_DIRECTORY, currentPhoto);// FIXME snprintf

            if(YvonnePhotoResize(currentPhoto, filesource, LOWQUALITY_RESOLUTION_W,LOWQUALITY_RESOLUTION_H) != ERROR_NO)
              YvonnePrint(YVONNE_MSG_ERROR, "ERROR resizing image file %s", filesource);

            //duplicate the lowquality photo to slowdown the video rythm
            unsigned int repeatEachImage;
            // TODO video fps control
            for (repeatEachImage = LOWQUALITY_REPEAT; repeatEachImage > 0 ; repeatEachImage--) {
              sprintf(filetarget, "%s/%s-%05d.jpg", LOWQUALITY_DIRECTORY, sceneName, sceneLowQualityIndex++);// FIXME snprintf
              if(link(filesource, filetarget)!=0) {
                YvonnePrint(YVONNE_MSG_ERROR, "ERROR linking images files");
                perror("");
              }
            }
            photoIndex++;
            //Generate current photo name
            sprintf(currentPhoto , "%s-%05d.jpg", sceneName, photoIndex);// FIXME strlcpy
          }
          else {
            YvonnePrint(YVONNE_MSG_WARNING, "Photo %s shoot has failed %d time(s)", currentPhoto, ++shootFailed);
            sleep(4);
            printf("Camera init again....\n");
            YvonnePhotoCaptureUnref(camera);
            if((YvonnePhotoCaptureInit(camera) != ERROR_NO) && (shootFailed >= CAMERA_INIT_ATTEMPT_MAX)) {
                YvonnePrint(YVONNE_MSG_ERROR, "No camera auto detected. Check battery maybe?");
                goto CLOSE_CAMERA;
            }
          }
        } else {} //FIXME source as stream
    }
    else {
      if (!streamAsSource) {
          if(!CameraExited) {
            gp_camera_exit (camera->cam,camera->ctx);
            CameraExited = true;
          }
          if(!quiet) printf("Shooting paused\n");
          sleep(3);
      }
    }
    //    sleep(1);
  }

CLOSE_CAMERA:
    if (!streamAsSource) {
        YvonnePhotoCaptureUnref(camera);
        free(camera);
    } else {
        free (sourceName);
    }

CLOSE_ARDUINO:
    if (remoteControl) {
        YvonneArduinoClose(fdArduinoModem, &oldtio);
    } else {
        YvonneTerminalRestore(ttysave);
    }

CLOSE_LOG:
    logLenght = sprintf(logMessage, "END of %s's log\n", sceneName);// FIXME snprintf
    write(logDescriptor, logMessage, logLenght*sizeof(char));
    close(logDescriptor);

CLOSE_SCENE_NAME:
    free(sceneName);
    YvonnePrint(YVONNE_MSG_INFO,"\nSuccessly close, see you next shot!");
    exit(EXIT_SUCCESS);
}

