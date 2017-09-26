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

#include "ansi-colors.h"

#include <getopt.h>
#include <error.h>
#include <stdbool.h>
#include <signal.h>

//TODO dossier basse qualité Has option
//TODO qualité basse qualité Has option
//TODO autoselect ffmpeg/avconv / + perso install (ffmpeg static install (from http://ffmpeg.gusari.org/static/))
//TODO test de presence d'un appareil photo
//TODO video fps option
//TODO void print_error(const char * errorstr,...) {printf(ANSI_COLOR_RED errorstr ANSI_COLOR_RESET
//TODO ctrl-z
//TODO warning error print
//TODO resume if log exist ?

//FIXME low battery programme hang ! ??? ---> STATE STOP +++ WARNING !!!!
/*
*** Contexterror ***              
PTP Store Not Available
Pathname on the camera: <�	��kMӳ�TQ���:��TR/kz�eiAQX!lg$g^�?�~̌����R��$`��uϪ�%T(=�F(xԺ�Vc������@T
                         �*6`y���H:�ę�����Y1�H�|������&��P�W#"���V�-Q�iX���ȑ$;ym�<�	��kMӳ�TQ���:��TR

*** Contexterror ***              
The path '<�	��kMӳ�TQ���:��TR' is not absolute.
Deleting

*** Contexterror ***              
The path '<�	��kMӳ�TQ���:��TR' is not absolute.
Wait for events from camera
yvonne-remote-lib.c YvonnePhotoResize 393 insufficient image data in file `./yvonne-00002.jpg' @ error/jpeg.c/ReadJPEGImage/1039
*/
//FIXME camera setting unavailable - gphoto release cam between capture ?
//FIXME configuration arduino pb !!! pb baud ?
//FIXME : en reprise (-f -v) la premiere video genere contient des anciennes images : pour reproduire, faire un shooting normal (20f) et 3 video. puis stop puis reprise -f 21 - v 4 , 


