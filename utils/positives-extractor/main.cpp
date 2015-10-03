#include <stdio.h>
#include <unistd.h>
#include "DetectRegions.h"

string getFilename(string s) {

    char sep = '/';
    char sepExt='.';

    #ifdef _WIN32
        sep = '\\';
    #endif

    size_t i = s.rfind(sep, s.length( ));
    if (i != string::npos) {
        string fn= (s.substr(i+1, s.length( ) - i));
        size_t j = fn.rfind(sepExt, fn.length( ));
        if (i != string::npos) {
            return fn.substr(0,j);
        }else{
            return fn;
        }
    }else{
        return "";
    }
}


int main(int argc, char *argv[]) {
    // int rc;

    printf("Initializing Plate Detector...\n");

    //cout << "OpenCV Automatic Number Plate Recognition\n";
    char* filename;
    Mat input_image;

    //Check if user specify image to process
    if(argc >= 2 )
    {
        filename= argv[1];
        //load image  in gray level
        input_image=imread(filename,1);
    }else{
        printf("Use:\n\t%s image\n",argv[0]);
        return 0;
    }

    string filename_whithoutExt=getFilename(filename);
    cout << "working with file: "<< filename_whithoutExt << "\n";
    //Detect posibles plate regions
    DetectRegions detectRegions;
    detectRegions.setFilename(filename_whithoutExt);
    detectRegions.saveRegions=false;
    detectRegions.showSteps=false;
    vector<Plate> posible_regions= detectRegions.run( input_image );
    printf("Plates detected: %zu\n\n", posible_regions.size());

    waitKey(0);

	//while(1) {
		// rc = controller->loop();
		// if(rc){
			// controller->reconnect();
		// }
	//	controller->step();

		//usleep(100000);
	//}

	return 0;
}

