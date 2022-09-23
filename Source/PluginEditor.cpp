/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "MIDIOutSelector.h"


//==============================================================================


ChanceMachineAudioProcessorEditor::ChanceMachineAudioProcessorEditor (ChanceMachineAudioProcessor& p)
: AudioProcessorEditor (&p), audioProcessor (p),
state (p.state),
midiSelect (p.midiSelect)

{
    // set the colour scheme for highlights
    auto colors =  highlightedLook.getCurrentColourScheme ();
    colors.setUIColour(juce::LookAndFeel_V4::ColourScheme::UIColour::outline, juce::Colours::white);
    highlightedLook.setColourScheme(colors);
    
    
    // FIRST ROW ----------
    
    // create 16 sliders
    int num_sliders = 16;
    
    for (int i=0; i<num_sliders; i++) {
        juce::Slider * stepChance = new juce::Slider;
        stepChance->setSliderStyle(juce::Slider::LinearBarVertical);
        stepChance->setTextBoxStyle(juce::Slider::NoTextBox, false, 90, 0);
        addAndMakeVisible (*stepChance);
        stepChances.add(stepChance);
        
        juce::AudioProcessorValueTreeState::SliderAttachment * scAttach =
            new juce::AudioProcessorValueTreeState::SliderAttachment(p.state, "chance" + std::to_string(i), *stepChance);
        stepChancesAttach.add(scAttach);
    }

    addLabelAndSetStyle(chanceLabel);

    
    // SECOND ROW ----------

    addLabelAndSetStyle (conditionsLabel);

    for (int i=0; i<num_sliders; i++) {
        juce::ComboBox * stepCondition = new juce::ComboBox;
        stepCondition->addItemList(audioProcessor.condition_options, 1);
        addAndMakeVisible (*stepCondition);
        stepConditions.add(stepCondition);
        stepCondition->setLookAndFeel(&comboBoxSmallerFont);
        
        juce::AudioProcessorValueTreeState::ComboBoxAttachment * condAttach =
            new juce::AudioProcessorValueTreeState::ComboBoxAttachment(p.state, "condition" + std::to_string(i), *stepCondition);
        stepConditionsAttach.add(condAttach);
    }


    // THIRD ROW ----------
    
    // create the combobox to select step length
    addLabelAndSetStyle (stepLengthLabel);

    stepLengthSelect.addItemList(audioProcessor.stepLength_options, 1);
    addAndMakeVisible (stepLengthSelect);
    auto stepLengthAttach = new juce::AudioProcessorValueTreeState::ComboBoxAttachment(p.state, "stepLength", stepLengthSelect);
    comboAttachments.add(stepLengthAttach);
    stepLengthSelect.setLookAndFeel(&comboBoxSmallerFont);

    
    // create the combobox to select when to reset
    addLabelAndSetStyle (resetLabel);

    resetSelect.addItemList(audioProcessor.reset_options, 1);
    addAndMakeVisible (resetSelect);
    resetSelect.setLookAndFeel(&comboBoxSmallerFont);

    auto resetAttach = new juce::AudioProcessorValueTreeState::ComboBoxAttachment(p.state, "reset", resetSelect);
    comboAttachments.add(resetAttach);

    // create the combobox to select what kind of message to send to midi out
    addLabelAndSetStyle (sendOutLabel);

    sendOutSelect.addItem("Forward host note", 1);
    sendOutSelect.addItem("CC", 2);
    sendOutSelect.addItem("CC inverted", 3);
    sendOutSelect.setSelectedId(1);
    addAndMakeVisible (sendOutSelect);
    sendOutSelect.setLookAndFeel(&comboBoxSmallerFont);

    auto sendOutAttach = new juce::AudioProcessorValueTreeState::ComboBoxAttachment(p.state, "sendOut", sendOutSelect);
    comboAttachments.add(sendOutAttach);

    
    // create the combobox to select which CC to send (when sending CC)
    addLabelAndSetStyle (CCLabel);

    for (auto i=0; i<=127; i++) {
        CCSelect.addItem(std::to_string(i), i+1);
    }
    CCSelect.setSelectedId(1);
    addAndMakeVisible (CCSelect);
    CCSelect.setLookAndFeel(&comboBoxSmallerFont);
    
    auto CCAttach = new juce::AudioProcessorValueTreeState::ComboBoxAttachment(p.state, "CC", CCSelect);
    comboAttachments.add(CCAttach);
    
    
    // disable CC ComboBox when not relevant
    sendOutSelect.onChange = [this] {
        auto i = sendOutSelect.getSelectedId();
        if (i>1) {
            CCLabel.setEnabled(true);
            CCSelect.setEnabled(true);
        }
        else {
            CCLabel.setEnabled(false);
            CCSelect.setEnabled(false);
        }
    };
    sendOutSelect.onChange();
    
    
    // create the combobox to select MIDI output to use
    addLabelAndSetStyle (midiOutputLabel);
    addAndMakeVisible (midiSelect);
    midiSelect.setLookAndFeel(&comboBoxSmallerFont);

    
    // create the combobox to select MIDI channel to use
    addLabelAndSetStyle (channelLabel);

    for (auto i=1; i<=16; i++) {
        channelSelect.addItem(std::to_string(i), i+1);
    }
    addAndMakeVisible (channelSelect);
    channelSelect.setLookAndFeel(&comboBoxSmallerFont);
    
    auto channelAttach = new juce::AudioProcessorValueTreeState::ComboBoxAttachment(p.state, "channel", channelSelect);
    comboAttachments.add(channelAttach);
    

    
    // for debugging info
    addLabelAndSetStyle (statusLabel);


    
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (830, 370);

    // start refresh timer
    startTimer (100);

    // register to receive step change notifications
    audioProcessor.addChangeListener(this);
    
    timerCallback();
}

