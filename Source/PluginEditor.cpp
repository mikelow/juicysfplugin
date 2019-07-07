/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "GuiConstants.h"

//==============================================================================
JuicySFAudioProcessorEditor::JuicySFAudioProcessorEditor(
    JuicySFAudioProcessor& p,
    AudioProcessorValueTreeState& valueTreeState)
: AudioProcessorEditor{&p}
, processor{p}
, valueTreeState{valueTreeState}
    //   sharedParams{p.getSharedParams()},
, midiKeyboard{p.keyboardState, SurjectiveMidiKeyboardComponent::horizontalKeyboard}
, tablesComponent{valueTreeState, p.getFluidSynthModel()}
, filePicker{valueTreeState, p.getFluidSynthModel()}
, slidersComponent{valueTreeState, p.getFluidSynthModel()}
{
    // set resize limits for this plug-in
    setResizeLimits(
        GuiConstants::minWidth,
        GuiConstants::minHeight,
        GuiConstants::maxWidth,
        GuiConstants::maxHeight);
    
    // int width, height;
    // {
    //     RangedAudioParameter *param {valueTreeState.getParameter("uiWidth")};
    //     jassert(dynamic_cast<AudioParameterInt*> (param) != nullptr);
    //     AudioParameterInt* castParam {dynamic_cast<AudioParameterInt*> (param)};
    //     width = castParam->get();
    // }
    // {
    //     RangedAudioParameter *param {valueTreeState.getParameter("uiHeight")};
    //     jassert(dynamic_cast<AudioParameterInt*> (param) != nullptr);
    //     AudioParameterInt* castParam {dynamic_cast<AudioParameterInt*> (param)};
    //     height = castParam->get();
    // }

    // valueTreeState.addParameterListener("uiWidthPersist", this);
    // valueTreeState.addParameterListener("uiHeightPersist", this);
    // valueTreeState.addParameterListener("uiWidth", this);
    // valueTreeState.addParameterListener("uiHeight", this);

    valueTreeState.state.addListener(this);

    setSize(GuiConstants::minWidth, GuiConstants::minHeight);

//    processor.subscribeToStateChanges(this);

    midiKeyboard.setName ("MIDI Keyboard");

    midiKeyboard.setWantsKeyboardFocus(false);
    tablesComponent.setWantsKeyboardFocus(false);

    setWantsKeyboardFocus(true);
    addAndMakeVisible (midiKeyboard);

    addAndMakeVisible(slidersComponent);
    addAndMakeVisible(tablesComponent);
    addAndMakeVisible(filePicker);

}

JuicySFAudioProcessorEditor::~JuicySFAudioProcessorEditor()
{
    // valueTreeState.removeParameterListener("uiWidthPersist", this);
    // valueTreeState.removeParameterListener("uiHeightPersist", this);
    // valueTreeState.removeParameterListener("uiWidth", this);
    // valueTreeState.removeParameterListener("uiHeight", this);
    valueTreeState.state.removeListener(this);
//    processor.unsubscribeFromStateChanges(this);
}

void JuicySFAudioProcessorEditor::valueTreePropertyChanged(ValueTree& treeWhosePropertyHasChanged,
                                               const Identifier& property) {
    if (&treeWhosePropertyHasChanged == &valueTreeState.state) {
        if (property == Identifier("uiWidth")) {
            // String soundFontPath{treeWhosePropertyHasChanged.getProperty("soundFontPath", "")};
            // if (soundFontPath.isNotEmpty()) {
            //     loadFont(soundFontPath);
            // }
            int value{treeWhosePropertyHasChanged.getProperty("uiWidth", GuiConstants::minWidth)};
            setSize(value, getHeight());
        } else if (property == Identifier("uiHeight")) {
            int value{treeWhosePropertyHasChanged.getProperty("uiHeight", GuiConstants::minHeight)};
            setSize(getWidth(), value);
        }
    }
}

// void JuicySFAudioProcessorEditor::parameterChanged(const String& parameterID, float newValue) {
//     // if (parameterID == "uiWidthPersist"
//     // || parameterID == "uiHeightPersist") {
//     if (parameterID == "uiWidth"
//     || parameterID == "uiHeight") {
//         RangedAudioParameter *param {valueTreeState.getParameter(parameterID)};
//         jassert(dynamic_cast<AudioParameterInt*> (param) != nullptr);
//         AudioParameterInt* castParam {dynamic_cast<AudioParameterInt*> (param)};
//         int value{castParam->get()};
//         // if (parameterID == "uiWidthPersist") {
//         //     setSize(value, getHeight());
//         // } else if (parameterID == "uiHeightPersist") {
//         //     setSize(getWidth(), value);
//         // }
//         if (parameterID == "uiWidth") {
//             setSize(value, getHeight());
//         } else if (parameterID == "uiHeight") {
//             setSize(getWidth(), value);
//         }
//     }
// }

