/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


void CustomLookAndFeel::drawRotarySlider(juce::Graphics & g,
                                   int x,
                                   int y,
                                   int width,
                                   int height,
                                   float sliderPosProportional,
                                   float rotaryStartAngle,
                                   float rotaryEndAngle,
                                   juce::Slider &slider)
{
    using namespace juce;
    
    auto bounds = Rectangle<float>(x, y ,width, height);
    
    auto enabled = slider.isEnabled();
    
    g.setColour(enabled ? Colours::black : Colours::slategrey);
    g.fillEllipse(bounds);
    
    g.setColour(enabled ? Colours::darkgrey : Colours::dimgrey);
    g.drawEllipse(bounds, 3.f);
    
    g.setColour(Colours::white);
    
    if(auto* rswl = dynamic_cast<RotarySliderWithLabels*>(&slider))
    {
        auto center = bounds.getCentre();
        
        Path p;
        
        Rectangle<float> r;
        r.setLeft(center.getX() - 2);
        r.setRight(center.getX() + 2);
        r.setTop(bounds.getY());
        r.setBottom(center.getY() - rswl->getTextHeight() * 1.5);
        
        p.addRoundedRectangle(r, 2.f);
        
        jassert(rotaryStartAngle < rotaryEndAngle);
        
        auto sliderAngRad = jmap(sliderPosProportional, 0.f, 1.f, rotaryStartAngle, rotaryEndAngle);
        
        p.applyTransform(AffineTransform().rotated(sliderAngRad, center.getX(), center.getY()));
        
        g.fillPath(p);
        
        g.setFont(rswl->getTextHeight());
        auto text = rswl->getDisplayString();
        auto strWidth = g.getCurrentFont().getStringWidth(text);
        
        r.setSize(bounds.getWidth()-6, rswl->getTextHeight() + 2);
        r.setCentre(center);
        
        g.setColour(Colours::black);
//        g.fillRect(r);
        
        g.setColour(Colours::white);
        g.drawFittedText(text, r.toNearestInt(), juce::Justification::centred, 1);
        
    }
    
}

void CustomLookAndFeel::drawToggleButton(juce::Graphics &g,
                                   juce::ToggleButton &toggleButton,
                                   bool shouldDrawButtonAsHighlighted,
                                   bool shouldDrawButtonAsDown)
{
    if(auto* pb = dynamic_cast<PowerButton*>(&toggleButton)){
        Path powerButton;
        
        auto bounds = toggleButton.getLocalBounds().removeFromBottom(toggleButton.getLocalBounds().getHeight() * 0.9);
        auto size = jmin(bounds.getWidth(), bounds.getHeight() - 1);
        auto r = Rectangle<float>(bounds.getX() + 15, bounds.getY(), size, size);
        
        float ang = 35.f;
        
        size -= 7;
        
        powerButton.addCentredArc(r.getCentreX(),
                                  r.getCentreY(),
                                  size * 0.5,
                                  size * 0.5,
                                  0.f,
                                  degreesToRadians(ang),
                                  degreesToRadians(360.f - ang),
                                  true);
        
        powerButton.startNewSubPath(r.getCentreX(), r.getY());
        powerButton.lineTo(r.getCentre());
        
        PathStrokeType pst(2, PathStrokeType::JointStyle::curved);
        
        auto colour = toggleButton.getToggleState() ? Colours::dimgrey : Colours::skyblue;
        
        g.setColour(colour);
        g.strokePath(powerButton, pst);
        g.drawEllipse(r, 2.f);
    }
//    else if(auto* analyzerButton = dynamic_cast<AnalyzerButton*>(&toggleButton))
//    {
//        auto colour = !toggleButton.getToggleState() ? Colours::dimgrey : Colours::green;
//
//        g.setColour(colour);
//
//        auto bounds = toggleButton.getLocalBounds();
//        g.drawRect(bounds);
//
//        g.strokePath(analyzerButton->randomPath, PathStrokeType(1.f));
//    }
    
}

void RotarySliderWithLabels::paint(juce::Graphics &g)
{
    using namespace juce;
    
    auto startAng = degreesToRadians(180.f + 45.f);
    auto endAng = degreesToRadians(180.f - 45.f) + MathConstants<float>::twoPi;
    auto range = getRange();
    auto sliderBounds = getSliderBounds();
    
//    g.setColour(Colours::red);
//    g.drawRect(getLocalBounds());
//    g.setColour(Colours::yellow);
//    g.drawRect(sliderBounds);
    
    
    getLookAndFeel().drawRotarySlider(g,
                                      sliderBounds.getX(),
                                      sliderBounds.getY(),
                                      sliderBounds.getWidth(),
                                      sliderBounds.getHeight(),
                                      jmap(getValue(),
                                           range.getStart(),
                                           range.getEnd(),
                                           0.0,
                                           1.0),
                                      startAng,
                                      endAng,
                                      *this);
    
    auto center = sliderBounds.toFloat().getCentre();
    auto radius = sliderBounds.getWidth() * 0.5f;
    
    g.setColour(Colours::black);
    g.setFont(getTextHeight());
    
    auto numChoices = labels.size();
    for(int i=0; i<numChoices; ++i)
    {
        auto pos = labels[i].pos;
        jassert(0.f <= pos);
        jassert(pos <= 1.f);
        
        auto ang = jmap(pos, 0.f, 1.f, startAng, endAng);
        
        auto c = center.getPointOnCircumference(radius + getTextHeight() * 0.5f + 1, ang);
        
        Rectangle<float> r;
        auto str = labels[i].label;
        r.setSize(g.getCurrentFont().getStringWidth(str), getTextHeight());
        r.setCentre(c);
        r.setY(r.getY() + getTextHeight());
        
        g.drawFittedText(str, r.toNearestInt(), juce::Justification::centred, 1);
        
    }
}

