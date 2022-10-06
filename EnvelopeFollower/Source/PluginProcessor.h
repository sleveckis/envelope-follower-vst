/*
  ==============================================================================

    PluginProcessor.h
    Created: 15 Feb 2022 10:52pm PST
    Last Updated: 5 Mar 2022 9:00pm PST
    Authors:
    - Arden Butterfield
    - Patrick Thomasma
    - Zacree Carroll
    - Ethan Killen (documentation)

    Description: Contains the API definitions for the EnvelopeFollowerAudioProcessor, EnvelopeVisualizer, and AudioInVislualizer component classes.
    Dependencies:
    - JuceHeader.h
    - SignalProcessor.h

  ==============================================================================
*/

#pragma once // Ensure that this header file is included at most once. Duplicate includes are ignored.

// Import external headers:
#include <JuceHeader.h> // Import the JUCE dependencies required to run this plugin.
#include "SignalProcessor.h" // Import the interface definition for the signal processor component.


// Are we sending OSC messages?
#define SEND_OSC 0 // No.
// Are we sending midi messages?
#define SEND_MIDI 1 // Yes.

/**
 * The HUD element responsible for displaying a rolling graph of the produced envelope.
 * 
 * Methods
 * -------
 * public EnvelopeVisualizer(): Prepares the buffer used to hold the waveform data to be visualized and sets the colorscheme of the visualizer.
 * 
 * Inherits
 * - juce::AudioVisualizerComponent
 * - juce::SettableTooltipClient
 * 
 * Owned by
 * - EnvelopeFollowerAudioProcessor
 */
class EnvelopeVisualiser : public juce::AudioVisualiserComponent, public juce::SettableTooltipClient
{
public:
    /**
     * The constructor for the EnvelopeVisualizer.
     * 
     * Sets the size of the buffer used to store the displayed envelope. (512 samples with the current implementation)
     * Sets the colors used to display the envelope. (Black background, green envelope with the current implementation)
     */
    EnvelopeVisualiser();
};

/**
 * The HUD element responsible for displaying a rolling graph of the input waveform.
 * 
 * Methods
 * -------
 * public AudioInVisualizer(): Prepares the buffer used to hold the waveform data to be visualized and sets the colorscheme of the visualizer.
 * 
 * Inherits
 * - juce::AudioVisualizerComponent
 * - juce::SettableTooltipClient
 * 
 * Owned by
 * - EnvelopeFollowerAudioProcessor
 */
class AudioInVisualiser : public juce::AudioVisualiserComponent, public juce::SettableTooltipClient
{
public:
    /**
     * The constructor for the AudioInVisualiser.
     * 
     * Sets the size of the buffer used to store the displayed waveform. (512 samples with the current implementation)
     * Sets the colors used to display the waveform. (Black background, green waveform with the current implementation)
     */
    AudioInVisualiser();
};

