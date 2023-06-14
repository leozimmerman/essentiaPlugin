/*
 * Copyright (C) 2016 Leo Zimmerman [http://www.leozimmerman.com.ar]
 *
 * ofxAudioAnalyzer is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Affero General Public License as published by the Free
 * Software Foundation (FSF), either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the Affero GNU General Public License
 * version 3 along with this program.  If not, see http://www.gnu.org/licenses/
 *
 * ---------------------------------------------------------------
 *
 * This project uses Essentia, copyrighted by Music Technology Group - Universitat Pompeu Fabra
 * using GNU Affero General Public License.
 * See http://essentia.upf.edu for documentation.
 *
 */

#include "ofxAudioAnalyzerUnit.h"
#include "ofxAAConfigurations.h"

#pragma mark - Main funcs
#define ACCUMULATED_BUFFER_SIZE 1024

ofxAudioAnalyzerUnit::ofxAudioAnalyzerUnit(int sampleRate, int bufferSize) {
    samplerate = sampleRate;
    framesize = ACCUMULATED_BUFFER_SIZE;
    
    audioBuffer.resize(bufferSize, 0.0);
    accumulatedAudioBuffer.resize(ACCUMULATED_BUFFER_SIZE, 0.0);
    
    network = new ofxaa::Network(samplerate, framesize);
}
//--------------------------------------------------------------
void ofxAudioAnalyzerUnit::analyze(const vector<float> & inBuffer){
    
//    if(inBuffer.size() != framesize){
//        cout<<"ofxAudioAnalyzerUnit: buffer requested to analyze size(" <<inBuffer.size()<<")doesnt match the buffer size already set: "<<framesize<< endl;
//    }
    
    //Cast of incoming audio buffer to Real
    for (int i=0; i<inBuffer.size();i++){
        audioBuffer[i] = (Real) inBuffer[i];
    }
    
    accumulatedAudioBuffer.erase(accumulatedAudioBuffer.begin(), accumulatedAudioBuffer.begin()+inBuffer.size());
    accumulatedAudioBuffer.insert(accumulatedAudioBuffer.end(), audioBuffer.begin(), audioBuffer.end());
    
    if (accumulatedAudioBuffer.size() != ACCUMULATED_BUFFER_SIZE) {
        cout<<"ALTO:"<< accumulatedAudioBuffer.size() << endl;
    }
    
    network->computeAlgorithms(accumulatedAudioBuffer, accumulatedAudioBuffer);
}

//--------------------------------------------------------------
void ofxAudioAnalyzerUnit::exit(){
    delete network;
}

//--------------------------------------------------------------

//----------------------------------------------
#pragma mark - Get values
//----------------------------------------------
float ofxAudioAnalyzerUnit::getValue(ofxAAValue value, float smooth, bool normalized){
    return network->getValue(value , smooth, normalized);
}

//----------------------------------------------
vector<float>& ofxAudioAnalyzerUnit::getValues(ofxAABinsValue value, float smooth , bool normalized){
    return network->getValues(value, smooth, normalized);
}
//----------------------------------------------
#pragma mark - Activates
//----------------------------------------------
void ofxAudioAnalyzerUnit::setActive(ofxAAValue valueType, bool state){
    network->getAlgorithmWithType(valueType)->isActive = state;
}
//----------------------------------------------
void ofxAudioAnalyzerUnit::setActive(ofxAABinsValue valueType, bool state){
    network->getAlgorithmWithType(valueType)->isActive = state;
}
//----------------------------------------------
bool ofxAudioAnalyzerUnit::getIsActive(ofxAAValue valueType){
    return network->getAlgorithmWithType(valueType)->isActive;
}
//----------------------------------------------
bool ofxAudioAnalyzerUnit::getIsActive(ofxAABinsValue valueType){
    return network->getAlgorithmWithType(valueType)->isActive;
}
//----------------------------------------------
float ofxAudioAnalyzerUnit::getMaxEstimatedValue(ofxAAValue valueType){
    return network->getMaxEstimatedValue(valueType);
}
//----------------------------------------------
float ofxAudioAnalyzerUnit::getMaxEstimatedValue(ofxAABinsValue valueType){
    return network->getMaxEstimatedValue(valueType);
}
//----------------------------------------------
float ofxAudioAnalyzerUnit::getMinEstimatedValue(ofxAAValue valueType){
    return network->getMinEstimatedValue(valueType);
}
//----------------------------------------------
float ofxAudioAnalyzerUnit::getMinEstimatedValue(ofxAABinsValue valueType){
    return network->getMinEstimatedValue(valueType);
}
//----------------------------------------------
void ofxAudioAnalyzerUnit::setMaxEstimatedValue(ofxAAValue valueType, float value){
    network->setMaxEstimatedValue(valueType, value);
}
//----------------------------------------------
void ofxAudioAnalyzerUnit::setMaxEstimatedValue(ofxAABinsValue valueType, float value){
    network->setMaxEstimatedValue(valueType, value);
}