juce::Rectangle<int> RotarySliderWithLabels::getSliderBounds() const
{
    auto bounds = getLocalBounds();
    
    auto size = juce::jmin(bounds.getWidth(), bounds.getHeight());
    
    size -= getTextHeight() * 2;
    juce::Rectangle<int> r;
    r.setSize(size, size);
    r.setCentre(bounds.getCentreX(), 0);
    r.setY(2);
    
    return r;
}

juce::String RotarySliderWithLabels::getDisplayString() const
{
    //return juce::String(getValue());
    if(auto* choiceParam = dynamic_cast<juce::AudioParameterChoice*>(param))
    {
        return choiceParam->getCurrentChoiceName();
    }
    
    juce::String str;
    bool addK = false;
    
    if(auto* floatParam = dynamic_cast<juce::AudioParameterFloat*>(param))
    {
        float val = getValue();
        if(val>999.f)
        {
            val /= 1000.f;
            addK = true;
        }
        str = juce::String(val, (addK ? 2 : 0));
    }
    else
    {
        jassertfalse; //this shouldn't happen!!
    }
    
    if(suffix.isNotEmpty())
    {
        str << " ";
        if(addK)
        {
            str << "k";
        }
        str << suffix;
    }
    return str;
}


void PathProducer::process(juce::Rectangle<float> fftBounds, double sampleRate)
{
    juce::AudioBuffer<float> tempIncomingBuffer;
    
    while (leftChannelFifo->getNumCompleteBuffersAvailable() > 0)
    {
        if(leftChannelFifo->getAudioBuffer(tempIncomingBuffer))
        {
            auto size = tempIncomingBuffer.getNumSamples();
            
            juce::FloatVectorOperations::copy(monoBuffer.getWritePointer(0, 0),
                                              monoBuffer.getReadPointer(0, size),
                                              monoBuffer.getNumSamples() - size);
            
            juce::FloatVectorOperations::copy(monoBuffer.getWritePointer(0, (monoBuffer.getNumSamples() - size)),
                                              tempIncomingBuffer.getReadPointer(0, 0),
                                              size);
            
            leftChannelFFTDataGenerator.produceFFTDataForRendering(monoBuffer, -48.f);
        }
    }
    
    const auto fftSize = leftChannelFFTDataGenerator.getFFTSize();
    const auto binWidth = sampleRate / (double)fftSize;
    
    while (leftChannelFFTDataGenerator.getNumAvailableFFTDataBlocks() > 0)
    {
        std::vector<float> fftData;
        if(leftChannelFFTDataGenerator.getFFTData(fftData))
        {
            pathProducer.generatePath(fftData, fftBounds, fftSize, binWidth, -48.f);
        }
    }
    
    while(pathProducer.getNumPathsAvailable())
    {
        pathProducer.getPath(leftChannelFFTPath);
    }
    
}




ResponseCurve::ResponseCurve(DistortionProjAudioProcessor& p) : audioProcessor(p),
leftPathProducer(audioProcessor.leftChannelFifo),
rightPathProducer(audioProcessor.rightChannelFifo)
{
    const auto& params = audioProcessor.getParameters();
    
    for(auto param : params)
    {
        param->addListener(this);
    }
    
    startTimerHz(60);
}


ResponseCurve::~ResponseCurve()
{
    const auto& params = audioProcessor.getParameters();
    
    for(auto param : params)
    {
        param->removeListener(this);
    }
}

void ResponseCurve::parameterValueChanged(int parameterIndex, float newValue)
{
    parametersChanged.set(true);
}



void ResponseCurve::timerCallback()
{
    if(shouldShowFFT)
    {
        auto fftBounds = getLocalBounds().toFloat();
        auto sampleRate = audioProcessor.getSampleRate();
        
        
        leftPathProducer.process(fftBounds, sampleRate);
        rightPathProducer.process(fftBounds, sampleRate);
    }
    
    if(parametersChanged.compareAndSetBool(false, true))
    {
        auto chainSettings = getChainSettings(audioProcessor.apvts);
        
        monoChain.setBypassed<ChainPositions::lowCut>(chainSettings.lowCutBypassed);
        monoChain.setBypassed<ChainPositions::highCut>(chainSettings.highCutBypassed);

        auto lowCutCoefficients = makeLowCutFilter(chainSettings, audioProcessor.getSampleRate());
        auto highCutCoefficients = makeHighCutFilter(chainSettings, audioProcessor.getSampleRate());
        
        updateCutFilter(monoChain.get<ChainPositions::lowCut>(), lowCutCoefficients);
        updateCutFilter(monoChain.get<ChainPositions::highCut>(), highCutCoefficients);
        
    }
    
    repaint();
}