/**
 * The EnvelopeFollowerAudioProcessor component for this plugin.
 * 
 * Manages DAW I/O, prompting the GUI to update when relevant parameters change, and routing input and output data through the processing pipeline.
 * 
 * Attributes
 * ----------
 * public juce::AudioParameterFloat* gain_user_param: A user-manager parameter corresponding to a scaling factor applied to input audio samples prior to proccessing.
 * public juce::AudioParameterFloat* min_pos_user_param: A user-managed parameter corresponding to the minimum output MIDI value.
 * public juce::AudioParameterFloat* max_pos_user_param: A user-manager parameter corresponding to the maximum output MIDI value.
 * public juce::AudioParameterFloat* low_pass_user_param: A user-manager parameter corresponding to the maximum considered input audio frequency.
 * public juce::AudioParameterFloat* hi_pass_user_param: A user-manager parameter corresponding to the minimum considered input audio frequency.
 * public juce::AudioParameterFloat* recovery_user_param: A user-manager parameter corresponding to the length of time required for the output envelope waveform to decay to half its value given sufficiently small input values.
 * public EnvelopeVisualiser EnvVisualiser: A GUI element responsible for visualizing the output envelope waveform.
 * public AudioInVisualiser AudioVisualiser: A GUI element responsible for visualizing the input audio waveform.
 * public std::String midi_info: A text string corresponding to the current last output MIDI value, channel, and type. Accessed for display by the GUI.
 * private float midi_message_rate: The rate at which MIDI messages are produced by this plugin.
 * private int samples_per_midi_message: The number of audio samples that must be processed per output MIDI message.
 * private int elapsed_since_midi: The number of samples processed since the last output MIDI message.
 * private int elapsed_since_drawer: The number of samples processed since the last GUI update.
 * private SignalProcessor signalProcessor: The sole component of the audio stream to MIDI stream pipeline. 
 * private int midi_channel: The MIDI channel this plugin outputs MIDI messages on.
 * private int midi_controller_type: The type of MIDI messages this plugin produces.
 * private int midi_value: The most recently output MIDI value.
 * private std::unique_ptr<juce::MidiOutput> output_device: The juce framework component used to output MIDI messages.
 * 
 * 
 * Methods
 * -------
 * public EnvelopeFollowerAudioProcessor(): The constructor for this component. Constructs the user-controller parameters for this plugin and register them for garbage collection at program termination.
 * public void prepareToPlay (double sampleRate, int samplesPerBlock): Handles resetting accumulators, 
 * public void releaseResources(): 
 * public bool isBusesLayoutSupported (const BusesLayout& layouts): Checks whether a given arrangement of input and output buses can be processed by this plugin.
 * public void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&): Processes a block of input audio data into MIDI messages.
 * public juce::AudioProcessorEditor* createEditor(): Creates the GUI management component for this plugin.
 * public bool hasEditor(): Returns whether this plugin should have a GUI.
 * public const juce::String getName(): Returns the name of this plugin.s
 * public double getTailLengthSeconds(): Returns the amount of time, in seconds, that this plugin will continue producing output after it stops receiving input data. (Always 0 for this implementation)
 * public int getNumPrograms(): Gets the number of programs that this plugin hosts. Returns 1 as, though this program does not support plugins, returning 0 can break some DAWs.
 * public int getCurrentProgram(): Would get the index of the currently running program for this plugin. Always returns 0 as this plugin does not support programs..
 * public void setCurrentProgram(int index): Would set the currently running program for this plugin by index. Does nothing as this plugin does not support programs.
 * public const juce::String getProgramName(int index): Gets the name of a program hosted by this plugin. Always returns an empty string as this plugin does not support programs.
 * public void changeProgramName(int index, const juce::String& newName): Sets the name of a program hosted by this plugin by index. Does nothing as this plugin does not support programs.
 * public bool acceptsMidi(): Returns whether this plugin accepts MIDI input. Returns False as this program does not handle MIDI input.
 * public bool producesMidi(): Returns whether this plugin produces MIDI output. Returns True as this program does produce MIDI output.
 * public bool isMidiEffect(): Returns whether this plugin does not process the audio buffer. Returns False as this program does parse the input audio buffers.
 * public int getMidiChannel(): Gets the MIDI channel this plugin outputs on.
 * public void setMidiChannel(int new_channel): Sets the MIDI channel this plugin outputs on.
 * public int getMidiType(): Gets the type of MIDI messages this plugin outputs.
 * public void setMidiType(int new_type): Sets the type of MIDI messages this plugin outputs.
 * public void getStateInformation(juce::MemoryBlock& destData): Writes the visible parameters of this plugin to memory as binary encoded XML.
 * public void setStateInformation(const void* data, int sizeInBytes): Loads and updates the visible parameters of this plugin from binary encoded XML stored in memory.
 * public void updateMathParams(): Updates the parameters of the SignalProcessor component.
 * public void sendCCMessage(int sample_number): Post an output MIDI message to the network interface.
 * 
 * Inherits:
 * - juce::AudioProcessor
 * 
 * Owns
 * - SignalProcessor
 * - EnvelopeFollowerAudioProcessorEditor
 * - EnvelopeVisualizer
 * - AudioInVisualizer
 */
class EnvelopeFollowerAudioProcessor  : public juce::AudioProcessor
{
public:
    // The user parameters: numbers corresponding to each of the knobs

