//
//  MeterUnit.cpp
//  EssentiaTest
//
//  Created by Leo on 05/05/2021.
//

#include "MeterUnit.h"
#include "StringUtils.h"

vector<ofxAAValue> availableValues { RMS, SPECTRAL_CENTROID, RMS, LOUDNESS } ;

namespace IDs
{
    static juce::String meter  { "Meter" };
    static juce::String algorithmType  { "algorithmType" };
    static juce::String smoothing  { "smoothing" };
    static juce::String resetMax  { "resetMax" };
    static juce::String maxEstimated  { "maxEstimated" };

    static juce::String outputMeter  { "outputMeter" };
    static juce::String historyPlot  { "historyPlot" };

    static juce::String IDwithIdx(juce::String ID, int idx) {
        return ID +":" + juce::String(idx);
    }
}

MeterUnit::MeterUnit(int idx) {
    _idx = idx;
    
    meterId = IDs::IDwithIdx(IDs::meter, _idx);
    algorithmTypeId = IDs::IDwithIdx(IDs::algorithmType, _idx);
    smoothingId = IDs::IDwithIdx(IDs::smoothing, _idx);
    resetMaxId = IDs::IDwithIdx(IDs::resetMax, _idx);
    maxEstimatedId = IDs::IDwithIdx(IDs::maxEstimated, _idx);
    outputMeterId = IDs::IDwithIdx(IDs::outputMeter, _idx);
    historyPlotId = IDs::IDwithIdx(IDs::historyPlot, _idx);
}

MeterUnit::~MeterUnit() {
    
}

void MeterUnit::setup(foleys::MagicProcessorState* magicState, juce::AudioProcessorValueTreeState* treeState, ofxAudioAnalyzer* audioAnalyzer) {
    
    _audioAnalyzer = audioAnalyzer;
    outputMeter  = magicState->createAndAddObject<foleys::MagicLevelSource>(outputMeterId);
    oscilloscope = magicState->createAndAddObject<foleys::MagicOscilloscope>(historyPlotId);
    
    smoothing = treeState->getRawParameterValue (smoothingId);
    jassert (smoothing != nullptr);
    
    treeState->addParameterListener (algorithmTypeId, this);
    treeState->addParameterListener (smoothingId, this);
    treeState->addParameterListener (resetMaxId, this);
    treeState->addParameterListener (maxEstimatedId, this);
}

unique_ptr<juce::AudioProcessorParameterGroup> MeterUnit::getParameterGroup() {
    auto generator = std::make_unique<juce::AudioProcessorParameterGroup>(meterId, TRANS (meterId), "|");
    
    auto options = juce::StringArray ("-NONE-");
    for (auto ofxaaValue : availableValues) {
        auto name = utils::valueTypeToString(ofxaaValue);
        options.add(name);
    }
    generator->addChild (std::make_unique<juce::AudioParameterChoice>(algorithmTypeId, "Type", options, 0),
                         std::make_unique<juce::AudioParameterFloat>(smoothingId, "Smoothing", juce::NormalisableRange<float>(0.0, 1.0, 0.01), 0.0f),
                         std::make_unique<juce::AudioParameterFloat>(maxEstimatedId, "Max Estimated", juce::NormalisableRange<float>(0.0, 100000.0, 0.01), 1.0f),
                         std::make_unique<juce::AudioParameterBool>(resetMaxId, "Reset Max", true));
    
    return generator;
}

void MeterUnit::parameterChanged (const juce::String& param, float value)
{
    if (param == algorithmTypeId) {
        if (value == 0) {
            setOfxaaValue(NONE);
        } else {
            int index = value - 1;
            if (index < availableValues.size()) {
                auto value = availableValues[index];
                setOfxaaValue(value);
            }
        }
    } else if (param == smoothingId) {
    } else if (param == resetMaxId) {
        outputMeter->resetMaxValue();
    } else if (param == maxEstimatedId) {
        if (currentOfxaaValue != NONE) {
            _audioAnalyzer->setMaxEstimatedValue(0, currentOfxaaValue, value);
        }
    }
}

void MeterUnit::setOfxaaValue(ofxAAValue value) {
    currentOfxaaValue = value;
    outputMeter->resetMaxValue();
}

void MeterUnit::prepareToPlay (double sampleRate, int samplesPerBlock) {
    outputMeter->setupSource (1, sampleRate, 500, 200); ///*** remove channels
    oscilloscope->prepareToPlay (750, 0);
}

void MeterUnit::process() {
    if (currentOfxaaValue != NONE) {
        float value = _audioAnalyzer->getValue(currentOfxaaValue, 0, *smoothing, false);
        float normalizedValue = _audioAnalyzer->getValue(currentOfxaaValue, 0, *smoothing, true);
        outputMeter->setValues(value, normalizedValue);
        oscilloscope->pushValue(normalizedValue);
    } else {
        outputMeter->setValues(0.0, 0.0);
        oscilloscope->pushValue(0.0);
    }
}
