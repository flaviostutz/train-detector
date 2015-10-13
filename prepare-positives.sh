#!/bin/bash

if [ "$1" == "" ]; then
  echo "Usage: $0 [directory containing images]"
  exit 1
fi

rm positive/* -R

for f in $1/*
do
   filename=$(basename $f)
   fname=${filename%.*}
   posdir=positive/$fname
   echo "Processing $fname..."
   opencv_createsamples -img $f -bg negative/negative.txt -info $posdir/positive.txt -num 8 -maxxangle 0.23 -maxyangle 0.23 -maxzangle 0.12 -w 125 -h 46 -bgcolor 255 -bgthresh 0
   awk '{print "'"$fname"'/" $0}' $posdir/positive.txt >> positive/positive.txt
done

nr_imgs=$(wc -l positive/positive.txt | awk '{print $1}')
cd positive
opencv_createsamples -info positive.txt -bg ../negative/negative.txt -vec vecfile.vec -num $nr_imgs -w 125 -h 46
