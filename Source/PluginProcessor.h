/*
  ==============================================================================

    Chance Machine

    A simple probability sequencer plugin for Mac (VST and AU). This plugin
    was built with the intention of adding probability capabilities to
    the Maschine 2 sequencer, but it should work just as well in other plugin hosts.

    https://github.com/borisdivjak/ChanceMachine

    Author:  Boris Divjak

    Built with JUCE.
    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

class ChanceMachineAudioProcessor;

#include <JuceHeader.h>
#include "MIDIOutSelector.h"

//==============================================================================
/**
*/
class ChanceMachineAudioProcessor  :    public juce::AudioProcessor,
                                    public juce::ChangeBroadcaster

{
public:
    std::string extracted(int i);
    
//==============================================================================
    ChanceMachineAudioProcessor();
    ~ChanceMachineAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    float noteOnVel;

    juce::AudioProcessorValueTreeState state;
    bool initialised = false;
    
    MIDIOutSelector midiSelect;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> midiSelectAttach;

    std::string statusMessage; // used for debugging
    
    const juce::StringArray condition_options =
        { "1:1", "1:2", "2:2", "1:4", "2:4", "3:4", "4:4",
          "1:8", "2:8", "3:8", "4:8", "5:8", "6:8", "7:8", "8:8",
          "1:16", "2:16", "3:16", "4:16", "5:16", "6:16", "7:16", "8:16",
          "9:16", "10:16", "11:16", "12:16", "13:16", "14:16", "15:16", "16:16"};

    const juce::StringArray stepLength_options =
        { "1 Bar", "1 / 2", "1 / 4", "1 / 8", "1 / 16" };
    
    const std::map<juce::String, int> stepLength_values =
        { {"1 Bar", 1}, {"1 / 2", 2}, {"1 / 4", 4}, {"1 / 8", 8}, {"1 / 16", 16} };

    const juce::StringArray reset_options =
        { "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16"};

    int currentStep = 0;
    
    

private:
    //==============================================================================

    int previous_steps = 0;
    bool step_on = true;

    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChanceMachineAudioProcessor)
};
