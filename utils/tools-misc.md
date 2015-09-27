#command line tool for manipulating images in batch
apt-get install imagemagick

#resize image to 320 width
mogrify -resize 320x240! -quality 100 -path ../imagens-resized *.jpg

#create cropped plates file from full car images based on x,y rectangles defined on plates.txt
awk -F " " '{system("./crop-region.sh "$1" "$39" "$40" "$41" "$42)}' plates.txt

#crop-region.sh
#!/bin/bash
echo "Cropping $1 region ($2x$3 $4x$5)"
convert -crop $(($4-$2))x$(($5-$3))+$2+$3 +repage $1 cropped-$1

