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
typefile=JPG
resolusionvideo=1024x576
framerepeat=5
#staticffmpeginstall=/home/path/to/ffmpeg-2.5.3-64bit-static
#videoconverter=avconv

#TODO / WIP force always landscape format (seems to be actually usefull)

#TODO facultativ format landscape test (slow)

#TODO pb in bq size 1024*576 ---> 866*576 (http://www.imagemagick.org/script/command-line-processing.php#geometry)

#TODO ffmpeg static dir has parameter option / avconv . or a way to automatically detect

#TODO cat *.jpg | ffmpeg -f image2pipe -r 120 -vcodec mjpeg -i - -vcodec mjpeg -s vga out2_120fpsMJPEGvga.avi
# https://github.com/SynchronisticSavage/VJ-scripts/blob/master/ffmpeg/notes.txt

#TODO DOC list dependencies

#TODO third parameter (rez) check validity

#TODO ratio option

usage ()
{
echo "---------------------------------------"
  echo "Create a mjpg i-frame only video (.avi) from images"
  echo "-d : images source directory (also used has video name)" ####MANDATORY ? read ?
  echo "-t : file type [optionnal] defaut : $typefile"
  echo "-s : video size HxL [optionnal] default : $resolusionvideo"
  echo "-r : image repetition [optionnal] default : $framerepeat"
  echo "----------      -------   --  ----- -  ---------- --------"
}

echoerror ()
{
  echo "---------------------------------------"
  echo $1
}

numparam=$#

if [ "$numparam" -eq 0 ]; then
  echoerror "Error : source directory missing (or not enough arguments)"
#read -p  "source folder : "  target
  usage
  exit 2
fi

let "remainder = $numparam % 2"
if [ "$remainder" -eq 1 ]; then
  echoerror "Error : odd arguments (or not enough arguments)"
  usage
  exit 2
fi

while [ $# -gt 0 ]
do
  case "$1" in
    -t) typefile=$2  #TODO test if $2 exist
        echo "t:$typefile"
        shift; shift ;;
    -s) resolusionvideo=$2 #TODO test if $2 exist
        echo "s:$resolusionvideo"
        shift; shift ;;
    -d) sourcedirectory=${2%/} # TODO test if $2 exist remove trailling / from directory name
        echo "d:$sourcedirectory"
        shift; shift ;;
    *)  shift ;;
  esac
done

if [ ! -d ${sourcedirectory} ]; then
  echoerror "ERROR : \"${sourcedirectory}\" folder doesn't exist. "
  exit 0
fi

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
	echoerror "Error : a video converter is missing, please install 'ffmpeg' or 'avconv' and try again"
	exit 2
fi

checkforanyfile=$(find ${sourcedirectory} -maxdepth 1 -type f -name "*.${typefile}"|head -n1)
if [ -z $checkforanyfile ]; then
  echo "ERROR : \"${sourcedirectory}\" folder contain any \"${typefile}\" file"
  echo "Bye"
  exit 0
fi

#################TEMPDIR#
if [ -d ${sourcedirectory}/tmp/ ]; then
  read -p "WARNING : Delete \"${sourcedirectory}/tmp/\" ? [y/N] " removedir
  if [ "$removedir" = "y" ]; then
    rm -r ${sourcedirectory}/tmp/
  else
    echo "Bye"
    exit 0;
  fi
fi

mkdir ${sourcedirectory}/tmp/
tmpdir=${sourcedirectory}/tmp
#TMPDIR##################

#print the number of files to go
filesDOTextension=(${sourcedirectory}/*.${typefile})
numfiles=${#filesDOTextension[@]}
echo "---------------------------------------"
echo "INFO : $numfiles images will be chewed"
echo "----- -----  --------------- ----- -------- "

step=1
echo "---------------------------------------"
echo "STEP $step : checking image orientation..."
echo "----- -----  --------------- ----- -------- "
for i in ${sourcedirectory}/*.${typefile}; do
  imsize=`identify -format "%[fx:w] %[fx:h]" ${i}`
  imwidth=`echo $imsize | cut -d' ' -f 1`
  imheight=`echo $imsize | cut -d' ' -f 2`
  if [ "$imwidth" -lt "$imheight" ]; then
    read -p "WARNING : ${i} orientation seems to be portrait :  $imwidth x $imheight - continue ? [y/N] " answer
    if [ "$answer" = "N" ]; then
      rm -r ${sourcedirectory}/tmp/
      echo "Bye"
      exit 0;
    fi
  fi
done

let "step += 1"
echo "---------------------------------------"
echo "STEP $step : massssssssssive renaming & duplication ${framerepeat}x ..."
echo "----- -----  --------------- ----- -------- "

a=0
for i in ${sourcedirectory}/*.${typefile}; do

  before=${i}
  lqfile=${before/${sourcedirectory}/${tmpdir}} #replace substr
  lqfile=${lqfile/.${typefile}/-lq.jpg}
#  echo ${lqfile}

  cp ${i} ${lqfile}
  mogrify -resize $resolusionvideo ${lqfile}

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
echo "STEP $step : converting to video using ${avconverter}"
echo "---------------- ------   ------- --------"
# -intra Use only intra frames.
#ajouter -intra

videoconvertion="${avconverter} -f image2 -r 25 -i ${tmpdir}/image-%04d.jpg -q:v 1 -vcodec mjpeg -aspect 16:9 -pix_fmt yuvj422p -s $resolusionvideo ${sourcedirectory}.avi"
eval "$videoconvertion"

#$staticffmpeginstall/ffmpeg -f image2 -r 25 -i ${target}/image-%04d.jpg -q:v 1 -vcodec mjpeg  -pix_fmt yuvj422p -s $resolution ${target}.avi
#avconv -f image2 -r 25 -i ${tmpdir}/image-%04d.jpg -q:v 1 -vcodec mjpeg -aspect 16:9 -pix_fmt yuvj422p -s $resolution ${target}.avi

if [ $? = 0 ]; then
  rm -r ${tmpdir}
  echo "---------------------------------------------"
  echo "video" ${sourcedirectory}.avi "is ready to be happily mixed !"
  echo "-------------  ----------------- --------  ----------"
else
  echo "--------------------------------------"
  echo "ERROR : $avconverter failled to create ${sourcedirectory}.avi"
  echo "--- -------- -------------- --------  ------ ----"
  exit 1
fi
