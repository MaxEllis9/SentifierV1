/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

template<typename T>
struct Fifo
{
    void prepare(int numChannels, int numSamples)
    {
        static_assert(std::is_same_v<T, juce::AudioBuffer<float>>,
                      "prepare(numChannels, numSamples) should only be used when Fifo is holding juce::AudioBuffer<float>");

        for(auto& buffer : buffers)
        {
            buffer.setSize(numChannels,
                           numSamples,
                           false,       //clear everything?
                           true,        //inlcuding extra sapce?
                           true);       //avoid reallocating if you can?
            buffer.clear();
        }
    }

    void prepare(size_t numElements)
    {
        static_assert(std::is_same_v<T, std::vector<float>>,
                      "prepare(numElements) should only be used when Fifo is holding std::vector<float>");
        for(auto& buffer : buffers)
        {
            buffer.clear();
            buffer.resize(numElements, 0);
        }
    }

    bool push(const T& t)
    {
        auto write = fifo.write(1);
        if(write.blockSize1 > 0){
            buffers[write.startIndex1] = t;
            return true;
        }
        return false;
    }

    bool pull(T& t)
    {
        auto read = fifo.read(1);
        if(read.blockSize1 > 0){
            t = buffers[read.startIndex1];
            return true;
        }
        return false;
    }

    int getNumAvailableForReading() const
    {
        return fifo.getNumReady();
    }
private:
    static constexpr int Capacity = 30;
    std::array<T, Capacity> buffers;
    juce::AbstractFifo fifo {Capacity};
};





enum Channel
{
    Right, //effectively 0
    Left   //effectviely 1
};






template<typename BlockType>
struct SingleChannelSampleFifo
{
    SingleChannelSampleFifo(Channel ch) : channelToUse(ch)
    {
        prepared.set(false);
    }

    void update(const BlockType buffer)
    {
        jassert(prepared.get());
        jassert(buffer.getNumChannels() > channelToUse);
        auto* channelPtr = buffer.getReadPointer(channelToUse);

        for(int i=0; i<buffer.getNumSamples(); ++i)
        {
            pushNextSampleIntoFifo(channelPtr[i]);
        }
    }

    void prepare(int bufferSize)
    {
        prepared.set(false);
        size.set(bufferSize);

        bufferToFill.setSize(1,            //channel
                             bufferSize,   //num samples
                             false,        //keepExistingContent
                             true,         //clear extra space
                             true);        //avoid reallocating
        audioBufferFifo.prepare(1, bufferSize);
        fifoIndex = 0;
        prepared.set(true);
    }

    //==============================================================================
    int getNumCompleteBuffersAvailable() const {return audioBufferFifo.getNumAvailableForReading();}
    bool isPrepared() const {return prepared.get();}
    int getSize() const {return size.get();}
    //==============================================================================
    bool getAudioBuffer(BlockType& buf) {return audioBufferFifo.pull(buf);}
private:
    Channel channelToUse;
    int fifoIndex = 0;
    Fifo<BlockType> audioBufferFifo;
    BlockType bufferToFill;
    juce::Atomic<bool> prepared = false;
    juce::Atomic<int> size = 0;

    void pushNextSampleIntoFifo(float sample)
    {
        if(fifoIndex == bufferToFill.getNumSamples()){
            auto ok = audioBufferFifo.push(bufferToFill);
            juce::ignoreUnused(ok);
            fifoIndex = 0;
        }
        bufferToFill.setSample(0, fifoIndex, sample);
        ++fifoIndex;
    }
};












struct ChainSettings{
    
    float lowCutFreq {0}, highCutFreq {0}, inputgain {0}, outputgain {0}, drive {0}, mix {0};
    int distortionMode {0};
    bool powerSwitch {true}, driveBypassed {false}, lowCutBypassed {false}, highCutBypassed {false},
        inputgainBypassed {false}, outputgainBypassed {false};
};

ChainSettings getChainSettings(AudioProcessorValueTreeState& apvts);



using Filter = dsp::IIR::Filter<float>;

using MonoChain = dsp::ProcessorChain<Filter, Filter>;
//    using MonoChain = dsp::ProcessorChain<Gain, Filter, Filter, WaveShape, Gain>;


