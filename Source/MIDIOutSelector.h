/*
  ==============================================================================

    MIDIOutSelector.h
    Created: 4 Jun 2022 9:02:31pm
    Author:  Boris Divjak

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================

struct MidiDeviceListEntry : juce::ReferenceCountedObject
{
    MidiDeviceListEntry (juce::MidiDeviceInfo info) : deviceInfo (info) {}

    juce::MidiDeviceInfo deviceInfo;
    std::unique_ptr<juce::MidiOutput> outDevice;

    using Ptr = juce::ReferenceCountedObjectPtr<MidiDeviceListEntry>;
};


//==============================================================================


class MIDIOutSelector : public juce::ComboBox,
                        private juce::Timer

{
public:
    MIDIOutSelector(const juce::String & name, ChanceMachineAudioProcessor& p);
    ~MIDIOutSelector();

    void openDevice (int index);
    void closeDevice (int index);
    void closeAllDevices ();
    void updateDeviceList (bool force = false);
    bool hasDeviceListChanged ();

    void closeUnpluggedDevices (const juce::Array<juce::MidiDeviceInfo>& currentlyPluggedInDevices);
    juce::ReferenceCountedObjectPtr<MidiDeviceListEntry> findDevice (juce::MidiDeviceInfo device) const;
    void sendToMidiOutputs (const juce::MidiMessage& msg);

    void selectionChanged();
    void updateMidiDropdown ();

    
    std::unique_ptr<juce::AudioParameterFloat> createParameter();

    juce::ReferenceCountedArray<MidiDeviceListEntry> midiOutputs;
    juce::String midiId = "";

private:
    void timerCallback() override;
    ChanceMachineAudioProcessor& audioProcessor;
    std::string& statusMessage; // used for debugging
};





