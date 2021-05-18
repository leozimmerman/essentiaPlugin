/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "ofxAudioAnalyzer.h"
#include "MeterUnit.h"
#include "OnsetsMeterUnit.h"

using namespace std;

//==============================================================================
/**
*/
class EssentiaPluginAudioProcessor  : public foleys::MagicProcessor, private juce::AudioProcessorValueTreeState::Listener
{
public:
    //==============================================================================
    EssentiaPluginAudioProcessor();
    ~EssentiaPluginAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

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
    
    void oscHostHasChanged(juce::String newOscHostAdress) override;
    void oscMainIDHasChanged (juce::String newOscMainID) override;
    void oscPortHasChanged(int newOscPort);
    void parameterChanged (const juce::String& param, float value) override;
    
private:
    void connectOscSender(const juce::String& targetHostName, int targetPortNumber);
    void sendOscData();
    void showConnectionErrorMessage (const juce::String& messageText);
    
    ofxAudioAnalyzer audioAnalyzer;
    
    MeterUnit unit0 = MeterUnit(0);
    MeterUnit unit1 = MeterUnit(1);
    MeterUnit unit2 = MeterUnit(2);
    MeterUnit unit3 = MeterUnit(3);
    vector<MeterUnit*> meterUnits { &unit0, &unit1, &unit2, &unit3 };
    OnsetsMeterUnit onsetsMeterUnit = OnsetsMeterUnit(99);
 
    juce::AudioProcessorValueTreeState treeState;
    
    juce::OSCSender oscSender;
    juce::String _oscHost = "127.0.0.1"; ///*** check on saving test
    juce::String _mainID = "";
    int _oscPort = 0;
    //==============================================================================

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EssentiaPluginAudioProcessor)
};
