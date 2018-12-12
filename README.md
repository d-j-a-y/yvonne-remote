yvonne-remote
=============

Server side control for ["Yvonne vas au Bal"](http://filomene.space/yvonne-va-au-bal)
live cinema performance using arduino, gphoto2, image magick, ffmpeg, ncurses.

Yvonne remote is a simple client/server model for automatic shooting sessions.
* [Remote](arduino-sketch), or local client control (ex : `--remoteno`).
* Shoots can be done over a stream (ex: `-s /dev/video0`) or from a reflex camera.

__User case :__
From a switch board, arduino based, it permist to drive the shooting process and
video generation of a digital reflex camera.

[![Yvonne remote out of binaries](http://media.tumblr.com/417ae335c7295ac8904b8843930aee39/tumblr_inline_n6otnevTaa1r1ocez.gif)](http://yvonne-tour.tumblr.com)

### HELP
```
 ./yvonne-remote --help
Usage: yvonne-remote [OPTIONS]

Options:
  -q  --quiet                Don't print out as much info
  -h, --help                 Print this help message and quit
  -n, --name=scenename       Name of the scene (defaut currentdir)
  -p, --port=serialport      Serial port the Arduino is connected to (defaut /dev/ttyACM0)
  -f, --fotogap=integer      Gap of scene (photo) numbering for resumption (optional)
  -v, --videogap=integer     Gap of video numbering for resumption (optional)
  -d  --delay=seconds        Delay between two shoot in seconds (default 5)
  -b, --baud=rate            Baudrate (bps) of Arduino (default 57600)
  -s, --stream=path_to       Path of stream source to useµ. Bypass the camera detection
  -R, --remoteno             No remote control

Dependecies: libgphoto2, ffmpeg, MagickWand, Ncurses.
```

### DEPENDENCIES
* ffmpeg
* libgphoto2
* MagickWand
* Ncurses

### INSTALL

```make```

```make install```

### SEE ALSO
[See the client control](arduino-sketch) hosted on Arduino who transmits over
COM port switche's states (start or pause the shooting, generate a video...).

### MORE INFOS
* [Gphoto2](http://gphoto.org)
* [FFmpeg](http://ffmpeg.org)
* [Arduino](http://arduino.cc)
* [Image Magick](http://www.imagemagick.org)
* [Ncurses](https://www.gnu.org/software/ncurses/ncurses.html)
