/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "math.h"

//==============================================================================
DistortionProjAudioProcessor::DistortionProjAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
//    outputGainParam = dynamic_cast<AudioParameterFloat*>(apvts.getParameter("outputgain"));
//    jassert(outputGainParam != nullptr);
//    inputGainParam = dynamic_cast<AudioParameterFloat*>(apvts.getParameter("inputgain"));
//    jassert(inputGainParam != nullptr);
//    driveParam = dynamic_cast<AudioParameterFloat*>(apvts.getParameter("drive"));
//    jassert(driveParam != nullptr);
//    mixParam = dynamic_cast<AudioParameterFloat*>(apvts.getParameter("mix"));
//    jassert(mixParam != nullptr);
//    highCutFreqParam = dynamic_cast<AudioParameterFloat*>(apvts.getParameter("highCut Freq"));
//    jassert(highCutFreqParam != nullptr);
//    lowCutFreqParam = dynamic_cast<AudioParameterFloat*>(apvts.getParameter("lowCut Freq"));
//    jassert(lowCutFreqParam != nullptr);
//
//    highCutBypassParam = dynamic_cast<AudioParameterBool*>(apvts.getParameter("highCut Bypass"));
//    jassert(highCutBypassParam != nullptr);
//    lowCutBypassParam = dynamic_cast<AudioParameterBool*>(apvts.getParameter("lowCut Bypass"));
//    jassert(lowCutBypassParam != nullptr);
//    driveBypassParam = dynamic_cast<AudioParameterBool*>(apvts.getParameter("drive Bypass"));
//    jassert(driveBypassParam != nullptr);
//
//    distortionTypeParam = dynamic_cast<AudioParameterChoice*>(apvts.getParameter("distortion mode"));
//    jassert(distortionTypeParam != nullptr);

}

DistortionProjAudioProcessor::~DistortionProjAudioProcessor()
{
}

//==============================================================================
const juce::String DistortionProjAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool DistortionProjAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool DistortionProjAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool DistortionProjAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double DistortionProjAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int DistortionProjAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int DistortionProjAudioProcessor::getCurrentProgram()
{
    return 0;
}

void DistortionProjAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String DistortionProjAudioProcessor::getProgramName (int index)
{
    return {};
}

void DistortionProjAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void DistortionProjAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    
    dsp::ProcessSpec spec;
    
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumInputChannels();
    spec.sampleRate = sampleRate;
    
    outputGain.prepare(spec);
    outputGain.setRampDurationSeconds(0.05);
    inputGain.prepare(spec);
    inputGain.setRampDurationSeconds(0.05);
    
    softClipper.prepare(spec);
    softClipper.setClipperType(Clipper::ClipType::kSoft);
    
    hardClipper.prepare(spec);
    hardClipper.setClipperType(Clipper::ClipType::kHard);
    
    diodeDistortion.prepare(spec);
    diodeDistortion.setClipperType(Clipper::ClipType::kDiode);
    
    saturation.prepare(spec);
    saturation.setDistortionType(Saturator::DistortionType::kSaturation);
    
    tubeDistortion.prepare(spec);
    tubeDistortion.setDistortionType(Saturator::DistortionType::kTube);
    
    tapeDistortion.prepare(spec);
    tapeDistortion.setDistortionType(Saturator::DistortionType::kTape);
    
    leftChannelFifo.prepare(samplesPerBlock);
    rightChannelFifo.prepare(samplesPerBlock);
    
    leftChain.prepare(spec);
    rightChain.prepare(spec);
    
    updateFilters();
    
//    mixControl.prepare(spec);

}

void DistortionProjAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool DistortionProjAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void DistortionProjAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    auto settings = getChainSettings(apvts);
    
    if(settings.powerSwitch==true){
    
        auto block = dsp::AudioBlock<float>(buffer);
        auto context = dsp::ProcessContextReplacing<float>(block);
        
        inputGain.setGainDecibels(settings.inputgain);
        outputGain.setGainDecibels(settings.outputgain);
        
        if(settings.inputgainBypassed==false){
            applyGain(buffer, inputGain);
        }
        
        rmsInLevelLeft = Decibels::gainToDecibels(buffer.getRMSLevel(0, 0, buffer.getNumSamples()));
        rmsInLevelRight = Decibels::gainToDecibels(buffer.getRMSLevel(1, 0, buffer.getNumSamples()));
                    
        switch(settings.distortionMode){
            case 0:
                break;
            case 1:
                softClipper.setParameter(viator_dsp::Clipper<float>::ParameterId::kPreamp, settings.drive);
                softClipper.setParameter(viator_dsp::Clipper<float>::ParameterId::kMix, settings.mix);
                softClipper.setParameter(viator_dsp::Clipper<float>::ParameterId::kBypass, settings.driveBypassed);
                softClipper.process(context);
                break;
            case 2:
                hardClipper.setParameter(viator_dsp::Clipper<float>::ParameterId::kPreamp, settings.drive);
                hardClipper.setParameter(viator_dsp::Clipper<float>::ParameterId::kMix, settings.mix);
                hardClipper.setParameter(viator_dsp::Clipper<float>::ParameterId::kBypass, settings.driveBypassed);
                hardClipper.process(context);
                break;
            case 3:
                saturation.setParameter(viator_dsp::Saturation<float>::ParameterId::kPreamp, settings.drive);
                saturation.setParameter(viator_dsp::Saturation<float>::ParameterId::kMix, settings.mix);
                saturation.setParameter(viator_dsp::Saturation<float>::ParameterId::kBypass, settings.driveBypassed);
                saturation.process(context);
                break;
            case 4:
                tapeDistortion.setParameter(viator_dsp::Saturation<float>::ParameterId::kPreamp, settings.drive);
                tapeDistortion.setParameter(viator_dsp::Saturation<float>::ParameterId::kMix, settings.mix);
                tapeDistortion.setParameter(viator_dsp::Saturation<float>::ParameterId::kBypass, settings.driveBypassed);
                tapeDistortion.process(context);
                break;
            case 5:
                tubeDistortion.setParameter(viator_dsp::Saturation<float>::ParameterId::kPreamp, settings.drive);
                tubeDistortion.setParameter(viator_dsp::Saturation<float>::ParameterId::kMix, settings.mix);
                tubeDistortion.setParameter(viator_dsp::Saturation<float>::ParameterId::kBypass, settings.driveBypassed);
                tubeDistortion.process(context);
                break;
            case 6:
                diodeDistortion.setParameter(viator_dsp::Clipper<float>::ParameterId::kPreamp, settings.drive);
                diodeDistortion.setParameter(viator_dsp::Clipper<float>::ParameterId::kMix, settings.mix);
                diodeDistortion.setParameter(viator_dsp::Clipper<float>::ParameterId::kBypass, settings.driveBypassed);
                diodeDistortion.process(context);
            default:
                break;
        }
        
        updateFilters();

        auto leftBlock = block.getSingleChannelBlock(0);
        auto rightBlock = block.getSingleChannelBlock(1);
        
        dsp::ProcessContextReplacing<float> leftContext(leftBlock);
        dsp::ProcessContextReplacing<float> rightContext(rightBlock);
        
        leftChain.process(leftContext);
        rightChain.process(rightContext);
        
        if(settings.outputgainBypassed==false){
            applyGain(buffer, outputGain);
        }
        
        rmsOutLevelLeft = Decibels::gainToDecibels(buffer.getRMSLevel(0, 0, buffer.getNumSamples()));
        rmsOutLevelRight = Decibels::gainToDecibels(buffer.getRMSLevel(1, 0, buffer.getNumSamples()));
        
        leftChannelFifo.update(buffer);
        rightChannelFifo.update(buffer);
    
    }

    
}


