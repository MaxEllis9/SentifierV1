//
//  Distortion.h
//  distortionProj
//
//  Created by Max Ellis on 23/05/2022.
//

#ifndef Distortion_h
#define Distortion_h


#endif /* Distortion_h */

#pragma once

#include <JuceHeader.h>
#include <cmath>

class Distortion
{
public:
    struct ControlParameters{
        int distortionType;
        float drive;
        float mix;
        float colour;
    } controlParams;
    
    Distortion();
    ~Distortion();
    float process(float sample);
    void prepare(AudioParameterFloat* drive, AudioParameterFloat* mix, AudioParameterFloat* colour, AudioParameterChoice* type);


private:
    float input, output = 0.f;
    float softClipThresh = 2.f / 3.f;
    
    float softClip(float sample, float drive);
    float hardClip(float sample, float drive);
    float saturation(float sample, float drive);
    float diode(float sample, float drive);
    float fuzz(float sample, float drive);
    float bitcrush(float sample, float drive);
};
