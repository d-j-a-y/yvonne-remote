yvonne-remote
=============

Remote control for ["Yvonne vas au Bal"](http://filomene.space/yvonne-va-au-bal) live cinema performance using arduino, gphoto2, image magick, ffmpeg.

Yvonne remote control is a remote control for Yvonne. From a switch board it permist
to drive the shooting process and video generation of a digital reflex camera.

Divided in two programs, the [first one](arduino-sketch) hosted on the Arduino transmits over the serial switche's states (start or pause the shooting, generate a video...), [the second](./) running in a Gnu/linux listen to those events and accordingly launch processes to capture photos or create a video...

[![Yvonne remote out of binaries](http://media.tumblr.com/417ae335c7295ac8904b8843930aee39/tumblr_inline_n6otnevTaa1r1ocez.gif)](http://yvonne-tour.tumblr.com)

### DEPENDENCIES
* ffmpeg
* libgphoto2
* MagickWand

### INSTALL

```make```

```make install```


### MORE INFOS
* [Gphoto2](http://gphoto.org)
* [FFmpeg](http://ffmpeg.org)
* [Arduino](http://arduino.cc)
* [Image Magick](http://www.imagemagick.org)
