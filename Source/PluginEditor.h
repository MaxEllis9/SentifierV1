/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once
//
//#include <opencv2/imgcodecs.hpp>
//#include <opencv2/highgui.hpp>
//#include <opencv2/imgproc.hpp>
//#include <iostream>
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "GainMeter.h"
#include "ImageAnalyser.h"

struct CustomRotarySlider : juce::Slider
{
    CustomRotarySlider() : juce::Slider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                                        juce::Slider::TextEntryBoxPosition::NoTextBox)
    {
        
    }
    
};



enum FFTOrder
{
    order2048 = 11,
    order4096 = 12,
    order8192 = 13
};

template<typename BlockType>
struct FFTDataGenerator
{
    void produceFFTDataForRendering(const juce::AudioBuffer<float>& audioData, const float negativeInfinity)
    {
        const auto fftSize = getFFTSize();
        
        fftData.assign(fftData.size(), 0);
        auto* readIndex = audioData.getReadPointer(0);
        std::copy(readIndex, readIndex + fftSize, fftData.begin());
        
        //first apply a windowing function to the data
        window->multiplyWithWindowingTable (fftData.data(), fftSize);
        
        //then render FFt data
        forwardFFT->performFrequencyOnlyForwardTransform (fftData.data());
        
        int numBins = (int)fftSize / 2;
        
        //normalise fft values
        for(int i=0; i<numBins; ++i)
        {
            fftData[i] /= (float)numBins;
        }
        
        //convert to dBs
        for(int i=0; i<numBins; ++i)
        {
            fftData[i] = juce::Decibels::gainToDecibels(fftData[i], negativeInfinity);
        }
        
        fftDataFifo.push(fftData);
    }
    
    void changeOrder(FFTOrder newOrder)
    {
        //when you change order, recreate the window, forwardfft, fifo, fftdata
        //also reset Fifo index
        //things that need recreating should be created on the heap using std::make_unique<>
        
        order = newOrder;
        auto fftSize = getFFTSize();
        
        forwardFFT = std::make_unique<juce::dsp::FFT>(order);
        window = std::make_unique<juce::dsp::WindowingFunction<float>>(fftSize, juce::dsp::WindowingFunction<float>::blackmanHarris);
        
        fftData.clear();
        fftData.resize(fftSize * 2, 0);
        
        fftDataFifo.prepare(fftData.size());
    }
    
    //==============================================================================
    int getFFTSize() const {return 1 << order;}
    int getNumAvailableFFTDataBlocks() const {return fftDataFifo.getNumAvailableForReading();}
    //==============================================================================
    bool getFFTData(BlockType& fftData) {return fftDataFifo.pull(fftData);}
private:
    FFTOrder order;
    BlockType fftData;
    std::unique_ptr<juce::dsp::FFT> forwardFFT;
    std::unique_ptr<juce::dsp::WindowingFunction<float>> window;
    
    Fifo<BlockType> fftDataFifo;
};






template<typename PathType>
struct AnalyzerPathGenerator
{

    void generatePath(const std::vector<float>& renderData,
                      juce::Rectangle<float> fftBounds,
                      int fftSize,
                      float binWidth,
                      float negativeInfinity)
    {
        auto top = fftBounds.getY();
        auto bottom = fftBounds.getHeight();
        auto width = fftBounds.getWidth();

        int numBins = (int)fftSize / 2;

        PathType p;
        p.preallocateSpace(3 * (int)fftBounds.getWidth());

        auto map = [bottom, top, negativeInfinity](float v)
        {
            return juce::jmap(v,
                              negativeInfinity,
                              0.f,
                              (float)bottom,
                              top);
        };

        auto y = map(renderData[0]);
        jassert(!std::isnan(y) && !std::isinf(y));

        p.startNewSubPath(0, y);

        const int pathResolution = 2; //you can draw line-to's every 'pathResolution' pixels

        for(int binNum=1; binNum<numBins; binNum += pathResolution)
        {
            y = map(renderData[binNum]);

            jassert(!std::isnan(y) && !std::isinf(y));

            if(!std::isnan(y) && !std::isinf(y)){
                auto binFreq = binNum * binWidth;
                auto normalizedBinX = juce::mapFromLog10(binFreq, 20.f, 20000.f);
                int binX = std::floor(normalizedBinX * width);
                p.lineTo(binX, y);
            }
        }

        pathFifo.push(p);
    }

    int getNumPathsAvailable() const
    {
        return pathFifo.getNumAvailableForReading();
    }

    bool getPath(PathType& path)
    {
        return pathFifo.pull(path);
    }

private:
    Fifo<PathType> pathFifo;
};



struct PathProducer
{
    PathProducer(SingleChannelSampleFifo<DistortionProjAudioProcessor::BlockType>& scsf) :
    leftChannelFifo(&scsf)
    {
        leftChannelFFTDataGenerator.changeOrder(FFTOrder::order8192);
        monoBuffer.setSize(1, leftChannelFFTDataGenerator.getFFTSize());
    }
    void process(juce::Rectangle<float> fftBounds, double sampleRate);
    juce::Path getPath() {return leftChannelFFTPath;}
private:
    SingleChannelSampleFifo<DistortionProjAudioProcessor::BlockType>* leftChannelFifo;