void ResponseCurve::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().reduced(2.5f, 0.f);
    
    auto& lowcut = monoChain.get<ChainPositions::lowCut>();
    auto& highcut = monoChain.get<ChainPositions::highCut>();
    
    auto sampleRate = audioProcessor.getSampleRate();
    auto w = bounds.getWidth();
    
    std::vector<double> mags;
    
    mags.resize(w);
    
    for(int i = 0; i < w; ++i)
    {
        double mag = 1.f;
        auto freq = mapToLog10(double(i) / double(w), 20.0, 20000.0);
        
        if(!monoChain.isBypassed<ChainPositions::lowCut>()){
            mag *= lowcut.coefficients->getMagnitudeForFrequency(freq, sampleRate);
        }
        if(!monoChain.isBypassed<ChainPositions::highCut>()){
            mag *= highcut.coefficients->getMagnitudeForFrequency(freq, sampleRate);
        }
    
        
        mags[i] = Decibels::gainToDecibels(mag);
    }
    
    Path responseCurve;
    
    const double outputMin = bounds.getBottom();
    const double outputMax = bounds.getY();
    auto map = [outputMin, outputMax](double input)
    {
        return jmap(input, -24.0, 24.0, outputMin, outputMax);
    };
    
    responseCurve.startNewSubPath(bounds.getX(), map(mags.front()));
    
    for(size_t i = 1; i<mags.size(); ++i)
    {
        responseCurve.lineTo(bounds.getX() + i, map(mags[i]));
    }
    
    if(shouldShowFFT)
    {
        auto leftChannelFFTPath = leftPathProducer.getPath();
        leftChannelFFTPath.applyTransform(AffineTransform().translation(bounds.getX(), bounds.getY()));
        
        g.setColour(Colours::skyblue);
        g.strokePath(leftChannelFFTPath, PathStrokeType(1.f));
        
        auto rightChannelFFTPath = rightPathProducer.getPath();
        rightChannelFFTPath.applyTransform(AffineTransform().translation(bounds.getX(), bounds.getY()));
        
        g.setColour(Colours::orangered);
        g.strokePath(rightChannelFFTPath, PathStrokeType(1.f));
    }
    
    g.setColour(Colours::white);
    g.strokePath(responseCurve, PathStrokeType(2.f));
}

juce::Rectangle<int> ResponseCurve::getRenderArea()
{
    auto bounds = getLocalBounds();
    auto titleBarArea = bounds.removeFromTop(bounds.getHeight() * 0.1);
    auto dropDownArea = titleBarArea.removeFromLeft(titleBarArea.getWidth() * 0.33);
    auto metersArea = bounds.removeFromRight(bounds.getWidth() * 0.15);
    auto thumbnailArea = metersArea.removeFromTop(metersArea.getHeight() * 0.33);
    
    return bounds = bounds.removeFromTop(bounds.getHeight() * 0.5);
}

juce::Rectangle<int> ResponseCurve::getAnalysisArea()
{
    auto bounds = getRenderArea();
    return bounds = bounds.reduced(22.5f, 15.f);
    
}

//==============================================================================
DistortionProjAudioProcessorEditor::DistortionProjAudioProcessorEditor (DistortionProjAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),

    responseCurveComponent(audioProcessor),

    GainMeterInL([&]() {return audioProcessor.getInRmsLevel(0); } ),
    GainMeterInR([&]() {return audioProcessor.getInRmsLevel(1); } ),
    GainMeterOutL([&]() {return audioProcessor.getOutRmsLevel(0); } ),
    GainMeterOutR([&]() {return audioProcessor.getOutRmsLevel(1); } ),

    driveKnob(*audioProcessor.apvts.getParameter("drive"), "dB"),
    mixKnob(*audioProcessor.apvts.getParameter("mix"), "%"),
    inputGainKnob(*audioProcessor.apvts.getParameter("inputgain"), "dB"),
    outputGainKnob(*audioProcessor.apvts.getParameter("outputgain"), "dB"),
    lowCutKnob(*audioProcessor.apvts.getParameter("lowCut Freq"), "Hz"),
    highCutKnob(*audioProcessor.apvts.getParameter("highCut Freq"), "Hz"),

    driveKnobAttachment(audioProcessor.apvts, "drive", driveKnob),
    highCutKnobAttachment(audioProcessor.apvts, "highCut Freq", highCutKnob),
    lowCutKnobAttachment(audioProcessor.apvts, "lowCut Freq", lowCutKnob),
    inputGainKnobAttachment(audioProcessor.apvts, "inputgain", inputGainKnob),
    outputGainKnobAttachment(audioProcessor.apvts, "outputgain", outputGainKnob),
    mixKnobAttachment(audioProcessor.apvts, "mix", mixKnob),

    onOffButtonAttachment(audioProcessor.apvts, "power switch", onOffSwitch),
    highCutBypassAttachment(audioProcessor.apvts, "highCut Bypass", highCutBypass),
    lowCutBypassAttachment(audioProcessor.apvts, "lowCut Bypass", lowCutBypass),
    driveBypassAttachment(audioProcessor.apvts, "drive Bypass", driveBypass),
    inputGainBypassAttachment(audioProcessor.apvts, "inputGain Bypass", inputGainBypass),
    outputGainBypassAttachment(audioProcessor.apvts, "outputGain Bypass", outputGainBypass),

    distortionTypeAttachment(audioProcessor.apvts, "distortion mode", distortionType),

