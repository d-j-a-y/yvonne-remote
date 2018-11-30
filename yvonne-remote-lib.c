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

#include "yvonne-remote.h"
#include "yvonne-remote-lib.h"
#include "yvonne-remote-ui.h"

char bufArduino[ARDUINO_TEXTMAX]; // FIXME malloc;

/**
 *  YvonneTerminalInit
 *  @param ttysave pointer to a terminal state structure
 *  @return Error Code
 *
 *  Setup the terminal character mode :no echo, canonical read
 */
int YvonneTerminalInit (struct termios * ttysave)
{
    struct termios ttystate;
    //get the terminal state
    tcgetattr(STDIN_FILENO, &ttystate);
    memcpy(ttysave, &ttystate, sizeof(struct termios));
    //turn off canonical mode and echo
    ttystate.c_lflag &= ~(ICANON | ECHO);
    //minimum of number input read.
    ttystate.c_cc[VMIN] = 1;
    //set the terminal attributes.
    tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);
    return ERROR_NO;
}

/**
 *  YvonneTerminalRestore
 *  @param ttysave terminal state structure
 *  @return Error Code
 *
 *  Restore the terminal character mode
 */
int YvonneTerminalRestore (struct termios ttysave)
{
    //set the terminal saved attributes.
    tcsetattr(STDIN_FILENO, TCSANOW, &ttysave);
    return ERROR_NO;
}

/**
 *  YvonneArduinoOpen
 *  @param strArduinoPort  ( /dev/ttyACM0 )
 *  @return Arduino Arduino file descriptor or ERROR_GENERIC
 *
 *  Open the connection with the Arduino
 */
int YvonneArduinoOpen (char* strArduinoPort)
{
  int fd;

/*
Open modem device for reading and writing and not as controlling tty
because we don't want to get killed if linenoise sends CTRL-C.
*/
//    fd = open(strArduinoPort, O_RDONLY | O_NOCTTY | O_NDELAY);
  fd = open(strArduinoPort, O_RDONLY | O_NONBLOCK);
  if(fd < 0) {
    return ERROR_GENERIC;
  }

  return fd;
}

/**
 *  YvonneArduinoInit
 *  @param fd Arduino file descriptor
 *  @param baudrate Arduino baudrate (bps)
 *  @param oldtio Connection parameters backup
 *  @return ERROR_NO if ok
 *
 *  Set the connection parameters (speed, binary, stop flag...) of the Arduino
 */
int YvonneArduinoInit (int fd, int baudrate, struct termios* oldtio)
{
  struct termios newtio;

  tcgetattr(fd,oldtio); // Save current serial port settings
  bzero(&newtio, sizeof(newtio)); // Clear struct for new port settings

  /*
  BAUDRATE: Set bps rate. You could also use cfsetispeed and cfsetospeed.
  CRTSCTS : output hardware flow control (only used if the cable has
          all necessary lines. See sect. 7 of Serial-HOWTO)
  CS8     : 8n1 (8bit,no parity,1 stopbit) : Arduino default
  CS7     : 7n1 (7bit,no parity,1 stopbit)
  CLOCAL  : local connection, no modem contol
  CREAD   : enable receiving characters
  */

  // No parity (8N1) Arduino default
  newtio.c_cflag &= ~PARENB;  //No parity bit
  newtio.c_cflag &= ~CSTOPB;  //1 stop bit
  newtio.c_cflag &= ~CSIZE;   //No bit mask for data bits
  newtio.c_cflag |= CS8;      //8 data bits

/* baudrate settings are defined in <asm/termbits.h>, which is
included by <termios.h> */    
  speed_t brate = baudrate; // let you override switch below if needed
  switch(baudrate) {
  case 4800:   brate=B4800;   break;
  case 9600:   brate=B9600;   break;
#ifdef B14400
  case 14400:  brate=B14400;  break;
#endif
  case 19200:  brate=B19200;  break;
#ifdef B28800
  case 28800:  brate=B28800;  break;
#endif
  case 38400:  brate=B38400;  break;
  case 57600:  brate=B57600;  break;
  case 115200: brate=B115200; break;
  }
  newtio.c_cflag |= brate;

  newtio.c_cflag &= ~CRTSCTS; //No flow control
  newtio.c_cflag |= CLOCAL; //Local line - do not change "owner" of port
  newtio.c_cflag |= CREAD; //Enable receiver

  //Disable software flow control (outgoing)
  //Disable software flow control (incoming)
  //Forbib any character to start flow again
  newtio.c_iflag &= ~(IXON | IXOFF | IXANY);

  newtio.c_oflag &= ~OPOST; //Raw output

  //Disable canonical input : raw
  //Disable echoing of input characters
  //Disable Echo erase character as BS-SP-BS
  //Disable SIGINTR, SIGSUSP, SIGDSUSP, and SIGQUIT signals
  newtio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

  newtio.c_cc[VTIME]    = 0;  //Don't wait for data
  newtio.c_cc[VMIN]     = 0;  //No blocking read

  if (tcsetattr(fd,TCSANOW,&newtio) == -1){
    perror("Error setting serial attributes : ");
    return ERROR_GENERIC;
  }

  // Wait a little before flushing the line
  usleep(200000);
  // Now clean the modem line and activate the settings for the port
  tcflush(fd, TCIOFLUSH);

  memset(bufArduino, '\0', ARDUINO_TEXTMAX);

  return ERROR_NO;
}

