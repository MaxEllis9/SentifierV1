//
//  Distortion.cpp
//  distortionProj - VST3
//
//  Created by Max Ellis on 23/05/2022.
//

#include "Distortion.h"

Distortion::Distortion(){
    controlParams.distortionType = 0;
    controlParams.drive = 1.f;
    controlParams.mix = 0.f;
    controlParams.colour = 0.f;
    
}

Distortion::~Distortion() {}

void Distortion::prepare(AudioParameterFloat* drive, AudioParameterFloat* mix, AudioParameterFloat* colour, AudioParameterChoice* type)
{
    controlParams.drive = *drive;
    controlParams.mix = *mix;
    controlParams.colour = *colour;
    controlParams.distortionType = *type;
    
}

float Distortion::process(float sample)
{
    input = sample;
    output = input * controlParams.drive;
    
    switch(controlParams.distortionType){
        case 0:
            return sample;
        case 1:
            output = saturation(input, controlParams.drive);
            break;
        case 2:
            output = diode(input, controlParams.drive);
            break;
        case 3:
            output = bitcrush(input, controlParams.drive);
            break;
        case 4:
            output = fuzz(input, controlParams.drive);
            break;
        case 5:
            output = hardClip(input, controlParams.drive);
            break;
        case 6:
            output = softClip(input, controlParams.drive);
            break;
        default:
            output = input;
    }
    
    return ((1.f - controlParams.mix) * input) + (controlParams.mix * output);
}

float Distortion::softClip(float sample, float drive)
{
    float newInput = sample * (drive / 10);
    float out = 0.0;
    
    if(newInput >= 1.0f){
        out = 1.0f;
    }
    else if ((newInput > -1) && (newInput < 1)){
        out = (3.f / 2.f) * (newInput - (powf(newInput, 3.f) / 3.f));
    }
    else if (newInput <= -1){
        out = -1.f;
    }
    
    return out * 0.5f;
}

float Distortion::hardClip(float sample, float drive)
{
    float out = sample * (drive / 10);
    
    if(out >= 1.f){
        out = 1.f;
    }
    else if((out <= -1)){
        out = -1.f;
    }
    
    return (1.5f * out) - (0.5 * out * out * out);
    
}

float Distortion::saturation(float sample, float drive)
{
    float quality = -1.5f;
    float distortion = 5;
    float out;
    
    float newInput = sample * (drive / 10);
    
    if(quality == 0){
        out = newInput / (1 - expf(-distortion * newInput));
        if(newInput ==  quality){
            out = 1 / distortion;
        }
    }
    else {
        out = ((newInput - quality) / (1 - expf(-distortion * (newInput - quality)))) + (quality / (1 - expf(distortion * quality)));
        if (newInput == quality){
            out = (1 / distortion) + (quality / (1 - expf(distortion * quality)));
        }
    }
    return out * 0.8f;
}

float Distortion::diode(float sample, float drive)
{
    //    gain = gain + 1.0f;
        
        float diodeClippingAlgorithm = expf((0.1f * input) / (0.0253f * 1.68f)) - 1.0f;
        float out = 2 / juce::MathConstants<float>::pi * atan(diodeClippingAlgorithm * (drive * 16));
        
        return out * 0.3f;
    //    out = out/log(gain);

}

float Distortion::fuzz(float sample, float drive)
{
    float newInput = sample * drive;
    float out;
    
    //Soft clipping
    if (newInput < 0.0f)
        out = -1.0f *  (1.0f - expf(-abs(newInput)));
    else
        out = 1.0f * (1.0f - expf(-abs(newInput)));
 
    //Half Wave Rectifier
    out = 0.5f * (out + abs(out));
    
    return out * 0.8f;
}

float Distortion::bitcrush(float sample, float drive)
{
    
    return 0.f;
    
}