onOffBulb(Colours::lawngreen)

{
    
    resetImage();
    
    
//        onOffSwitch.setToggleState(true, dontSendNotification);
//        driveBypass.setToggleState(false, dontSendNotification);
//        lowCutBypass.setToggleState(false, dontSendNotification);
//        highCutBypass.setToggleState(false, dontSendNotification);
//        inputGainBypass.setToggleState(false, dontSendNotification);
//        outputGainBypass.setToggleState(false, dontSendNotification);
//        if(auto* comp = safePtr.getComponent())
//        {
//            comp->responseCurveComponent.toggleAnalysisEnablement(true);
//            comp->GainMeterInL.toggleMeterEnablement(true);
//            comp->GainMeterInR.toggleMeterEnablement(true);
//            comp->GainMeterOutL.toggleMeterEnablement(true);
//            comp->GainMeterOutR.toggleMeterEnablement(true);
//            comp->onOffBulb.setState(true);
//        }
    
    loadImageButton.onClick = [&]() {
        File imageFile = audioProcessor.loadImageFile();
        auto newThumbnail = ImageCache::getFromFile(imageFile);
        imageUpload.setImage(newThumbnail);

        std::string imagePath = imageFile.getFullPathName().toStdString();
        std::vector<float> imageHSVvalues = imageAnalyser.processImage(imagePath);
        int hueFromImage = imageHSVvalues[0];
        auto saturationFromImage = jmap(imageHSVvalues[1], 0.f, 255.f, 0.f, 20.f);
        int valueFromImage = imageHSVvalues[2];
        
        std::cout << saturationFromImage;
        
        switch(hueFromImage)
        {
            case 0 ... 15 :
//                output.append("Saturation\n\n");
                distortionType.setSelectedId(4);

            break;
                
            case 164 ... 179 :
//                output.append("Saturation\n\n");
                distortionType.setSelectedId(4);

            break;
                
            case 105 ... 132  :
//                output.append("Hard Clip\n\n");
                distortionType.setSelectedId(3);

            break;
                
            case 133 ... 163 :
//                output.append("Soft Clip\n\n");
                distortionType.setSelectedId(2);
            break;
                
            case 46 ... 76 :
//                output.append("Diode Distortion\n\n");
                distortionType.setSelectedId(7);

            break;
                
            case 16 ... 45 :
//                output.append("Tape Distortion\n\n");
                distortionType.setSelectedId(5);

            break;
                
            case 77 ... 104 :
//                output.append("Tube Distortion\n\n");
                distortionType.setSelectedId(6);

            break;
        }
        
        switch(valueFromImage)
        {
            case 0 ... 85 :
//                output.append("Saturation\n\n");
                highCutKnob.setValue(400);
                lowCutKnob.setValue(0);
                highCutKnob.setDoubleClickReturnValue(true,400);
                lowCutKnob.setDoubleClickReturnValue(true, 0);

            break;
                
            case 86 ... 170  :
//                output.append("Hard Clip\n\n");
                highCutKnob.setValue(2000);
                lowCutKnob.setValue(401);
                highCutKnob.setDoubleClickReturnValue(true,2000);
                lowCutKnob.setDoubleClickReturnValue(true, 401);
            break;
                
            case 171 ... 255 :
//                output.append("Saturation\n\n");
                highCutKnob.setValue(22000);
                lowCutKnob.setValue(2001);
                highCutKnob.setDoubleClickReturnValue(true,22000);
                lowCutKnob.setDoubleClickReturnValue(true, 2001);
            break;
        }

        driveKnob.setValue(saturationFromImage);
        driveKnob.setDoubleClickReturnValue(true, saturationFromImage);
        imageAnalysisOutput.setText(imageAnalyser.getAnalysisOutputString(hueFromImage, saturationFromImage, valueFromImage));
    };
    
    
    
    loadImageButton.setColour(TextButton::ColourIds::buttonColourId, Colours::black);
    menuButton.setColour(TextButton::ColourIds::buttonColourId, Colours::black);
    
    auto highPassSymbol = ImageCache::getFromMemory(BinaryData::lowCutNewWhite_png, BinaryData::lowCutNewWhite_pngSize);
    highPassSymbolImg.setImage(highPassSymbol);

    auto lowPassSymbol = ImageCache::getFromMemory(BinaryData::highCutNewWhite_png, BinaryData::highCutNewWhite_pngSize);
    lowPassSymbolImg.setImage(lowPassSymbol);
    
    auto switchOnImg = ImageCache::getFromMemory(BinaryData::switchOnBlue_png, BinaryData::switchOnBlue_pngSize);
    
    auto switchOffImg = ImageCache::getFromMemory(BinaryData::switchOffBlue_png, BinaryData::switchOffBlue_pngSize);
    
    onOffSwitch.setToggleable(true);
    onOffSwitch.setToggleState(true, dontSendNotification);
    onOffSwitch.setClickingTogglesState(true);
    onOffSwitch.setImages(false, true, true, switchOffImg, 1.f, {}, switchOffImg, 1.f, {}, switchOnImg, 1.f, {});
        
    for(auto* comps : getComponents()){
        addAndMakeVisible(comps);
    }
    
    for(auto* buttons : getButtons()){
        addAndMakeVisible(buttons);
    }
    
    driveKnob.labels.add({0.f, "0dB"});
    driveKnob.labels.add({1.f, "20dB"});
    
    mixKnob.labels.add({0.f, "Dry"});
    mixKnob.labels.add({1.f, "Wet"});
    
    lowCutKnob.labels.add({0.f, "1Hz"});
    lowCutKnob.labels.add({1.f, "22KHz"});
    
    highCutKnob.labels.add({0.f, "1Hz"});
    highCutKnob.labels.add({1.f, "22kHz"});
    
    inputGainKnob.labels.add({0.f, "-24dB"});
    inputGainKnob.labels.add({1.f, "+24dB"});
    
    outputGainKnob.labels.add({0.f, "-24dB"});
    outputGainKnob.labels.add({1.f, "+24dB"});

    
    menuPopUp.addSectionHeader("Menu");
    menuPopUp.addItem(1, "Init patch");
//    menuPopUp.addItem(2, "Save preset");
//    menuPopUp.addItem(3, "Load preset");
    
    onOffButton.setLookAndFeel(&lnf);
    driveBypass.setLookAndFeel(&lnf);
    lowCutBypass.setLookAndFeel(&lnf);
    highCutBypass.setLookAndFeel(&lnf);
    inputGainBypass.setLookAndFeel(&lnf);
    outputGainBypass.setLookAndFeel(&lnf);
    
//    createLabel("IN", inputGainLabel);
//    createLabel("OUT", outputGainLabel);
//    createLabel("Insert Name Here", nameLabel);
    
    inputGainLabel.setText("IN", dontSendNotification);
    inputGainLabel.setJustificationType(juce::Justification::centred);

    outputGainLabel.setText("OUT", dontSendNotification);
    outputGainLabel.setJustificationType(juce::Justification::centred);

    nameLabel.setText("Insert Name Here", dontSendNotification);
    nameLabel.setJustificationType(juce::Justification::centred);
    
    driveKnobLabel.setText("Drive", dontSendNotification);
    driveKnobLabel.setJustificationType(juce::Justification::centred);
    
    mixKnobLabel.setText("Mix", dontSendNotification);
    mixKnobLabel.setJustificationType(juce::Justification::centred);
    
    inGainLabel.setText("In Gain", dontSendNotification);
    inGainLabel.setJustificationType(juce::Justification::centred);
    
    outGainLabel.setText("Out Gain", dontSendNotification);
    outGainLabel.setJustificationType(juce::Justification::centred);
    
    imageAnalysisOutput.setMultiLine(true, true);
    imageAnalysisOutput.setReadOnly(true);
    imageAnalysisOutput.setColour(TextEditor::ColourIds::backgroundColourId, Colours::black);
    imageAnalysisOutput.setColour(TextEditor::ColourIds::outlineColourId, Colours::black);
    imageAnalysisOutput.setScrollbarsShown(true);
    imageAnalysisOutput.setText("Upload an JPEG or PNG image to generate a patch");
    
    distortionType.addItem("Distortion Type", 1);
    distortionType.addItem("Soft Clip", 2);
    distortionType.addItem("Hard Clip", 3);
    distortionType.addItem("Saturation", 4);
    distortionType.addItem("Tape Distortion", 5);
    distortionType.addItem("Tube Distortion", 6);
    distortionType.addItem("Diode Distortion", 7);
    distortionType.setSelectedId(1);
    distortionType.setColour(ComboBox::ColourIds::backgroundColourId, Colours::black);
    distortionType.setColour(ComboBox::ColourIds::outlineColourId, Colours::black);

    auto safePtr = juce::Component::SafePointer<DistortionProjAudioProcessorEditor>(this);
    
    onOffBulb.setState(true);
    
    onOffSwitch.onClick = [safePtr]()
    {
        if(auto* comp = safePtr.getComponent())
        {
            auto enabled = comp->onOffSwitch.getToggleState();
            comp->responseCurveComponent.toggleAnalysisEnablement(enabled);
            comp->GainMeterInL.toggleMeterEnablement(enabled);
            comp->GainMeterInR.toggleMeterEnablement(enabled);
            comp->GainMeterOutL.toggleMeterEnablement(enabled);
            comp->GainMeterOutR.toggleMeterEnablement(enabled);
            comp->onOffBulb.setState(enabled);
        }
    };
    
    onOffSwitch.setComponentID("onOffSwitch");

    menuButton.onClick = [&]()
    {
        menuPopUp.showMenuAsync(PopupMenu::Options(), [&] (int result)
                           {
            if(result == 0)
            {
                
            }
            else if(result == 1)
            {
                initialisePlugin();
            }
            else if(result == 2)
            {
                audioProcessor.savePreset();
            }
            else if(result == 3)
            {
                audioProcessor.loadPreset();
            }
        });
    };
        
    setSize (1000, 550);
}