    /// <summary>
    ///     The user manage parameter which controls the offset between the output MIDI values and the input audio data.
    /// </summary>
    juce::AudioParameterFloat* gain_user_param; // dB
    /// <summary>
    ///     The user manage parameter which controls the minimum value of the output MIDI messages.
    /// </summary>
    juce::AudioParameterFloat* min_pos_user_param; // percent
    /// <summary>
    ///     The user manage parameter which controls the maximum value of the output MIDI messages.
    /// </summary>
    juce::AudioParameterFloat* max_pos_user_param; // percent
    /// <summary>
    ///     The user manage parameter which controls the maximum input audio frequency that this plugin consideres when generating the envelope.
    /// </summary>
    juce::AudioParameterFloat* low_pass_user_param; // Hz
    /// <summary>
    ///     The user manage parameter which controls the minimum input audio frequency that this plugin consideres when generating the envelope.
    /// </summary>
    juce::AudioParameterFloat* hi_pass_user_param; // Hz
    /// <summary>
    ///     The user manage parameter which controls how long it takes for the produced envelope to decay.
    /// </summary>
    juce::AudioParameterFloat* recovery_user_param; // unitless


    // GUI

    // These need to be public because the editor needs to be able to draw it.
    /// <summary>
    ///     The GUI element that displays the envelope this plugin has produced.
    /// 
    ///     Disposed of when this component is disposed of.
    /// </summary>
    EnvelopeVisualiser EnvVisualiser;
    /// <summary>
    ///     The GUI element that displays the audio waveform this plugin has processed.
    /// 
    ///     Disposed of when this component is disposed of.
    /// </summary>
    AudioInVisualiser AudioVisualiser;
    /// <summary>
    ///     A string outlining the current MIDI output channel, message type, and value.
    ///     Periodically updated as the processor processes an input buffer.
    ///     Displayed on the GUI.
    /// 
    ///     Disposed of when this component is disposed of.
    /// </summary>
    std::string midi_info;


    /**
     * The constructor for the EnvelopeFollowerAudioProcessor component.
     * 
     * Constructs the end user managed parameters for the processor and registers them for garbage collection.
     */
    EnvelopeFollowerAudioProcessor();
    ~EnvelopeFollowerAudioProcessor() override;


    // Primary functionality of the EnvelopeFollowerAudioProcessor:

    /**
     * Used to handle expensive memory allocations, select parameter updates, and accumulator/buffer resets before the DAW starts using this plugin to process audio data.
     * 
     * Runs before EnvelopeFollowerAudioProcessor::processBlock is used for a set of audio input by the DAW.
     * Relays updates for the expected sample frequency to the audio pipeline component(s) and the envelope and waveform displays.
     * Clears rolling buffer for the envelope and waveform displays used by the GUI.
     * Regenerates the MIDI output component this uses.
     * 
     * Arguments
     * ---------
     * double sampleRate: The expected frequency of audio samples we expect to receive each time processBlock is called once this has executed. (audip samples/second)
     * int samplesPerBlock: The number of audio samples we expect to receive in each audio buffer every time processBlock is called once this has executed.
     */
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;

    /**
     * Currently a NOOP (No-operation) awaiting further expansion.  
     * 
     * Runs after EnvelopeFollowerAudioProcessor::processBlock is used for a set of audio input by the DAW.
     * Intended to be used to release memory allocated for processing a series of audio blocks.
     * Currently does nothing as we do not allocate memory in prepareToPlay()
     * If that changes this will release that memory.
     */
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations

    /**
     * Checks whether a given arrangement of input, output and throughput audio and MIDI buffers can be processed by this plugin.
     * 
     * Only returns true if the input layout is mono (one buffer) or stereo (two buffers) and the output layout matches the input layout as that is all this plugin supports.
     * All other inputs return false.
     * 
     * Arguments
     * ---------
     * const BusesLayout& layouts: The buffer arrangement to check for viability.
     * 
     * Returns
     * -------
     * bool: True if the buffer arrangement can be processed, False otherwise.
     */
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    /**
     * Takes a set of audio sample buffers and converts them into a series of MIDI messages posted to the network interface.
     * 
     * Essentially the main loop for this VST3 plugin.
     * Will be called by the mounting DAW repeatedly during audio playback.
     * EnvelopeFollowerAudioProcessor::prepareToPlay will always be called before a series of calls to this by the DAW.
     * EnvelopeFollowerAudioProcessor::releaseResources will always be called after a series of calls to this by the DAW.
     * 
     * Arguments
     * ---------
     * juce::AudioBuffer<float>&: The set of audio sample buffers used for DAW IO.
     * juce::MidiBuffer&: The set of MIDI message buffers used for DAW IO. 
     */
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;


    // GUI management:

    /**
     * Creates and returns a reference to the GUI management component for this plugin.
     * 
     * Used to create the EnvelopeFollowerAudioProcessorEditor component for this plugin.
     * 
     * Returns
     * -------
     * juce::AudioProcessorEditor*: A pointer to the newly created GUI management component.
     */
    juce::AudioProcessorEditor* createEditor() override;

    /**
     * Returns whether or not this plugin should have a GUI.
     * 
     * Always returns True for this implementation as we want a GUI.
     * 
     * Returns
     * -------
     * bool: True if this plugin should have a GUI, False otherwise.
     */
    bool hasEditor() const override;

    /**
     * Returns the name of this VST3 plugin.
     * 
     * Returns
     * -------
     * juce::String: The name of this VST3 plugin.
     */
    const juce::String getName() const override;

    /**
     * Returns the amount of time relative to output data that this plugin will continue producing output after input has ceased.
     * 
     * Always returns 0.0 for this implementation.
     * This plugin should stop producing output immediately once the input has ceased.
     * 
     * Returns
     * -------
     * double: The amount of time relative to output data that this plugin will continue producing output after input has ceased.
     */
    double getTailLengthSeconds() const override;


    // Plugin program handling: (effectively unimplemented)

    /**
     * Returns the number of programs this plugin wants the DAW to think we have.
     * 
     * Always returns 1 because though programs are not implemented for this plugin many DAWs can break if a plugin reports that it doesn't have any.
     * 
     * Returns
     * -------
     * int: The number of programs this plugin wants the DAW to think we have. (Always 1 for this implementation)
     */
    int getNumPrograms() override;

    /**
     * Gets the index of the currently active program for this plugin.
     * 
     * Always returns 0 because programs are not implemented for this plugin
     * 
     * Returns
     * -------
     * int: The index of the currently active program for this plugin. (Always 0 for this implementation)
     */
    int getCurrentProgram() override;

    /**
     * Sets the currently actived program for this plugin.
     * 
     * Is a NOOP (No-operation) because programs are not implemented for this plugin.
     * 
     * Arguments
     * ---------
     * int index: The index of the program that should become the active program.
     */
    void setCurrentProgram (int index) override;

    /**
     * Returns the name of the program associated with the given index.
     * 
     * Always returns an empty string as programs are not implemented for this plugin.
     * 
     * Arguments
     * ---------
     * int index: The index of the program we should return the name of.
     * 
     * Returns
     * -------
     * juce::String: The name of the program at the given index. (Always an empty string for this imlementation)
     */
    const juce::String getProgramName (int index) override;

    /**
     * Changes the name of one of this plugins programs.
     * 
     * Is a NOOP (No-operation) because programs are not implemented for this plugin.
     * 
     * Arguments
     * ---------
     * int index: The index of the program to rename.
     * const juce::String& newName: The name that the indexed program should be renamed to.
     */
    void changeProgramName (int index, const juce::String& newName) override;
    

    // MIDI

    /**
     * Returns whether or not this plugin accepts MIDI input.
     *
     * Returns False as this plugin does not accept MIDI input.
     *
     * Returns
     * -------
     * bool: True if this plugin accepts MIDI input, False otherwise. (Always False for this implementation)
     */
    bool acceptsMidi() const override;

    /**
     * Returns whether or not this plugin produces MIDI output.
     *
     * Returns True as this plugin produces MIDI output.
     *
     * Returns
     * -------
     * bool: True if this plugin produces MIDI output, False otherwise. (Always True for this implementation)
     */
    bool producesMidi() const override;

    /**
     * Returns whether or not this plugin only affects MIDI and does no audio processing.
     * 
     * Returns False as this plugin does process the audio input.
     * 
     * Returns
     * -------
     * bool: True if this plugin only affects the MIDI buffers and does not process the audio buffers. (Always False for this implementation)
     */
    bool isMidiEffect() const override;

    /**
     * Returns the index of the MIDI channel used by this plugin.
     * 
     * Returns
     * -------
     * int: The index of the MIDI channel used by this plugin.
     */
    int getMidiChannel();

    /**
     * Sets the index of the MIDI channel used by this plugin.
     *
     * Arguments
     * ---------
     * int new_channel: The new index of the MIDI channel to be used by this plugin.
     */
    void setMidiChannel(int new_channel);

    /**
     * Gets the type of MIDI message this processor is producing.
     * 
     * Returns
     * -------
     * int: The type of MIDI message this processor is producing.
     */
    int getMidiType();

