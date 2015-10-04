/*****************************************************************************
*   Number Plate Recognition using SVM and Neural Networks
******************************************************************************
*   by David Mill�n Escriv�, 5th Dec 2012
*   http://blog.damiles.com
******************************************************************************
*   Ch5 of the book "Mastering OpenCV with Practical Computer Vision Projects"
*   Copyright Packt Publishing 2012.
*   http://www.packtpub.com/cool-projects-with-opencv/book
*****************************************************************************/

#include "DetectRegions.h"

using namespace std;
using namespace cv;

int c;
char title[128];

void DetectRegions::setFilename(string s) {
    filename=s;
}

DetectRegions::DetectRegions() {
    showSteps=false;
    saveRegions=false;
}

vector<Plate> DetectRegions::segment(Mat input) {
    vector<Plate> output;

    //convert image to gray
    Mat img_gray = input;
    //img_gray = histeq(img_gray);
    cvtColor(img_gray, img_gray, CV_BGR2GRAY);
    blur(img_gray, img_gray, Size(5,5));

    printf("\nPHASE 1: OVERALL IMAGE FILTERING\n");

    printf("Image size is %dx%d\n", input.cols, input.rows);

    //Finde vertical lines. Car plates have high density of vertical lines
    printf("Applying Sobel filter on entire image...\n");
    Mat img_sobel;
    Sobel(img_gray, img_sobel, CV_8U, 1, 0, 3, 1, 0, BORDER_DEFAULT);
    if(showSteps)
        imshow("Sobel", img_sobel);

    //threshold image
    printf("Applying Threshold filter on entire image...\n");
    Mat img_threshold;
    threshold(img_sobel, img_threshold, 0, 255, CV_THRESH_OTSU+CV_THRESH_BINARY);
    if(showSteps)
        imshow("Threshold", img_threshold);

    //Morphplogic operation close
    printf("Applying Morphology (Close) filter on entire image...\n");
    Mat element = getStructuringElement(MORPH_RECT, Size(17, 3) );
    morphologyEx(img_threshold, img_threshold, CV_MOP_CLOSE, element);
    if(showSteps)
        imshow("Close", img_threshold);

    //Find contours of possibles plates
    printf("Finding contours on entire image...\n");
    vector< vector< Point> > contours;
    findContours(img_threshold,
            contours, // a vector of contours
            CV_RETR_EXTERNAL, // retrieve the external contours
            CV_CHAIN_APPROX_NONE); // all pixels of each contours

    //Remove patch that are no inside limits of aspect ratio and area.
    //Start to iterate to each contour found
    vector<vector<Point> >::iterator itc = contours.begin();
    vector<RotatedRect> rects;
    printf("Checking raw contour rect fitness... ");
    int ncontours = 0;
    while (itc!=contours.end()) {
        ncontours++;
        //Create bounding rect of object
        RotatedRect mr = minAreaRect(Mat(*itc));
        if(!verifySizes(mr)) {
            //itc = contours.erase(itc);
            ++itc;
        } else {
            ++itc;
            rects.push_back(mr);
        }
    }
    printf("%d/%d valid contours detected.\n", (int)rects.size(), (int)ncontours);

    // Draw blue contours on a white image
    cv::Mat contourImg;
    input.copyTo(contourImg);
    cv::drawContours(contourImg, contours,
            -1, // draw all contours
            cv::Scalar(255,0,0), // in blue
            2); // with a thickness of 1

    //if(showSteps)
        //imshow("Contours", contourImg);

    printf("\nPHASE 2: FLOODFILL CONTOURS AND DETECT REGIONS AREAS\n");

    //iterate over each rect identified from contours
    printf("Floodfilling contour areas around center...\n");
    vector<RotatedRect> rects2;
    for(int i=0; i<rects.size(); i++){
        RotatedRect mr = rects[i];

        //For better rect cropping for each posible box
        //Make floodfill algorithm because the plate has white background
        //And then we can retrieve more clearly the contour box
        circle(contourImg, mr.center, 3, Scalar(0,255,0), -1);
        //get the min size between width and height
        float minSize=(mr.size.width < mr.size.height)?mr.size.width:mr.size.height;
        minSize = minSize - minSize*0.5;
        //initialize rand and get 5 points around center for floodfill algorithm
        //srand ( time(NULL) );
        srand(0);
        //Initialize floodfill parameters and variables
        Mat mask;
        mask.create(contourImg.rows + 2, contourImg.cols + 2, CV_8UC1);
        mask = Scalar::all(0);
        int loDiff = 30;//30
        int upDiff = 30;//30
        int connectivity = 4;//4
        int newMaskVal = 255;//255
        int NumSeeds = 10;//10
        Rect ccomp;
        int flags = connectivity + (newMaskVal << 8 ) + CV_FLOODFILL_FIXED_RANGE + CV_FLOODFILL_MASK_ONLY;

        for(int j=0; j<NumSeeds; j++){
            Point seed;
            seed.x=mr.center.x+rand()%(int)minSize-(minSize/2);
            seed.y=mr.center.y+rand()%(int)minSize-(minSize/2);
            circle(contourImg, seed, 1, Scalar(0,255,255), -1);
            int area = floodFill(input, mask, seed, Scalar(255,0,0), &ccomp, Scalar(loDiff, loDiff, loDiff), Scalar(upDiff, upDiff, upDiff), flags);
        }

        //if(showSteps)
            //imshow("MASK", mask);
        //cvWaitKey(0);

        //Check new floodfill mask match for a correct patch.
        //Get all points detected for get Minimal rotated Rect
        vector<Point> pointsInterest;
        Mat_<uchar>::iterator itMask = mask.begin<uchar>();
        Mat_<uchar>::iterator end = mask.end<uchar>();
        for( ; itMask!=end; ++itMask) {
            //printf("POS %d,%d\n", itMask.pos().x, itMask.pos().y);
            if(*itMask==255) {
                pointsInterest.push_back(itMask.pos());
                //circle(contourImg, itMask.pos(), 1, Scalar(0,255,0), -1);
            }
        }

        RotatedRect minRect = minAreaRect(pointsInterest);
        rects2.push_back(minRect);
    }

    printf("Validating flooded areas (are they plates?)...\n");
    for(int i=0; i<rects2.size(); i++){
        RotatedRect minRect = rects2[i];
        printf(">> Candidate plate area %d... ", i+1);
        if(verifySizes(minRect)) {
            //Draw rotated rectangle
            Point2f rect_points[4]; minRect.points(rect_points);
            for(int j=0; j<4; j++) {
                line(contourImg, rect_points[j], rect_points[(j+1)%4], Scalar(0,0,255), 2, 8);
            }
            imshow("Contours", contourImg);

            printf("Rotating, resizing and croping candidate area...\n");

            //Get rotation matrix
            float r = (float)minRect.size.width / (float)minRect.size.height;
            float angle = minRect.angle;
            if(r<1) {
                angle=90+angle;
            }
            Mat rotmat = getRotationMatrix2D(minRect.center, angle,1);

            //Create and rotate image
            Mat img_rotated;
            warpAffine(input, img_rotated, rotmat, input.size(), CV_INTER_CUBIC);

            //Crop image
            Size rect_size = minRect.size;
            if(r < 1) {
                swap(rect_size.width, rect_size.height);
            }
            Mat img_crop;
            getRectSubPix(img_rotated, rect_size, minRect.center, img_crop);

            //Resize image to default
            Mat resultResized;
            resultResized.create(33,33*3.07, CV_8UC3);//brazil plates
            resize(img_crop, resultResized, resultResized.size(), 0, 0, INTER_CUBIC);

            //Equalize croped image
            Mat grayResult;
            cvtColor(resultResized, grayResult, CV_BGR2GRAY);
            //grayResult = histeq(grayResult);
        //            blur(grayResult, grayResult, Size(3,3));
        //            grayResult=histeq(grayResult);

            printf("Verifying if it looks like a plate... ");
            if(verifyPossibleLettersInside(grayResult)) {
            //if(true) {
            //if(false) {
                printf("CONFIRMED! %d\n", i+1);
                output.push_back(Plate(grayResult,minRect.boundingRect()));

            } else {
              sprintf(title, "NOT A PLATE! %d\n", i+1);
              //imshow(title, grayResult);
              printf("%s", title);
            }
        } else {
          printf("rejected.\n");
        }
    }

    printf("\nPHASE 3: GENERATE RESULTS\n");

    for(int i=0; i<output.size(); i++){
        Plate plate = output[i];
        if(saveRegions) {
            stringstream ss(stringstream::in | stringstream::out);
            ss << "tmp/" << filename << "_" << i << ".jpg";
            imwrite(ss.str(), plate.plateImg);
        }

        sprintf(title, "Found plate %d\n", i+1);
        imshow(title, plate.plateImg);
    }

    return output;
}