//void DistortionProjAudioProcessorEditor::createLabel(String string, Label label)
//{
//    label.setText(string, dontSendNotification);
//    label.setJustificationType(juce::Justification::centred);
//}

DistortionProjAudioProcessorEditor::~DistortionProjAudioProcessorEditor()
{
    onOffButton.setLookAndFeel(nullptr);
    driveBypass.setLookAndFeel(nullptr);
    lowCutBypass.setLookAndFeel(nullptr);
    highCutBypass.setLookAndFeel(nullptr);
    inputGainBypass.setLookAndFeel(nullptr);
    outputGainBypass.setLookAndFeel(nullptr);

}

void DistortionProjAudioProcessorEditor::resetImage()
{
    auto defaultThumbnail = ImageCache::getFromMemory(BinaryData::imageUpload_png, BinaryData::imageUpload_pngSize);
    if(!defaultThumbnail.isNull()){
        imageUpload.setImage(defaultThumbnail);
    }
    else{
        jassert(!defaultThumbnail.isNull());
    }
    
}

void DistortionProjAudioProcessorEditor::initialisePlugin()
{
    driveKnob.setValue(0);
    mixKnob.setValue(50);
    lowCutKnob.setValue(1);
    highCutKnob.setValue(22000);
    inputGainKnob.setValue(0);
    outputGainKnob.setValue(0);
    driveKnob.setDoubleClickReturnValue(true, 0);
    mixKnob.setDoubleClickReturnValue(true, 50);
    lowCutKnob.setDoubleClickReturnValue(true, 1);
    highCutKnob.setDoubleClickReturnValue(true, 22000);
    inputGainKnob.setDoubleClickReturnValue(true, 0);
    outputGainKnob.setDoubleClickReturnValue(true, 0);
    distortionType.setSelectedId(1);
    resetImage();
    imageAnalysisOutput.setText("Upload an JPEG or PNG image to generate a patch");
    for(auto button : getButtons())
    {
        if(!button->getToggleState() && button->getComponentID().equalsIgnoreCase("onOffSwitch"))
        {
            button->triggerClick();
        }
        else if(button->getToggleState() && !button->getComponentID().equalsIgnoreCase("onOffSwitch"))
        {
            button->triggerClick();
        }
    }
}