//==============================================================================
bool DistortionProjAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* DistortionProjAudioProcessor::createEditor()
{
    return new DistortionProjAudioProcessorEditor (*this);
//    return new GenericAudioProcessorEditor(*this);
    
}

//==============================================================================
void DistortionProjAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    
    MemoryOutputStream mos(destData, true);
    apvts.state.writeToStream(mos);
}

void DistortionProjAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    
    auto tree = ValueTree::readFromData(data, sizeInBytes);
    if(tree.isValid()){
        apvts.replaceState(tree);
    }
}

ChainSettings getChainSettings(AudioProcessorValueTreeState& apvts)
{
    ChainSettings settings;
    
    settings.highCutFreq = apvts.getRawParameterValue("highCut Freq")->load();
    settings.lowCutFreq = apvts.getRawParameterValue("lowCut Freq")->load();
    settings.drive = apvts.getRawParameterValue("drive")->load();
    settings.inputgain = apvts.getRawParameterValue("inputgain")->load();
    settings.outputgain = apvts.getRawParameterValue("outputgain")->load();
    settings.mix = apvts.getRawParameterValue("mix")->load();
    settings.distortionMode = apvts.getRawParameterValue("distortion mode")->load();
    settings.powerSwitch = apvts.getRawParameterValue("power switch")->load() > 0.5f;
    settings.driveBypassed = apvts.getRawParameterValue("drive Bypass")->load() > 0.5f;
    settings.highCutBypassed = apvts.getRawParameterValue("highCut Bypass")->load() > 0.5f;
    settings.lowCutBypassed = apvts.getRawParameterValue("lowCut Bypass")->load() > 0.5f;
    settings.inputgainBypassed = apvts.getRawParameterValue("inputGain Bypass")->load() > 0.5f;
    settings.outputgainBypassed = apvts.getRawParameterValue("outputGain Bypass")->load() > 0.5f;
    
    
    return settings;
}


void updateCoefficients(Coefficients &old, const Coefficients &replacement)
{
    *old = *replacement;
}

void DistortionProjAudioProcessor::updateLowCutFilters(const ChainSettings &chainSettings)
{
    auto lowCutCoefficients = makeLowCutFilter(chainSettings, getSampleRate());
    auto& leftLowCut = leftChain.get<ChainPositions::lowCut>();
    auto& rightLowCut = rightChain.get<ChainPositions::lowCut>();
    
    leftChain.setBypassed<ChainPositions::lowCut>(chainSettings.lowCutBypassed);
    rightChain.setBypassed<ChainPositions::lowCut>(chainSettings.lowCutBypassed);

    
    updateCutFilter(leftLowCut, lowCutCoefficients);
    updateCutFilter(rightLowCut, lowCutCoefficients);
}

void DistortionProjAudioProcessor::updateHighCutFilters(const ChainSettings &chainSettings)
{
    auto highCutCoefficients = makeHighCutFilter(chainSettings, getSampleRate());
    auto& leftHighCut = leftChain.get<ChainPositions::highCut>();
    auto& rightHighCut = rightChain.get<ChainPositions::highCut>();
    
    leftChain.setBypassed<ChainPositions::highCut>(chainSettings.highCutBypassed);
    rightChain.setBypassed<ChainPositions::highCut>(chainSettings.highCutBypassed);

    updateCutFilter(leftHighCut, highCutCoefficients);
    updateCutFilter(rightHighCut, highCutCoefficients);
}

void DistortionProjAudioProcessor::updateFilters()
{
    auto chainSettings = getChainSettings(apvts);
    
    updateLowCutFilters(chainSettings);
    updateHighCutFilters(chainSettings);
}



