#include "PluginProcessor.h"
#include "PluginEditor.h"

EfxNavAudioProcessor::EfxNavAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), apvts(*this, nullptr, "Parameters", createParameterLayout())
#endif
{
}

EfxNavAudioProcessor::~EfxNavAudioProcessor()
{
}

juce::AudioProcessorValueTreeState::ParameterLayout EfxNavAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Space (Reverb) Parameters
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"space_size", 1}, "Space Size", 0.0f, 1.0f, 0.5f));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"space_depth", 1}, "Space Depth", 0.0f, 1.0f, 0.5f));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"space_time", 1}, "Space Time", 0.0f, 1.0f, 0.5f));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"space_mix", 1}, "Space Mix", 0.0f, 1.0f, 0.33f));

    return { params.begin(), params.end() };
}

const juce::String EfxNavAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool EfxNavAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool EfxNavAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool EfxNavAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double EfxNavAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int EfxNavAudioProcessor::getNumPrograms()
{
    return 1;
}

int EfxNavAudioProcessor::getCurrentProgram()
{
    return 0;
}

void EfxNavAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String EfxNavAudioProcessor::getProgramName (int index)
{
    return {};
}

void EfxNavAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

void EfxNavAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();

    spaceReverb.prepare(spec);
}

void EfxNavAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool EfxNavAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void EfxNavAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Actualizar parámetros de Reverb desde APVTS
    float size = apvts.getRawParameterValue("space_size")->load();
    float depth = apvts.getRawParameterValue("space_depth")->load();
    float time = apvts.getRawParameterValue("space_time")->load();
    float mix = apvts.getRawParameterValue("space_mix")->load();
    
    spaceReverb.setParameters(size, depth, time, mix);

    // Procesar audio a través del módulo Space
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    
    spaceReverb.process(context);
}

bool EfxNavAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* EfxNavAudioProcessor::createEditor()
{
    return new EfxNavAudioProcessorEditor (*this);
    //return new juce::GenericAudioProcessorEditor(*this); // Para test rápido
}

void EfxNavAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void EfxNavAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState != nullptr)
        if (xmlState->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new EfxNavAudioProcessor();
}
