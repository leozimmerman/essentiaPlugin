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

 
 - Remove algorithms that are not going to be used from network
 

 - Add OSC send panel
 - Check state saving
 */

juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout(const vector<MeterUnit*>* meterUnits, OnsetsMeterUnit* onsetsMeterUnit)
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    for (auto unit: *meterUnits) {
        auto generator = unit->getParameterGroup();
        layout.add (std::move (generator));
    }
    auto onsetsGenerator = onsetsMeterUnit->getParameterGroup();
    layout.add(std::move (onsetsGenerator));
    
    auto oscGenerator = std::make_unique<juce::AudioProcessorParameterGroup>("OSC", TRANS ("OSC"), "|");
    oscGenerator->addChild(std::make_unique<juce::AudioParameterFloat>(IDs::oscPort, "OSC PORT", juce::NormalisableRange<float>(1, 65535, 1), 9001));
    layout.add(std::move (oscGenerator));
    return layout;
}

//==============================================================================
// MARK: Init
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
treeState (*this, nullptr, "PARAMETERS", createParameterLayout(&meterUnits, &onsetsMeterUnit))
{
    
    audioAnalyzer.setup(44100, 1024, 1); ///*** this can be polished
    
    for (auto unit: meterUnits) {
        unit->setup(&magicState, &treeState, &audioAnalyzer);
    }
    onsetsMeterUnit.setup(&magicState, &treeState, &audioAnalyzer);
    treeState.addParameterListener (IDs::oscPort, this);
    magicState.setGuiValueTree (BinaryData::magic_xml, BinaryData::magic_xmlSize);
}

EssentiaTestAudioProcessor::~EssentiaTestAudioProcessor()
{
}

// MARK: Preparte to play
void EssentiaTestAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    audioAnalyzer.reset(sampleRate, samplesPerBlock, getTotalNumOutputChannels());
   
    for (auto unit: meterUnits) {
        unit->prepareToPlay(sampleRate, samplesPerBlock);
    }
    onsetsMeterUnit.prepareToPlay(sampleRate, samplesPerBlock);
    magicState.prepareToPlay (sampleRate, samplesPerBlock);
}

// MARK: Process block
void EssentiaTestAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    audioAnalyzer.analyze(buffer);
    
    for (auto unit: meterUnits) {
        unit->process();
    }
    onsetsMeterUnit.process();
    
    sendOscData();
}
//==============================================================================
// MARK: OSC
void EssentiaTestAudioProcessor::parameterChanged (const juce::String& param, float value) {
    if (param == IDs::oscPort) {
        oscPortHasChanged(value);
    }
}

void EssentiaTestAudioProcessor::oscHostHasChanged (juce::String newOscHostAdress) {
    _oscHost = newOscHostAdress;
    std::cout<< "HOST: "<< newOscHostAdress << std::endl;
    connectOscSender(_oscHost, _oscPort);
}

void EssentiaTestAudioProcessor::oscPortHasChanged(int newOscPort) {
    _oscPort = newOscPort;
    std::cout<< "PORT: "<< newOscPort << std::endl;
    connectOscSender(_oscHost, _oscPort);
}

void EssentiaTestAudioProcessor::connectOscSender(const juce::String& targetHostName, int targetPortNumber) {
    oscSender.disconnect();
    if (! oscSender.connect (targetHostName, targetPortNumber)) {
        juce::Logger::outputDebugString(&"Error: could not connect to UDP port:" [ targetPortNumber]);
    }
}

void EssentiaTestAudioProcessor::sendOscData() {
    string root = "/analyzer";
    for (int i=0; i<meterUnits.size(); ++i) {
        if (meterUnits[i]->isEnabled()) {
            juce::String address = root + "/" + std::to_string(i+1);
            juce::OSCAddressPattern addressPattern = juce::OSCAddressPattern(address);
            oscSender.send(addressPattern, meterUnits[i]->getValue());
        }
    }
    
    if (onsetsMeterUnit.isEnabled()) {
        juce::String address = root + "/onsets";
        juce::OSCAddressPattern addressPattern = juce::OSCAddressPattern(address);
        oscSender.send(addressPattern, onsetsMeterUnit.getValue());
    }
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

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new EssentiaTestAudioProcessor();
}
