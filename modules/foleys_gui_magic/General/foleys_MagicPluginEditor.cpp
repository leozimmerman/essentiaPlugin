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

#define PLUGIN_VERSION "v0.0.2"


namespace foleys
{

MagicPluginEditor::MagicPluginEditor (MagicProcessorState& stateToUse, std::unique_ptr<MagicGUIBuilder> builderToUse)
  : juce::AudioProcessorEditor (*stateToUse.getProcessor()),
    processorState (stateToUse),
    builder (std::move (builderToUse))
{
#if JUCE_MODULE_AVAILABLE_juce_opengl && FOLEYS_ENABLE_OPEN_GL_CONTEXT
    oglContext.attachTo (*this);
#endif

    if (builder.get() == nullptr)
    {
        builder = std::make_unique<MagicGUIBuilder>(processorState);
        builder->registerJUCEFactories();
        builder->registerJUCELookAndFeels();
    }

#if FOLEYS_SAVE_EDITED_GUI_IN_PLUGIN_STATE
    auto guiTree = processorState.getValueTree().getChildWithName ("magic");
    if (guiTree.isValid())
        ///*** setConfigTree (guiTree);
        builder->createGUI (*this);
    else
        builder->createGUI (*this);
#else  // FOLEYS_SAVE_EDITED_GUI_IN_PLUGIN_STATE
    auto guiTree = processorState.getGuiTree();
    if (guiTree.isValid())
        setConfigTree (guiTree);
#endif // FOLEYS_SAVE_EDITED_GUI_IN_PLUGIN_STATE

    updateSize();

#if FOLEYS_SHOW_GUI_EDITOR_PALLETTE
    if (!guiTree.isValid() && processorState.getValueTree().isValid())
        processorState.getValueTree().addChild (builder->getConfigTree(), -1, nullptr);

    builder->attachToolboxToWindow (*this);
#endif
    
    addAndMakeVisible (pluginVersionLabel);
    pluginVersionLabel.setFont (juce::Font (10.0, juce::Font::bold));
    pluginVersionLabel.setComponentID("pluginVersionLabel");
    pluginVersionLabel.setEditable(false);
    pluginVersionLabel.setColour (juce::Label::textColourId, juce::Colours::white);
    pluginVersionLabel.setJustificationType (juce::Justification::centredRight);
    pluginVersionLabel.setText(PLUGIN_VERSION, juce::dontSendNotification);
    
    
    addAndMakeVisible (hostLabel);
    hostLabel.setFont (juce::Font (20.0, juce::Font::bold));
    hostLabel.setComponentID("hostLabel");
    hostLabel.setEditable(true);
    
    hostLabel.setColour (juce::Label::textColourId, juce::Colours::lightgreen);
    hostLabel.setJustificationType (juce::Justification::centredRight);
    hostLabel.addListener(this);
    
    addAndMakeVisible (mainIDLabel);
    mainIDLabel.setComponentID("mainIDLabel");
    mainIDLabel.setFont (juce::Font (20.0, juce::Font::bold));
    mainIDLabel.setEditable(true);
    
    mainIDLabel.setColour (juce::Label::textColourId, juce::Colours::lightblue);
    mainIDLabel.setJustificationType (juce::Justification::centredRight);
    mainIDLabel.addListener(this);
    
    updateOscLabelsTexts(false);
}

MagicPluginEditor::~MagicPluginEditor()
{
#if JUCE_MODULE_AVAILABLE_juce_opengl && FOLEYS_ENABLE_OPEN_GL_CONTEXT
    oglContext.detach();
#endif
}

void MagicPluginEditor::updateOscLabelsTexts(bool sendNotification) {
    juce::String hostAddress = DEFAULT_OSC_HOST;
    processorState.getLastHostAddress(hostAddress);
    
    juce::String mainId = DEFAULT_OSC_MAIN_ID;
    processorState.getLastMaindId(mainId);

    auto doSend = sendNotification ? juce::sendNotification : juce::dontSendNotification;
    mainIDLabel.setText (mainId, doSend);
    hostLabel.setText (hostAddress, doSend);
}

void MagicPluginEditor::labelTextChanged (juce::Label* labelThatHasChanged) {
    if (labelThatHasChanged->getComponentID() == "hostLabel") {
        processorState.setOscIPAdress(labelThatHasChanged->getText());
    } else if (labelThatHasChanged->getComponentID() == "mainIDLabel") {
        processorState.setOscMainID(labelThatHasChanged->getText());
    }
}

void MagicPluginEditor::updateSize()
{
    const auto rootNode = builder->getGuiRootNode();

    int width = rootNode.getProperty (IDs::width, 770); // Initial default width
    int height = rootNode.getProperty (IDs::height, 560); // Initial default height

    bool resizable = builder->getStyleProperty (IDs::resizable, builder->getGuiRootNode());
    bool resizeCorner = builder->getStyleProperty (IDs::resizeCorner, builder->getGuiRootNode());

#if JUCE_IOS
    resizable = false;
    resizeCorner = false;
#endif

    if (resizable)
    {
        processorState.getLastEditorSize (width, height);

        auto maximalBounds = juce::Desktop::getInstance().getDisplays().getTotalBounds (true);
        int minWidth = rootNode.getProperty (IDs::minWidth, 10);
        int minHeight = rootNode.getProperty (IDs::minHeight, 10);
        int maxWidth = rootNode.getProperty (IDs::maxWidth, maximalBounds.getWidth());
        int maxHeight = rootNode.getProperty (IDs::maxHeight, maximalBounds.getHeight());
        setResizable (resizable, resizeCorner);
        setResizeLimits (minWidth, minHeight, maxWidth, maxHeight);
    }

    setSize (width, height);
}

void MagicPluginEditor::setConfigTree (const juce::ValueTree& gui)
{
    // Set default values
    auto rootNode = gui.getChildWithName (IDs::view);
    auto& undo = builder->getUndoManager();
    if (! rootNode.hasProperty (IDs::resizable)) rootNode.setProperty (IDs::resizable, true, &undo);
    if (! rootNode.hasProperty (IDs::resizeCorner)) rootNode.setProperty (IDs::resizeCorner, true, &undo);

    processorState.setGuiValueTree (gui);
    builder->createGUI (*this);

    updateSize();
}

MagicGUIBuilder& MagicPluginEditor::getGUIBuilder()
{
    // This shouldn't be possible, since the builder instance is created if none was supplied...
    jassert (builder);

    return *builder;
}

void MagicPluginEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);
}

void MagicPluginEditor::resized()
{
    builder->updateLayout();

    processorState.setLastEditorSize (getWidth(), getHeight());
    
    int portCompWidth = 200;
    int labelWidth = 200;
    hostLabel.setBounds (getWidth() - portCompWidth - labelWidth - 10,  getHeight() - 30, labelWidth,  30);
    mainIDLabel.setBounds (10,  getHeight() - 30, 100,  30);
    pluginVersionLabel.setBounds (portCompWidth,  getHeight() - 20, 100,  30);

}

} // namespace foleys