//void JuicySFAudioProcessorEditor::getStateInformation (XmlElement& xml) {
//    // save
//    xml.setAttribute ("uiWidth", getWidth());
//    xml.setAttribute ("uiHeight", getHeight());
//}
//
//void JuicySFAudioProcessorEditor::setStateInformation (XmlElement* xmlState) {
//    // load
//    setSize (xmlState->getIntAttribute ("uiWidth", getWidth()),
//            xmlState->getIntAttribute ("uiHeight", getHeight()));
//}

//==============================================================================
void JuicySFAudioProcessorEditor::paint (Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));

//    g.setColour (Colours::white);
//    g.setFont (15.0f);
//    g.drawFittedText ("Hello World!", getLocalBounds(), Justification::centred, 1);

    if (!focusInitialized) {
        if (!hasKeyboardFocus(false) && isVisible()) {
            grabKeyboardFocus();
        }
        if (getCurrentlyFocusedComponent() == this) {
            focusInitialized = true;
        }
    }
}

void JuicySFAudioProcessorEditor::resized()
{
    const int padding{8};
    const int pianoHeight{70};
    const int filePickerHeight{25};
    // const int slidersHeight{150};
    Rectangle<int> r{getLocalBounds()};
    filePicker.setBounds(r.removeFromTop(filePickerHeight + padding).reduced(padding, 0).withTrimmedTop(padding));

    // Rectangle<int> r2 (getLocalBounds());
    // slidersComponent.setBounds(r2.removeFromLeft(filePickerWidth + padding).reduced(padding, 0).withTrimmedLeft(padding));

    midiKeyboard.setBounds (r.removeFromBottom (pianoHeight).reduced(padding, 0));

    Rectangle<int> rContent{r.reduced(0, padding)};
    slidersComponent.setBounds(rContent.removeFromRight(slidersComponent.getDesiredWidth() + padding).withTrimmedRight(padding));

    tablesComponent.setBounds(rContent);

    valueTreeState.state.setPropertyExcludingListener(this, "uiWidth", getWidth(), nullptr);
    valueTreeState.state.setPropertyExcludingListener(this, "uiHeight", getHeight(), nullptr);
    
    // {
    //     RangedAudioParameter *param {valueTreeState.getParameter("uiWidth2")};
    //     jassert(dynamic_cast<AudioParameterInt*> (param) != nullptr);
    //     AudioParameterInt* castParam {dynamic_cast<AudioParameterInt*> (param)};
    //     *castParam = getWidth();
    // }
    // {
    //     RangedAudioParameter *param {valueTreeState.getParameter("uiHeight2")};
    //     jassert(dynamic_cast<AudioParameterInt*> (param) != nullptr);
    //     AudioParameterInt* castParam {dynamic_cast<AudioParameterInt*> (param)};
    //     *castParam = getHeight();
    // }

//    sharedParams.setUiWidth(getWidth());
//    sharedParams.setUiHeight(getHeight());

//    Rectangle<int> r2 (getLocalBounds());
//    r2.reduce(0, padding);
//    r2.removeFromBottom(pianoHeight);
//    r2.removeFromTop(filePickerHeight);
//    tablesComponent.setBounds (r2);
//
//    Rectangle<int> r3 (getLocalBounds());
//    r3.removeFromTop(filePickerHeight);
//
//    filePicker.setBounds(r3);
}

bool JuicySFAudioProcessorEditor::keyPressed(const KeyPress &key) {
//    if (!hasKeyboardFocus(false))
//        return false;
//    if (key.getKeyCode() == KeyPress::upKey){
//    }
//    cout << "hey\n";
    const int cursorKeys[] = {
            KeyPress::leftKey,
            KeyPress::rightKey,
            KeyPress::upKey,
            KeyPress::downKey
    };
    if (any_of(
            begin(cursorKeys),
            end(cursorKeys),
            [&](int i) { return i == key.getKeyCode(); }
    )) {
        return tablesComponent.keyPressed(key);
    } else {
        return midiKeyboard.keyPressed(key);
    }
//    for(auto childComponent : getChildren()) {
//        if (childComponent->keyPressed(key)) return true;
//    }
    return false;
}

bool JuicySFAudioProcessorEditor::keyStateChanged (bool isKeyDown) {
    return midiKeyboard.keyStateChanged(isKeyDown);
//    for(auto childComponent : getChildren()) {
//        if (childComponent->keyStateChanged(isKeyDown)) return true;
//    }
//    return false;
}

//FilePickerFragment& JuicySFAudioProcessorEditor::getFilePicker() {
//    return filePicker;
//}
//
//SlidersFragment& JuicySFAudioProcessorEditor::getSliders() {
//    return slidersComponent;
//}