enum ChainPositions
{
    lowCut,
    highCut
};

using Coefficients = Filter::CoefficientsPtr;

void updateCoefficients(Coefficients& old, const Coefficients& replacement);


template<typename ChainType, typename CoefficientType>
void updateCutFilter(ChainType& chain, const CoefficientType& coefficients)
{
    updateCoefficients(chain.coefficients, coefficients[0]);
//        chain.template setBypassed<Index>(false);
}



inline auto makeLowCutFilter(const ChainSettings& chainSettings, double sampleRate)
{
    return dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(chainSettings.lowCutFreq, sampleRate, 1) ;
}

inline auto makeHighCutFilter(const ChainSettings& chainSettings, double sampleRate)
{
    return dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod(chainSettings.highCutFreq, sampleRate, 1);
}

//==============================================================================
/**
*/
class DistortionProjAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    DistortionProjAudioProcessor();
    ~DistortionProjAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    static AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    
    File getPresetsFolder();
    void savePreset();
    void loadPreset();
    
    juce::AudioProcessorValueTreeState apvts {
        *this,
        nullptr,
        "Parameters",
        createParameterLayout()
    };
    
    float getInRmsLevel(int channel){
        if(channel == 0){
            return rmsInLevelLeft;
        }
        return rmsInLevelRight;
    }
    
    float getOutRmsLevel(int channel){
        if(channel == 0){
            return rmsOutLevelLeft;
        }
        return rmsOutLevelRight;
    }
    
    File loadImageFile();
//
//    using BlockType = juce::AudioBuffer<float>;
//    SingleChannelSampleFifo<BlockType> leftChannelFifo {Channel::Left};
//    SingleChannelSampleFifo<BlockType> rightChannelFifo {Channel::Right};
    
    using BlockType = juce::AudioBuffer<float>;
    SingleChannelSampleFifo<BlockType> leftChannelFifo { Channel::Left };
    SingleChannelSampleFifo<BlockType> rightChannelFifo { Channel::Right };


private:
    //==============================================================================
    
    using Gain = dsp::Gain<float>;
    using WaveShape = dsp::WaveShaper<float>;
    using Mix = dsp::DryWetMixer<float>;
    using Clipper = viator_dsp::Clipper<float>;
    using Saturator = viator_dsp::Saturation<float>;
    
    MonoChain leftChain, rightChain;

    Gain outputGain, inputGain;
    
    Clipper softClipper, hardClipper, diodeDistortion;
    Saturator saturation, tubeDistortion, tapeDistortion;
    
    AudioParameterFloat* outputGainParam {nullptr};
    AudioParameterFloat* inputGainParam {nullptr};
    AudioParameterFloat* driveParam {nullptr};
    AudioParameterFloat* colourParam {nullptr};
    AudioParameterFloat* mixParam {nullptr};
    AudioParameterFloat* lowCutFreqParam {nullptr};
    AudioParameterFloat* highCutFreqParam {nullptr};
    
    AudioParameterBool* powerSwitchParam {nullptr};
    AudioParameterBool* highCutBypassParam {nullptr};
    AudioParameterBool* lowCutBypassParam {nullptr};
    AudioParameterBool* driveBypassParam {nullptr};
    AudioParameterBool* inGainBypassParam {nullptr};
    AudioParameterBool* outGainBypassParam {nullptr};
    AudioParameterBool* colourBypassParam {nullptr};


    
    AudioParameterChoice* distortionTypeParam {nullptr};
    
//    Distortion distortion;
    
//    dsp::DryWetMixer<float> mixControl;
    
    
    
    float rmsInLevelLeft, rmsInLevelRight, rmsOutLevelLeft, rmsOutLevelRight;
    
    template<typename T, typename U>
    void applyGain(T& buffer, U& gain)
    {
        auto block = dsp::AudioBlock<float>(buffer);
        auto ctx = dsp::ProcessContextReplacing<float>(block);
        gain.process(ctx);
    }

        
    void updateHighCutFilters(const ChainSettings& chainSettings);
    
    void updateLowCutFilters(const ChainSettings& chainSettings);
    
    void updateFilters();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DistortionProjAudioProcessor)
};
