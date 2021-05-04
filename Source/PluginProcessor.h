/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "ofxAudioAnalyzer.h"
using namespace std;

//==============================================================================
/**
*/
class EssentiaTestAudioProcessor  : public foleys::MagicProcessor,
                                    private juce::AudioProcessorValueTreeState::Listener
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
    
    void parameterChanged (const juce::String& param, float value) override;

private:
    ofxAudioAnalyzer audioAnalyzer;
    
    //==============================================================================
    void setOfxaaValue(ofxAAValue value);
    ofxAAValue currentOfxaaValue = NONE;
    foleys::MagicLevelSource* outputMeter  = nullptr;
    foleys::MagicPlotSource*  oscilloscope = nullptr;
    
    atomic<bool>* resetMax  = nullptr;
    atomic<float>* smoothing  = nullptr;
    atomic<float>* maxEstimated  = nullptr;
    
    //==============================================================================
    
    
    
    juce::AudioProcessorValueTreeState treeState;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EssentiaTestAudioProcessor)
};