ChanceMachineAudioProcessorEditor::~ChanceMachineAudioProcessorEditor()
{
    stopTimer();
    audioProcessor.removeChangeListener(this);

    int num_sliders = 16;
    
    for (int i=0; i<num_sliders; i++) {
        stepChances[i]->setLookAndFeel(nullptr);
        stepConditions[i]->setLookAndFeel(nullptr);
    }

    stepLengthSelect.setLookAndFeel(nullptr);
    resetSelect.setLookAndFeel(nullptr);
    sendOutSelect.setLookAndFeel(nullptr);
    CCSelect.setLookAndFeel(nullptr);
    midiSelect.setLookAndFeel(nullptr);
    channelSelect.setLookAndFeel(nullptr);
    
    midiSelect.setVisible(false);
}


void ChanceMachineAudioProcessorEditor::changeListenerCallback(juce::ChangeBroadcaster *source)
{
    int step = audioProcessor.currentStep;
    int num_sliders = 16;
    
    for (int i=0; i<num_sliders; i++) {
        stepChances[i]->setLookAndFeel(nullptr);
        if (i == step) {
            stepChances[i]->setLookAndFeel(&highlightedLook);
        }
    }

}

void ChanceMachineAudioProcessorEditor::timerCallback()
{
    statusLabel.setText(audioProcessor.statusMessage, juce::dontSendNotification);
}




//==============================================================================


void ChanceMachineAudioProcessorEditor::paint (juce::Graphics& g)

