/*
  ==============================================================================

    ResponseCurve.cpp
    Created: 25 May 2022 12:57:00pm
    Author:  Max Ellis

  ==============================================================================
*/

#include "ResponseCurve.h"

//ResponseCurve::ResponseCurve(DistortionProjAudioProcessor& p) : audioProcessor(p)
//{
//    const auto& params = audioProcessor.getParameters();
//    
//    for(auto param : params)
//    {
//        param->addListener(this);
//    }
//    
//    startTimerHz(60);
//}
//
//
//ResponseCurve::~ResponseCurve()
//{
//    const auto& params = audioProcessor.getParameters();
//    
//    for(auto param : params)
//    {
//        param->removeListener(this);
//    }
//}
//
//void ResponseCurve::parameterValueChanged(int parameterIndex, float newValue)
//{
//    parametersChanged.set(true);
//}
//
//void ResponseCurve::timerCallback()
//{
//    if(parametersChanged.compareAndSetBool(false, true))
//    {
//        auto chainSettings = getChainSettings(audioProcessor.apvts);
//        auto lowCutCoefficients = makeLowCutFilter(chainSettings, audioProcessor.getSampleRate());
//        auto highCutCoefficients = makeHighCutFilter(chainSettings, audioProcessor.getSampleRate());
//        
//        updateCutFilter(monoChain.get<ChainPositions::lowCut>(), lowCutCoefficients);
//        updateCutFilter(monoChain.get<ChainPositions::highCut>(), highCutCoefficients);
//        
//        repaint();
//    }
//}
//
//void ResponseCurve::paint(juce::Graphics& g)
//{
//    auto bounds = getLocalBounds();
//    auto titleBarArea = bounds.removeFromTop(bounds.getHeight() * JUCE_LIVE_CONSTANT(0.1));
//    auto dropDownArea = titleBarArea.removeFromLeft(titleBarArea.getWidth() * 0.33);
//    auto dropDown = dropDownArea.reduced(5.f);
//    auto titleBar = titleBarArea.reduced(5.f);
//    auto metersArea = bounds.removeFromRight(bounds.getWidth() * JUCE_LIVE_CONSTANT(0.15));
//    auto thumbnailArea = metersArea.removeFromTop(metersArea.getHeight() * JUCE_LIVE_CONSTANT(0.33));
//    auto thumbNail = thumbnailArea.reduced(5.f);
//    auto meters = metersArea.reduced(5.f);
//    auto responseArea = bounds.removeFromTop(bounds.getHeight() * JUCE_LIVE_CONSTANT(0.5));
//    auto response = responseArea.reduced(5.f);
//    auto graphAreaSection = responseArea.reduced(20.f, 15.f);
//    auto graphArea = graphAreaSection.reduced(5.f);
//    auto parameters = bounds.removeFromLeft(bounds.getWidth() * JUCE_LIVE_CONSTANT(0.75));
//    
//    auto& lowcut = monoChain.get<ChainPositions::lowCut>();
//    auto& highcut = monoChain.get<ChainPositions::highCut>();
//    
//    auto sampleRate = audioProcessor.getSampleRate();
//    auto w = graphArea.getWidth();
//
//    
//    std::vector<double> mags;
//    
//    mags.resize(w);
//    
//    for(int i = 0; i < w; ++i)
//    {
//        double mag = 1.f;
//        auto freq = mapToLog10(double(i) / double(w), 20.0, 20000.0);
//        
//        if(!monoChain.isBypassed<ChainPositions::lowCut>()){
//            mag *= lowcut.coefficients->getMagnitudeForFrequency(freq, sampleRate);
//        }
//        if(!monoChain.isBypassed<ChainPositions::highCut>()){
//            mag *= highcut.coefficients->getMagnitudeForFrequency(freq, sampleRate);
//        }
//    
//        
//        mags[i] = Decibels::gainToDecibels(mag);
//    }
//    
//    Path responseCurve;
//    
//    const double outputMin = graphArea.getBottom();
//    const double outputMax = graphArea.getY();
//    auto map = [outputMin, outputMax](double input)
//    {
//        return jmap(input, -24.0, 24.0, outputMin, outputMax);
//    };
//    
//    responseCurve.startNewSubPath(graphArea.getX(), map(mags.front()));
//    
//    for(size_t i = 1; i<mags.size(); ++i)
//    {
//        responseCurve.lineTo(graphArea.getX() + i, map(mags[i]));
//    }
//    
//    g.setColour(Colours::white);
//    g.strokePath(responseCurve, PathStrokeType(2.f));
//}