bool DetectRegions::verifySizes(RotatedRect mr) {
    //printf("Validating rect aspect (%d,%d) (%d,%d)...\n", (int)mr.center.x, (int)mr.center.y, (int)mr.size.width, (int)mr.size.height);

    float error=0.4;//0.4
    //Spain car plate size: 52x11 aspect 4,7272
    //float aspect=4.7272;
    float aspect=3.07;//brasil aspect
    //Set a min and max area. All other patchs are discarded
    int amin= 10*10*aspect; // 15*15 minimum area
    int amax= 300*300*aspect; // 125*125 maximum area
    //Get only patchs that match to a respect ratio.
    float rmin = aspect-aspect*error;
    float rmax = aspect+aspect*error;

    int area = mr.size.height * mr.size.width;
    float r = (float)mr.size.width / (float)mr.size.height;
    if(r<1) {
        r = (float)mr.size.height / (float)mr.size.width;
    }

    // if(mr.size.width>20 && mr.size.height>20 && mr.center.x>30 && mr.center.y>30) {
    //   return true;
    // } else {
    //   return false;
    // }

    if(( area < amin || area > amax )
         || ( r < rmin || r > rmax )
         || (mr.angle < -35 || mr.angle > 35)
         || (mr.size.width < mr.size.height)) {
        return false;
    } else {
        return true;
    }

}

