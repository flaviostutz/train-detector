http://stackoverflow.com/questions/16058080/how-to-train-cascade-properly
http://www.memememememememe.me/training-haar-cascades/
mkdir positives

#CREATE POSITIVE SAMPLES WITH RANDOM BACKGROUNDS AND RANDOM ANGLES
#for each positive image, do
opencv_createsamples -img positive200x61.jpg -bg negative/negative.txt -info positive/positive.txt -num 1000 -maxxangle 0.0 -maxyangle 0.0 -maxzangle 0.2 -w 100 -h 31 -bgcolor 255 -bgthresh 0

#CREATE CROPPED POSITIVE SAMPLES FOR TRAINING
opencv_createsamples -info positive/positive.txt -bg negative/negative.txt -vec vecfile.vec -num 1000 -w 100 -h 31

#view generated images
opencv_createsamples -vec vecfile.vec -w 100 -h 31

#TRAIN CASCADE TO GENERATE cascade.xml FILE
opencv_traincascade -data ../out -vec ../vecfile.vec -bg negative.txt -numPos 100 -numNeg 200 -numStages 20 -precalcValBufSize 1024 -precalcIdxBufSize 1024 -minHitRate 0.995 -maxFaseAlarmRate 0.5 -w 100 -h 31 -numThreads 4 -featureType LBP
opencv_traincascade -data ../out -vec ../vecfile.vec -bg negative.txt -numPos 100 -numNeg 200 -numStages 20 -precalcValBufSize 1024 -precalcIdxBufSize 1024 -minHitRate 0.995 -maxFaseAlarmRate 0.5 -w 100 -h 31 -numThreads 4 -featureType HAAR
