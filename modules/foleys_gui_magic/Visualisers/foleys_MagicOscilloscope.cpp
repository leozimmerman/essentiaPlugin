/*
 ==============================================================================
    Copyright (c) 2019-2021 Foleys Finest Audio - Daniel Walz
    All rights reserved.

    License for non-commercial projects:

    Redistribution and use in source and binary forms, with or without modification,
    are permitted provided that the following conditions are met:
    1. Redistributions of source code must retain the above copyright notice, this
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the names of its contributors
       may be used to endorse or promote products derived from this software without
       specific prior written permission.

    License for commercial products:

    To sell commercial products containing this module, you are required to buy a
    License from https://foleysfinest.com/developer/pluginguimagic/

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
    IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
    BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
    LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
    OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
    OF THE POSSIBILITY OF SUCH DAMAGE.
 ==============================================================================
 */


namespace foleys
{


MagicOscilloscope::MagicOscilloscope (int channelToDisplay)
  : channel (channelToDisplay)
{
}
void MagicOscilloscope::pushValue(const float value) {
   
    if (values.size() == 0) return ;
    
    for (unsigned long i = values.size()-1; i>0 ; --i) {
        values[i] = values[i-1];
    }
    values[0] = value;
    resetLastDataFlag();
}

void MagicOscilloscope::pushSamples (const juce::AudioBuffer<float>& buffer)
{
    juce::Logger::outputDebugString("Magic Oscilloscope: deprecated pushShamples() called");
}

void MagicOscilloscope::createPlotPaths (juce::Path& path, juce::Path& filledPath, juce::Rectangle<float> bounds, MagicPlotComponent&)
{

    int pos = 0;
    
    path.clear();
    path.startNewSubPath (bounds.getX(),
                          juce::jmap (values [pos], 0.0f, 1.0f, bounds.getBottom(), bounds.getY()));

    for (int i = 1; i < values.size(); ++i)
    {
        ++pos;

        path.lineTo (juce::jmap (float (i),   0.0f, float (values.size()), bounds.getX(), bounds.getRight()),
                     juce::jmap (values [pos], 0.0f, 1.0f,                 bounds.getBottom(), bounds.getY()));
    }

    filledPath = path;
    filledPath.lineTo (bounds.getBottomRight());
    filledPath.lineTo (bounds.getBottomLeft());
    filledPath.closeSubPath();
}

void MagicOscilloscope::prepareToPlay (double sampleRateToUse, int)
{
    sampleRate = sampleRateToUse;

    samples.setSize (1, static_cast<int> (sampleRate));
    samples.clear();
    
    values.clear();
    values.resize(sampleRate, 0.0);
    
    writePosition.store (0);
}


} // namespace foleys