void usage(void)
{
    printf("Usage: yvonne-remote [OPTIONS]\n"
    "\n"
    "Options:\n"
    "  -q  --quiet                Don't print out as much info\n"
    "  -h, --help                 Print this help message and quit\n"
    "  -s, --scene=scenename      Name of the scene (defaut currentdir)\n" 
    "  -p, --port=serialport      Serial port the Arduino is connected to (defaut %s)\n"
    "  -f, --fotogap=integer      Gap of scene (photo) numbering for resumption (optional)\n"
    "  -v, --videogap=integer     Gap of video numbering for resumption (optional)\n"    
    "  -d  --delay=seconds        Delay between two shoot in seconds (default %d)\n"
    "  -b, --baud=rate            Baudrate (bps) of Arduino (default %d)\n"
    "\n"
    "Dependecies: gphoto2, ffmpeg, imagemagick\n"
    "\n", ARDUINO_DEFAULT_PORT, SHOOTING_DEFAULT_DELAY, ARDUINO_57600_BAUDRATE);
    exit(EXIT_SUCCESS);
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
    int videoIndex = 1;    
    char arduinoDeviceName[SERIAL_PORT_MAXLENGHT];
    strcpy(arduinoDeviceName, ARDUINO_DEFAULT_PORT);

    char sceneName [SCENE_NAME_MAXLENGHT];
    strcpy(sceneName,SCENE_DEFAULT_NAME); //TODO getcwd current dir

    char logMessage [LOG_MAXLENGHT];
    int logLenght = 0;

    char filesource	[256];
    char filetarget	[256];

    Camera *gpcamera;
    GPContext *gpcontext;

    char buf[CWD_MAXLENGHT];
    if (getcwd(buf, CWD_MAXLENGHT)) {
        printf("%s\n", buf);
    }
    else { //FIXME allocate ?
        error(0, 0, ANSI_COLOR_RED "ERROR get current working dir" ANSI_COLOR_RESET);
        perror("");
        exit(ERROR_GENERIC);
    }

    /* parse options */
    int option_index = 0, opt;
    static struct option loptions[] = {
        {"help",      no_argument,       0, 'h'},
        {"port",      required_argument, 0, 'p'},
        {"baud",      required_argument, 0, 'b'},
        {"scene",     required_argument, 0, 's'},
        {"fotogap",   required_argument, 0, 'f'},
        {"videogap",  required_argument, 0, 'v'},        
        {"delay",     required_argument, 0, 'd'},
        {"quiet",     no_argument,       0, 'q'},
        {NULL,        0,                 0,  0}
    };

    while(1) {
        opt = getopt_long (argc, argv, "hqp:b:s:v:f:d:",
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
                break;
            case 'b':
                baudrate = strtol(optarg,NULL,10);
                if( !quiet ) printf("Baudrate (bps) of Arduino set to %d\n",baudrate);
                break;
            case 'p':
                if(strlen(optarg)+1 <= SERIAL_PORT_MAXLENGHT) {
                  strcpy(arduinoDeviceName, optarg);
                  if(!quiet) printf("Arduino port set to %s\n",arduinoDeviceName);
                }
                else {
                  error(0, 0, "Can't assign Arduino serial port to %s (buffer lenght reached)\n",optarg);
                  exit(ERROR_GENERIC);
                }
                break;
            case 'f':
                  photoIndex = strtol(optarg,NULL,10);
                  if(!quiet) printf("Scene numbering start from %d\n",photoIndex);
                break;
            case 'v':
                  videoIndex = strtol(optarg,NULL,10);
                  if(!quiet) printf("Video numbering start from %d\n",videoIndex);
                break;
            case 's':
                if(strlen(optarg)+1 <= SCENE_NAME_MAXLENGHT) {
                  strcpy(sceneName, optarg);
                  if(!quiet) printf("Scene name set to %s\n",sceneName);
                }
                else {
                  error(0, 0, "Can't assign Scene name to %s (buffer lenght reached)\n",optarg);
                  exit(ERROR_GENERIC);
                }
                break;
        }
    }

    char logFile[]="yvonne.log";
    int logDescriptor;
    if ((logDescriptor=open(logFile, O_RDWR|O_CREAT|O_EXCL)) == -1) {
        error(0, 0, ANSI_COLOR_RED "ERROR can't create logfile \"%s\"" ANSI_COLOR_RESET,logFile);
        perror("");
        exit(ERROR_GENERIC);
    }
    fchmod(logDescriptor, S_IRUSR|S_IWUSR);
    logLenght = sprintf(logMessage, "BEGIN of %s's log\n", sceneName);
    write(logDescriptor, logMessage, logLenght*sizeof(char));

    signal(SIGINT, intHandler);

    //! establish the connection with the arduino (mega2560) remote
    int fdArduinoModem;
    struct termios oldtio;

    if((fdArduinoModem = YvonneArduinoOpen(arduinoDeviceName)) == ERROR_GENERIC) {
        error(0, 0, ANSI_COLOR_RED "OpenArduinoConnection failed at %s"  ANSI_COLOR_RESET,arduinoDeviceName);
        perror("");
        exit(ERROR_GENERIC);
    }
    printf(ANSI_COLOR_CYAN "Arduino listen from %s\n" ANSI_COLOR_RESET, arduinoDeviceName);

    if (YvonneArduinoInit(fdArduinoModem, baudrate, &oldtio)) {
        perror("InitArduinoConnection failed!");
        exit(ERROR_GENERIC);
    }

//    YvonnePhotoCaptureInit(gpcamera, gpcontext);

    gp_camera_new (&gpcamera);
    gpcontext = gp_context_new();

    // set callbacks for camera messages
    gp_context_set_error_func(gpcontext, YvonnePhotoCaptureError, NULL);
    gp_context_set_message_func(gpcontext, YvonnePhotoCaptureMessage, NULL);

    //This call will autodetect cameras, take the first one from the list and use it
    printf("Camera init. Can take more than 10 seconds depending on the "
    "memory card's contents (remove card from camera to speed up).\n");
    int ret = gp_camera_init(gpcamera, gpcontext);
    if (ret < GP_OK) {
        gp_camera_free(gpcamera);
        printf(ANSI_COLOR_RED "No camera auto detected. Maybe check the battery ?\n" ANSI_COLOR_RESET);
        goto CLOSE_ARDUINO; //FIXME goto
    }
    //give other access to the camera
    gp_camera_exit (gpcamera,gpcontext);
    bool CameraExited = true;

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

    int sceneLowQualityIndex = 1;

    char* stopIndex = 0;
    char* startIndex = 0;

    unsigned int shootFailed = 0;

    //! here start the (interesting) work
    while (!StateQuit && keepRunning) {
    //Listen to the Arduino remote
    charReceived = read(fdArduinoModem,bufArduino,TEXTMAX);

    if (charReceived == -1 && errno == EAGAIN) {
        if(!quiet) printf("Nothing to read from Arduino\n");
    }
    else {
        bufArduino[charReceived]='\0';
        if(!quiet) printf("Arduino string readed : %s CaracteresRecus : %d\n", bufArduino, charReceived);

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

    //Generate current photo name
    sprintf(currentPhoto , "./%s-%05d.jpg", sceneName, photoIndex);
    //Print info to output
    if(!quiet || !StateStop)
        printf(ANSI_COLOR_CYAN "current photo : %s \n" ANSI_COLOR_RESET, currentPhoto);

    //For Video generation
    if (StateVideo)
    {
        //resize the photo
        //TODO change LOWQUALITY_DIRECTORY to command line parameter
        //INFO -vframes n or -frames:v to control the quantity of frames

        //INFO 25 f/s because all images are duplicated N time
        sprintf(commandLine, "ffmpeg -f image2 -start_number %d -r 25 -i \"%s/%s-%%05d.jpg\" -q:v 1 -vcodec mjpeg -s %s -aspect 16:9 %s/%s-%d.avi", startSequence, LOWQUALITY_DIRECTORY, sceneName, LOWQUALITY_RESOLUTION, VIDEO_DIRECTORY, sceneName, videoIndex);
        if (YvonneExecuteForked ("video generation", commandLine) != ERROR_NO) {
            printf(ANSI_COLOR_RED "ERROR during video generation cmd line : %s \n" ANSI_COLOR_RESET, commandLine);//TODO do something better
        }
        else {
            printf (ANSI_COLOR_GREEN "#### -------------------------------------------- ####\n" ANSI_COLOR_RESET);
            printf (ANSI_COLOR_GREEN "####                                              ####\n" ANSI_COLOR_RESET);
            printf (ANSI_COLOR_GREEN "#### A fresh video will be available to be chewed ####\n" ANSI_COLOR_RESET);
            printf (ANSI_COLOR_GREEN "#### ----> %s-%d.avi                   ####\n" ANSI_COLOR_RESET, sceneName, videoIndex);
            printf (ANSI_COLOR_GREEN "#### -------------------------------------------- ####\n" ANSI_COLOR_RESET);
        }

        logLenght = sprintf(logMessage, "VIDEO %s-%d.avi : %d ---> %d\n", sceneName, videoIndex,startSequence, sceneLowQualityIndex);
        write(logDescriptor, logMessage, logLenght*sizeof(char));

        startSequence = sceneLowQualityIndex;
        videoIndex++;

        StateVideo = 0;
    }

    //Exit now!
    if(StateQuit || !keepRunning) continue;

    // to command the shooting and resize the image
    if (!StateStop)
    {
      if(CameraExited) ; //FIXME will solve some ContextError ?
      //take the photo
      //TODO use direclty libgphoto2?
      //CURRENTPHOTO - sprintf(commandLine, "gphoto2 --capture-image-and-download -F 1 -I %d --filename ./%s-%05d.jpg", shootingDelay, sceneName, photoIndex);
      //IFDEF GPHOTOCMD
    //      sprintf(commandLine, "gphoto2 --capture-image-and-download -F 1 -I %d --filename %s", shootingDelay, currentPhoto);
    //      if (YvonneExecute ("capture", commandLine) != ERROR_NO)
    //        printf(ANSI_COLOR_RED "ERROR during capture cmd line : %s" ANSI_COLOR_RESET, commandLine);//TODO do something better

      printf("Capturing to file %s\n", currentPhoto);
      if(YvonnePhotoCapture(gpcamera, gpcontext, currentPhoto) != ERROR_NO){
        printf(ANSI_COLOR_YELLOW "Photo %s shoot has failed %d time(s).\n"  ANSI_COLOR_RESET, currentPhoto, ++shootFailed);
        sleep(1);
        if(shootFailed >= MAX_SHOOT_RETRY_BEFORE_INIT){
          printf("Camera init again....\n");
          int ret = gp_camera_init(gpcamera, gpcontext);
          if (ret < GP_OK) {
              printf(ANSI_COLOR_RED "ERROR : No camera auto detected. Check battery maybe?\n" ANSI_COLOR_RESET);
              gp_camera_free(gpcamera);
              goto CLOSE_CAMERA;
          }
        }
      }
      else {
        CameraExited = false;
        shootFailed = 0;
        if (open(currentPhoto, O_RDONLY) == -1) {
          printf(ANSI_COLOR_YELLOW "Photo %s is missing, shoot has failed ?\n" ANSI_COLOR_RESET, currentPhoto);
        }
        else {
          sprintf(filesource, "%s/%s", TMP_DIRECTORY, currentPhoto);

          if(YvonnePhotoResize(currentPhoto, filesource, LOWQUALITY_RESOLUTION_W,LOWQUALITY_RESOLUTION_H) != ERROR_NO)
            printf(ANSI_COLOR_RED "ERROR file resize" ANSI_COLOR_RESET);

          //duplicate the lowquality photo to slowdown the video rythm
          int repeatEachImage = 5;
          // TODO video fps control
          for (repeatEachImage = 5; repeatEachImage > 0 ; repeatEachImage--) {
            sprintf(filetarget, "%s/%s-%05d.jpg", LOWQUALITY_DIRECTORY, sceneName, sceneLowQualityIndex++);
            if(YvonneFileCopyBin (filesource, filetarget) != ERROR_NO)
              printf(ANSI_COLOR_RED "ERROR during duplicate file" ANSI_COLOR_RESET);
          }
          photoIndex++;
        }
      }
    }
    else {
      if(!CameraExited){
        gp_camera_exit (gpcamera,gpcontext);
        CameraExited = true;
      }
      if(!quiet) printf("Shooting paused\n");
      sleep(3);
    }
    //    sleep(1);
    }

CLOSE_CAMERA:
//FIXME
//    YvonnePhotoCaptureUnref(gpcamera, gpcontext);
    // close camera
    gp_camera_unref(gpcamera);
    gp_context_unref(gpcontext);

CLOSE_ARDUINO:
    YvonneArduinoClose(fdArduinoModem, &oldtio);

//CLOSE_LOG:
    //close log
    logLenght = sprintf(logMessage, "END of %s's log\n", sceneName);
    write(logDescriptor, logMessage, logLenght*sizeof(char));
    close(logDescriptor);
    printf(ANSI_COLOR_CYAN "\nSuccessly close, see you next shot!\n" ANSI_COLOR_RESET);
    exit(EXIT_SUCCESS);
}

