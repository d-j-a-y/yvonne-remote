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
#include "yvonne-remote-ui.h"

#include <getopt.h>
#include <error.h>
#include <stdbool.h>

char *const fileFormat[] = {"jpg",
                            "png",
                            NULL
                           };

char *const renderMessages[] = {"RENDER DONE",
                                "RENDER FAILED",
                                NULL
                                };

/* When a SIGUSR1 signal arrives, set this variable. */
volatile sig_atomic_t usr_interrupt = 0;
volatile sig_atomic_t exec_status = -1;

volatile sig_atomic_t yrc_stateField = YRC_STATE_PHOTO;

void forksyncHandler (int sig)
{
    int stat_val;
    /*pid_t child_pid;
    child_pid = */wait(&stat_val); //! Synchro thread TODO check if not deadlock pb
    if(!WIFEXITED(stat_val))
        exec_status = ERROR_EXEC_FAIL;
    else {
        exec_status = (WEXITSTATUS(stat_val) == EXIT_SUCCESS) ? ERROR_NO : ERROR_EXEC_COMMAND_FAIL;
    }
    usr_interrupt = 1;
}

bool volatile keepRunning = true;

void interruptHandler(int dummy)
{
    keepRunning = false;
}


void usage (void)
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
    "Dependecies: libgphoto2, ffmpeg, MagickWand, Ncurses.\n"
    "\n", ARDUINO_DEFAULT_PORT, SHOOTING_DEFAULT_DELAY, ARDUINO_57600_BAUDRATE);

}