AudioProcessorValueTreeState::ParameterLayout DistortionProjAudioProcessor::createParameterLayout()
{
    AudioProcessorValueTreeState::ParameterLayout layout;
    
    auto freqRange = NormalisableRange<float>(1.f, 22000.f, 1.f, 0.25f);

    auto gainRange = NormalisableRange<float>(-24.f, 24.f, 0.5f, 1.f);
    
    layout.add(std::make_unique<AudioParameterFloat>("lowCut Freq",
                                                     "lowCut Freq",
                                                     freqRange,
                                                     1.f));
    layout.add(std::make_unique<AudioParameterFloat>("highCut Freq",
                                                     "highCut Freq",
                                                     freqRange,
                                                     22000.f));
    
    layout.add(std::make_unique<AudioParameterFloat>("outputgain",
                                                     "outputGain",
                                                     gainRange,
                                                     0.f));
    layout.add(std::make_unique<AudioParameterFloat>("inputgain",
                                                     "inputGain",
                                                     gainRange,
                                                     0.f));
    
    layout.add(std::make_unique<AudioParameterFloat>("mix",
                                                     "Mix",
                                                     NormalisableRange<float>(0.f, 100.f, 1.f, 1.f),
                                                     50.f));
    layout.add(std::make_unique<AudioParameterFloat>("drive",
                                                     "Drive",
                                                     NormalisableRange<float>(0.f, 20.f, 0.5f, 1.f),
                                                     0.f));
    
    StringArray distortionType;
    distortionType.add("None");
    distortionType.add("Soft Clip");
    distortionType.add("Hard Clip");
    distortionType.add("Saturation");
    distortionType.add("Tape Distortion");
    distortionType.add("Tube Distortion");
    distortionType.add("Diode Distortion");
    
    layout.add(std::make_unique<AudioParameterChoice>("distortion mode",
                                                      "Distortion Mode",
                                                      distortionType,
                                                      0
                                                      ));
    
    layout.add(std::make_unique<AudioParameterBool>("highCut Bypass",
                                                    "highCut Bypass",
                                                    false
                                                    ));
    layout.add(std::make_unique<AudioParameterBool>("lowCut Bypass",
                                                    "lowCut Bypass",
                                                    false
                                                    ));
    layout.add(std::make_unique<AudioParameterBool>("inputGain Bypass",
                                                    "inputGain Bypass",
                                                    false
                                                    ));
    layout.add(std::make_unique<AudioParameterBool>("outputGain Bypass",
                                                    "outputGain Bypass",
                                                    false
                                                    ));
    layout.add(std::make_unique<AudioParameterBool>("drive Bypass",
                                                    "drive Bypass",
                                                    false
                                                    ));
    layout.add(std::make_unique<AudioParameterBool>("power switch",
                                                    "Power switch",
                                                    true
                                                    ));
            
    return layout;
    
}

File DistortionProjAudioProcessor::loadImageFile()
{
    FileChooser chooser { "Please upload an image" };
    File file;
    
    if(chooser.browseForFileToOpen()){
        file = chooser.getResult();
    }
    
    return file;
}

File DistortionProjAudioProcessor::getPresetsFolder()
{
    File rootFolder = File::getSpecialLocation(File::SpecialLocationType::userDesktopDirectory);
    
    auto presetFolder {rootFolder.getChildFile("distortionPresets")};
    if(presetFolder.createDirectory().wasOk())
    {
        rootFolder = presetFolder;
    }
    
    return presetFolder;
}

void DistortionProjAudioProcessor::savePreset()
{
    
    File presetFolder = getPresetsFolder();
    File preset;
    
    auto presetFile {presetFolder.getChildFile("preset1")};
    if(presetFile.create().wasOk())
    {
        preset = presetFile;
    }
    
    FileChooser chooser {"Choose file to save"};
    
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    xml->writeTo(preset);
    
    if(chooser.browseForFileToSave(true))
    {
        chooser.getResult() = preset;
    }
}

void DistortionProjAudioProcessor::loadPreset()
{
    FileChooser chooser {"Select the preset you want to use"};
    File preset;

    if(chooser.browseForFileToOpen())
    {
        preset = chooser.getResult();
    }

    auto xml {juce::XmlDocument::parse(preset)};
    apvts.replaceState(juce::ValueTree::fromXml(*xml));
}



//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DistortionProjAudioProcessor();
}
