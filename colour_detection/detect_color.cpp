#include <iostream>
#include "opencv2/opencv.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
using namespace cv;
using namespace std;

int main()
{
    //Mat image=imread("output.png",1); //1 because it's coloured
     
    //Capture the image from the webcam
    Mat image; //cap >> image;
    VideoCapture cap(0); cap >> image;

    if(image.empty())
        std::cerr << "Something is wrong with the picam." << endl;;
        
    //namedWindow("image", CV_WINDOW_FREERATIO);
    //imshow("image",image); //display the original image
    //waitKey(1);

    //Converting image from BGR to HSV color space.
    Mat OutputImageRed;
    cvtColor(image, OutputImageRed, COLOR_BGR2HSV);

    Mat maskred1, maskred2;
    // Creating masks to detect the upper and lower red
    inRange(OutputImageRed, Scalar(140, 100, 70), Scalar(180, 255, 255), maskred1);
    inRange(OutputImageRed, Scalar(0, 100, 70), Scalar(20, 255, 255), maskred2);

    //Generating the final mask
    maskred1 = maskred1+maskred2;

    //Now for green 
    Mat OutputImageGreen;
    cvtColor(image, OutputImageGreen, COLOR_BGR2HSV);

    Mat maskgreen1, maskgreen2;
    // Creating masks to detect the upper and lower red
    //inRange(OutputImageGreen, Scalar(40, 100, 70), Scalar(80, 255, 255), maskgreen1);
    inRange(OutputImageGreen, Scalar(40, 100, 70), Scalar(80, 255, 255), maskgreen1);

   //inRange(OutputImageBlue, Scalar(170, 120, 70), Scalar(100, 255, 255), maskred2);
    
    //Generating the final mask
   // maskgreen1 = maskgreen1+maskgreen2;

    int red_count,green_count;
    red_count=countNonZero(maskred1);
    green_count=countNonZero(maskgreen1);

    if (red_count > green_count) 
        cout << "The color is red" << endl;
    else if (red_count < green_count) 
        cout << "The color is green" << endl;
    else
        cout << "No green or red detected" << endl;
    
    cout << red_count << " " << green_count << endl;
    //namedWindow("Output_Red", CV_WINDOW_FREERATIO);
    //imshow("Output_Red", maskred1);

    //namedWindow("Output_Green", CV_WINDOW_FREERATIO);
    //imshow("Output_Green", maskgreen1);
    //waitKey(0);

}
