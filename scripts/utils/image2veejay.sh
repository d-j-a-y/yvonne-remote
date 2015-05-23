#!/bin/bash

version=150521
extension=JPG
resolution=800x600
staticffmpeginstall=/home/path/to/ffmpeg-2.5.3-64bit-static

case $# in
  0) echo "---------------------------------------"
  echo "Erreur : $0: not enough arguments"
  echo "Génère une vidéo (mjpg 800x600) à partir d'une série d'images"
  echo "arg1) Nom du dossier contenant les images (qui donne le nom a la vidéo)"
  echo "arg2) Extension des images (JPG par défaut)"
#  echo "arg3) Nombre pour la numérotation (4 par défaut)"  
  echo "arg3) Résolution de la vidéo HxL (800x600 par défaut)"  
  echo "----------      -------   --  ----- -  ---------- --------"
  exit 2 ;;
  2) extension=$2 
esac

echo "---------------------------------------"
echo "renommage en massssssssssssssssssse !"
echo "----- -----  --------------- ----- -------- "

#valeur par defaut si pas de second parametre
#a=${2:-0}
#echo ${a}

###rename files to sequential numbers
#http://stackoverflow.com/questions/3211595/renaming-files-in-a-folder-to-sequential-numbers
#by http://stackoverflow.com/users/377927/gauteh
#remixed
#a=0
#for i in $1/*.$extension; do
#  new=$(printf "%04d.jpg" ${a}) #04 pad to length of 4
#  mv ${i} $1/image-${new}
#  let a=a+1
#done
#endremix

##rename sequential and duplicate the file
a=0
for i in $1/*.$extension; do
  new=$(printf "%04d.jpg" ${a}) #04 pad to length of 4
  cp ${i} $1/image-${new} 
  let a=a+1
  echo -n "-"

  new=$(printf "%04d.jpg" ${a}) #04 pad to length of 4
  cp ${i} $1/image-${new} 
  let a=a+1
  echo -n "-"

  new=$(printf "%04d.jpg" ${a}) #04 pad to length of 4
  cp ${i} $1/image-${new}
  let a=a+1
  echo -n "-"

  new=$(printf "%04d.jpg" ${a}) #04 pad to length of 4
  cp ${i} $1/image-${new} 
  let a=a+1
  echo -n "-"
  
  new=$(printf "%04d.jpg" ${a}) #04 pad to length of 4
  cp ${i} $1/image-${new}
  let a=a+1
  echo -n "-|"
done

echo "---------------------------------------"
echo "debut de conversionnnnnnn "
echo "---------------- ------   ------- --------"
# -intra Use only intra frames.
#ajouter -intra

# "%sffmpeg -f image2 -start_number %d -r 25 -i \"./%s/%s-%%05d.jpg\" -q:v 1 -vcodec mjpeg -s %s ./video/%s-%d.avi", FFMPEG_STATIC_BUILD_INSTALL, startSequence, LOWQUALITY_DIRECTORY, sceneName, LOWQUALITY_RESOLUTION, sceneName, videoIndex);
$staticffmpeginstall/ffmpeg -f image2 -r 25 -i $1/image-%04d.jpg -q:v 1 -vcodec mjpeg -s $resolution $1.avi
#avconv -f image2 -r 25 -i $1/image-%04d.jpg -q:v 1 -vcodec mjpeg -s $resolution $1.avi
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
