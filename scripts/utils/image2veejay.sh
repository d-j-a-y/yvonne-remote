#!/bin/bash

#######################ABOUT#######################
#Generate a fullframe mjpg video (.avi) from given folder's images
##########################AUTHOR####################
#Jerome Blanchi aka d-j-a-y (c) WTFPL
#############ADDITIONAL CREDITS#######################
###rename files to sequential numbers
#http://stackoverflow.com/questions/3211595/renaming-files-in-a-folder-to-sequential-numbers
#by http://stackoverflow.com/users/377927/gauteh
#######################################################


version=170729
extension=JPG
resolution=1024x576
framerepeat=5
staticffmpeginstall=/home/path/to/ffmpeg-2.5.3-64bit-static
videoconverter=avconv

#TODO / WIP force always landscape format (seems to be actually usefull)

#TODO pb in bq size 1024*576 ---> 866*576 (http://www.imagemagick.org/script/command-line-processing.php#geometry)

#TODO ffmpeg static dir has parameter option / avconv . or a way to automatically detect

#TODO cat *.jpg | ffmpeg -f image2pipe -r 120 -vcodec mjpeg -i - -vcodec mjpeg -s vga out2_120fpsMJPEGvga.avi
# https://github.com/SynchronisticSavage/VJ-scripts/blob/master/ffmpeg/notes.txt

#TODO DOC list dependencies

#TODO third parameter (rez) check validity
#TODO nice option parameter ! while [ $# -gt 0 ]; do blabla shift


#while [ $# -gt 0 ]
#do
#  case "$1" in
#    -t) typefile=$2  #extension
#        echo "t:$2"
#        shift; shift
#    -s) resolusionvideo=$2 #resolusion
#        echo "s:$2"
#        shift; shift
#    -d) sourcedirectory=$2 #target
#        echo "d:$2"
#        shift; shift
#  esac
#done


#TODO ratio option

usage ()
{
echo "---------------------------------------"
  echo "Create a mjpg i-frame only video (.avi) from images"
  echo "-d : source directory of images and video name" ####MANDATORY ? read ?
  echo "-t : file type (defaut : $extension)"
  echo "-s : video size HxL (default : $resolution)"
  echo "-r : image repetition (default : $framerepeat)"
  echo "----------      -------   --  ----- -  ---------- --------"
}


#case $# in
#  0) echo "---------------------------------------"
#  echo "Erreur : $0: not enough arguments"
#  echo "Génère une vidéo (mjpg 1024x576) à partir d'une série d'images"
#  echo "arg1) Nom du dossier contenant les images (qui donne le nom a la vidéo)"
#  echo "arg2) Extension des images (JPG par défaut)"
##  echo "arg3) Nombre pour la numérotation (4 par défaut)"
#  echo "arg3) Résolution de la vidéo HxL (1024x576 par défaut)"
#  echo "arg4) Nombre de repetition des images (5 par défaut)"
#  echo "----------      -------   --  ----- -  ---------- --------"
#  exit 2 ;;
#  4) framerepeat=$4
#     resolution=$3
#     extension=$2 ;;
#  3) resolution=$3
#     extension=$2 ;;
#  2) extension=$2 ;;
#esac

#debug echo $extension $resolution $framerepeat 

#remove trailling / from directory name
target=${1%/}

numparam=$#

if [ "$numparam" -eq 0 ]; then
  echo "---------------------------------------"
#read -p  "source folder : "  target
  echo "Error : not enough arguments (source directory missing)"
  usage
  exit 2
fi

let "remainder = $numparam % 2"
if [ "$remainder" -eq 1 ]; then
  echo "---------------------------------------"
  echo "Error : not enough arguments (odd arguments)"
  usage
  exit 2
fi

echo "TESSSSSST ARGS"
while [ $# -gt 0 ]
do
  case "$1" in
    -t) typefile=$2  #extension
        echo "t:$2"
        shift; shift ;;
    -s) resolusionvideo=$2 #resolusion
        echo "s:$2"
        shift; shift ;;
    -d) sourcedirectory=$2 #target
        echo "d:$2"
        shift; shift ;;
    *)  shift ;;
  esac