int main(int argc, char** argv)
{
    // assign defautl value
    int lastError = ERROR_NO;
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
    char *photoFormat = fileFormat[JPG];

    strcpy(arduinoDeviceName, ARDUINO_DEFAULT_PORT);// FIXME strlcpy

    char *sceneName= YvonneGetSceneName();
    if(!(sceneName)) {
      error(0, 0, ANSI_COLOR_RED "Can't get current working dir" ANSI_COLOR_RESET);
      perror("");
      exit(ERROR_GENERIC);
    }
//    printf("Scene name set to '%s'\n", sceneName);

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
            case 'f': // FIXME logfile should be happend not created
                  photoIndex = strtol(optarg,NULL,10);
                  sceneLowQualityIndex = 1 + (photoIndex * LOWQUALITY_REPEAT);
                  if(!quiet) printf("Scene numbering start from %d\n",photoIndex);
                break;
            case 'v': // FIXME logfile should be happend not created
                  videoIndex = strtol(optarg,NULL,10);
                  if(!quiet) printf("Video numbering start from %d\n",videoIndex);
                break;
            case 'n':
                  free(sceneName);
                  sceneName = malloc(strlen(optarg)*sizeof(char));
                  strcpy(sceneName, optarg);// FIXME strlcpy
//                  if(!quiet) printf("Scene name assigned to '%s'\n",sceneName);
                break;
            case 's':
                  streamAsSource = true;
                  sourceName = malloc(strlen(optarg)*sizeof(char));
                  strcpy(sourceName, optarg);// FIXME strlcpy
                  photoFormat = fileFormat[PNG];
                  if(!quiet) printf("Source set to '%s'\n",sourceName);
                break;
            case 'R':
                  remoteControl = false;
                  if(!quiet) printf("Remote control OFF\n");
                break;
        }
    }

    if(!quiet) printf("Scene name assigned to '%s'\n",sceneName);

    //! log file creation and start
    char logFile[]="yvonne.log"; // TODO scene name + horodate
    char logMessage [LOG_MAXLENGHT];
    int logDescriptor, logLenght = 0;

    if ((logDescriptor=open(logFile, O_WRONLY|O_CREAT|O_EXCL, 0644)) == -1) {
      yrc_coloredPrintf(YVONNE_MSG_ERROR, "Can't create logfile \"%s\"", logFile);
      perror("--->");
      goto CLOSE_SCENE_NAME;
    }
    logLenght = sprintf(logMessage, "# BEGIN of %s's log\n", sceneName);//FIXME snprintf TODO date more info
    write(logDescriptor, logMessage, logLenght*sizeof(char));

    //! setup interrupt handler
    signal(SIGINT, interruptHandler);

    //! establish the connection with the adhoc local or remote control
    int fdArduinoModem = -1;
    struct termios oldtio;
    WINDOW *menu_win = NULL;
    WINDOW *msg_win = NULL;

    if (remoteControl) { // arduino remote control
    //! establish the connection with the arduino (mega2560) remote
        if((fdArduinoModem = YvonneArduinoOpen(arduinoDeviceName)) == ERROR_GENERIC) {
          yrc_coloredPrintf(YVONNE_MSG_ERROR, "FATAL! Can't connect to remote. Arduino connection failed at %s", arduinoDeviceName);
          perror("");
          goto CLOSE_LOG;
        }
        printf(ANSI_COLOR_CYAN "Remote control open from %s\n" ANSI_COLOR_RESET, arduinoDeviceName);

        if (YvonneArduinoInit(fdArduinoModem, baudrate, &oldtio) != ERROR_NO) {
          yrc_coloredPrintf(YVONNE_MSG_ERROR, "FATAL! Can't talk to remote. Arduino initialization failed");
          perror("");
          goto CLOSE_CONTROL;
        }
    } else { // local terminal (ncurses)
        if (yrc_uiSetup() != ERROR_NO) {
          yrc_coloredPrintf(YVONNE_MSG_ERROR, "FATAL! Can't setup the user interface."); // TODO fallback to tty
          perror("");
          goto CLOSE_LOG;
        }
        if (yrc_menuOpen (&menu_win) != ERROR_NO) {
          yrc_coloredPrintf(YVONNE_MSG_ERROR, "FATAL! Can't create ui menu");// TODO fallback to tty
          perror("");
          goto CLOSE_CONTROL;
        }
        if (yrc_errorOpen (&msg_win) != ERROR_NO) {
          yrc_coloredPrintf(YVONNE_MSG_ERROR, "FATAL! Can't create ui error window");
          perror("");
          goto CLOSE_CONTROL;
        }

        if (FALSE) { // local terminal tty
        if (YvonneTerminalInit(&ttysave) != ERROR_NO) {
          yrc_coloredPrintf(YVONNE_MSG_ERROR, "FATAL! Can't setup the terminal.");
          perror("");
          goto CLOSE_LOG;
        }
        }
    }

    //! initialize image selected source
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
    } else {
        //TODO Setup the stream
        // if stream exist
        // get stream properties
    }

    //! images folders creation
    if(photoIndex == 0) {
      mkdir(LOWQUALITY_DIRECTORY,S_IFDIR|S_IRWXU|S_IRWXG);
      mkdir(VIDEO_DIRECTORY,S_IFDIR|S_IRWXU|S_IRWXG);
      mkdir(TMP_DIRECTORY,S_IFDIR|S_IRWXU|S_IRWXG);
    }

    char currentPhoto[TEXTMAX_PHOTO];
    //! Current photo name generation
    sprintf(currentPhoto , "%s-%05d.%s", sceneName, photoIndex, photoFormat);// FIXME strlcpy

    struct sigaction usr_action;
    sigset_t block_mask;

    if (!remoteControl) {
        //~ if ncurseok (not faillback))
        yrc_uiPrintHelp();
        yrc_menuPrint(menu_win, 1);
        yrc_uiPrintLayout ();

        /* Establish the signal handler. */ //FIXME up to camera init ? (2) 
        sigfillset (&block_mask);
        usr_action.sa_handler = forksyncHandler;
        usr_action.sa_mask = block_mask;
        usr_action.sa_flags = 0;
        if (sigaction (SIGUSR1, &usr_action, NULL) != 0) {
            yrc_coloredPrintf(YVONNE_MSG_ERROR, "FATAL! Registring user signal.");
            goto CLOSE_CAMERA;
        }
        //~ ELSE yrc_coloredPrintf (YVONNE_MSG_UI, "Enter command :\n\t- 's' : Start shooting\n\t- 'p' : Pause shooting\n\t- 'v' : Video generation\n\t- 'q' : Quit\n--> ");
    }

    char commandLine[LINE_BUFFER];
    unsigned int shootFailed = 0;
    int startSequence = 1;

    //! here start the (interesting) work
  while (!(yrc_stateField & YRC_STATE_QUIT) && keepRunning) {

    if(remoteControl) {
    //! Listen to the Arduino remote and adjust state
        yrc_stateMachineArduino (&yrc_stateField, fdArduinoModem);
    } else if (TRUE) {
    //! Listen to the local ncurses control and adjust state
        yrc_stateMachineLocal ( &yrc_stateField , menu_win );
    } else { // TODO TEST ME PLEASE !
        yrc_stateMachineLocalFailback (& yrc_stateField);
    }

    if(usr_interrupt) {
        char *renderMsg = renderMessages[RENDER_OK];
        switch (exec_status) {
            case ERROR_NO:
                if (!remoteControl){
                    yrc_uiPrintMessage(msg_win, YVONNE_MSG_VIDEO_BANNER,
                                        "A fresh ## --> %s-%d.avi <-- ## is available to be chewed.",
                                        sceneName, videoIndex);
                } else {
                    yrc_coloredPrintf(YVONNE_MSG_VIDEO_BANNER, "#### -------------------------------------------- ####\n");
                    yrc_coloredPrintf(YVONNE_MSG_VIDEO_BANNER, "####                                              ####\n");
                    yrc_coloredPrintf(YVONNE_MSG_VIDEO_BANNER, "#### A fresh video is available to be chewed      ####\n");
                    yrc_coloredPrintf(YVONNE_MSG_VIDEO_BANNER, "#### ----> %s-%d.avi                   ####\n", sceneName, videoIndex);
                    yrc_coloredPrintf(YVONNE_MSG_VIDEO_BANNER, "#### -------------------------------------------- ####\n");
                }
                videoIndex++;
            break;
            case ERROR_EXEC_COMMAND_FAIL: //FIXME if remotecontrol
                yrc_uiPrintMessage(msg_win, YVONNE_MSG_ERROR, "ERROR during video generation : ffmpeg failed.");
                renderMsg = renderMessages[RENDER_KO];
            case ERROR_EXEC_FAIL: //FIXME if remotecontrol
                yrc_uiPrintMessage(msg_win, YVONNE_MSG_ERROR, "ERROR during video generation call.");
                renderMsg = renderMessages[RENDER_KO];
            break;
            default:
                yrc_uiPrintMessage(msg_win, YVONNE_MSG_ERROR, "UnEXpected ERROR during video generation!");
                renderMsg = renderMessages[RENDER_KO];
            break;
        }
        logLenght = sprintf(logMessage, "VIDEO %s-%.3d.avi\t%s\n", //FIXME snprintf
                            sceneName,
                            videoIndex,
                            renderMsg );
        write(logDescriptor, logMessage, logLenght*sizeof(char));
        usr_interrupt = 0;
    }

    if (!(yrc_stateField & YRC_STATE_VIDEO_PROGRESS) && (yrc_stateField & YRC_STATE_VIDEO))
    { //! Entering in Video generation
    //TODO  block video state re entrance
      yrc_stateField &= ~YRC_STATE_VIDEO; // state reached

      //resize the photo
      //TODO change LOWQUALITY_DIRECTORY to command line parameter

// *************TEST CASE******
// ffmpeg -hide_banner -f image2 -start_number 10 -r 25 -i "tmp/yvonne-%05d.jpg" -q:v 1 -c:v mjpeg -s 1024x576 -aspect 16:9 video/yvonne-001.avi
//***********************
      //INFO 25 f/s because all images are duplicated N time
      sprintf(commandLine, "ffmpeg -hide_banner -f image2 -start_number %d -r 25 -i \"%s/%s-%%05d.%s\" -q:v 1 -c:v mjpeg -s %s -aspect 16:9 %s/%s-%.3d.avi", 
                                                    startSequence,
                                                    LOWQUALITY_DIRECTORY,
                                                    sceneName,
                                                    photoFormat,
                                                    LOWQUALITY_RESOLUTION,
                                                    VIDEO_DIRECTORY,
                                                    sceneName,
                                                    videoIndex);// FIXME snprintf
      if (yrc_ExecuteForked ("video generation", commandLine, NULL) != ERROR_NO) {
        yrc_coloredPrintf(YVONNE_MSG_ERROR, "ERROR during video generation cmd line : %s", commandLine);
      } else {
        yrc_stateField &= YRC_STATE_VIDEO_PROGRESS;

        logLenght = sprintf(logMessage, "VIDEO %s-%.3d.avi\tsrc : %s-%.3d ---> %s-%.3d\tlst photo %s-%d\n", //FIXME snprintf
                          sceneName,
                          videoIndex,
                          sceneName,
                          startSequence,
                          sceneName,
                          sceneLowQualityIndex-1,
                          sceneName,
                          photoIndex-1);
        write(logDescriptor, logMessage, logLenght*sizeof(char));

        startSequence = sceneLowQualityIndex; //FIXME
      }
    }

    //Try to exit now!
    if((yrc_stateField & YRC_STATE_QUIT) || !keepRunning) break;

    if(remoteControl) {
        yrc_coloredPrintf(YVONNE_MSG_INFO, "Current photo : %s", currentPhoto); //TODO MEGA NICE PRINT 
    }else {
        yrc_uiPrintMediaIndex(photoIndex, videoIndex);
    }
    // Shoot and image resize
    if (yrc_stateField & YRC_STATE_PHOTO) {
        if(remoteControl) {
          if(!quiet) yrc_coloredPrintf(YVONNE_MSG_INFO, "Will capture to file %s", currentPhoto);
        }
        bool captureOK = TRUE;
        if (!streamAsSource) { //Catpture from gphoto lib
          // Photoshooting using gphoto
          if(CameraExited) ; //FIXME will solve some ContextError ?
//          if(!quiet) printf("Capturing to file %s\n", currentPhoto);
          // take the photo now!
          if(YvonnePhotoCapture(camera, currentPhoto) == ERROR_NO){
            CameraExited = false;
            shootFailed = 0;
          } else {
            captureOK = FALSE;
            // in local control mode, queue for tty print
            yrc_coloredPrintf(YVONNE_MSG_WARNING, "Photo %s shoot has failed %d time(s)", currentPhoto, ++shootFailed);
            sleep(4);
            // in local control mode, queue for tty print
            yrc_coloredPrintf(YVONNE_MSG_INFO, "Camera init again....");
            YvonnePhotoCaptureUnref(camera);
            if((YvonnePhotoCaptureInit(camera) != ERROR_NO) && (shootFailed >= CAMERA_INIT_ATTEMPT_MAX)) {
                // in local control mode, queue for tty print
                yrc_coloredPrintf(YVONNE_MSG_ERROR, "FATAL! No camera auto detected.");
                yrc_coloredPrintf(YVONNE_MSG_INFO,  "Check battery maybe?");
                goto CLOSE_CAMERA;
            }
            //clear queue
          }
        } else { // Stream has source, capture from video4linux2 device.
            sprintf(commandLine, "ffmpeg -n -hide_banner -t 1 -f video4linux2 -i \"%s\" -f image2 -vframes 1 -s %s %s",
                                sourceName,
                                LOWQUALITY_RESOLUTION,
                                currentPhoto);// FIXME snprintf
            //TODO -y overwrite without ask, optional too
            //TODO -n nerver overwrite : exit
            if ((lastError = yrc_Execute ("stream capture", commandLine)) != ERROR_NO) {
                if(remoteControl) //FIXME error ouptut last error
                    yrc_coloredPrintf(YVONNE_MSG_ERROR, "Executing stream capture : %s .", commandLine);
                else
                    yrc_uiPrintMessage(msg_win, YVONNE_MSG_ERROR, "Executing stream capture : '%s' .", commandLine);
                captureOK = FALSE;
            }
        }

        if (captureOK == TRUE){
            sprintf(filesource, "%s/%s", TMP_DIRECTORY, currentPhoto);// FIXME snprintf
            if(YvonnePhotoResize(currentPhoto, filesource, LOWQUALITY_RESOLUTION_W,LOWQUALITY_RESOLUTION_H) != ERROR_NO) {
              yrc_coloredPrintf(YVONNE_MSG_ERROR, "Resizing image file %s .", filesource);//FIXME error not trapt
            } else {
                // duplicate the lowquality photo to slowdown the video rythm
                unsigned int repeatEachImage;
                // TODO video fps control
                for (repeatEachImage = LOWQUALITY_REPEAT; repeatEachImage > 0 ; repeatEachImage--) {
                  sprintf(filetarget, "%s/%s-%05d.%s", LOWQUALITY_DIRECTORY, sceneName, sceneLowQualityIndex++, photoFormat);// FIXME snprintf
                  if(link(filesource, filetarget)!=0) {
                    yrc_coloredPrintf(YVONNE_MSG_ERROR, "Linking images files .");
                    perror(""); // Print last error
                    pause ();
                  }
                }
                //! Generate next photo name
                photoIndex++;
                sprintf(currentPhoto , "%s-%05d.%s", sceneName, photoIndex, photoFormat);// FIXME strlcpy
            }
        }

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

CLOSE_CONTROL:
    if (remoteControl) {
        YvonneArduinoClose(fdArduinoModem, &oldtio);
    } else {
//        YvonneTerminalRestore(ttysave);
        if(menu_win) yrc_menuClose (menu_win);
        if(msg_win) yrc_errorClose (msg_win);
        yrc_uiRestore ();
    }

CLOSE_LOG:
    logLenght = sprintf(logMessage, "# END of %s's log\n", sceneName);// FIXME snprintf
    write(logDescriptor, logMessage, logLenght*sizeof(char));
    close(logDescriptor);

CLOSE_SCENE_NAME:
    free(sceneName);
    yrc_coloredPrintf(YVONNE_MSG_INFO,"Finally \'%s\' successly close,\nsee you next sh00ting!", argv[0]);
    exit(EXIT_SUCCESS);
}

