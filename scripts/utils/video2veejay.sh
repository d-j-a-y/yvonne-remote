#!/bin/bash

version=161031
resolution=800x600

#TODO list dependencies
#TODO option pour pattern matching :
###find . -name \*.mov -exec /home/frijol/Sources/#Scripts/video2veejay.sh '{}' 1920x1080 \;

help_me()
{
  echo "---------------------------------------"
  echo "Erreur : $0: not enough arguments"
  echo "Convertie une video dans format veejay (mjpg iframe only 800x600)"
  echo "arg1) Nom de la video"
  echo "arg2) Résolution de la vidéo HxL (800x600 par défaut)"
  echo "----------      -------   --  ----- -  ---------- --------"
}


case $# in
  0) help_me
    exit 2 ;;
  1) ;;
  2) resolution=$2 ;;
  *) help_me
    exit 2 ;;
esac

inputfile=$(basename "${1}")
#echo $filename
extension="${inputfile##*.}"
#echo $extension
outputfile="${inputfile%.*}.avi"

if [ -f "$outputfile" ]
then
    echo "---------------------- - - - "
    echo "$outputfile existe deja ----------- - - - "
    outputfile="${inputfile}".avi
    echo "$outputfile seras utilise------ - - - "
    echo "---------------------- - - - "

fi

echo "---------------------------------------"
echo "debut de conversionnnnnnn "
echo "---------------- ------   ------- --------"
# -intra Use only intra frames.
#ajouter -intra

#yuvj422p
avconv -i "${1}" -r 25 -q:v 1 -vcodec mjpeg -acodec pcm_s16le -s $resolution "${outputfile}"

if [ $? = 0 ]
then
  echo "---------------------------------------------"
  echo "La video" $outputfile "est prete a etre joyeusement remixée"
  echo "-------------  ----------------- --------  ----------"
else
  echo "--------------------------------------"
  echo "Problème durant la génération de $outputfile"
  echo "--- -------- -------------- --------  ------ ----"
  exit 1
fi
