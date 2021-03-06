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

MagicProcessorState::MagicProcessorState (juce::AudioProcessor& processorToUse)
  : processor (processorToUse)
{
}

juce::StringArray MagicProcessorState::getParameterNames() const
{
    return parameters.getParameterNames();
}

juce::PopupMenu MagicProcessorState::createParameterMenu() const
{
    juce::PopupMenu menu;
    int index = 0;
    addParametersToMenu (processor.getParameterTree(), menu, index);
    return menu;
}

void MagicProcessorState::addParametersToMenu (const juce::AudioProcessorParameterGroup& group, juce::PopupMenu& menu, int& index) const
{
    for (const auto& node : group)
    {
        if (const auto* parameter = node->getParameter())
        {
            if (const auto* withID = dynamic_cast<const juce::AudioProcessorParameterWithID*>(parameter))
                menu.addItem (++index, withID->paramID);
        }
        else if (const auto* groupNode = node->getGroup())
        {
            juce::PopupMenu subMenu;
            addParametersToMenu (*groupNode, subMenu, index);
            menu.addSubMenu (groupNode->getName(), subMenu);
        }
    }
}

juce::RangedAudioParameter* MagicProcessorState::getParameter (const juce::String& paramID)
{
    return parameters.getParameter (paramID);
}

void MagicProcessorState::updateParameterMap()
{
    parameters.updateParameterMap();
}

std::unique_ptr<juce::SliderParameterAttachment> MagicProcessorState::createAttachment (const juce::String& paramID, juce::Slider& slider)
{
    if (auto* parameter = getParameter (paramID))
        return std::make_unique<juce::SliderParameterAttachment>(*parameter, slider);

    // You have connected a control to a parameter that doesn't exist. Please fix your GUI.
    // You may safely click continue in your debugger
    jassertfalse;
    return {};
}

std::unique_ptr<juce::ComboBoxParameterAttachment> MagicProcessorState::createAttachment (const juce::String& paramID, juce::ComboBox& combobox)
{
    if (auto* parameter = getParameter (paramID))
        return std::make_unique<juce::ComboBoxParameterAttachment>(*parameter, combobox);

    // You have connected a control to a parameter that doesn't exist. Please fix your GUI.
    // You may safely click continue in your debugger
    jassertfalse;
    return {};
}

std::unique_ptr<juce::ButtonParameterAttachment> MagicProcessorState::createAttachment (const juce::String& paramID, juce::Button& button)
{
    if (auto* parameter = getParameter (paramID))
        return std::make_unique<juce::ButtonParameterAttachment>(*parameter, button);

    // You have connected a control to a parameter that doesn't exist. Please fix your GUI.
    // You may safely click continue in your debugger
    jassertfalse;
    return {};
}

void MagicProcessorState::setOscIPAdress(const juce::String address) {
    if (oscListener != nullptr) {
        oscListener->oscHostHasChanged(address);
        setLastHostAddress(address);
    }
}

void MagicProcessorState::setOscMainID(const juce::String mainID) {
    if (oscListener != nullptr) {
        oscListener->oscMainIDHasChanged(mainID);
        setLastMainId(mainID);
    }
}

juce::AudioProcessor* MagicProcessorState::getProcessor()
{
    return &processor;
}

//----------------------------------------
// MARK: OSC
void MagicProcessorState::addOscListener(OscHostListener* listener) {
    oscListener = listener;
}

void MagicProcessorState::setLastHostAddress(juce::String address) {
    auto oscNode = getValueTree().getOrCreateChildWithName (IDs::oscData, nullptr);
    oscNode.setProperty (IDs::hostAddress,  address,  nullptr);
}

bool MagicProcessorState::getLastHostAddress(juce::String& address) {
    auto oscNode = getValueTree().getOrCreateChildWithName (IDs::oscData, nullptr);
    if (oscNode.hasProperty (IDs::hostAddress) == false)
        return false;

    address  = oscNode.getProperty (IDs::hostAddress);
    return true;
}

void MagicProcessorState::setLastMainId(juce::String identifier) {
    auto oscNode = getValueTree().getOrCreateChildWithName (IDs::oscData, nullptr);
    oscNode.setProperty (IDs::mainId,  identifier,  nullptr);
}

bool MagicProcessorState::getLastMaindId(juce::String& identifier) {
    auto oscNode = getValueTree().getOrCreateChildWithName (IDs::oscData, nullptr);
    if (oscNode.hasProperty (IDs::mainId) == false)
        return false;

    identifier  = oscNode.getProperty (IDs::mainId);
    return true;
}

//----------------------------------------

void MagicProcessorState::setLastEditorSize (int  width, int  height)
{
    auto sizeNode = getValueTree().getOrCreateChildWithName (IDs::lastSize, nullptr);
    sizeNode.setProperty (IDs::width,  width,  nullptr);
    sizeNode.setProperty (IDs::height, height, nullptr);
}

bool MagicProcessorState::getLastEditorSize (int& width, int& height)
{
    auto sizeNode = getValueTree().getOrCreateChildWithName (IDs::lastSize, nullptr);
    if (sizeNode.hasProperty (IDs::width) == false || sizeNode.hasProperty (IDs::height) == false)
        return false;

    width  = sizeNode.getProperty (IDs::width);
    height = sizeNode.getProperty (IDs::height);
    return true;
}