/**
 *  YvonneArduinoClose
 *  @param fd Arduino file descriptor
 *  @param oldtio Connection parameters to restore
 *
 *  Close the connection with the Arduino
 */
void YvonneArduinoClose (int fd, struct termios* oldtio)
{
  /* restore the old port settings */
  tcsetattr(fd,TCSANOW,oldtio);
  /* close the file */
  close(fd);
}

/**
 *  yrc_stateMachineArduino
 *  @param pointer to int for bit field
 *  @param Arduino file descriptor @see YvonneArduinoOpen
 *
 *  Update the state from COM/Arduino message stupid protocol
 */
void yrc_stateMachineArduino (int* yrc_stateField, int fdArduinoModem) {
    char* stopIndex = 0;
    char* startIndex = 0;

    ssize_t charReceived = read(fdArduinoModem,bufArduino,ARDUINO_TEXTMAX); //FIXME sizeof bufArduino

    if (charReceived == 0 || (charReceived <= -1 && errno == EAGAIN)) {
      if(!TRUE) printf("Nothing to read from Arduino\n"); //FIXME quiet
    }
    else {
      bufArduino[(unsigned char )charReceived]='\0'; //always > 0
      if(!TRUE) printf("Arduino string readed : %s - %ld\n", bufArduino, charReceived); // FIXME quiet

      if (charReceived) {
        //~ StateVideo = (strstr(bufArduino, "VIDEO") ? 1 : 0);
        //~ StateQuit = (strstr(bufArduino, "QUIT") ? 1 : 0);
        (*yrc_stateField) |= (strstr(bufArduino, "VIDEO") ? YRC_STATE_VIDEO : 0);
        (*yrc_stateField) |= (strstr(bufArduino, "QUIT") ? YRC_STATE_QUIT : 0);
//TODO          Speed = (strstr(bufArduino, "SPEED1") ? 1 : 0);
        stopIndex = strstr_last(bufArduino, "STOP");
        startIndex = strstr_last(bufArduino, "START");

        if (startIndex || stopIndex) {
            if (startIndex > stopIndex) {
                (*yrc_stateField) |= YRC_STATE_PHOTO;
            } else {
                (*yrc_stateField) &= ~YRC_STATE_PHOTO;
            }
        }

        memset(bufArduino, '\0', charReceived);
      }
    }
}

/**
 *  YvonneGetSceneName
 *  @return NULL terminates string
 *
 *  Return the scene name from the current directory. This function calls malloc(),
 *  caller is responsible for freeing the memory.
 */
char* YvonneGetSceneName()
{
  char *currdir = NULL;
  char *sceneName = NULL;

  char *buf=NULL;
  if ((buf = getcwd(NULL, 0))) {
    currdir = strrchr(buf,'/')+1;
    sceneName = malloc(strlen(currdir)*sizeof(char));
    strcpy(sceneName, currdir);
    free(buf);
  }

  return sceneName;
}


/**
 *  YvonneExecute
 *  @param strCommandName Name of the Command line to execute inside a shell
 *  @param strCommandLine Command line to execute inside a shell
 *  @return ERROR_NO if ok, ERROR if the pipe can't be open //FIXME false true
 *
 *  Execute the command line
 */
int YvonneExecute (char* strCommandName, char* strCommandLine)
{
  FILE *pipe;
  //get a pipe for the command line
  pipe = popen(strCommandLine, "r");
  if(pipe == NULL) {
    perror("pipe :");
    return ERROR_GENERIC;
  }

  char outputLine[LINE_BUFFER];
  int linenr=1;
  //read the command line output  from the pipe line by line
  while (fgets(outputLine, LINE_BUFFER, pipe) != NULL) {
    printf("%s n°%d : %s",strCommandName, linenr, outputLine);
    ++linenr;
  }

  //close the pipe
  pclose(pipe);

  return ERROR_NO;
}

