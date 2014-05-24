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
#include "yvonne-remote-lib.h"

#include <getopt.h>
#include <error.h>

//TODO ffmpeg static install (from http://ffmpeg.gusari.org/static/) Has option
//TODO dossier basse qualité Has option
//TODO qualité basse qualité Has option
//TODO option ffmpeg Has option

void usage(void)
{
    printf("Usage: yvonne-remote [OPTIONS]\n"
    "\n"
    "Options:\n"
    "  -q  --quiet                Don't print out as much info\n"
    "  -h, --help                 Print this help message and quit\n"
    "  -s, --scene=scenename      Name of the scene (defaut currentdir)\n" 
    "  -p, --port=serialport      Serial port the Arduino is connected to (defaut %s)\n"
    "  -g, --gap=integer          Gap of scene numbering for resumption (optional)\n"
    "  -d  --delay=seconds        Additional Delay between shooting in seconds (default %d)\n"
    "  -b, --baud=rate            Baudrate (bps) of Arduino (default %d)\n"
    "\n"
    "Dependecies: gphoto2, ffmpeg, imagemagick\n"
    "\n", ARDUINO_DEFAULT_PORT, SHOOTING_DEFAULT_DELAY, ARDUINO_DEFAULT_BAUDRATE);
    exit(EXIT_SUCCESS);
}

int main(int argc, char** argv)
{
    // assign defautl value
    int baudrate = ARDUINO_DEFAULT_BAUDRATE;
    char quiet=0;
    int shootingDelay = SHOOTING_DEFAULT_DELAY;
    int sceneIndex = 0;
    char arduinoDeviceName[SERIAL_PORT_MAXLENGHT];
    strcpy(arduinoDeviceName, ARDUINO_DEFAULT_PORT);

    char sceneName [SCENE_NAME_MAXLENGHT];
    strcpy(sceneName,SCENE_DEFAULT_NAME); //TODO getcwd current dir

    char buf[250];
    if (getcwd(buf, 250))
      printf("%s\n", buf);

    exit(EXIT_SUCCESS);      

    /* parse options */
    int option_index = 0, opt;
    static struct option loptions[] = {
        {"help",      no_argument,       0, 'h'},
        {"port",      required_argument, 0, 'p'},
        {"baud",      required_argument, 0, 'b'},
        {"scene",     required_argument, 0, 's'},
        {"gap",       required_argument, 0, 'g'},
        {"delay",     required_argument, 0, 'd'},
        {"quiet",     no_argument,       0, 'q'},
        {NULL,        0,                 0,  0}
    };

    while(1) {
        opt = getopt_long (argc, argv, "hqp:b:s:g:d:",
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
        case 'g':
              sceneIndex = strtol(optarg,NULL,10);
              if(!quiet) printf("Scene numbering start from %d\n",sceneIndex);
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
    printf("Arduino listen from %s\n", arduinoDeviceName);

    if (InitArduinoConnection(fdArduinoModem, baudrate, &oldtio))
    {
        perror("InitArduinoConnection failed!");
        exit(ERROR_GENERIC);
    }

    if(sceneIndex == 0) mkdir(LOWQUALITY_DIRECTORY,S_IFDIR|S_IRWXU|S_IRWXG);

	int charReceived;
	int startSequence = 0;
  int videoIndex = 0;
//TODO malloc?
	char bufArduino[TEXTMAX];
  int i;
  int Stop = 1;
  int Video = 0;  
  char commandLine[LINE_BUFFER];

  //! here start the (interesting) work
  for (i=0 ; i < 10; i++) //TODO WHILE NOTDONE
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
        if(!quiet) printf("String readed : %s CaracteresRecus : %d\n", bufArduino, charReceived);
        if (strncmp(bufArduino, "STOP", charReceived-1) == 0)
            //returnvalue = STOP_ACTION;
            Stop = 1;
        if (strncmp(bufArduino, "START", charReceived-1) == 0)
            //returnvalue = START_ACTION;
            Stop = 0;
        if (strncmp(bufArduino, "VIDEO", charReceived-1) == 0)
            Video = 1;
            //returnvalue = VIDEO_ACTION;
        if (strncmp(bufArduino, "SPEED1", charReceived-1) == 0)
            ; //TODO
            //returnvalue = SPEED1_ACTION;

        // clean the buffer
        memset(bufArduino, '\0', charReceived);   
    }

    //Execute some shell command

    //For video generation
    if (Video)
    {
      //resize the photo 
      //TODO change LOWQUALITY_DIRECTORY to command line parameter
      //INFO -vframes n or -frames:v to control the quantity of frames
      sprintf(commandLine, "%sffmpeg -f image2 -start_number %d -r 6 -i \"./%s/%s-%%05d.jpg\" -q:v 1 -s %s %s-%d.mpg", FFMPEG_STATIC_BUILD_INSTALL, startSequence, LOWQUALITY_DIRECTORY, sceneName, LOWQUALITY_RESOLUTION, sceneName, videoIndex);
      if (ExecuteCommandLine ("video generation", commandLine) != ERROR_NO)
        ;//do something

      startSequence = sceneIndex;
      videoIndex++;

      Video = 0;
    }

    // to command the shooting and resize the image
    if (!Stop)
    {
      //take the photo
      //TODO use direclty libgphoto2?
      sprintf(commandLine, "gphoto2 --capture-image-and-download -F 1 -I %d --filename ./%s-%05d.jpg", shootingDelay, sceneName, sceneIndex);
      if (ExecuteCommandLine ("capture", commandLine) != ERROR_NO)
        ;//do something    

      //resize the photo
      sprintf(commandLine, "mogrify -resize %s -path %s ./%s-%05d.jpg", LOWQUALITY_RESOLUTION, LOWQUALITY_DIRECTORY, sceneName, sceneIndex);
      if (ExecuteCommandLine ("resize", commandLine) != ERROR_NO)
        ;//do something

      sceneIndex++;
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

