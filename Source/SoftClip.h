/*
  ==============================================================================

    SoftClip.h
    Created: 23 May 2022 5:26:01pm
    Author:  Max Ellis

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>



class SoftClip
{
public:
    SoftClip();
    
    void prepare(dsp::ProcessSpec& spec);
    
    void processBlock(dsp::AudioBlock<float>& block)
    {
        for(int channel = 0; channel < block.getNumChannels(); ++channel)
        {
            float* data = block.getChannelPointer(channel);
            
            for(int sample = 0; sample < block.getNumSamples(); ++sample)
            {
                data[sample] = piDivisor * std::atanf(data[sample] * mDrive.getNextValue());
            }
        }
    }
    
    void setDrive(float newDrive);
    
private:
    
    float mSampleRate;
    juce::SmoothedValue<float> mDrive;
    float piDivisor = 2.0f / MathConstants<float>::pi;
    
};