    juce::AudioBuffer<float> monoBuffer;

    FFTDataGenerator<std::vector<float>> leftChannelFFTDataGenerator;

    AnalyzerPathGenerator<juce::Path> pathProducer;

    juce::Path leftChannelFFTPath;
};




struct CustomLookAndFeel : juce::LookAndFeel_V4
{
    void drawRotarySlider(juce::Graphics&,
                          int x, int y, int width, int height,
                          float sliderPosProportional,
                          float rotaryStartAngle,
                          float rotaryEndAngle,
                          juce::Slider& slider) override;
    
    void drawToggleButton(juce::Graphics& g,
                          juce::ToggleButton& toggleButton,
                          bool shouldDrawButtonAsHighlighted,
                          bool shouldDrawButtonAsDown) override;
};

struct PowerButton : juce::ToggleButton {};

struct RotarySliderWithLabels : juce::Slider
{
    RotarySliderWithLabels(juce::RangedAudioParameter& rap, const juce::String& unitSuffix) :
    juce::Slider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                 juce::Slider::TextEntryBoxPosition::NoTextBox),
    param(&rap),
    suffix(unitSuffix)
    {
        setLookAndFeel(&lnf);
        
    }
    
    ~RotarySliderWithLabels()
    {
        setLookAndFeel(nullptr);
    }
    
    struct LabelPos
    {
        float pos;
        juce::String label;
    };
    
    juce::Array<LabelPos> labels;
    
    void paint(juce::Graphics& g) override;
    
    juce::Rectangle<int> getSliderBounds() const;
    
    int getTextHeight() const {return 14;}
    
    juce::String getDisplayString() const;
private:
    CustomLookAndFeel lnf;
    
    juce::RangedAudioParameter* param;
    juce::String suffix;
};


struct ResponseCurve : juce::Component, juce::AudioProcessorParameter::Listener, juce::Timer
{
public:
    ResponseCurve(DistortionProjAudioProcessor&);
    ~ResponseCurve();

    //==============================================================================
    void timerCallback() override;
    void paint (juce::Graphics&) override;
    
    void parameterValueChanged (int parameterIndex, float newValue) override;

    void parameterGestureChanged (int parameterIndex, bool gestureIsStarting) override {}
    
    void toggleAnalysisEnablement(bool enabled)
    {
        shouldShowFFT = enabled;
    }
    
private:
    DistortionProjAudioProcessor& audioProcessor;
    juce::Atomic<bool> parametersChanged {false};
    
    MonoChain monoChain;
    
    juce::Rectangle<int> getAnalysisArea();
    juce::Rectangle<int> getRenderArea();
    
    PathProducer leftPathProducer, rightPathProducer;
    
    bool shouldShowFFT = true;
    
};


//==============================================================================

class DistortionProjAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    DistortionProjAudioProcessorEditor (DistortionProjAudioProcessor&);
    ~DistortionProjAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void resetImage();
    void initialisePlugin();
    
//    void createLabel(String string, Label label);

    
private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    DistortionProjAudioProcessor& audioProcessor;
        
    ResponseCurve responseCurveComponent;
    
    Label inputGainLabel,
    outputGainLabel,
    nameLabel,
    driveKnobLabel,
    mixKnobLabel,
    inGainLabel,
    outGainLabel;
    
    Gui::GainMeter GainMeterInL,
    GainMeterInR,
    GainMeterOutL,
    GainMeterOutR;
    
    Gui::Bulb onOffBulb;
    
    RotarySliderWithLabels driveKnob,
    highCutKnob,
    lowCutKnob,
    inputGainKnob,
    outputGainKnob,
    mixKnob;
    
    PowerButton onOffButton,
    highCutBypass,
    lowCutBypass,
    driveBypass,
    colourBypass,
    inputGainBypass,
    outputGainBypass;
    
    ImageButton onOffSwitch;
    
    ComboBox distortionType;
    
    ImageComponent imageUpload, lowPassSymbolImg, highPassSymbolImg;
    
    TextButton loadImageButton {"Upload Image"}, menuButton {"Menu"};
    
    TextEditor imageAnalysisOutput;
    
    std::vector<juce::Component*> getComponents();
    std::vector<juce::Button*> getButtons();

    using APVTS = juce::AudioProcessorValueTreeState;
    
    APVTS::SliderAttachment driveKnobAttachment,
    highCutKnobAttachment,
    lowCutKnobAttachment,
    inputGainKnobAttachment,
    outputGainKnobAttachment,
    mixKnobAttachment;
    
    APVTS::ButtonAttachment onOffButtonAttachment,
    highCutBypassAttachment,
    lowCutBypassAttachment,
    driveBypassAttachment,
    inputGainBypassAttachment,
    outputGainBypassAttachment;
    
    APVTS::ComboBoxAttachment distortionTypeAttachment;
    
    CustomLookAndFeel lnf;
    
    ImageAnalyser imageAnalyser;
    
    PopupMenu menuPopUp;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DistortionProjAudioProcessorEditor)
};
