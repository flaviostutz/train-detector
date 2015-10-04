#!/bin/bash

#example usage: /detect-all-images.sh raw-images-br plate*.jpg

for file in $1/$2
do
  build/positives-extractor $file >> results.out &
done