/**
 *  YvonneExecuteForked
 *  @param strCommandName, name of the Command line to execute inside a shell
 *  @param strCommandLine, Command line to execute inside a shell
 *  @return ERROR_NO if ok, ERROR if the pipe can't be open //FIXME false true
 *
 *  Execute the command line in a child process and exit
 */
int YvonneExecuteForked (char* strCommandName, char* strCommandLine)
{
  FILE *pipe;
  pid_t process_id;

  process_id = fork();
  //if forked with success return, if fail or child : follow
  if(process_id && process_id != -1) return ERROR_NO;

  //get a pipe for the command line
  pipe = popen(strCommandLine, "r");
  if(pipe == NULL) {
    perror("pipe :");
    return ERROR_GENERIC;
  }

  char outputLine[LINE_BUFFER];
  int linenr=1;
  //read the command line output  from the pipe line by line
  while (fgets(outputLine, LINE_BUFFER, pipe) != NULL) {
    printf("%s n°%d : %s",strCommandName, linenr, outputLine);
    ++linenr;
  }

  //close the pipe
  pclose(pipe);

  if(process_id == 0)     exit(EXIT_SUCCESS);
  return ERROR_NO;
}

/**
 *  strstr_last
 *  @param str1 The string to search in
 *  @param str2 The substring to find
 *  @return If substring found, pointer to the last substring. 0 if not found
 *
 *  Locate in a string the last occurence of substring
 */
char* strstr_last (const char* str1, const char* str2)
{
  char* strp;
  char* strp2;  
  int len1, len2;

  len2 = strlen(str2);
  if(len2==0)
    return (char*)str1;

  len1 = strlen(str1);
  if(len1 - len2 <= 0) {
    if(strcmp(str1,str2)==0)
        return (char*)str1;   
    return 0;
  }
  
  // get the first occurence if exist
  strp2 = strstr(str1, str2);
  if(!strp2)
    return 0;
    
  strp = (char*)(str1 + len1 - len2);
  while(strp != str1) {
    if(*strp == *str2) {
      if(strncmp(strp,str2,len2)==0)
        return strp;
    }
    strp--;
  }
  return strp2;
}

/**
 *  YvonnePhotoResize
 *  @param filesource File name to resize
 *  @param filetarget Resized file name
 *  @param width Target file width
 *  @param height Target file height
 *  @return ERROR_NO if ok, ERROR if the file can't be resized //FIXME false true
 *
 *  Resize an image to desired size (code from imagickwand example)
 */
int YvonnePhotoResize (char* filesource, char* filetarget, long width, long height){
#define ThrowWandException(wand) \
{ \
  char \
    *description; \
 \
  ExceptionType \
    severity; \
 \
  description=MagickGetException(wand,&severity); \
  (void) fprintf(stderr,"%s %s %lu %s\n",GetMagickModule(),description); \
  description=(char *) MagickRelinquishMemory(description); \
  exit(-1); \
}

  MagickBooleanType
    status;

  MagickWand
    *magick_wand;

  /*
    Read an image.
  */
  MagickWandGenesis();
  magick_wand=NewMagickWand();
  status=MagickReadImage(magick_wand,filesource);
  if (status == MagickFalse)
    ThrowWandException(magick_wand);
//  { FIXME
//  magick_wand=DestroyMagickWand(magick_wand);
//  MagickWandTerminus();
//  }
  /*
    Turn the images into a thumbnail sequence.
  */
  MagickResetIterator(magick_wand);
  while (MagickNextImage(magick_wand) != MagickFalse)
    MagickResizeImage(magick_wand,width,height,LanczosFilter,1.0);
  /*
    Write the image then destroy it.
  */
  status=MagickWriteImages(magick_wand,filetarget,MagickTrue);
  if (status == MagickFalse)
    ThrowWandException(magick_wand);
  magick_wand=DestroyMagickWand(magick_wand);
  MagickWandTerminus();
  return(ERROR_NO);
}


/**
 *  YvonnePhotoCaptureInit
 *  @param cam The camera
 *  @return ERROR_NO if the camera is connected, ERROR code on error
 *
 *  Attempt to connect to and initialize the camera. This
 *  may fail if the camera is in use by another application,
 *  has gone to sleep or has been disconnected from the port.
 *
 *  This block execution of the caller until completion.
 */
