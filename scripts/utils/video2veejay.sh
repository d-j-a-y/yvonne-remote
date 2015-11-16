#!/bin/bash

version=151113
resolution=800x600
staticffmpeginstall=/home/path/to/ffmpeg-2.5.3-64bit-static

case $# in
  0) echo "---------------------------------------"
  echo "Erreur : $0: not enough arguments"
  echo "Convertie une video dans format veejay (mjpg iframe only 800x600)"
  echo "arg1) Nom de la video"
  echo "arg2) Résolution de la vidéo HxL (800x600 par défaut)"
  echo "----------      -------   --  ----- -  ---------- --------"
  exit 2 ;;
  2) ;;
  *) $resolution=$2 ;;
esac

echo "---------------------------------------"
echo "debut de conversionnnnnnn "
echo "---------------- ------   ------- --------"
# -intra Use only intra frames.
#ajouter -intra

# "%sffmpeg -f image2 -start_number %d -r 25 -i \"./%s/%s-%%05d.jpg\" -q:v 1 -vcodec mjpeg -s %s ./video/%s-%d.avi", FFMPEG_STATIC_BUILD_INSTALL, startSequence, LOWQUALITY_DIRECTORY, sceneName, LOWQUALITY_RESOLUTION, sceneName, videoIndex);
#$staticffmpeginstall/ffmpeg -f image2 -r 25 -i $1/image-%04d.jpg -q:v 1 -vcodec mjpeg -s $resolution $1.avi

avconv -i $1 -q:v 1 -vcodec mjpeg -pix_fmt yuvj422p -s $resolution $1.avi
#avconv -f image2 -r 25 -i $1/image-%04d.jpg -q:v 1 -vcodec rawvideo -pix_fmt yuv422p -acodec none -s $resolution $1.avi

#ffmpeg -f image2 -i $1/image-%04d.jpg -r 24 -vcodec mjpeg -q:v 1 -b:v 12000 -s $resolution $1.avi

if [ $? = 0 ]
then
  echo "---------------------------------------------"
  echo "La video" $1.avi "est prete a etre joyeusement remixée"
  echo "-------------  ----------------- --------  ----------"
else
  echo "--------------------------------------"
  echo "Problème durant la génération de $1.avi"
  echo "--- -------- -------------- --------  ------ ----"
  exit 1
fi
