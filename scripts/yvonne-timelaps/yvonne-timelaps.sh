#!/bin/bash

#--------------------------------------------------------------------#
# version 20140517
#
# This script was used to automate yvonne's shoting before yvonne-remote
# exist : https://www.github.com/d-j-a-y/yvonne-remote
#
# Copyright (c) Jérôme Blanchi aka d.j.a.y
#
# This work is free. You can redistribute it and/or modify it under 
# the terms of the Do What The Fuck Want To Public Licence, Version 2,
# as published by Jérôme Blanchi. See COPYING file for more details.
#--------------------------------------------------------------------#

#echo "------------------------------------"
#echo " Attention limite à 4000 images!!!!"
#echo "------------------------"

if [ $# -lt 1 ]; then
  echo "---------------------------------------"
  echo 2 "$0: not enough arguments"
  echo "1er argument obligatoire, nom de la serie de photos"
  echo "2em optionel, decalage de la serie de photos"
  echo "----------      -------   --  ----- -  ---------- --------  kb "
  exit 2

fi

echo "------------------------------------"
echo " Départ de la boucle infinie"
echo "------------------------"

#si pas reprise, repertoire basse qualité
if [ $# -lt 2 ]; then
  mkdir bq640
fi

#valeur par defaut si pas de second parametre
nbphoto=${2:-0}

#for ((i=nbphoto;i<4000;i+=1)); do

while [ 1==1 ]
do
  nomfichier=$(printf '%s-%05d.jpg' $1 $nbphoto) #$i
  gphoto2 --capture-image-and-download -F 1 -I 3 --filename ./$nomfichier
  echo $nomfichier est la $nbphoto eme photo
  mogrify -resize 640x480 -path ./bq640 ./$nomfichier
  nbphoto=`expr $nbphoto + 1`
  #Time between pictures
  #sleep 1s

  #Arduino telecommande?
done
