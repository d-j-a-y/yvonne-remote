#!/bin/bash

version=151116
extension=JPG
resolution=800x600
framerepeat=5
staticffmpeginstall=/home/path/to/ffmpeg-2.5.3-64bit-static

#TODO force always landscape format (seems to be actually usefull)
#TODO ffmpeg static dir has parameter option / avconv . or a way to automatically detect

#TODO DOC list dependencies

#TODO third parameter (rez) check validity
#TODO nice option parameter ! while [ $# -gt 0 ]; do blabla shift

case $# in
  0) echo "---------------------------------------"
  echo "Erreur : $0: not enough arguments"
  echo "Génère une vidéo (mjpg 800x600) à partir d'une série d'images"
  echo "arg1) Nom du dossier contenant les images (qui donne le nom a la vidéo)"
  echo "arg2) Extension des images (JPG par défaut)"
#  echo "arg3) Nombre pour la numérotation (4 par défaut)"  
  echo "arg3) Résolution de la vidéo HxL (800x600 par défaut)"
  echo "arg4) Nombre de repetition des images (5 par défaut)"
  echo "----------      -------   --  ----- -  ---------- --------"
  exit 2 ;;
  4) framerepeat=$4
     resolution=$3
     extension=$2 ;;
  3) resolution=$3
     extension=$2 ;;
  2) extension=$2
esac

echo "---------------------------------------"
echo "renommage en massssssssssssssssssse !"
echo "----- -----  --------------- ----- -------- "

###rename files to sequential numbers
#http://stackoverflow.com/questions/3211595/renaming-files-in-a-folder-to-sequential-numbers
#by http://stackoverflow.com/users/377927/gauteh

#debug echo $extension $resolution $framerepeat 

#remove trailling / from directory name
target=${1%/}
#print the number of files to go
numfiles=(${target}/*.${extension})
numfiles=${#numfiles[@]}
echo -n $numfiles

##rename sequential and duplicate the file
a=0

for i in ${target}/*.$extension; do
  cp ${i} ${i}-lq.jpg
  mogrify -resize $resolution ${i}-lq.jpg

  indexrepeat=framerepeat

  while (( $indexrepeat > 0 ))
  do
    new=$(printf "%04d.jpg" ${a}) #04 pad to length of 4
    cp ${i}-lq.jpg ${target}/image-${new}
    let "a += 1"

    echo -n "-"

    let "indexrepeat -= 1"
  done

  let "numfiles -= 1"
  echo -n "|${numfiles}"
done

echo ""
echo "---------------------------------------"
echo "debut de conversionnnnnnn "
echo "---------------- ------   ------- --------"
# -intra Use only intra frames.
#ajouter -intra

# "%sffmpeg -f image2 -start_number %d -r 25 -i \"./%s/%s-%%05d.jpg\" -q:v 1 -vcodec mjpeg -s %s ./video/%s-%d.avi", FFMPEG_STATIC_BUILD_INSTALL, startSequence, LOWQUALITY_DIRECTORY, sceneName, LOWQUALITY_RESOLUTION, sceneName, videoIndex);

#$staticffmpeginstall/ffmpeg -f image2 -r 25 -i ${target}/image-%04d.jpg -q:v 1 -vcodec mjpeg  -pix_fmt yuvj422p -s $resolution ${target}.avi
avconv -f image2 -r 25 -i ${target}/image-%04d.jpg -q:v 1 -vcodec mjpeg  -pix_fmt yuvj422p -s $resolution ${target}.avi

#ffmpeg -f image2 -i $1/image-%04d.jpg -r 24 -vcodec mjpeg -q:v 1 -b:v 12000 -s $resolution $1.avi

if [ $? = 0 ]
then
  echo "---------------------------------------------"
  echo "La video" ${target}.avi "est prete a etre joyeusement remixée"
  echo "-------------  ----------------- --------  ----------"
else
  echo "--------------------------------------"
  echo "Problème durant la génération de ${target}.avi"
  echo "--- -------- -------------- --------  ------ ----"
  exit 1
fi