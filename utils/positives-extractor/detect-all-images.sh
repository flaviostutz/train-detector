#!/bin/bash

for file in $1/$2
do
  build/positives-extractor $file >> results.out &
done

