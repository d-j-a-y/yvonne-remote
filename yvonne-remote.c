/*****************************************************
* yvonne-remote - pre-release 0.alpha
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
*
*****************************************************/

#include "yvonne-remote.h"
#include "ansi-colors.h"
#include "yvonne-remote-lib.h"

#include <getopt.h>
#include <error.h>

//TODO ffmpeg static install (from http://ffmpeg.gusari.org/static/) Has option
//TODO dossier basse qualité Has option
//TODO qualité basse qualité Has option
//TODO option ffmpeg Has option
//TODO test de presence d'un appareil photo

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

    char buf[250];
    if (getcwd(buf, 250))
      printf("%s\n", buf);

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
            if(strlen(optarg)+1 <= SERIAL_PORT_MAXLENGHT)
            {
              strcpy(arduinoDeviceName, optarg);
              if(!quiet) printf("Arduino port set to %s\n",arduinoDeviceName);
            }
            else
            {
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
            if(strlen(optarg)+1 <= SCENE_NAME_MAXLENGHT)
            {
              strcpy(sceneName, optarg);
              if(!quiet) printf("Scene name set to %s\n",sceneName);
            }
            else
            {
              error(0, 0, "Can't assign Scene name to %s (buffer lenght reached)\n",optarg);
              exit(ERROR_GENERIC);
            }
            break;
        }
    }

    //! establish the connection with the arduino (mega2560) remote
    int fdArduinoModem;
    struct termios oldtio;

    if((fdArduinoModem = OpenArduinoConnection(arduinoDeviceName)) == ERROR_GENERIC)
    {
        error(0, 0, "OpenArduinoConnection failed at %s",arduinoDeviceName);
        exit(ERROR_GENERIC);
    }
    printf(ANSI_COLOR_CYAN "Arduino listen from %s\n" ANSI_COLOR_RESET, arduinoDeviceName);

    if (InitArduinoConnection(fdArduinoModem, baudrate, &oldtio))
    {
        perror("InitArduinoConnection failed!");
        exit(ERROR_GENERIC);
    }

    if(photoIndex == 0) mkdir(LOWQUALITY_DIRECTORY,S_IFDIR|S_IRWXU|S_IRWXG);
    
    mkdir("video",S_IFDIR|S_IRWXU|S_IRWXG);
    mkdir("tmp",S_IFDIR|S_IRWXU|S_IRWXG);       

	int charReceived;
	int startSequence = 0;
//TODO malloc?
	char bufArduino[TEXTMAX];