//==============================================================================
void DistortionProjAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll(Colour(Colours::black));
//    red colour Colour(158, 39, 46)
    
    auto bounds = getLocalBounds();
    auto titleBarArea = bounds.removeFromTop(bounds.getHeight() * 0.1);
    auto dropDownArea = titleBarArea.removeFromLeft(titleBarArea.getWidth() * 0.33);
    auto initButtonArea = titleBarArea.removeFromRight(titleBarArea.getWidth() * 0.15).reduced(5.f);
    auto dropDown = dropDownArea.reduced(5.f);
    auto titleBar = titleBarArea.reduced(5.f);
    auto metersArea = bounds.removeFromRight(bounds.getWidth() * 0.15);
    auto thumbnailArea = metersArea.removeFromTop(metersArea.getHeight() * 0.33);
    auto thumbNail = thumbnailArea.reduced(5.f);
    auto meters = metersArea.reduced(5.f);
    auto responseArea = bounds.removeFromTop(bounds.getHeight() * 0.5);
    auto response = responseArea.reduced(5.f);
    auto graphAreaSection = responseArea.reduced(22.5f, 15.f);
    auto graphArea = graphAreaSection.reduced(5.f);
    auto parameters = bounds.removeFromLeft(bounds.getWidth() * 0.75);
    auto dials = parameters.removeFromLeft(parameters.getWidth() * 0.8).reduced(5.f);
    auto powerSwitchArea = parameters;
    
    
    //draw outline boxes
//    g.setColour(Colours::black);
//    g.drawRoundedRectangle(response.toFloat(), 4.f, 1.f);
//    g.drawRoundedRectangle(titleBar.toFloat(), 4.f, 1.f);
//    g.drawRoundedRectangle(thumbnail.toFloat(), 4.f, 1.f);
//    g.drawRoundedRectangle(dropDown.toFloat(), 4.f, 1.f);
    
    g.setColour(Colour(158, 39, 46));
    g.fillRect(titleBar.toFloat());
    g.fillRect(dropDown.toFloat());
    g.fillRect(meters.toFloat());
    g.fillRect(thumbNail.toFloat());
    g.fillRect(graphArea.toFloat());
    g.fillRect(dials.toFloat());
    g.fillRect(response.toFloat());
    g.fillRect(powerSwitchArea.reduced(5.f).toFloat());
    g.fillRect(bounds.toFloat().reduced(5.f));
    g.fillRect(initButtonArea.toFloat());

    
