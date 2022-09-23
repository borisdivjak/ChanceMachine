/*
  ==============================================================================

    MIDIOutSelector.cpp
    Created: 4 Jun 2022 9:02:42pm
    Author:  Boris Divjak

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "MIDIOutSelector.h"
#include "PluginEditor.h"


MIDIOutSelector::MIDIOutSelector(
    const juce::String & name,
    ChanceMachineAudioProcessor& p) :
    audioProcessor(p),
    statusMessage(p.statusMessage)

{
    startTimer (500);
    onChange = [this] { selectionChanged(); };
    timerCallback();
}

MIDIOutSelector::~MIDIOutSelector()

{
    stopTimer();
    midiOutputs.clear();
}


//==============================================================================


void MIDIOutSelector::openDevice (int index)
{
    if (midiOutputs[index]->outDevice.get() == nullptr) {
        midiOutputs[index]->outDevice = juce::MidiOutput::openDevice (midiOutputs[index]->deviceInfo.identifier);
    }
            
    if (midiOutputs[index]->outDevice.get() != nullptr) {
        midiOutputs[index]->outDevice->startBackgroundThread();
    }
    else {
        DBG ("MidiDemo::openDevice: open output device for index = " << index << " failed!");
    }
}

//==============================================================================


void MIDIOutSelector::closeDevice (int index)
{
        jassert (midiOutputs[index]->outDevice.get() != nullptr);
        midiOutputs[index]->outDevice->stopBackgroundThread();
        midiOutputs[index]->outDevice.reset();
}


//==============================================================================


void MIDIOutSelector::closeAllDevices ()
{
    for (auto i=0; i<midiOutputs.size(); i++) {
        if (midiOutputs[i]->outDevice.get() != nullptr) closeDevice(i);
    }
}


//==============================================================================


bool MIDIOutSelector::hasDeviceListChanged ()
{
    auto availableDevices = juce::MidiOutput::getAvailableDevices();

    if (availableDevices.size() != midiOutputs.size())
        return true;

    for (auto i = 0; i < availableDevices.size(); ++i)
        if (availableDevices[i] != midiOutputs[i]->deviceInfo)
            return true;

    return false;
}


//==============================================================================


void MIDIOutSelector::updateDeviceList (bool force)
{
    int selectedDevice = -1;

    if (hasDeviceListChanged () || midiOutputs.size() == 0 || force)
    {
        auto availableDevices = juce::MidiOutput::getAvailableDevices();
        closeUnpluggedDevices (availableDevices);
        closeAllDevices();

        juce::ReferenceCountedArray<MidiDeviceListEntry> newDeviceList;

        // add all currently plugged-in devices to the device list
        for (auto& newDevice : availableDevices)
        {
            MidiDeviceListEntry::Ptr entry = findDevice (newDevice);

            if (entry == nullptr)
                entry = new MidiDeviceListEntry (newDevice);

            // add device to the midiouts list - this includes the unique identifier
            newDeviceList.add (entry);
                
            // check state to see if device should be selected and opened
            if (audioProcessor.initialised) {
                if (midiId == newDevice.identifier) {
                    selectedDevice = newDeviceList.size() - 1;
                }
            }
        }
        
        midiOutputs = newDeviceList;

        // open the device that was supposed to be open
        if (selectedDevice > -1) openDevice (selectedDevice);

        // update the dropdown
        updateMidiDropdown();
    }

}


//==============================================================================


void MIDIOutSelector::selectionChanged ()
{
    auto newSelectedItem = getSelectedId();
    juce::String identifier = "";
    juce::String name = "";

    closeAllDevices();

    if (newSelectedItem > 1) {
        openDevice(newSelectedItem - 2);
        name = midiOutputs[newSelectedItem - 2]->deviceInfo.name;
        identifier = midiOutputs[newSelectedItem - 2]->deviceInfo.identifier;
    }
        
    midiId = identifier;
}

//==============================================================================


void MIDIOutSelector::updateMidiDropdown ()
{
    juce::ReferenceCountedArray<MidiDeviceListEntry>& midiDevices = midiOutputs;

    // clear the combo box and add first item (no midi out)
    clear();
    addItem("None", 1); // reserve ID = 1 for 'None'
    auto select = 1;

    for (auto midiDevice : midiDevices)
    {
        // add midi device to ComboBox dropdown list
        addItem(midiDevice->deviceInfo.name, getNumItems() + 1);

        if (midiDevice->outDevice.get() != nullptr) {
            select = getNumItems();
        }
    }

    setSelectedId(select);
}
//==============================================================================


void MIDIOutSelector::closeUnpluggedDevices (const juce::Array<juce::MidiDeviceInfo>& currentlyPluggedInDevices)
{
    for (auto i = midiOutputs.size(); --i >= 0;)
    {
        auto& d = *midiOutputs[i];

        if (! currentlyPluggedInDevices.contains (d.deviceInfo))
        {
            if (d.outDevice.get() != nullptr) closeDevice (i);
            midiOutputs.remove (i);
        }
    }
}


//==============================================================================


juce::ReferenceCountedObjectPtr<MidiDeviceListEntry> MIDIOutSelector::findDevice (juce::MidiDeviceInfo device) const
{
    for (auto& d : midiOutputs)
        if (d->deviceInfo == device)
            return d;

    return nullptr;
}


//==============================================================================


void MIDIOutSelector::sendToMidiOutputs (const juce::MidiMessage& msg)

{
    for (auto midiOutput : midiOutputs)
        if (midiOutput->outDevice.get() != nullptr) {
            jassert(midiOutput->outDevice->isBackgroundThreadRunning());
            midiOutput->outDevice->sendBlockOfMessages(
                juce::MidiBuffer(msg),
                juce::Time::getMillisecondCounter(),
                1000 );
        }
}


//==============================================================================


std::unique_ptr<juce::AudioParameterFloat> MIDIOutSelector::createParameter ()
{
    updateDeviceList();

    auto margin = 0.5f / (midiOutputs.size() + 1);
    auto attributes = juce::AudioParameterFloatAttributes().withStringFromValueFunction (
                [&] (auto x, auto) {
                    auto range = midiOutputs.size() + 1;
                    auto choice = static_cast<int>(x * range);
                    if (choice == 0) return juce::String("None");
                    else return juce::String (midiOutputs[choice-1]->deviceInfo.name);
                });

    auto range = juce::NormalisableRange<float> (0.0f + margin, 1.0f - margin, nullptr, nullptr,
                // snapToLegalValueFunc
                [&] (auto s, auto e, float value) {
                    auto range = midiOutputs.size() + 1;
                    auto margin = 0.5f / range;
                    auto v = static_cast<int>(value * range);
                    return (static_cast<float>(v) / range) + margin;
                });

    return std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID("midiSelect", 1000), "Midi Out", range, 0.0f, attributes);
    
}


//==============================================================================


void MIDIOutSelector::timerCallback()
{
    if (hasDeviceListChanged() || getNumItems() == 0) {
        updateDeviceList();
    }
}


