/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "StringUtils.h"


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
    oscGenerator->addChild(std::make_unique<juce::AudioParameterInt>(IDs::oscPort,            // parameterID
                                                                     IDs::oscPortName,            // parameter name
                                                                     MIN_OSC_PORT,              // minimum value
                                                                     MAX_OSC_PORT,              // maximum value
                                                                     DEFAULT_OSC_PORT));
    layout.add(std::move (oscGenerator));
    return layout;
}

//==============================================================================
// MARK: Init
EssentiaPluginAudioProcessor::EssentiaPluginAudioProcessor()
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
    
    magicState.addOscListener(this);
}

EssentiaPluginAudioProcessor::~EssentiaPluginAudioProcessor()
{
}

// MARK: Preparte to play
void EssentiaPluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    audioAnalyzer.reset(sampleRate, samplesPerBlock, getTotalNumOutputChannels());
   
    for (auto unit: meterUnits) {
        unit->prepareToPlay(sampleRate, samplesPerBlock);
    }
    onsetsMeterUnit.prepareToPlay(sampleRate, samplesPerBlock);
    magicState.prepareOscData();
    magicState.prepareToPlay (sampleRate, samplesPerBlock);
}

// MARK: Process block
void EssentiaPluginAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
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
void EssentiaPluginAudioProcessor::parameterChanged (const juce::String& param, float value) {
    if (param == IDs::oscPort) {
        oscPortHasChanged(value);
    }
}

void EssentiaPluginAudioProcessor::oscMainIDHasChanged (juce::String newOscMainID) {
    oscManager.setMaindId(newOscMainID);
}

void EssentiaPluginAudioProcessor::oscHostHasChanged (juce::String newOscHostAdress) {
    oscManager.setOscHost(newOscHostAdress);
}

void EssentiaPluginAudioProcessor::oscPortHasChanged(int newOscPort) {
    oscManager.setOscPort(newOscPort);
}

void EssentiaPluginAudioProcessor::sendOscData() {
    for (auto unit: meterUnits) {
        if (unit->isEnabled()) {
            oscManager.sendValue(unit->getValue(), unit->getTypeName());
        }
    }
     
    if (onsetsMeterUnit.isEnabled()) {
        oscManager.sendValue(onsetsMeterUnit.getValue(), onsetsMeterUnit.getTypeName());
    }
}

void EssentiaPluginAudioProcessor::postSetStateInformation() {
    magicEditor->updateOscLabelsTexts(true);
}
//==============================================================================

const juce::String EssentiaPluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool EssentiaPluginAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool EssentiaPluginAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool EssentiaPluginAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double EssentiaPluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int EssentiaPluginAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int EssentiaPluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void EssentiaPluginAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String EssentiaPluginAudioProcessor::getProgramName (int index)
{
    return {};
}

void EssentiaPluginAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================


void EssentiaPluginAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool EssentiaPluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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
    return new EssentiaPluginAudioProcessor();
}
