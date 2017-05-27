#!/bin/bash

version=170527
extension=JPG
resolution=1024x576
framerepeat=5
staticffmpeginstall=/home/path/to/ffmpeg-2.5.3-64bit-static

#TODO force always landscape format (seems to be actually usefull)
#TODO ffmpeg static dir has parameter option / avconv . or a way to automatically detect

#TODO DOC list dependencies

#TODO third parameter (rez) check validity
#TODO nice option parameter ! while [ $# -gt 0 ]; do blabla shift


#while [ $# -gt 0 ]
#do
#    echo "$1"
#    shift
#done


#TODO ratio option

#WIP tmp folder : check exist, remove after //// 


case $# in
  0) echo "---------------------------------------"
  echo "Erreur : $0: not enough arguments"
  echo "Génère une vidéo (mjpg 1024x576) à partir d'une série d'images"
  echo "arg1) Nom du dossier contenant les images (qui donne le nom a la vidéo)"
  echo "arg2) Extension des images (JPG par défaut)"
#  echo "arg3) Nombre pour la numérotation (4 par défaut)"  
  echo "arg3) Résolution de la vidéo HxL (1024x576 par défaut)"
  echo "arg4) Nombre de repetition des images (5 par défaut)"
  echo "----------      -------   --  ----- -  ---------- --------"
  exit 2 ;;
  4) framerepeat=$4
     resolution=$3
     extension=$2 ;;
  3) resolution=$3
     extension=$2 ;;
  2) extension=$2 ;;
esac


###rename files to sequential numbers
#http://stackoverflow.com/questions/3211595/renaming-files-in-a-folder-to-sequential-numbers
#by http://stackoverflow.com/users/377927/gauteh

#debug echo $extension $resolution $framerepeat 

#remove trailling / from directory name
target=${1%/}

if [ ! -d ${target} ]
then
  echo "ERROR : \"${target}\" folder doesn't exist. "
  exit 0
fi

#print the number of files to go
filesDOTextension=(${target}/*.${extension})
numfiles=${#filesDOTextension[@]}

shopt -s nullglob dotglob

if [ "$filesDOTextension" =  "" ]
then
  echo "ERROR : \"${target}\" folder contain any \"${extension}\" file"
  shopt -u nullglob dotglob
  exit 0
fi

shopt -u nullglob dotglob

if [ -d ${target}/tmp/ ]
then
  read -p "WARNING : Delete \"${target}/tmp/\" ? [y/N] " removedir
  if [ "$removedir" = "y" ]
  then
    rm -r ${target}/tmp/
  else
    exit 0;
  fi
fi

mkdir ${target}/tmp/
tmpdir=${target}/tmp

echo "---------------------------------------"
echo "renommage en massssssssssssssssssse !"
echo "frame repeat : ${framerepeat}"
echo "$numfiles to go for ${target}.avi"
echo "----- -----  --------------- ----- -------- "


##rename sequential and duplicate the file
a=0

for i in ${target}/*.${extension}; do

  before=${i}
  lqfile=${before/${target}/${tmpdir}} #replace substr
  lqfile=${lqfile/.${extension}/-lq.jpg}
#  echo ${lqfile}

  cp ${i} ${lqfile}
  mogrify -resize $resolution ${lqfile}

  indexrepeat=framerepeat

  while (( $indexrepeat > 0 ))
  do
    new=$(printf "%04d.jpg" ${a}) #04 pad to length of 4
    ln ${lqfile} ${tmpdir}/image-${new}
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

#$staticffmpeginstall/ffmpeg -f image2 -r 25 -i ${target}/image-%04d.jpg -q:v 1 -vcodec mjpeg  -pix_fmt yuvj422p -s $resolution ${target}.avi
avconv -f image2 -r 25 -i ${tmpdir}/image-%04d.jpg -q:v 1 -vcodec mjpeg -aspect 16:9 -pix_fmt yuvj422p -s $resolution ${target}.avi

if [ $? = 0 ]
then
  rm -r ${tmpdir}
  echo "---------------------------------------------"
  echo "La video" ${target}.avi "est prete a etre joyeusement remixée"
  echo "-------------  ----------------- --------  ----------"
else
  echo "--------------------------------------"
  echo "Problème durant la génération de ${target}.avi"
  echo "--- -------- -------------- --------  ------ ----"
  exit 1
fi
