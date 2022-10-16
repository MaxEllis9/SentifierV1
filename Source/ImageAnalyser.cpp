/*
  ==============================================================================

    ImageAnalyser.cpp
    Created: 14 Jun 2022 4:00:21pm
    Author:  Max Ellis

  ==============================================================================
*/

#include "ImageAnalyser.h"

using namespace cv;

std::vector<float> ImageAnalyser::processImage(std::string path)
{
    Mat img = imread(path);
    Mat imgHSV, imgHLS;
    
    cvtColor(img, imgHSV, COLOR_BGR2HSV);
    cvtColor(img, imgHLS, COLOR_BGR2HLS);
    
//    Scalar avg = cv::mean(img);
//    Scalar avg2 = cv::mean(imgHLS);
    Scalar avg3 = cv::mean(imgHSV);
    
    std::vector<float> HSVvalues;
    HSVvalues.push_back(avg3[0]);
    HSVvalues.push_back(avg3[1]);
    HSVvalues.push_back(avg3[2]);
    
//    imshow("Image", img);
//    imshow("ImageHSV", imgHSV);
//    imshow("ImageHLS", imgHLS);

    
//    waitKey(0);

//    ***** Histogram code *****
//    Mat src = imread( samples::findFile( parser.get<String>( "@input" ) ), IMREAD_COLOR );
//        if( src.empty() )
//        {
//            return EXIT_FAILURE;
//        }
//        vector<Mat> bgr_planes;
//        split( src, bgr_planes );
//        int histSize = 256;
//        float range[] = { 0, 256 }; //the upper boundary is exclusive
//        const float* histRange[] = { range };
//        bool uniform = true, accumulate = false;
//        Mat b_hist, g_hist, r_hist;
//        calcHist( &bgr_planes[0], 1, 0, Mat(), b_hist, 1, &histSize, histRange, uniform, accumulate );
//        calcHist( &bgr_planes[1], 1, 0, Mat(), g_hist, 1, &histSize, histRange, uniform, accumulate );
//        calcHist( &bgr_planes[2], 1, 0, Mat(), r_hist, 1, &histSize, histRange, uniform, accumulate );
//        int hist_w = 512, hist_h = 400;
//        int bin_w = cvRound( (double) hist_w/histSize );
//        Mat histImage( hist_h, hist_w, CV_8UC3, Scalar( 0,0,0) );
//        normalize(b_hist, b_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat() );
//        normalize(g_hist, g_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat() );
//        normalize(r_hist, r_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat() );
//        for( int i = 1; i < histSize; i++ )
//        {
//            line( histImage, Point( bin_w*(i-1), hist_h - cvRound(b_hist.at<float>(i-1)) ),
//                  Point( bin_w*(i), hist_h - cvRound(b_hist.at<float>(i)) ),
//                  Scalar( 255, 0, 0), 2, 8, 0  );
//            line( histImage, Point( bin_w*(i-1), hist_h - cvRound(g_hist.at<float>(i-1)) ),
//                  Point( bin_w*(i), hist_h - cvRound(g_hist.at<float>(i)) ),
//                  Scalar( 0, 255, 0), 2, 8, 0  );
//            line( histImage, Point( bin_w*(i-1), hist_h - cvRound(r_hist.at<float>(i-1)) ),
//                  Point( bin_w*(i), hist_h - cvRound(r_hist.at<float>(i)) ),
//                  Scalar( 0, 0, 255), 2, 8, 0  );
//        }
//        imshow("Source image", src );
//        imshow("calcHist Demo", histImage );
//        waitKey();
    
    return HSVvalues;
}

std::string ImageAnalyser::getAnalysisOutputString(int hue, int saturation, int value)
{
    std::string output = "Distortion type determined by hue: ";
    
    switch(hue)
    {
        case 0 ... 15 :
            output.append("Saturation\n\n");
        break;
            
        case 164 ... 179 :
            output.append("Saturation\n\n");
        break;
            
        case 105 ... 132  :
            output.append("Hard Clip\n\n");
        break;
            
        case 133 ... 163 :
            output.append("Soft Clip\n\n");
        break;
            
        case 46 ... 76 :
            output.append("Diode Distortion\n\n");
        break;
            
        case 16 ... 45 :
            output.append("Tape Distortion\n\n");
        break;
            
        case 77 ... 104 :
            output.append("Tube Distortion\n\n");
        break;
    }
    
    output.append("Drive value selected by saturation levels: ");
    output += std::to_string(saturation);
    output.append("dB\n\n");
    
    output.append("Frequency range distorted determined by image brightness levels:");
    switch(value)
    {
        case 0 ... 85 :
            output.append(" 0Hz-400Hz");
        break;
            
        case 86 ... 170  :
            output.append(" 401Hz-2000Hz");
        break;
            
        case 171 ... 255 :
            output.append(" 2001Hz-20000Hz");
        break;
    }

    return output;
}