void MagicProcessorState::prepareOscData() {
    juce::String hostAddress = DEFAULT_OSC_HOST;
    getLastHostAddress(hostAddress);
    setOscIPAdress(hostAddress);
    
    juce::String mainId = DEFAULT_OSC_MAIN_ID;
    getLastMaindId(mainId);
    setOscMainID(mainId);
}

void MagicProcessorState::getStateInformation (juce::MemoryBlock& destData)
{
    auto newState = getValueTree();
    parameters.saveParameterValues (newState);

    juce::MemoryOutputStream stream (destData, false);
    newState.writeToStream (stream);
}

void MagicProcessorState::setStateInformation (const void* data, int sizeInBytes, juce::AudioProcessorEditor* editor)
{
    auto tree = juce::ValueTree::readFromData (data, size_t (sizeInBytes));
    if (tree.isValid() == false)
        return;

    auto newState = getValueTree();
    if (newState.getType() != tree.getType())
        return;

    newState.copyPropertiesAndChildrenFrom (tree, nullptr);

    parameters.loadParameterValues (newState);

    if (editor)
    {
        int width, height;

        if (getLastEditorSize (width, height))
            editor->setSize (width, height);
    }
}

void MagicProcessorState::updatePlayheadInformation (juce::AudioPlayHead* playhead)
{
    if (playhead == nullptr)
        return;

    juce::AudioPlayHead::CurrentPositionInfo info;
    playhead->getCurrentPosition (info);

    bpm.store (info.bpm);
    timeInSeconds.store (info.timeInSeconds);
    timeSigNumerator.store (info.timeSigNumerator);
    timeSigDenominator.store (info.timeSigDenominator);
    isPlaying.store (info.isPlaying);
    isRecording.store (info.isRecording);
}

void MagicProcessorState::setPlayheadUpdateFrequency (int frequency)
{
    if (frequency > 0)
        startTimerHz (frequency);
    else
        stopTimer();
}

void MagicProcessorState::processMidiBuffer (juce::MidiBuffer& buffer, int numSamples, bool injectIndirectEvents)
{
    getKeyboardState().processNextMidiBuffer (buffer, 0, numSamples, injectIndirectEvents);

    midiMapper.processMidiBuffer (buffer);
}

void MagicProcessorState::mapMidiController (int cc, const juce::String& parameterID)
{
    midiMapper.mapMidiController (cc, parameterID);
}

int MagicProcessorState::getLastController() const
{
    return midiMapper.getLastController();
}

juce::ValueTree MagicProcessorState::createDefaultGUITree() const
{
    juce::ValueTree rootNode {IDs::view, {{ IDs::id, IDs::root }}};

    auto current = rootNode;
    auto plotNames = getObjectIDsByType<MagicPlotSource>();

    if (plotNames.isEmpty() == false)
    {
        juce::StringArray colours { "orange", "blue", "red", "silver", "green", "cyan", "brown", "white" };
        int nextColour = 0;

        juce::ValueTree child { IDs::view, {
            { IDs::id, "plot-view" },
            { IDs::styleClass, "plot-view" }}};

        for (auto plotName : plotNames)
        {
            child.appendChild ({IDs::plot, {
                { IDs::source, plotName },
                { "plot-color", colours [nextColour++] }}}, nullptr);

            if (nextColour >= colours.size())
                nextColour = 0;
        }

        current.appendChild (child, nullptr);

        juce::ValueTree params { IDs::view, {
            { IDs::styleClass, "parameters nomargin" }}};

        current.appendChild (params, nullptr);
        current = params;
    }

    createDefaultFromParameters (current, processor.getParameterTree());

    return rootNode;
}

void MagicProcessorState::createDefaultFromParameters (juce::ValueTree& node, const juce::AudioProcessorParameterGroup& tree) const
{
    for (const auto& sub : tree.getSubgroups (false))
    {
        auto child = juce::ValueTree (IDs::view, {
            {IDs::caption, sub->getName()},
            {IDs::styleClass, "group"}});

        createDefaultFromParameters (child, *sub);
        node.appendChild (child, nullptr);
    }

    for (const auto& param : tree.getParameters (false))
    {
        auto child = juce::ValueTree (IDs::slider);
        if (dynamic_cast<juce::AudioParameterChoice*>(param) != nullptr)
            child = juce::ValueTree (IDs::comboBox);
        else if (dynamic_cast<juce::AudioParameterBool*>(param) != nullptr)
            child = juce::ValueTree (IDs::toggleButton);

        child.setProperty (IDs::caption, param->getName (64), nullptr);
        if (const auto* parameterWithID = dynamic_cast<juce::AudioProcessorParameterWithID*>(param))
            child.setProperty (IDs::parameter, parameterWithID->paramID, nullptr);

        node.appendChild (child, nullptr);
    }
}


void MagicProcessorState::timerCallback()
{
    getPropertyAsValue ("playhead:bpm").setValue (bpm.load());
    getPropertyAsValue ("playhead:timeInSeconds").setValue (timeInSeconds.load());
    getPropertyAsValue ("playhead:timeSigNumerator").setValue (timeSigNumerator.load());
    getPropertyAsValue ("playhead:timeSigDenominator").setValue (timeSigDenominator.load());
    getPropertyAsValue ("playhead:isPlaying").setValue (isPlaying.load());
    getPropertyAsValue ("playhead:isRecording").setValue (isRecording.load());
}

} // namespace foleys