//    draws response graph background
        g.setColour(Colours::black);

    g.fillRect(graphAreaSection.toFloat());

    Array<float> freqs
    {
        20, //30, 40,
        50, 100,
        200, //300, 400,
        500, 1000,
        2000, //3000, 4000,
        5000, 10000,
        20000
    };
    
    auto left = graphArea.getX();
    auto right = graphArea.getRight();
    auto top = graphArea.getY();
    auto bottom = graphArea.getBottom();
    auto width = graphArea.getWidth();
    
    Array<float> xs;
    for(auto f : freqs)
    {
        auto normX = mapFromLog10(f, 20.f, 20000.f);
        xs.add(left + width * normX);
    }
    
    g.setColour(Colours::dimgrey);
    for(auto x : xs)
    {
        g.drawVerticalLine(x, top, bottom);
    }
    
    Array<float> gain
    {
        -24, -12, 0, 12, 24
    };
    
    for(auto gDb : gain)
    {
        auto y = jmap(gDb, -24.f, 24.f, float(bottom), float(top));
        g.setColour(gDb == 0.f ? Colour(0u, 172u, 1u) : Colours::darkgrey);
        g.drawHorizontalLine(y, left, right);
        
    };
    
    g.setColour(Colours::black);
    const int fontHeight = 10;
    g.setFont(fontHeight);
    
    for(int i=0; i<freqs.size(); ++i)
    {
        auto f = freqs[i];
        auto x = xs[i];
        
        bool addK = false;
        juce::String str;
        if(f > 999.f){
            addK = true;
            f /= 1000.f;
        }
        str << f;
        if(addK){
            str << "k";
        }
        str << "Hz";
        
        auto textWidth = g.getCurrentFont().getStringWidth(str);
        Rectangle<int> r;
        r.setSize(textWidth, fontHeight);
        r.setCentre(x, 0);
        r.setY(response.getY());
        
        g.drawFittedText(str, r, juce::Justification::centred, 1);
    }
    
    
    for(auto gDb : gain)
    {
        auto y = jmap(gDb, -24.f, 24.f, float(bottom), float(top));
        juce::String str;
        if(gDb > 0){
            str << "+";
        }
        str <<gDb;
        
        auto textWidth = g.getCurrentFont().getStringWidth(str);
        Rectangle<int> r;
        r.setSize(textWidth, fontHeight);
        r.setX(graphAreaSection.getRight());
        r.setCentre(r.getCentreX(), y);
        
        
        g.setColour(gDb == 0.f ? Colour(0u, 172u, 1u) : Colours::lightgrey);
        
        g.drawFittedText(str, r, juce::Justification::centred, 1);
        
        str.clear();
        str << (gDb - 24.f);
        r.setX(response.getX());
        textWidth = g.getCurrentFont().getStringWidth(str);
        r.setSize(textWidth, fontHeight);
        g.setColour(Colours::lightgrey);
        g.drawFittedText(str, r, juce::Justification::centred, 1);
    }
    
    
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
    
}

void DistortionProjAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    
    
    
    auto bounds = getLocalBounds();
    auto titleBarArea = bounds.removeFromTop(bounds.getHeight() * 0.1);
    auto dropDownArea = titleBarArea.removeFromLeft(titleBarArea.getWidth() * 0.33);
    auto dropDown = dropDownArea.reduced(5.f);
    auto initButton = titleBarArea.removeFromRight(titleBarArea.getWidth() * 0.15).reduced(10.f);
    auto titleBar = titleBarArea.reduced(5.f);
    auto metersArea = bounds.removeFromRight(bounds.getWidth() * 0.15);
    auto thumbnailArea = metersArea.removeFromTop(metersArea.getHeight() * 0.33);
    auto thumbNail = thumbnailArea.reduced(5.f);
    auto meters = metersArea.reduced(5.f);
    auto responseArea = bounds.removeFromTop(bounds.getHeight() * 0.5);
    auto response = responseArea.reduced(5.f);
    auto graphAreaSection = responseArea.reduced(20.f, 15.f);
    auto graphArea = graphAreaSection.reduced(5.f);
    auto parameters = bounds.removeFromLeft(bounds.getWidth() * 0.75);
    
    auto uploadButton = thumbNail.removeFromBottom(thumbNail.getHeight() * 0.35);
    
    auto meterLabels = meters.removeFromTop(meters.getHeight() * 0.15);
    auto metersLeft = meters.removeFromLeft(meters.getWidth() * 0.5);
    auto metersRight = meters;
    
    auto leftMeter = metersLeft.reduced(8.f);
    auto rightMeter = metersRight.reduced(8.f);
    auto leftLeftMeter = leftMeter.removeFromLeft(leftMeter.getWidth() * 0.5);
    auto rightLeftMeter = leftMeter;
    auto leftRightMeter = rightMeter.removeFromLeft(rightMeter.getWidth() * 0.5);
    auto rightRightMeter = rightMeter;

    auto dials = parameters.removeFromLeft(parameters.getWidth() * 0.8).reduced(5.f, 10.f);
    auto driveNmix = dials.removeFromLeft(dials.getWidth() * 0.333);
    auto highNlowCut = dials.removeFromLeft(dials.getWidth() * 0.5);
    auto gains = dials.removeFromLeft(dials.getWidth());
    auto onOffBounds = parameters.removeFromBottom(parameters.getHeight() * 0.8).reduced(5.f);
    auto onOffBulbBounds = parameters;
    auto lowCut = highNlowCut.removeFromTop(highNlowCut.getHeight() * 0.5);
    auto lowCutLabel = lowCut.removeFromTop(highNlowCut.getHeight() * 0.18);
    auto lowcutBypassArea = lowCutLabel.removeFromLeft(34);
    lowCutLabel = lowCutLabel.removeFromLeft(92);
    lowCutLabel = lowCutLabel.removeFromBottom(lowCutLabel.getHeight() * 0.9);
    auto highCut = highNlowCut;
    auto highCutLabel = highCut.removeFromTop(highCut.getHeight() * 0.18);
    auto highcutBypassArea = highCutLabel.removeFromLeft(40);
    highCutLabel = highCutLabel.removeFromLeft(90);
    auto inputgain = gains.removeFromTop(gains.getHeight() * 0.5);
    auto inGainLabelArea = inputgain.removeFromTop(inputgain.getHeight() * 0.18);
    auto inGainBypassArea = inGainLabelArea.removeFromLeft(34);
    inGainLabelArea = inGainLabelArea.removeFromLeft(100);
    auto outputgain = gains;
    auto outGainLabelArea = outputgain.removeFromTop(outputgain.getHeight() * 0.18);
    auto outGainBypassArea = outGainLabelArea.removeFromLeft(34);
    outGainLabelArea = outGainLabelArea.removeFromLeft(100);
    auto drive = driveNmix.removeFromTop(driveNmix.getHeight() * 0.5);
    auto driveLabel = drive.removeFromTop(drive.getHeight() * 0.18);
    auto driveBypassArea = driveLabel.removeFromLeft(34);
    driveLabel = driveLabel.removeFromLeft(97);
    auto mixSpacer = driveNmix.removeFromTop(driveNmix.getHeight() * 0.18);
    auto mix = driveNmix;
    auto imageAnalysisOutputArea = bounds.reduced(12.5f);

    driveBypass.setBounds(driveBypassArea);
    driveKnobLabel.setBounds(driveLabel);
    driveKnob.setBounds(drive);

    menuButton.setBounds(initButton);
    
    outputGainBypass.setBounds(outGainBypassArea);
    outGainLabel.setBounds(outGainLabelArea);
    outputGainKnob.setBounds(outputgain);
    
    inputGainBypass.setBounds(inGainBypassArea);
    inGainLabel.setBounds(inGainLabelArea);
    inputGainKnob.setBounds(inputgain);

    mixKnob.setBounds(mix);
    
    highCutBypass.setBounds(highcutBypassArea);
    lowPassSymbolImg.setBounds(highCutLabel.reduced(10.f, 0));
    highCutKnob.setBounds(highCut);

    lowCutBypass.setBounds(lowcutBypassArea);
    highPassSymbolImg.setBounds(lowCutLabel.reduced(10.f, 0));
    lowCutKnob.setBounds(lowCut);
    
    onOffSwitch.setBounds(onOffBounds.reduced(20.f));
    
    onOffBulb.setBounds(onOffBulbBounds.reduced(8.f));
    onOffBulb.setCentrePosition(611, 344);
    
    distortionType.setBounds(dropDown.reduced(5.f));
    
    GainMeterInL.setBounds(leftLeftMeter);
    GainMeterInR.setBounds(rightLeftMeter);
    GainMeterOutL.setBounds(leftRightMeter);
    GainMeterOutR.setBounds(rightRightMeter);

    
    imageUpload.setBounds(thumbNail.reduced(10.f));
    
    loadImageButton.setBounds(uploadButton.reduced(5.f));
    
    responseCurveComponent.setBounds(graphArea);
    
    inputGainLabel.setBounds(meterLabels.removeFromLeft(meterLabels.getWidth() * 0.5));
    outputGainLabel.setBounds(meterLabels);
    
    nameLabel.setBounds(titleBar);
    
    
    mixKnobLabel.setBounds(mixSpacer);
    
    imageAnalysisOutput.setBounds(imageAnalysisOutputArea.getX(), imageAnalysisOutputArea.getY(), imageAnalysisOutputArea.getWidth(), imageAnalysisOutputArea.getHeight());
    
}



std::vector<juce::Component*> DistortionProjAudioProcessorEditor::getComponents()
{
    return
    {
        &driveKnob,
        &highCutKnob,
        &lowCutKnob,
        &inputGainKnob,
        &outputGainKnob,
        &mixKnob,
        &responseCurveComponent,
        &GainMeterOutR,
        &GainMeterOutL,
        &GainMeterInL,
        &GainMeterInR,
        &outputGainLabel,
        &inputGainLabel,
        &nameLabel,
        &imageAnalysisOutput,
        &distortionType,
        &imageUpload,
        &imageAnalysisOutput,
        &lowPassSymbolImg,
        &highPassSymbolImg,
        &driveKnobLabel,
        &mixKnobLabel,
        &inGainLabel,
        &outGainLabel,
        &loadImageButton,
        &menuButton,
        &onOffBulb
        
    };
}

std::vector<juce::Button*> DistortionProjAudioProcessorEditor::getButtons()
{
    return
    {
        &onOffButton,
        &highCutBypass,
        &lowCutBypass,
        &driveBypass,
        &colourBypass,
        &inputGainBypass,
        &outputGainBypass,
        &onOffSwitch
    };
}
