//
//  OscMeterUnit.cpp
//  EssentiaTest
//
//  Created by Leo on 05/05/2021.
//

#include "OnsetsMeterUnit.h"
#include "StringUtils.h"

OnsetsMeterUnit::OnsetsMeterUnit(int idx) {
    _idx = idx;
    
    meterId = IDs::IDwithIdx(IDs::meter, _idx);
    algorithmTypeId = IDs::IDwithIdx(IDs::algorithmType, _idx);
    resetId = IDs::IDwithIdx(IDs::resetOnsets, _idx);
    alphaId = IDs::IDwithIdx(IDs::alpha, _idx);
    timeTresholdId = IDs::IDwithIdx(IDs::timeTreshold, _idx);
    silenceTresholdId = IDs::IDwithIdx(IDs::silenceTreshold, _idx);
    outputMeterId = IDs::IDwithIdx(IDs::outputMeter, _idx);
    historyPlotId = IDs::IDwithIdx(IDs::historyPlot, _idx);
}

OnsetsMeterUnit::~OnsetsMeterUnit() {
    
}

void OnsetsMeterUnit::setup(foleys::MagicProcessorState* magicState, juce::AudioProcessorValueTreeState* treeState, ofxAudioAnalyzer* audioAnalyzer) {
    
    _audioAnalyzer = audioAnalyzer;
    outputMeter  = magicState->createAndAddObject<foleys::MagicLevelSource>(outputMeterId);
    oscilloscope = magicState->createAndAddObject<foleys::MagicOscilloscope>(historyPlotId);
    
    alpha = treeState->getRawParameterValue (alphaId);
    jassert (alpha != nullptr);
    
    timeTreshold = treeState->getRawParameterValue (timeTresholdId);
    jassert (timeTreshold != nullptr);
    
    silenceTreshold = treeState->getRawParameterValue (silenceTresholdId);
    jassert (silenceTreshold != nullptr);
    
    
    treeState->addParameterListener (algorithmTypeId, this);
    treeState->addParameterListener (resetId, this);
    treeState->addParameterListener (alphaId, this);
    treeState->addParameterListener (silenceTresholdId, this);
    treeState->addParameterListener (timeTresholdId, this);
    
    updateOnsetPtr();
    
}

unique_ptr<juce::AudioProcessorParameterGroup> OnsetsMeterUnit::getParameterGroup() {
    auto generator = std::make_unique<juce::AudioProcessorParameterGroup>(meterId, TRANS (meterId), "|");
    
    auto options = juce::StringArray ("-NONE-");
    auto name = utils::valueTypeToString(ONSETS);
    options.add(name);

    string typeParameterName = "Type:Onsets";
    string alphaParameterName = "Alpha:Onsets";
    string silenceTrshParameterName = "SilenceTreshold:Onsets";
    string timeTrshParameterName = "TimeTreshold:Onsets";
    string resetParameterName = "Reset:Onsets";
    
    generator->addChild (std::make_unique<juce::AudioParameterChoice>(algorithmTypeId, typeParameterName, options, 0),
                         std::make_unique<juce::AudioParameterFloat>(alphaId, alphaParameterName, juce::NormalisableRange<float>(0.0, 1.0, 0.01), 0.001f),
                         std::make_unique<juce::AudioParameterFloat>(silenceTresholdId, silenceTrshParameterName, juce::NormalisableRange<float>(0.0, 1.0, 0.01), 0.01f),
                         std::make_unique<juce::AudioParameterFloat>(timeTresholdId, timeTrshParameterName, juce::NormalisableRange<float>(0.0, 1000.0, 1.0), 1.0f),
                         std::make_unique<juce::AudioParameterBool>(resetId, resetParameterName, true));
    
    return generator;
}

void OnsetsMeterUnit::parameterChanged (const juce::String& param, float value)
{
    if (param == algorithmTypeId) {
        if (value == 0) {
            setOfxaaValue(NONE);
        } else {
            setOfxaaValue(ONSETS);
        }
    } else if (param == resetId) {
        onsetPtr->reset();
    } else if (param == alphaId) {
        onsetPtr->setOnsetAlpha(*alpha);
    } else if (param == silenceTresholdId) {
        onsetPtr->setOnsetSilenceThreshold(*silenceTreshold);
    } else if (param == timeTresholdId) {
        onsetPtr->setOnsetTimeThreshold(*timeTreshold);
    }
}

void OnsetsMeterUnit::prepareToPlay (double sampleRate, int samplesPerBlock) {
    outputMeter->setupSource (1); ///*** remove channels
    updateOnsetPtr();
}

bool OnsetsMeterUnit::isEnabled() {
    return currentOfxaaValue != NONE;
}

float OnsetsMeterUnit::getValue() {
    return outputMeter->getNormalizedValue();
}

string OnsetsMeterUnit::getTypeName() {
    return utils::valueTypeToString(currentOfxaaValue);
}

void OnsetsMeterUnit::updateOnsetPtr() {
    auto ptrs = _audioAnalyzer->getChannelAnalyzersPtrs();
    if (ptrs.size() == 0) return;
    auto channelUnit = ptrs[0];
    onsetPtr = channelUnit->getOnsetsPtr();
}

void OnsetsMeterUnit::setOfxaaValue(ofxAAValue value) {
    currentOfxaaValue = value;
}


void OnsetsMeterUnit::process() {
    if (currentOfxaaValue == ONSETS) {
        bool onsetValue = onsetPtr->getValue();//getValue(currentOfxaaValue, 0, *smoothing, false);
        float value = onsetValue ? 1.0 : 0.0;
        outputMeter->setValues(value, value);
        oscilloscope->pushValue(value);
    } else {
        outputMeter->setValues(0.0, 0.0);
        oscilloscope->pushValue(0.0);
    }
}
