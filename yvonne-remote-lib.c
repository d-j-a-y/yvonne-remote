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

// TODO error checking in InitArduinoConnection

/**
 *   YvonneArduinoOpen
 *   @Param : strArduinoPort  ( /dev/ttyACM0 )
 *   @Return : Descripteur de fichier vers Arduino sinon ERROR_GENERIC
 */
int YvonneArduinoOpen (char* strArduinoPort)
{
    int fd;

/*
Open modem device for reading and writing and not as controlling tty
because we don't want to get killed if linenoise sends CTRL-C.
*/
//    fd = open(strArduinoPort, O_RDONLY | O_NOCTTY | O_NDELAY);
    fd = open(strArduinoPort, O_RDWR | O_NONBLOCK);
    if(fd < 0)
    {
        perror("OpenArduinoConnection : ");
        return ERROR_GENERIC;
    }

    return fd;
}

/**
 *   YvonneArduinoInit
 *   @Param[in] iFileDescriptor , Descripteur de fichier
 *   @Param[in] baudrate , Baudrate (bps) of Arduino
 *   @Param[out] *oldtio , structure pour acceuillir ancienne valeur port
 *   @Return Yvonne Error Code
 */
int YvonneArduinoInit (int iFileDescriptor, int baudrate, struct termios* oldtio)
{
    struct termios newtio;

    tcgetattr(iFileDescriptor,oldtio); /* save current serial port settings */
    tcgetattr(iFileDescriptor,&newtio); /* save current serial port settings */    
    //bzero(&newtio, sizeof(newtio)); /* clear struct for new port settings */

//    speed_t brate = BAUDRATE;
//    cfsetispeed(&newtio, brate);
//    cfsetospeed(&newtio, brate);    

    /*
    BAUDRATE: Set bps rate. You could also use cfsetispeed and cfsetospeed.
    CRTSCTS : output hardware flow control (only used if the cable has
            all necessary lines. See sect. 7 of Serial-HOWTO)
    CS8     : 8n1 (8bit,no parity,1 stopbit) : Arduino default
    CS7     : 7n1 (7bit,no parity,1 stopbit)
    CLOCAL  : local connection, no modem contol
    CREAD   : enable receiving characters
    */
//    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;    
	  /* Parité desactivé. */
//	  newtio.c_cflag &= PARENB;

	  newtio.c_cflag &= ~PARENB;
    /* 1 bits de stop. */
//    newtio.c_cflag &= CSTOPB;

    newtio.c_cflag &= ~CSTOPB;
    newtio.c_cflag &= ~CSIZE;    
    /* 2 bits de stop. */
    //newtio.c_cflag |= CSTOPB;

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
    newtio.c_cflag |= brate | CS8;
    //no flow control
    newtio.c_cflag &= ~CRTSCTS;
    //disable auto reset
    //newtio.c_cflag &= ~HUPCL; // not effectiv
    newtio.c_cflag |= CLOCAL | CREAD;

    /*
    IGNPAR  : ignore bytes with parity errors
    ICRNL   : map CR to NL (otherwise a CR input on the other computer
            will not terminate input)
    otherwise make device raw (no other input processing)
    */
//    newtio.c_iflag = INPCK | ICRNL;
//    newtio.c_iflag = IGNPAR;
    newtio.c_iflag &= ~(IXON | IXOFF | IXANY);
    /*
     Raw output.
    */
     newtio.c_oflag &= ~OPOST;

    /*
      ICANON  : enable canonical input
      disable all echo functionality, and don't send signals to calling program
    */
//     newtio.c_lflag = ICANON;
     //newtio.c_lflag = 0;
    newtio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    /*
      initialize all control characters
      default values can be found in /usr/include/termios.h, and are given
      in the comments, but we don't need them here
    */
//     newtio.c_cc[VINTR]    = 0;     /* Ctrl-c */
//     newtio.c_cc[VQUIT]    = 0;     /* Ctrl-\ */
//     newtio.c_cc[VERASE]   = 0;     /* del */
//     newtio.c_cc[VKILL]    = 0;     /* @ */
//     newtio.c_cc[VEOF]     = 4;     /* Ctrl-d */
     newtio.c_cc[VTIME]    = 0;     /* inter-character timer unused */

     newtio.c_cc[VMIN]     = 0;     /* blocking read until 1 character arrives */
     //newtio.c_cc[VMIN]     = 1;     /* blocking read until 1 character arrives */

//     newtio.c_cc[VSWTC]    = 0;     /* '\0' */
//     newtio.c_cc[VSTART]   = 0;     /* Ctrl-q */
//     newtio.c_cc[VSTOP]    = 0;     /* Ctrl-s */
//     newtio.c_cc[VSUSP]    = 0;     /* Ctrl-z */
//     newtio.c_cc[VEOL]     = 0;     /* '\0' */
//     newtio.c_cc[VREPRINT] = 0;     /* Ctrl-r */
//     newtio.c_cc[VDISCARD] = 0;     /* Ctrl-u */
//     newtio.c_cc[VWERASE]  = 0;     /* Ctrl-w */
//     newtio.c_cc[VLNEXT]   = 0;     /* Ctrl-v */
//     newtio.c_cc[VEOL2]    = 0;     /* '\0' */

    tcsetattr(iFileDescriptor,TCSANOW,&newtio);

    //! Wait a little before flushing the line
    usleep(200000);
    /*
    now clean the modem line and activate the settings for the port
    */
    tcflush(iFileDescriptor, TCIOFLUSH);
    
//    tcsetattr(iFileDescriptor,TCSAFLUSH,&newtio);    //TODO Test
/*
    int status;
    if(ioctl(iFileDescriptor, TIOCMGET, &status) == -1)
    {
        perror("pb dans get status");
        return ERROR_GENERIC;
    }

    status |= TIOCM_DTR;
    status |= TIOCM_RTS;

    if(ioctl(iFileDescriptor, TIOCMSET, &status) == -1)
    {
        perror("pb dans set status");
        return ERROR_GENERIC;
    }
*/
    return ERROR_NO;
}

