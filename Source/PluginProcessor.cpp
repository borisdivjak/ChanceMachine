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

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <random>

std::string ChanceMachineAudioProcessor::extracted(int i) {
    auto display_name = "Chance " + std::to_string(i+1);
    return display_name;
}

//==============================================================================
ChanceMachineAudioProcessor::ChanceMachineAudioProcessor()
     :  AudioProcessor (BusesProperties()
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                       ),
        state (*this, nullptr),
        midiSelect("MIDI Out", *this)


{

    // FIRST ROW PARAMETERS ----------------------------------------------------
    // assume 16 parameters (for 16 chance sliders)
    int num_params = 16;
    
    for (int i=0; i<num_params; i++) {
        auto name = "chance" + std::to_string(i);
        std::string display_name = extracted(i);
        auto attributes = juce::AudioParameterFloatAttributes().withStringFromValueFunction (
                        [] (auto x, auto) {
                            return juce::String (static_cast<int>(x * 100));
                        }).withLabel ("%");
        state.createAndAddParameter({ std::make_unique<juce::AudioParameterFloat> (juce::ParameterID(name, 1+i),  display_name, juce::NormalisableRange<float> (0.0f, 1.0f), 1.0f, attributes)});
        
    }


    // SECOND ROW PARAMETERS ----------------------------------------------------

    for (int i=0; i<num_params; i++) {
        auto name = "condition" + std::to_string(i);
        auto display_name = "Trig " + std::to_string(i+1);
        state.createAndAddParameter(
                    std::make_unique<juce::AudioParameterChoice> (juce::ParameterID(name, num_params+1),
                    display_name, condition_options, 0));
        
    }

    
    // THIRD ROW PARAMETERS ----------------------------------------------------

    state.createAndAddParameter(
            std::make_unique<juce::AudioParameterChoice> (juce::ParameterID("stepLength", 40),
            "Step Length", stepLength_options, stepLength_options.indexOf("1 / 16") ));
    
    state.createAndAddParameter(
            std::make_unique<juce::AudioParameterChoice> (juce::ParameterID("reset", 41),
            "Reset", reset_options, reset_options.indexOf("16")));

    state.createAndAddParameter(
            std::make_unique<juce::AudioParameterChoice> (juce::ParameterID("sendOut", 42),
            "Send Out", juce::StringArray {"Fwd host note", "CC", "CC inverted"}, 0));

    juce::StringArray CCOptions;
    for (auto i=0; i<=127; i++) {
        CCOptions.add("CC " + std::to_string(i));
    }
    state.createAndAddParameter(
            std::make_unique<juce::AudioParameterChoice> (juce::ParameterID("CC", 43),
            "CC", CCOptions, 0));


    midiSelect.updateDeviceList();

    juce::StringArray midiOptions({"To Host"});
    for (auto midiOut : midiSelect.midiOutputs) {
        midiOptions.add(midiOut->deviceInfo.name);
    }
    
    state.createAndAddParameter(
            std::make_unique<juce::AudioParameterChoice> (juce::ParameterID("midiSelect", 44),
            "Midi Out", midiOptions, 0));


    juce::StringArray channelOptions;
    for (auto i=1; i<=16; i++) {
        channelOptions.add(std::to_string(i));
    }
    state.createAndAddParameter(
            std::make_unique<juce::AudioParameterChoice> (juce::ParameterID("channel", 43),
            "Channel", channelOptions, 0));


    // END PARAMTER SETUP --------------------------------------------
    
    
    
    midiSelectAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(state, "midiSelect", midiSelect);

    state.state = juce::ValueTree("ChancePlugin");
    state.state.setProperty ("version", "0.2i", nullptr);

    // store the saved MIDI interface
    state.state.setProperty ("savedMIDIId", "", nullptr);
    initialised = true;
}

ChanceMachineAudioProcessor::~ChanceMachineAudioProcessor()
{
}

//==============================================================================

const juce::String ChanceMachineAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ChanceMachineAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool ChanceMachineAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool ChanceMachineAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double ChanceMachineAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ChanceMachineAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int ChanceMachineAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ChanceMachineAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String ChanceMachineAudioProcessor::getProgramName (int index)
{
    return {};
}

void ChanceMachineAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}


//==============================================================================


void ChanceMachineAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void ChanceMachineAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ChanceMachineAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif


//==============================================================================
//======================      PROCESS BLOCK      ===============================
//==============================================================================


void ChanceMachineAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist100(0,99); // distribution in range [0, 99]

    float chance = 100; // start with a 100 - 100% chance
    float midi_time = 0;
    int reset = 16; // return to start after this amount of steps
    float qnotes_per_bar = 4;  // how many quarter notes per bar - assume 4
    float note_unit = 4; // i.e. 4 sixteenth notes per quarter note; use 2 for eight note etc.
    float bpm = 120.0f; // assumed bpm – we'll read this from host if available
    bool step_changed = false;
    int channel = 1;
    
    // get values from state if available
    auto stepLength_param = state.getParameter ("stepLength");
    if (stepLength_param != nullptr) {
        auto stepLength = stepLength_values.find( stepLength_param->getCurrentValueAsText() ) -> second;
        note_unit = static_cast<float>(stepLength) / 4;
    }

    auto reset_param = state.getParameter ("reset");
    if (reset_param != nullptr) {
        reset = std::stoi(reset_param->getCurrentValueAsText().toStdString());
    }

    auto channel_param = state.getParameter ("channel");
    if (channel_param != nullptr) {
        channel = std::stoi(channel_param ->getCurrentValueAsText().toStdString());
    }

    
    juce::MidiBuffer processedMidi;

    // get real playhead position / time from host, if available
    juce::AudioPlayHead *playHead = getPlayHead();
    if(playHead != NULL) {
        if (auto pos = playHead->getPosition()) {
            if (auto real_time = pos->getPpqPosition()) {
                midi_time = *real_time;
            }
            if (auto signature = pos->getTimeSignature()) {
                qnotes_per_bar = static_cast<float>(signature->numerator) / static_cast<float>(signature->denominator) * 4;
            }
            if (auto b = pos->getBpm()) {
                bpm = static_cast<float>(*b);
            }
        }
    }
    
    // figure out which step we're on
    // assume a safe number for host latency (in seconds)
    auto latency = 0.015f;
    auto bps = bpm / 60.0f;
    int steps_total = static_cast<int>((midi_time + (latency * bps)) * note_unit);
    int step = steps_total % reset;
    int cycle = steps_total / reset;

    // on step change
    if (steps_total != previous_steps) {
        currentStep = step;
        previous_steps = steps_total;
        step_changed = true;
        
        // read chance from appropriate slider
        auto chance_param = state.getParameter ("chance" + std::to_string(step));
        if (chance_param != nullptr) {
            chance = chance_param ->getValue() * 100;
        }
        
        // read trigger condition parameter
        juce::String condition = "1:1";
        auto condition_param = state.getParameter ("condition" + std::to_string(step));
        if (condition_param != nullptr) {
            condition = condition_param ->getCurrentValueAsText();
        }
        juce::StringArray c;
        c.addTokens (condition, ":", "");

        // set step on or off state, depending the condition parameter
        // so only turn on every A out of B cycles (c[0] = A; c[1] = B)
        int current_cycle = cycle % std::stoi(c[1].toStdString()) + 1;
        if (current_cycle == std::stoi(c[0].toStdString()) ) {
            step_on = true;
        }
        else {
            step_on = false;
        }

        // set step to off if required, depending on the chance setting
        float rnd = dist100(rng);
        if (chance <= rnd) step_on = false;

        sendChangeMessage ();
    }
    
    // check what kind of message we want to send (notes or CC)
    auto sendOutParam = state.getParameter ("sendOut");
    int sendOut = static_cast<int>(sendOutParam ->convertFrom0to1(sendOutParam ->getValue()));

    // process midi message when available
    // only process notes if selected option is to forward incoming midi notes
    if (sendOut == 0) {
        for (const auto metadata : midiMessages)
        {
            auto message = metadata.getMessage();
            auto time = metadata.samplePosition;
            
            // only pass through note on messsage according to chance setting (step_on),
            // but let other messages through normally
            if ((message.isNoteOn() && step_on) || message.isNoteOn()==false) {
                // set the chosen output channel
                message.setChannel(channel);

                // send to selected external MIDI outputs
                midiSelect.sendToMidiOutputs (message);

                // add to host's MIDI buffer
                processedMidi.addEvent (message, time);
            }
        }
        midiMessages.swapWith (processedMidi);
    }
    
    // if we're sending out CC
    if (sendOut > 0 && step_changed) {
        auto CCParam = state.getParameter ("CC");
        int CC = static_cast<int>(CCParam ->convertFrom0to1(CCParam ->getValue()));
        auto value = 0; // default value is set to off
        
        // if send CC (127 for on)
        if (sendOut == 1) {
            if (step_on) value = 127;
        }
        // if send inverted CC (127 for off)
        else {
            if (!step_on) value = 127;
        }

        auto message = juce::MidiMessage::controllerEvent (channel, CC, value);
        
        // send to selected external MIDI outputs
        midiSelect.sendToMidiOutputs (message);
        
        // add to host's MIDI buffer
        midiMessages.addEvent(message, 0);
    }
    
}


//==============================================================================
//==============================================================================
//==============================================================================


bool ChanceMachineAudioProcessor::hasEditor() const

{
    return true; // (change this to false if you choose to not supply an editor)
}


//==============================================================================


juce::AudioProcessorEditor* ChanceMachineAudioProcessor::createEditor()

{
    auto editor = new ChanceMachineAudioProcessorEditor (*this);
    return editor;
}

//==============================================================================


void ChanceMachineAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.

    // make sure we save the midi out interface setting
    state.state.setProperty("savedMIDIId", midiSelect.midiId, nullptr);

    // Store an xml representation of our state.
    if (auto xmlState = state.copyState().createXml())
        copyXmlToBinary (*xmlState, destData);

}

void ChanceMachineAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.

    // Restore our plug-in's state from the xml representation stored in the above
    // method.
    if (auto xmlState = getXmlFromBinary (data, sizeInBytes)) {
        auto newState = juce::ValueTree::fromXml (*xmlState);
        if (newState.hasProperty("version")) {
            // only replace if it's the same version of the tree (i.e. has same properties)
            if (newState.getProperty("version") == state.state.getProperty("version")) {
                state.replaceState (newState);

                // if a midi out was previously open, open it now
                midiSelect.midiId = state.state.getProperty("savedMIDIId").toString();
                midiSelect.updateDeviceList(true);
            }
        }
    }

}


//==============================================================================
// This creates new instances of the plugin..


juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ChanceMachineAudioProcessor();
}