done

# "ffmpeg avconv" test inspired from 
# https://askubuntu.com/questions/636050/how-to-check-if-two-or-more-programs-are-installed-using-a-bash-script
i=0; n=0; progs=(ffmpeg avconv);
for p in "${progs[@]}"; do
    if hash "$p" &>/dev/null
    then
        echo "$p is installed"
        avconverter=$p
    else
        echo "$p is not installed"
        let n++
    fi
done

if [ $n = 2 ] ; then
	echo "Error : a video converter is missing, please install 'ffmpeg' or 'avconv' and try again"
	exit 2
fi


if [ ! -d ${target} ]; then
  echo "ERROR : \"${target}\" folder doesn't exist. "
  exit 0
fi

checkforanyfile=$(find ${target} -maxdepth 1 -type f -name "*.${extension}"|head -n1)
if [ -z $checkforanyfile ]; then
  echo "ERROR : \"${target}\" folder contain any \"${extension}\" file"
  echo "Bye"
  exit 0
fi

#################TEMPDIR#
if [ -d ${target}/tmp/ ]; then
  read -p "WARNING : Delete \"${target}/tmp/\" ? [y/N] " removedir
  if [ "$removedir" = "y" ]; then
    rm -r ${target}/tmp/
  else
    echo "Bye"
    exit 0;
  fi
fi

mkdir ${target}/tmp/
tmpdir=${target}/tmp
#TMPDIR##################

#print the number of files to go
filesDOTextension=(${target}/*.${extension})
numfiles=${#filesDOTextension[@]}
echo "---------------------------------------"
echo "INFO : $numfiles images will be chewed"
echo "----- -----  --------------- ----- -------- "

step=1
echo "---------------------------------------"
echo "STEP $step : checking image orientation !"
echo "----- -----  --------------- ----- -------- "
for i in ${target}/*.${extension}; do
  imsize=`identify -format "%[fx:w] %[fx:h]" ${i}`
  imwidth=`echo $imsize | cut -d' ' -f 1`
  imheight=`echo $imsize | cut -d' ' -f 2`
  if [ "$imwidth" -lt "$imheight" ]; then
    read -p "WARNING : ${i} orientation seems to be portrait :  $imwidth x $imheight - continue ? [y/N] " answer
    if [ "$answer" = "N" ]; then
      rm -r ${target}/tmp/
      echo "Bye"
      exit 0;
    fi
  fi
done

let "step += 1"
echo "---------------------------------------"
echo "STEP $step : massssssssssive renaming and duplicated ${framerepeat} times !"
echo "----- -----  --------------- ----- -------- "

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

let "step += 1"
echo "---------------------------------------"
echo "STEP $step : converting to video using ${videoconverter}"
echo "---------------- ------   ------- --------"
# -intra Use only intra frames.
#ajouter -intra

videoconvertion="${videoconverter} -f image2 -r 25 -i ${tmpdir}/image-%04d.jpg -q:v 1 -vcodec mjpeg -aspect 16:9 -pix_fmt yuvj422p -s $resolution ${target}.avi"
eval "$videoconvertion"

#$staticffmpeginstall/ffmpeg -f image2 -r 25 -i ${target}/image-%04d.jpg -q:v 1 -vcodec mjpeg  -pix_fmt yuvj422p -s $resolution ${target}.avi
#avconv -f image2 -r 25 -i ${tmpdir}/image-%04d.jpg -q:v 1 -vcodec mjpeg -aspect 16:9 -pix_fmt yuvj422p -s $resolution ${target}.avi

if [ $? = 0 ]; then
  rm -r ${tmpdir}
  echo "---------------------------------------------"
  echo "video" ${target}.avi "is ready to be happily mixed !"
  echo "-------------  ----------------- --------  ----------"
else
  echo "--------------------------------------"
  echo "ERROR : $videoconverter failled to create ${target}.avi"
  echo "--- -------- -------------- --------  ------ ----"
  exit 1
fi