//TODO State Mask	
  int StateStop = 1;
  int StateVideo = 0;
  int StateQuit = 0;
  char commandLine[LINE_BUFFER];
  char currentPhoto[TEXTMAX_PHOTO];
  
  int sceneLowQualityIndex = 0;
  
  char* stopIndex = 0;
  char* startIndex = 0;
  

  //! here start the (interesting) work
  while (!StateQuit)
  {
    //Listen to the Arduino remote
    charReceived = read(fdArduinoModem,bufArduino,TEXTMAX);

    if (charReceived == -1 && errno == EAGAIN)
    {
        if(!quiet) printf("Nothing to read from Arduino\n");
    }
    else
    {
        bufArduino[charReceived]='\0';
        if(!quiet) printf("Arduino string readed : %s CaracteresRecus : %d\n", bufArduino, charReceived);
        
        if (charReceived)
        {
          StateVideo = (strstr(bufArduino, "VIDEO") ? 1 : 0);
          StateQuit = (strstr(bufArduino, "QUIT") ? 1 : 0);          

//TODO          Speed = (strstr(bufArduino, "SPEED1") ? 1 : 0);          
          
          stopIndex = strstr_last(bufArduino, "STOP");
          startIndex = strstr_last(bufArduino, "START");
          
          if (startIndex || stopIndex)
            StateStop = (startIndex > stopIndex) ? 0 : 1;
            
          // clean the buffer
          memset(bufArduino, '\0', charReceived);            
        }  
    }

    //Generate current photo name
    sprintf(currentPhoto , "./%s-%05d.jpg", sceneName, photoIndex);
    printf("current photo : %s \n", currentPhoto);
    
    //Execute some shell command
    
    //For Video generation
    if (StateVideo)
    {
      //resize the photo 
      //TODO change LOWQUALITY_DIRECTORY to command line parameter
      //INFO -vframes n or -frames:v to control the quantity of frames
      
      //INFO 25 f/s because all images are duplicated N time
      sprintf(commandLine, "%sffmpeg -f image2 -start_number %d -r 25 -i \"./%s/%s-%%05d.jpg\" -q:v 1 -vcodec mjpeg -s %s ./video/%s-%d.avi", FFMPEG_STATIC_BUILD_INSTALL, startSequence, LOWQUALITY_DIRECTORY, sceneName, LOWQUALITY_RESOLUTION, sceneName, videoIndex);
      if (ExecuteCommandLineForked ("video generation", commandLine) != ERROR_NO)
      {
          printf(ANSI_COLOR_RED "ERROR during video generation cmd line : %s \n" ANSI_COLOR_RESET, commandLine);//TODO do something better
      }
      else
      {
          printf (ANSI_COLOR_GREEN "#### -------------------------------------------- ####\n" ANSI_COLOR_RESET);
          printf (ANSI_COLOR_GREEN "####                                              ####\n" ANSI_COLOR_RESET);
          printf (ANSI_COLOR_GREEN "#### A fresh video will be available to be chewed ####\n" ANSI_COLOR_RESET);
          printf (ANSI_COLOR_GREEN "#### ----> %s-%d.avi                   ####\n" ANSI_COLOR_RESET, sceneName, videoIndex);
          printf (ANSI_COLOR_GREEN "#### -------------------------------------------- ####\n" ANSI_COLOR_RESET);
      }

      startSequence = sceneLowQualityIndex;
      videoIndex++;

      StateVideo = 0;
    }
    
    //Exit now!
    if(StateQuit) continue;    

    // to command the shooting and resize the image
    if (!StateStop)
    {
      //take the photo
      //TODO use direclty libgphoto2?
      //CURRENTPHOTO - sprintf(commandLine, "gphoto2 --capture-image-and-download -F 1 -I %d --filename ./%s-%05d.jpg", shootingDelay, sceneName, photoIndex);
      sprintf(commandLine, "gphoto2 --capture-image-and-download -F 1 -I %d --filename %s", shootingDelay, currentPhoto);     
      if (ExecuteCommandLine ("capture", commandLine) != ERROR_NO)
      {
          printf("ERROR during capture cmd line : %s", commandLine);//TODO do something better
      }

      //test file creation
      if (open(currentPhoto, O_RDONLY) == -1)
      {
          //don't do photo's modification, do not increase photo's photoIndex
          printf("Photo %s shoot has failed.\n", currentPhoto);
      }
      else
      {

  /*
        //resize the photo
        sprintf(commandLine, "mogrify -resize %s -path %s ./%s-%05d.jpg", LOWQUALITY_RESOLUTION, LOWQUALITY_DIRECTORY, sceneName, photoIndex);
        if (ExecuteCommandLine ("resize", commandLine) != ERROR_NO)
          ;//TODO do something          
        
  */      
        //resize the photo
        //CURRENTPHOTO - sprintf(commandLine, "mogrify -resize %s -path %s ./%s-%05d.jpg", LOWQUALITY_RESOLUTION, "./tmp", sceneName, photoIndex);
        sprintf(commandLine, "mogrify -resize %s -path %s %s", LOWQUALITY_RESOLUTION, "./tmp", currentPhoto);      
        if (ExecuteCommandLine ("resize", commandLine) != ERROR_NO)
        {
            printf("ERROR during resize cmd line : %s", commandLine);//TODO do something better
        }
             // TODO TODO TODO TODO TODO TODO TODO TODO 
        int repeatEachImage = 5;        
        for (repeatEachImage = 5; repeatEachImage > 0 ; repeatEachImage--)
        {        
          //duplicate the lowquality photo to slowdown the video rythm
          //CURRENTPHOTO - sprintf(commandLine, "cp %s/%s-%05d.jpg %s/%s-%05d.jpg", "./tmp", sceneName, photoIndex, LOWQUALITY_DIRECTORY, sceneName, sceneLowQualityIndex++);
          sprintf(commandLine, "cp %s/%s %s/%s-%05d.jpg", "./tmp", currentPhoto, LOWQUALITY_DIRECTORY, sceneName, sceneLowQualityIndex++);
          
          if (ExecuteCommandLine ("duplicate", commandLine) != ERROR_NO)
          {
              printf("ERROR during duplicate cmd line : %s", commandLine);//TODO do something better
          }
        }

        photoIndex++;
      }
    }    
    else
    {
      if(!quiet) printf("Shooting paused\n");
      sleep(3);
    }
//    sleep(1);
  }

    CloseArduinoConnection(fdArduinoModem, &oldtio);

    exit(EXIT_SUCCESS);
}