/**
 *  YvonneArduinoClose
 *  @Param : fd, Descripteur de fichier de la connexion
 *  @Return :
 */
void YvonneArduinoClose (int fd, struct termios* oldtio)
{
    /* restore the old port settings */
    tcsetattr(fd,TCSANOW,oldtio);
    /* close the file */
    close(fd);
}

/**
 *  YvonneExecute : Execute the command line
 *  @Param : strCommandName, name of the Command line to execute inside a shell 
 *  @Param : strCommandLine, Command line to execute inside a shell
 *  @Return : Yvonne Error code
 */
int YvonneExecute (char* strCommandName, char* strCommandLine)
{
    FILE *pipe;
    //get a pipe for the command line
    pipe = popen(strCommandLine, "r");
    if(pipe == NULL)
    {
        perror("pipe :");
        return ERROR_GENERIC;
    }

    char outputLine[LINE_BUFFER];
    int linenr=1;
    //read the command line output  from the pipe line by line
    while (fgets(outputLine, LINE_BUFFER, pipe) != NULL)
    {
        printf("%s n°%d : %s",strCommandName, linenr, outputLine);
        ++linenr;
    }

    //close the pipe
    pclose(pipe);

    return ERROR_NO;
}

/**
 *  YvonneExecuteForked : Execute the command line in a child process and exit
 *  @Param : strCommandName, name of the Command line to execute inside a shell  
 *  @Param : strCommandLine, Command line to execute inside a shell
 *  @Return : Yvonne Error code
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
    if(pipe == NULL)
    {
        perror("pipe :");
        return ERROR_GENERIC;
    }

    char outputLine[LINE_BUFFER];
    int linenr=1;
    //read the command line output  from the pipe line by line
    while (fgets(outputLine, LINE_BUFFER, pipe) != NULL)
    {
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
 *  locate the last substring
 *  @Param : where to find the substring
 *  @Param : the substring to find
 *  @Return : ptr to the last substring or NULL if not found
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
  if(len1 - len2 <= 0)
  {
    if(strcmp(str1,str2)==0)
        return (char*)str1;   
    return 0;
  }
  
  // get the first occurence if exist
  strp2 = strstr(str1, str2);
  if(!strp2)
    return 0;
    
  strp = (char*)(str1 + len1 - len2);
  while(strp != str1)
  {
    if(*strp == *str2)
    {
      if(strncmp(strp,str2,len2)==0)
        return strp;
    }
    strp--;
  }
  return strp2;
}

/**
 *  YvonneFileCopyBin
 *  duplicate a binary file
 *  @Param : source file
 *  @Param : target file
 *  @Return : Error Code
 */
int YvonneFileCopyBin (char* filesource, char* filetarget){
   FILE *fsource, *ftarget;
 
   fsource = fopen(filesource, "rb");
   ftarget = fopen(filetarget, "wb");
 
   size_t l1;
   unsigned char buffer[8192]; 

   //Data to be read
   while((l1 = fread(buffer, 1, sizeof buffer, fsource)) > 0) {
    /* size_t l2 = */ fwrite(buffer, 1, l1, ftarget);
/*
     if(l2 < l1) {
       if(ferror(fd2))
         // handle error
       else
         // Handle media full
     }
*/
   }

   fclose(fsource);
   fclose(ftarget);

   return ERROR_NO;
}

