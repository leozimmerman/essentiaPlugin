/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

/**
 TODO:

 - Add algorithm switch
 - Set standard Max estimated value for algorithm
 - Encapsulate meter structures to make four of them
 - Remove algorithms that are not going to be used from network
 - Add ONSET meter
 - Add OSC send panel
 - Check state saving
 */


namespace IDs
{
    static juce::String algorithmType  { "algorithmType" };
    static juce::String smoothing  { "smoothing" };
    static juce::String resetMax  { "resetMax" };
    static juce::String maxEstimated  { "maxEstimated" };


    static juce::Identifier oscilloscope { "oscilloscope" };
}

juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    auto generator = std::make_unique<juce::AudioProcessorParameterGroup>("Meter", TRANS ("Meter"), "|");
    auto options = juce::StringArray ("None", "SPECTRAL_ROLLOFF", "Power", "Square");
    generator->addChild (std::make_unique<juce::AudioParameterChoice>(IDs::algorithmType, "Type", options, 0),
                         std::make_unique<juce::AudioParameterFloat>(IDs::smoothing, "Smoothing", juce::NormalisableRange<float>(0.0, 1.0, 0.01), 0.5f),
                         std::make_unique<juce::AudioParameterFloat>(IDs::maxEstimated, "Max Estimated", juce::NormalisableRange<float>(0.0, 100000.0, 0.01), 1.0f),
                         std::make_unique<juce::AudioParameterBool>(IDs::resetMax, "Enabled", true));

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
    treeState (*this, nullptr, "PARAMETERS", createParameterLayout())
{
    outputMeter  = magicState.createAndAddObject<foleys::MagicLevelSource>("outputMeter");
    
    audioAnalyzer.setup(44100, 1024, 1); ///*** this can be polished
    
    
    smoothing = treeState.getRawParameterValue (IDs::smoothing);
    jassert (smoothing != nullptr);
    
    treeState.addParameterListener (IDs::algorithmType, this);
    treeState.addParameterListener (IDs::smoothing, this);
    treeState.addParameterListener (IDs::resetMax, this);
    treeState.addParameterListener (IDs::maxEstimated, this);
    
    magicState.setGuiValueTree (BinaryData::magic_xml, BinaryData::magic_xmlSize);
}

EssentiaTestAudioProcessor::~EssentiaTestAudioProcessor()
{
}

//==============================================================================

void EssentiaTestAudioProcessor::parameterChanged (const juce::String& param, float value)
{
    if (param == IDs::algorithmType) {
        cout<<"type: "<<value<<endl;
    } else if (param == IDs::smoothing) {
        cout<<"smoothing: "<<value<<endl;
    } else if (param == IDs::resetMax) {
        outputMeter->resetMaxValue();
    } else if (param == IDs::maxEstimated) {
       /// audioAnalyzer.setMaxEstimatedValue(0, POWER, value);
    }
}

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
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    outputMeter->setupSource (getTotalNumOutputChannels(), sampleRate, 500, 200);
    audioAnalyzer.reset(sampleRate, samplesPerBlock, getTotalNumOutputChannels());
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

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());



   audioAnalyzer.analyze(buffer);

    auto normValue = audioAnalyzer.getValue(POWER, 0, *smoothing, true);
    outputMeter->setValues(audioAnalyzer.getValue(POWER, 0, *smoothing, false), normValue);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new EssentiaTestAudioProcessor();
}