bool DetectRegions::verifyPossibleLettersInside(Mat image) {

  int minLetterWidth = image.rows*0.23;
  int maxLetterWidth = image.cols*0.30;
  int minLetterHeight = image.rows*0.25;
  int maxLetterHeight = image.rows*0.85;

  printf("minLetterWidth: %d, minLetterHeight: %d, maxLetterWidth: %d, maxLetterHeight: %d\n", minLetterWidth, minLetterHeight, maxLetterWidth, maxLetterHeight);

  Mat img_work;
  image.copyTo(img_work);
  //blur(img_work, img_work, Size(3,3));

  //threshold image
  threshold(img_work, img_work, 0, 255, CV_THRESH_OTSU+CV_THRESH_BINARY);
  // sprintf(title, "Threshold-letters %d\n", image.cols);
  // imshow(title, img_work);

  vector< vector< Point> > contours;
  findContours(img_work,
          contours, // a vector of contours
          CV_RETR_LIST, // retrieve the external contours
          CV_CHAIN_APPROX_SIMPLE); // all pixels of each contours

  drawContours(img_work, contours,
        -1, // draw all contours
        cv::Scalar(255,0,0), // in blue
        1); // with a thickness of 1

  //imshow("letters-contours", img_work);

  //identify rects from contours
  vector<vector<Point> >::iterator itc = contours.begin();
  vector<RotatedRect> rects;
  while (itc!=contours.end()) {
      //Create bounding rect of object
      RotatedRect rect = minAreaRect(Mat(*itc));
      ++itc;

      rect.angle = (float)((int)rect.angle%90);
      //printf("Rotated rects %d %d %d %d %d\n", (int)rect.center.x, (int)rect.center.y, (int)rect.size.width, (int)rect.size.height, (int)rect.angle);
      if(rect.angle>=-15 && rect.angle<=15
          && rect.size.width >= minLetterWidth && rect.size.height >= minLetterHeight
          && rect.size.width <= maxLetterWidth && rect.size.height <= maxLetterHeight) {
        rects.push_back(rect);

        //show found rectangle
        Point2f rect_points[4];
        rect.points(rect_points);
        for(int j = 0; j<4; j++) {
            line(img_work, rect_points[j], rect_points[(j+1)%4], Scalar(255,0,0), 1, 8);
        }
      }
  }
  //if(showSteps) {
      //sprintf(title, "Mask-letters %d\n", image.cols);
      //imshow(title, img_work);
  //}

  //validate blobs if they seem to be plate letters
  int totalWidth = 0;
  int lastY = 0;
  for(int i=0; i<rects.size(); i++) {
    RotatedRect rect = rects[i];
    //printf("Found blobs %zu %d %d %d\n", rects.size(), (int)rect.size.width, (int)rect.size.height, lastY);
    if((lastY==0 || (rect.center.y >= (lastY-2)) && (rect.center.y <= (lastY+2)))) {
        // ) {
        totalWidth += rect.size.width;
        lastY = rect.center.y;
    }
  }

  sprintf(title, "Letters %d %d %d", image.rows, image.cols, totalWidth);
  imshow(title, img_work);

  //verify if letters with is normal
  if(totalWidth <= image.cols*0.90 && totalWidth >= image.cols*0.30) {
    return true;
  } else {
    return false;
  }
}


vector<Plate> DetectRegions::run(Mat input){

    //Segment image by white
    vector<Plate> tmp=segment(input);

    //return detected and posibles regions
    return tmp;
}

Mat DetectRegions::histeq(Mat in) {
    Mat out(in.size(), in.type());
    if(in.channels()==3){
        Mat hsv;
        vector<Mat> hsvSplit;
        cvtColor(in, hsv, CV_BGR2HSV);
        split(hsv, hsvSplit);
        equalizeHist(hsvSplit[2], hsvSplit[2]);
        merge(hsvSplit, hsv);
        cvtColor(hsv, out, CV_HSV2BGR);
    }else if(in.channels()==1){
        equalizeHist(in, out);
    }

    return out;

}
