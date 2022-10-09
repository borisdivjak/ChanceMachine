# Chance Machine

Chance Machine is a 16-step probability sequencer plugin for Mac (VST and AU). It was developed as an add on for Maschine 2, allowing users to add randomisation and variation, while keeping patterns short and easy to edit.

<img width="827" alt="Chance Machine plugin" src="https://user-images.githubusercontent.com/5494990/194737452-4e4a30c7-b965-42d9-a296-6c60e0388bac.png">

Key features:
* Set probability for each step independently
* Conditional triggering of steps, for example once every 8 bars
* Variable step length and pattern length
* Send out steps as MIDI notes or as MIDI CC values
* Select any MIDI output — useful for hosts like Maschine that don’t receive MIDI directly from plugins
* Convenient parameter mapping that works well on Maschine MK3 devices

## What you’ll need:
* Maschine 2 (or another VST or AU host)
* Mac OS 10.11 or greater

## Installation

Download the [latest release for MacOS](https://github.com/borisdivjak/ChanceMachine/raw/v1.0.0/Release/ChanceMachine-1.0.0.pkg) and double click the .pkg file to start the installation. The installer should copy the VST and AU versions of the plugin into the respective folders:
* /Library/Audio/Plug-Ins/VST
* /Library/Audio/Plug-Ins/Components

If you ever wish to uninstall the plugin, simply delete the ChanceMachine.vst and ChanceMachine.component files from those folders.

When you start Maschine (or any other DAW) you should now be able to see the Chance Machine plugin along with other plugins in the folder ‘Boris’.

## Set up

You can use Chance Machine in a number of different ways. If you are planning on using it with software drum kits or instruments, you will need to set up a virtual MIDI device. You can do this in the Audio MIDI Setup app on your Mac. Read more about it [here](https://support.apple.com/en-au/guide/audio-midi-setup/ams1013/mac).

Make sure you add the virtual MIDI device as one of the chosen MIDI input devices in your DAW preferences.

The examples below are for users of Maschine 2 software, but similar setup should work within other DAWs.

### Using Chance Machine with external MIDI instruments in Maschine 2

* Add Chance Machine as the first plugin on the ‘sound’ (rather than ‘group’) that you will use to control your external MIDI instrument.
* Open the plugin and set the MIDI output (and channel) to the output that corresponds to your external instrument.
* Set ‘Message to send’ to ‘Forward host note’.

### Using Chance Machine with a sample based drum kit in Maschine 2
* Load a sample based Maschine kit into a new group (e.g. the ‘808 Kit’ from the Maschine library)
* Select the ‘sound’ that you would like to apply Chance Machine to (e.g. ‘ClosedHH 808’)
* Add the Chance Machine plugin and open it
* Set MIDI output to your virtual MIDI device
* Set ‘Message to send’ to ‘CC inverted’
* Select the CC you’d like to use – you should select a different CC for each new ‘sound’ you want to use the plugin with (if you’re unsure which CC to select, numbers 70-80 are a pretty safe choice)
* Go to the ‘Sampler’ instrument settings and go to the ‘Pitch/Envelope’ page
* Use the MIDI learn function on the ‘start’\* control knob (you might need to press ‘play’ in Maschine for this to work properly)
* You should now see your selected CC number listed under that control and you can repeat this process for other sounds in your drum kit

\* Note: the ‘start’ knob controls the starting point of a sample, so turning this knob all the way to the right effectively turns the sound off. You could achieve a similar effect by controlling the ‘attack’ knob, as a really long attack means you won’t hear much of the drum sound.

A similar process should also work with other internal instruments, like Battery 4 or Kontakt instruments.

## Using the sequencer

Once you’ve set up the plugin, using the sequencer should be pretty straightforward. One thing to keep in mind is that Chance Machine doesn’t trigger any sounds on its own. You need to play/record the notes in Maschine first, or select the steps in Maschine’s step sequencer. Chance Machine then controls the probability of notes actually triggering the sound.

### Probability sliders
The 16 sliders control the probability of sounds getting triggered for each step. For example, if you set all sliders to 50%, you should now hear only about half of the notes. If you set all sliders to 0%, you shouldn’t hear anything. If you set the first slider to 100% and all other sliders to 0%, you should only hear notes on the first beat of each bar.

### Trigger conditions
Use this feature to trigger sounds every Nth cycle of the pattern (e.g. only every second bar). By selecting 1:8, for example, you can trigger a step on the first repetition every 8 cycles. Setting it to 8:8 triggers the step on the last repetition of the 8 cycles. This can be useful, for example, to add a cymbal hit every 8 bars, or a tom fill every few bars.

### Step length and reset
Changing these controls allows you to adjust the length of the steps and the pattern. Changing the length of the pattern, in particular, can result in some interesting polymetric patterns, as this is not linked to the length of the pattern in Maschine itself.  

## Limitations

* This plugin will not work as expected when exporting the song or its parts via the ‘Export Audio’ command
* The plugin settings don’t change when you change the pattern or create a new pattern in Maschine, so creating multiple patterns with desired results can be tricky


