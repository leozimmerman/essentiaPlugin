/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "ofxAudioAnalyzer.h"
#include "MeterUnit.h"

using namespace std;

//==============================================================================
/**
*/
class EssentiaTestAudioProcessor  : public foleys::MagicProcessor
{
public:
    //==============================================================================
    EssentiaTestAudioProcessor();
    ~EssentiaTestAudioProcessor() override;

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

private:
    ofxAudioAnalyzer audioAnalyzer;
    
    MeterUnit unit0 = MeterUnit(0);
    MeterUnit unit1 = MeterUnit(1);
    MeterUnit unit2 = MeterUnit(2);
    MeterUnit unit3 = MeterUnit(3);
    vector<MeterUnit*> meterUnits { &unit0, &unit1, &unit2, &unit3 };
 
    juce::AudioProcessorValueTreeState treeState;
    //==============================================================================

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EssentiaTestAudioProcessor)
};
