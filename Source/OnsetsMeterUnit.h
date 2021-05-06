//
//  OscMeterUnit.hpp
//  EssentiaTest
//
//  Created by Leo on 05/05/2021.
//

#pragma once

#include <JuceHeader.h>
#include "ofxAudioAnalyzer.h"
using namespace std;

class OnsetsMeterUnit: private juce::AudioProcessorValueTreeState::Listener {
public:
    
    OnsetsMeterUnit(int idx);
    ~OnsetsMeterUnit();
    
    void setup(foleys::MagicProcessorState* magicState, juce::AudioProcessorValueTreeState* treeState, ofxAudioAnalyzer* audioAnalyzer);
    void parameterChanged (const juce::String& param, float value) override;
    void prepareToPlay (double sampleRate, int samplesPerBlock);
    void process();
    
    void updateOnsetPtr();
    
    int getId() { return _idx; }
    unique_ptr<juce::AudioProcessorParameterGroup> getParameterGroup();
    
    juce::String meterId;
    juce::String algorithmTypeId;
    juce::String resetId;
    juce::String alphaId;
    juce::String timeTresholdId;
    juce::String silenceTresholdId;
    juce::String outputMeterId;
    juce::String historyPlotId;
    
private:
    void setOfxaaValue(ofxAAValue value);
    
    int _idx;
    ofxAudioAnalyzer* _audioAnalyzer;

    ofxAAValue currentOfxaaValue = NONE;
    foleys::MagicLevelSource* outputMeter  = nullptr;
    
    ofxAAOnsetsAlgorithm* onsetPtr = nullptr;
    
    atomic<bool>* reset  = nullptr;
    atomic<float>* alpha  = nullptr;
    atomic<float>* timeTreshold  = nullptr;
    atomic<float>* silenceTreshold  = nullptr;
    
};

