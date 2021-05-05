/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "StringUtils.h"
/**
 TODO:

 
 - Encapsulate meter structures to make four of them
 - Remove algorithms that are not going to be used from network
 
 - Add ONSET meter
 - Add OSC send panel
 - Check state saving
 */

juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout(vector<MeterUnit>& meterUnits)
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    for (auto unit: meterUnits) {
        auto generator = unit.getParameterGroup();
        layout.add (std::move (generator));
    }
    return layout;
}

juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout(MeterUnit meterUnit)
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    
    auto generator = meterUnit.getParameterGroup();
    layout.add (std::move (generator));
    
    return layout;
}

//==============================================================================
EssentiaTestAudioProcessor::EssentiaTestAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : MagicProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
#endif
    treeState (*this, nullptr, "PARAMETERS", createParameterLayout(unit))
{
    
    unit.setup(&magicState, &treeState, &audioAnalyzer);
    
    audioAnalyzer.setup(44100, 1024, 1); ///*** this can be polished
    magicState.setGuiValueTree (BinaryData::magic_xml, BinaryData::magic_xmlSize);
}

EssentiaTestAudioProcessor::~EssentiaTestAudioProcessor()
{
}

//==============================================================================


const juce::String EssentiaTestAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool EssentiaTestAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool EssentiaTestAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool EssentiaTestAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double EssentiaTestAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int EssentiaTestAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int EssentiaTestAudioProcessor::getCurrentProgram()
{
    return 0;
}

void EssentiaTestAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String EssentiaTestAudioProcessor::getProgramName (int index)
{
    return {};
}

void EssentiaTestAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void EssentiaTestAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    audioAnalyzer.reset(sampleRate, samplesPerBlock, getTotalNumOutputChannels());
   
    unit.prepareToPlay(sampleRate, samplesPerBlock);
    
    magicState.prepareToPlay (sampleRate, samplesPerBlock);
 
}

void EssentiaTestAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool EssentiaTestAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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
void EssentiaTestAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    audioAnalyzer.analyze(buffer);
    
    unit.process();
    
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new EssentiaTestAudioProcessor();
}