int YvonnePhotoCaptureInit (YvonneCamera *cam) {
  cam->ctx = gp_context_new();

  // set callbacks for camera messages
  gp_context_set_error_func(cam->ctx, YvonnePhotoCaptureError, NULL);
  gp_context_set_message_func(cam->ctx, YvonnePhotoCaptureMessage, NULL);

  gp_camera_new(&cam->cam);
//    gp_camera_set_abilities(priv->cam, cap);
//    gp_camera_set_port_info(priv->cam, port);

  //This call will autodetect cameras, take the first one from the list and use it
  yrc_uiPrint(YVONNE_MSG_INFO,"Camera init. Can take more than 10 seconds depending on the "
  "memory card's contents (remove card from camera to speed up).");
  int ret = gp_camera_init(cam->cam, cam->ctx);
  if (ret < GP_OK) {
    printf("No camera auto detected.\n");
    return ERROR_GENERIC;
  }

  return ERROR_NO;
}



/**
 *  YvonnePhotoCaptureUnref
 *  @param cam the camera
 *  @return Always ERROR_NO
 *
 *  Release the camera
 */
int YvonnePhotoCaptureUnref (YvonneCamera* cam) {
  // close camera
  gp_camera_unref(cam->cam);
  gp_context_unref(cam->ctx);
  return ERROR_NO;
}


/**
 *  YvonnePhotoCapture
 *  @param cam The camera
 *  @param filemane The target file to capture the file in
 *  @return ERROR_NO if everything fine. false if false // FIXME
 *
 *  Capture a photo from the camera and save it to a file
 *  (code inspired http://sepharads.blogspot.de/2011/11/camera-tethered-capturing-using.html)
 */
int YvonnePhotoCapture (YvonneCamera* cam, const char *filename) {
    int fd, retval;
    CameraFile *file;
    CameraFilePath camera_file_path;

    // take a shot
    retval = gp_camera_capture(cam->cam, GP_CAPTURE_IMAGE, &camera_file_path, cam->ctx);

    if (retval) {
        return ERROR_GENERIC;
    // do some error handling, probably return from this function
    //TODO
    }

    printf("Pathname on the camera: %s/%s\n", camera_file_path.folder, camera_file_path.name);

    fd = open(filename, O_CREAT | O_WRONLY, 0644);
    // create new CameraFile object from a file descriptor
    retval = gp_file_new_from_fd(&file, fd);

    if (retval) {
      return ERROR_GENERIC;
    //TODO error handling
    }

    // copy picture from camera
    retval = gp_camera_file_get(cam->cam, camera_file_path.folder, camera_file_path.name,
    GP_FILE_TYPE_NORMAL, file, cam->ctx);

    if (retval) {
      gp_file_free(file);
      return ERROR_GENERIC;
    //TODO error handling
    }

    printf("Deleting\n");
    // remove picture from camera memory
    retval = gp_camera_file_delete(cam->cam, camera_file_path.folder, camera_file_path.name,
    cam->ctx);

    if (retval) {
      gp_file_free(file);
      return ERROR_GENERIC;
    //TODO error handling
    }

    // free CameraFile object
    gp_file_free(file);

    // Code from here waits for camera to complete everything.
    // Trying to take two successive captures without waiting
    // will result in the camera getting randomly stuck.
    int waittime = 2000;
    CameraEventType type;
    void *data;

    printf("Wait for events from camera\n");
    while(1) {
        retval = gp_camera_wait_for_event(cam->cam, waittime, &type, &data, cam->ctx);
        if(type == GP_EVENT_TIMEOUT) {
            break;
        }
        else if (type == GP_EVENT_CAPTURE_COMPLETE) {
            printf("Capture completed\n");
            waittime = 100;
        }
        else if (type != GP_EVENT_UNKNOWN) {
            printf("Unexpected event received from camera: %d\n", (int)type);
        }
    }
/*
    lseek(fd, 0, SEEK_SET);
    char buff;
    if (read(fd, &buff, sizeof(char)) == -1) {
      printf(ANSI_COLOR_YELLOW "Photo %s is empty, shoot has failed ?\n" ANSI_COLOR_RESET, filename);
      return ERROR_GENERIC;
    }
*/
    close (fd);

    return ERROR_NO;
}

/**
 *  YvonnePhotoCaptureError
 *  @param context The gphoto capture context
 *  @param str Gphoto2 context
 *  @param data
 *
 *  Gphoto2 error handler
 */
void YvonnePhotoCaptureError (GPContext *context, const char *str, void *data){
  yrc_uiPrint(YVONNE_MSG_ERROR, "\n*** Contexterror ***\n%s", str);
  //keepRunning = false;
}

/**
 *  YvonnePhotoCaptureMessage
 *  @param context The gphoto capture context
 *  @param str Gphoto2 context
 *  @param data
 *
 *  Gphoto2 message handler
 */
void YvonnePhotoCaptureMessage (GPContext *context, const char *str, void *data) {
  fprintf  (stderr, ANSI_COLOR_YELLOW "%s\n" ANSI_COLOR_RESET, str);
  fflush   (stderr);
}