{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
//    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void ChanceMachineAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    
    
    auto margin = 10;
    auto margin_out = 20;
    auto margin_v = 30;
    auto num_cols = 16;
    auto col = (getWidth() - (2 * margin_out) - ((num_cols - 1) * margin)) / num_cols;
    auto num_rows = 8;
    auto row = (getHeight() - (2 * margin_out) - ((num_rows - 1) * margin_v)) / num_rows;

    auto first_row_height = row * 4 + 3 * margin_v;
    auto second_row_height = row * 2 + 1 * margin_v;
    auto third_row_height = row * 2 + 1 * margin_v;
    auto second_row_y = first_row_height + margin_out + margin_v;
    auto third_row_y = first_row_height + second_row_height + margin_out + 2*margin_v;

    
    // FIRST ROW ---------------------------------------------
    
    chanceLabel.setBounds (     margin_out,                     // x
                                margin_out,                     // y
                                col * 6 + margin * 5,           // width
                                20 );                           // height
    
    for (int i=0; i<stepChances.size(); i++) {
        stepChances.getUnchecked(i)->setBounds (margin_out + i*col + i*margin,  // x
                                                margin_out + 30,                // y
                                                col,                            // width
                                                first_row_height - 30);    // height
    }

    
    // SECOND ROW ---------------------------------------------

    conditionsLabel.setBounds ( margin_out,                     // x
                                second_row_y,                   // y
                                col * 8 + margin * 7,           // width
                                20 );                           // height


    for (int i=0; i<stepConditions.size(); i++) {
        stepConditions.getUnchecked(i)->setBounds (margin_out + i*col + i*margin,  // x
                                                   second_row_y + 28,              // y
                                                   col,                            // width
                                                   24);                            // height
    }


    
    // THIRD ROW ---------------------------------------------
    
    stepLengthLabel.setBounds ( margin_out,                     // x
                                third_row_y,                   // y
                                col * 3 + margin * 2,           // width
                                20 );                           // height

    stepLengthSelect.setBounds (    margin_out,
                                    third_row_y + 28,
                                    col * 3 + margin * 2,
                                    24 );


    
    resetLabel.setBounds (      margin_out + col*3 + margin*3,  // x
                                third_row_y,                   // y
                                col * 3 + margin * 2,           // width
                                20 );                           // height

    resetSelect.setBounds  (    margin_out + col*3 + margin*3,
                                third_row_y + 28,
                                col * 3 + margin * 2,
                                24 );

    sendOutLabel.setBounds (    margin_out + col*8 + margin*8,  // x
                                third_row_y,                   // y
                                col * 4 + margin * 3,           // width
                                20 );                           // height

    sendOutSelect.setBounds  (  margin_out + col*8 + margin*8,
                                third_row_y + 28,
                                col * 3 + margin * 2,
                                24 );

    CCLabel.setBounds (         margin_out + col*11 + margin*11,  // x
                                third_row_y,                   // y
                                col,           // width
                                20 );                           // height

    CCSelect.setBounds  (       margin_out + col*11 + margin*11,
                                third_row_y + 28,
                                col,
                                24 );

    midiOutputLabel.setBounds ( margin_out + col*12 + margin*12,  // x
                                third_row_y,                   // y
                                col * 3 + margin * 2,           // width
                                20 );                           // height

    midiSelect.setBounds  (     margin_out + col*12 + margin*12,
                                third_row_y + 28,
                                col * 3 + margin * 2,
                                24 );

    channelLabel.setBounds (    margin_out + col*15 + margin*15,  // x
                                third_row_y,                   // y
                                col,           // width
                                20 );                           // height

    channelSelect.setBounds  (  margin_out + col*15 + margin*15,
                                third_row_y + 28,
                                col,
                                24 );



    statusLabel.setBounds (       margin_out,
                                getHeight() - 2*margin,
                                col*16 + margin*15,
                                20);


}


//==============================================================================


void ChanceMachineAudioProcessorEditor::addLabelAndSetStyle (juce::Label& label)

{
    label.setBorderSize (juce::BorderSize<int>(0));
    label.setFont (juce::Font (14.00f, juce::Font::plain));
    label.setJustificationType (juce::Justification::centredLeft);
    label.setEditable (false, false, false);
    label.setColour (juce::TextEditor::textColourId, juce::Colours::black);
    label.setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

    addAndMakeVisible (label);
}

//==============================================================================



void ChanceMachineAudioProcessorEditor::valueChanged (juce::Value&)
{
}





juce::Font ComboBoxSmallerFont::getComboBoxFont  (juce::ComboBox& box)

{
//    return { juce::jmin (14.0f, (float) box.getHeight() * 0.85f) };
    return 14.0f;
}



void ComboBoxSmallerFont::drawComboBox (juce::Graphics& g, int width, int height, bool,
                                   int, int, int, int, juce::ComboBox& box)
{
    auto cornerSize = box.findParentComponentOfClass<juce::ChoicePropertyComponent>() != nullptr ? 0.0f : 3.0f;
    juce::Rectangle<int> boxBounds (0, 0, width, height);

    g.setColour (box.findColour (juce::ComboBox::backgroundColourId));
    g.fillRoundedRectangle (boxBounds.toFloat(), cornerSize);

    g.setColour (box.findColour (juce::ComboBox::outlineColourId));
    g.drawRoundedRectangle (boxBounds.toFloat().reduced (0.5f, 0.5f), cornerSize, 1.0f);

    juce::Rectangle<int> arrowZone (width - 15, 0, 12, height);
    juce::Path path;
    path.startNewSubPath ((float) arrowZone.getX() + 3.0f, (float) arrowZone.getCentreY() - 1.0f);
    path.lineTo ((float) arrowZone.getCentreX(), (float) arrowZone.getCentreY() + 1.5f);
    path.lineTo ((float) arrowZone.getRight() - 3.0f, (float) arrowZone.getCentreY() - 1.0f);

    g.setColour (box.findColour (juce::ComboBox::arrowColourId).withAlpha ((box.isEnabled() ? 0.9f : 0.2f)));
    g.strokePath (path, juce::PathStrokeType (2.0f));
}


void ComboBoxSmallerFont::positionComboBoxText (juce::ComboBox& box, juce::Label& label)
{
    label.setBounds (0, 1,
                     box.getWidth() - 9,
                     box.getHeight() - 2);

    label.setFont (getComboBoxFont (box));
}