/**
 *  YvonnePhotoResize
 *  resize an image (code from imagickwand example)
 *  @Param : source file
 *  @Param : target file
 *  @Param : target widht
 *  @Param : target height
 *  @Return : ImagickWand Error Code
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
 *  init libgphoto2 camera capture
 *  @Param : gphoto2 camera
 *  @Param : gphoto2 context
 *  @Return : Yvonne Error Code
 */
int YvonnePhotoCaptureInit (Camera *camera, GPContext *context) {
    gp_camera_new (&camera);
    context = gp_context_new();

    // set callbacks for camera messages
    gp_context_set_error_func(context, YvonnePhotoCaptureError, NULL);
    gp_context_set_message_func(context, YvonnePhotoCaptureMessage, NULL);

    //This call will autodetect cameras, take the first one from the list and use it
    printf("Camera init. Can take more than 10 seconds depending on the "
    "memory card's contents (remove card from camera to speed up).\n");
    int ret = gp_camera_init(camera, context);
    if (ret < GP_OK) {
        printf("No camera auto detected.\n");
        gp_camera_free(camera);
        return 1; //FIXME
    }

    return ERROR_NO;
}

/**
 *  YvonnePhotoCaptureUnref
 *  release libgphoto2 camera
 *  @Param : gphoto2 camera
 *  @Param : gphoto2 context
 *  @Return : Yvonne Error Code
 */
int YvonnePhotoCaptureUnref (Camera *camera, GPContext *context) {
    // close camera
    gp_camera_unref(camera);
    gp_context_unref(context);

    return ERROR_NO;
}


/**
 *  YvonnePhotoCapture
 *  capture a photo from camera using libgphoto2 (code from http://sepharads.blogspot.de/2011/11/camera-tethered-capturing-using.html)
 *  @Param : gphoto2 camera
 *  @Param : gphoto2 context
 *  @Param : target file
 *  @Return : Yvonne error code
 */
int YvonnePhotoCapture (Camera *camera, GPContext *context, const char *filename) {
    int fd, retval;
    CameraFile *file;
    CameraFilePath camera_file_path;

 // this was done in the libphoto2 example code, but doesn't seem to be necessary
 // NOP: This gets overridden in the library to /capt0000.jpg
 //snprintf(camera_file_path.folder, 1024, "/");
 //snprintf(camera_file_path.name, 128, "foo.jpg");

    // take a shot
    retval = gp_camera_capture(camera, GP_CAPTURE_IMAGE, &camera_file_path, context);

    if (retval) {
    // do some error handling, probably return from this function
    //TODO
    }

    printf("Pathname on the camera: %s/%s\n", camera_file_path.folder, camera_file_path.name);

    fd = open(filename, O_CREAT | O_WRONLY, 0644);
    // create new CameraFile object from a file descriptor
    retval = gp_file_new_from_fd(&file, fd);

    if (retval) {
    //TODO error handling
    }

    // copy picture from camera
    retval = gp_camera_file_get(camera, camera_file_path.folder, camera_file_path.name,
    GP_FILE_TYPE_NORMAL, file, context);

    if (retval) {
    //TODO error handling
    }

    printf("Deleting\n");
    // remove picture from camera memory
    retval = gp_camera_file_delete(camera, camera_file_path.folder, camera_file_path.name,
    context);

    if (retval) {
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
        retval = gp_camera_wait_for_event(camera, waittime, &type, &data, context);
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
    return ERROR_NO;
}

/**
 *  YvonnePhotoCaptureError
 *  capture a photo from camera using gphoto2 lib (code from http://sepharads.blogspot.de/2011/11/camera-tethered-capturing-using.html)
 *  @Param : gphoto2 camera
 *  @Param : gphoto2 context
 *  @Param : target file
 *  @Return : 
 */
void YvonnePhotoCaptureError (GPContext *context, const char *format, va_list args, void *data) {
    fprintf(stderr, "*** Contexterror ***\n");
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
}

/**
 *  YvonnePhotoCaptureMessage
 *  capture a photo from camera using gphoto2 lib (code from http://sepharads.blogspot.de/2011/11/camera-tethered-capturing-using.html)
 *  @Param : gphoto2 camera
 *  @Param : gphoto2 context
 *  @Param : target file
 *  @Return : 
 */
void YvonnePhotoCaptureMessage (GPContext *context, const char *format, va_list args, void *data) {
    vprintf(format, args);
    printf("\n");
}
