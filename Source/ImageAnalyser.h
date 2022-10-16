/*
  ==============================================================================

    ImageAnalyser.h
    Created: 14 Jun 2022 4:00:21pm
    Author:  Max Ellis

  ==============================================================================
*/

#pragma once

#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>

struct ImageAnalyser
{
public:
    std::vector<float> processImage(std::string path);
    void setParameterValues();
    std::string getAnalysisOutputString(int hue, int saturation, int value);
    
private:
    
};
