/*
  ==============================================================================

    SoftClip.cpp
    Created: 23 May 2022 5:26:10pm
    Author:  Max Ellis

  ==============================================================================
*/

#include "SoftClip.h"

SoftClip::SoftClip() : mSampleRate(44100.0f), mDrive(1.0f)
{
    
}

void SoftClip::prepare(dsp::ProcessSpec &spec)
{
    mSampleRate = spec.sampleRate;
    mDrive.reset(mSampleRate, 0.05);
}

void SoftClip::setDrive(float newDrive)
{
    mDrive.setTargetValue(Decibels::decibelsToGain(newDrive));
}
