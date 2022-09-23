/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "MIDIOutSelector.h"
#include "PluginProcessor.h"

//==============================================================================
/**
*/


class ComboBoxSmallerFont  :  public juce::LookAndFeel_V4
{
    juce::Font getComboBoxFont (juce::ComboBox& box) override;
    void drawComboBox (juce::Graphics& g, int width, int height, bool,
                  int, int, int, int, juce::ComboBox& box) override;
    void positionComboBoxText (juce::ComboBox& box, juce::Label& label) override;
};



class ChanceMachineAudioProcessorEditor  :  public juce::AudioProcessorEditor,
                                        public juce::ChangeListener,
                                        private juce::Value::Listener,
                                        private juce::Timer

{
public:
    ChanceMachineAudioProcessorEditor (ChanceMachineAudioProcessor&);
    ~ChanceMachineAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
private:
    void changeListenerCallback (juce::ChangeBroadcaster *source) override;
    void timerCallback() override;
    void valueChanged (juce::Value&) override;
    void addLabelAndSetStyle (juce::Label& label);

    
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    ChanceMachineAudioProcessor& audioProcessor;
    juce::AudioProcessorValueTreeState& state;


    // first row components
    juce::Label chanceLabel       { "Chance Label", "Probability per step:" };
    juce::OwnedArray<juce::Slider> stepChances;
    juce::OwnedArray<juce::AudioProcessorValueTreeState::SliderAttachment> stepChancesAttach;

    
    // second row components
    juce::Label conditionsLabel       { "Conditions Label", "Trigger conditions (every A out of B cycles):" };
    juce::OwnedArray<juce::ComboBox> stepConditions;
    juce::OwnedArray<juce::AudioProcessorValueTreeState::ComboBoxAttachment> stepConditionsAttach;


    // third row components
    juce::Label stepLengthLabel   { "Step Length Label", "Step length:" };
    juce::ComboBox stepLengthSelect;
    juce::Label resetLabel   { "Reset Label", "Reset after:" };
    juce::ComboBox resetSelect;

    juce::Label sendOutLabel   { "Send Label", "Message to send:" };
    juce::ComboBox sendOutSelect;

    juce::Label CCLabel   { "CC Label", "CC:" };
    juce::ComboBox CCSelect;

    juce::Label midiOutputLabel   { "Midi Output Label", "MIDI Output:" };
    MIDIOutSelector& midiSelect;

    juce::Label channelLabel   { "Channel Label", "Ch:" };
    juce::ComboBox channelSelect;

    juce::Label statusLabel         { "Test Label", "" };

    
    juce::OwnedArray<juce::AudioProcessorValueTreeState::ComboBoxAttachment> comboAttachments;

    
    juce::LookAndFeel_V4 basicLook;
    juce::LookAndFeel_V4 highlightedLook;
    ComboBoxSmallerFont comboBoxSmallerFont;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChanceMachineAudioProcessorEditor)
};