    /**
     * Sets the type of MIDI message this processor is producing.
     * 
     * Arguments
     * ---------
     * int new_type:    The type of MIDI message this processor will produce.
     *                  See https://www.paulcecchettimusic.com/full-list-of-midi-cc-numbers/ for more detail.
     */
    void setMidiType(int new_type);

    /**
     * Compiles and outputs all user-visible parameters to binary encoded XML.
     * 
     * Arguments
     * ---------
     * juce::MemoryBlock& destData: The section of memory to write the compiled data to.
     */
    void getStateInformation (juce::MemoryBlock& destData) override;

    /**
     * Updates all user-visible parameters from a block of binary encoded XML.
     * 
     * Can update
     * - EnvelopeFollowerAudioProcessor::gain_user_param from the XML tag "gain"
     * - EnvelopeFollowerAudioProcessor::min_pos_user_param from the XML tag "min"
     * - EnvelopeFollowerAudioProcessor::max_pos_user_param from the XML tag "max"
     * - EnvelopeFollowerAudioProcessor::low_pass_user_param from the XML tag "lo"
     * - EnvelopeFollowerAudioProcessor::hi_pass_user_param from the XML tag "hi"
     * - EnvelopeFollowerAudioProcessor::recovery_user_param from the XML tag "recovery"
     * - EnvelopeFollowerAudioProcessor::midi_channel from the XML tag "channel"
     * - EnvelopeFollowerAudioProcessor::midi_controller_type from the XML tag "type"
     * 
     * Arguments
     * ---------
     * const void* data: A pointer to what should be a block of binary encoded XML to read.
     * int sizeInBytes: The size of the memory block to read.
     */
    void setStateInformation (const void* data, int sizeInBytes) override;
    
private:
    /// <summary>
    ///     The number of MIDI messages that we should output per second.
    /// </summary>
    float midi_message_rate;
    /// <summary>
    ///     The number of audio samples that should be processed to produce each MIDI output message.
    /// </summary>
    int samples_per_midi_message;
    /// <summary>
    ///     The number of samples that have been processed since the last MIDI output message was produced.
    /// </summary>
    int elapsed_since_midi;
    /// <summary>
    ///     The number of samples that have been processed since the last GUI update.
    /// </summary>
    int elapsed_since_drawer;
    
    /// <summary>
    ///     The sole component of the audio processing pipeline used by this processor.
    /// </summary>
    SignalProcessor signalProcessor;

    /// <summary>
    ///     Which MIDI channel this processor is outputting on.
    /// </summary>
    int midi_channel = 1;

    /// <summary>
    ///     What type of MIDI message this processor produces.
    /// 
    ///     Defaults to one from the undefined bank.
    ///     See https://www.paulcecchettimusic.com/full-list-of-midi-cc-numbers/ for details.
    /// </summary>
    int midi_controller_type = 14;

    /// <summary>
    ///     The current value of the MIDI message to output.
    /// </summary>
    int midi_value = 0;

    /// <summary>
    ///     The component responsible for relaying produced midi blocks to the hardware's network ports for external use.
    /// </summary>
    std::unique_ptr<juce::MidiOutput> output_device;
    
    /// The maximum  number of plugins that can be running on a computer at once. Note that each plugin will have its own midi device.
    const int MAX_INSTANCES = 512;
    
    /**
     * Updates the parameters of the SignalProcessor component from locally held values.
     * 
     * Responsible for updating:
     * - SignalProcessor::min_val
     * - SignalProcessor::max_val
     * - SignalProcessor::gain
     * - SignalProcessor::lowFilter::cutoff_frequency
     * - SignalProcessor::highFilter::cutoff_frequency
     * - SignalProcessor::recovery_time and SignalProcessor::decay
     */
    void updateMathParams();

    /**
     * Posts a produced MIDI message to one of the hardware's output ports.
     * 
     * Arguments
     * ---------
     * int sample_number: The index of the audio sample that prompted the message to be produced. (unused)
     */
    void sendCCMessage(int sample_number);
    
    /**
     * makes a new MIDI output device, deleting the old one if one exists already. Called by prepareToPlay()
     */
    void makeMIDIOutputDevice();

    // A macro that prevents memory leaks and by-value copying of this component from the JUCE framework.
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EnvelopeFollowerAudioProcessor)
};
