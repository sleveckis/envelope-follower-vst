/*
  ==============================================================================

    PluginProcessor.cpp
    Created: 15 Feb 2022 10:52pm PST
    Last Updated: 5 Mar 2022 9:00pm PST
    Authors:
    - Arden Butterfield
    - Patrick Thomasma
    - Zacree Carroll
    - Ethan Killen (documentation)

    Description: Contains the implementation of the EnvelopeFollowerAudioProcessor, EnvelopeVisualizer, and AudioInVisualizer component classes. 
    Dependencies:
    - PluginProcessor.h
    - PluginEditor.h
    - math.h

  ==============================================================================
*/

// Import external headers
#include "PluginProcessor.h" // Import the interface definition for the implementations in this file.
#include "PluginEditor.h" // Import the interface definition for the GUI manager component so it can be reference by the implementation.
#include <math.h> // Import the standard math library for usage in the implementation.


/*
 * Splitting the two visualisers into separate classes allows for more versatility in modifying them
 * The Idea here is that the audio in object will be "stacked" on top of the envelope follower data object
 * so when you look at the vst display that is what you are seeing.
 *
 */

/**
 * The constructor for the EnvelopeVisualizer.
 *
 * Sets the size of the buffer used to store the displayed envelope. (512 samples with the current implementation)
 * Sets the colors used to display the envelope. (Black background, green envelope with the current implementation)
 */
EnvelopeVisualiser::EnvelopeVisualiser() : AudioVisualiserComponent(1)
{
    setBufferSize(512); // Set the size of the buffer used to store the displayed envelope.
    setColours(juce::Colours::black, juce::Colours::green); // Set the colors used to display the envelope. (green on black)
}

/**
 * The constructor for the AudioInVisualiser.
 *
 * Sets the size of the buffer used to store the displayed waveform. (512 samples with the current implementation)
 * Sets the colors used to display the waveform. (Black background, green waveform with the current implementation)
 */
AudioInVisualiser::AudioInVisualiser() : AudioVisualiserComponent(1)
{
    setBufferSize(512); // Set the size of the buffer used to store the displayed waveform.
    setColours(juce::Colours::black, juce::Colours::red); // Set the colors used to display the waveform. (red on black)
}

/**
 * The constructor for the EnvelopeFollowerAudioProcessor component.
 *
 * Constructs the end user managed parameters for the processor and registers them for garbage collection.
 */
EnvelopeFollowerAudioProcessor::EnvelopeFollowerAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties() // Set desired DAW audio I/O buffer layout to stereo in stereo out..
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    // Consturct a managed value for each user managed parameter.
    gain_user_param = new juce::AudioParameterFloat("gain", "gain", juce::NormalisableRange<float> (-30.0, 30.0), 0.0);
    min_pos_user_param = new juce::AudioParameterFloat("min", "min", juce::NormalisableRange<float> (0.0, 100.0), 0.0);
    max_pos_user_param = new juce::AudioParameterFloat("max", "max", juce::NormalisableRange<float> (0.0, 100.0), 100.0);
    low_pass_user_param = new juce::AudioParameterFloat("low pass", "low pass", juce::NormalisableRange<float> (0.0, 20000.0), 20000.0);
    hi_pass_user_param = new juce::AudioParameterFloat("high pass", "high pass", juce::NormalisableRange<float> (0.0, 20000.0), 0.0);
    recovery_user_param = new juce::AudioParameterFloat("recovery time", "recovery time", juce::NormalisableRange<float>(0.0, 1.0), 0.0);

    // Register each user managed parameter to be deleted when this processor is deleted.
    addParameter(gain_user_param);
    addParameter(min_pos_user_param);
    addParameter(max_pos_user_param);
    addParameter(low_pass_user_param);
    addParameter(hi_pass_user_param);
    addParameter(recovery_user_param);

    // Set the number of MIDI messages output per second to ten.
    midi_message_rate = 10;
}

EnvelopeFollowerAudioProcessor::~EnvelopeFollowerAudioProcessor()
{
    
}

/**
 * Returns the name of this VST3 plugin.
 *
 * Returns
 * -------
 * juce::String: The name of this VST3 plugin.
 */
const juce::String EnvelopeFollowerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

/**
 * Returns whether or not this plugin accepts MIDI input.
 *
 * Returns False as this plugin does not accept MIDI input.
 *
 * Returns
 * -------
 * bool: True if this plugin accepts MIDI input, False otherwise. (Always False for this implementation)
 */
bool EnvelopeFollowerAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false; // Return false as this plugin does not consider input MIDI data.
   #endif
}

/**
 * Returns whether or not this plugin produces MIDI output.
 *
 * Returns True as this plugin produces MIDI output.
 *
 * Returns
 * -------
 * bool: True if this plugin produces MIDI output, False otherwise. (Always True for this implementation)
 */
bool EnvelopeFollowerAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true; // Return true as this plugin produces MIDI.
   #else
    return false;
   #endif
}

/**
 * Returns whether or not this plugin only affects MIDI and does no audio processing.
 *
 * Returns False as this plugin does process the audio input.
 *
 * Returns
 * -------
 * bool: True if this plugin only affects the MIDI buffers and does not process the audio buffers. (Always False for this implementation)
 */
bool EnvelopeFollowerAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false; // Return false as this plugin processes audio data.
   #endif
}

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
double EnvelopeFollowerAudioProcessor::getTailLengthSeconds() const
{
    return 0.0; // This plugin should stop as soon as input stops. Ergo, it has no tail.
}

/**
 * Returns the number of programs this plugin wants the DAW to think we have.
 *
 * Always returns 1 because though programs are not implemented for this plugin many DAWs can break if a plugin reports that it doesn't have any.
 *
 * Returns
 * -------
 * int: The number of programs this plugin wants the DAW to think we have. (Always 1 for this implementation)
 */
int EnvelopeFollowerAudioProcessor::getNumPrograms()
{
    return 1;   // NB: Some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even with no programs implemented.
}

/**
 * Gets the index of the currently active program for this plugin.
 *
 * Always returns 0 because programs are not implemented for this plugin
 *
 * Returns
 * -------
 * int: The index of the currently active program for this plugin. (Always 0 for this implementation)
 */
int EnvelopeFollowerAudioProcessor::getCurrentProgram()
{
    return 0; // This plugin does not implement any programs. Just return 0.
}

/**
 * Sets the currently actived program for this plugin.
 *
 * Is a NOOP (No-operation) because programs are not implemented for this plugin.
 *
 * Arguments
 * ---------
 * int index: The index of the program that should become the active program.
 */
void EnvelopeFollowerAudioProcessor::setCurrentProgram (int index)
{
}

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
const juce::String EnvelopeFollowerAudioProcessor::getProgramName (int index)
{
    return {}; // No implemented programs implies no program names. Ergo, return an empty string.
}

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
void EnvelopeFollowerAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

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
void EnvelopeFollowerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Set the number of audio samples the audio processing pipeline should expect per second.
    signalProcessor.setSamplingFrequency(sampleRate);
    
    // Set the number of audio samples that should be processed per produced MIDI message.
    samples_per_midi_message = (int) (sampleRate / midi_message_rate);
    // Reset the number of audio samples that have been processed since the last MIDI output/GUI state update.
    elapsed_since_midi = 0;
    elapsed_since_drawer = 0;

    // Clear the rolling buffers for both the envelope and the input waveform displays.
    EnvVisualiser.clear();
    AudioVisualiser.clear();
    // Set the number of samples the envelope and input waveform displays should expect per second.
    EnvVisualiser.setSamplesPerBlock(samplesPerBlock);
    AudioVisualiser.setSamplesPerBlock(samplesPerBlock);

    makeMIDIOutputDevice();
}

/**
 * Creates a MIDI output device for us to send MIDI to. This proved to work more consistently than sending to the IAC driver bus.
 */
void EnvelopeFollowerAudioProcessor::makeMIDIOutputDevice()
{
     // This process is a critical section, so we need to lock it. The MessageManagerLock
    // will ensure that no other conflicting MIDI operations happen at the same time,
    // for as long as the lock is in existence. The lock gets destroyed when it passes
    // out of scope at the end of the function call, so the code is unlocked.
    const juce::MessageManagerLock mmLock;
    // In case multiple instances of the plugin are running, each one
    // needs to have its own unique Midi Device, with a unique name.
    output_device = nullptr;
    std::string name;
    std::string base_name = "Envelope Follower Midi Device ";
    for (int i = 0; i < MAX_INSTANCES; i++) {
        name = base_name + std::to_string(i);
        output_device = juce::MidiOutput::createNewDevice(name);
        if (output_device) {
            return;
        }
    }
    std::cout << "Unable to create output device";
}

/**
 * Currently a NOOP (No-operation) awaiting further expansion.
 *
 * Runs after EnvelopeFollowerAudioProcessor::processBlock is used for a set of audio input by the DAW.
 * Intended to be used to release memory allocated for processing a series of audio blocks.
 * Currently does nothing as we do not allocate memory in prepareToPlay()
 * If that changes this will release that memory.
 */
void EnvelopeFollowerAudioProcessor::releaseResources() { }

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
bool EnvelopeFollowerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // Reject the layout if the input isn't mono (one channel) or stereo (two channels).
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

   #if ! JucePlugin_IsSynth
    // Reject the layout if the output layout doesn't match the input layout.
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    // Otherwise accept that we can process the layout.
    return true;
  #endif
}
#endif

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
void EnvelopeFollowerAudioProcessor::updateMathParams()
{
    // TODO: Don't use the user param values: those are in units like
    // decibels and percents. Instead, we need to first convert them
    // to linear gain and midi scaled values (i.e. 0-127 instead of 0-100).
    
    // Fetch the values of the user-managed parameters rescaled to be functional.    
    float amp_gain = pow(10.0, (gain_user_param->get() / 20.0)); // source: https://en.wikipedia.org/wiki/Decibel
    float min_value_scaled = (min_pos_user_param->get() / 100.0) * 127.0;
    float max_value_scaled = (max_pos_user_param->get() / 100.0) * 127.0;
    float lowpass_value = low_pass_user_param->get();
    float highpass_value = hi_pass_user_param->get();
    float recovery_time = recovery_user_param->get();
    
    // Update the values of the parameters on the signal processing pipeline.
    signalProcessor.setMaxValue(max_value_scaled);
    signalProcessor.setMinValue(min_value_scaled);
    signalProcessor.setGainValue(amp_gain);
    signalProcessor.setLowpassValue(lowpass_value);
    signalProcessor.setHighpassValue(highpass_value);
    signalProcessor.setRecoveryTimeValue(recovery_time);
}

/**
 * Takes a set of audio sample buffers and converts them into a series of MIDI messages posted to the network interface.
 *
 * Essentially the main loop for this plugin.
 * Will be called by the mounting DAW repeatedly during audio playback.
 * EnvelopeFollowerAudioProcessor::prepareToPlay will always be called before a series of calls to this by the DAW.
 * EnvelopeFollowerAudioProcessor::releaseResources will always be called after a series of calls to this by the DAW.
 *
 * Arguments
 * ---------
 * juce::AudioBuffer<float>&: The set of audio sample buffers used for DAW IO.
 * juce::MidiBuffer&: The set of MIDI message buffers used for DAW IO.
 */
void EnvelopeFollowerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    /*juce::String id = juce::MidiOutput::getDefaultDevice().identifier;
    output_device = juce::MidiOutput::openDevice(id);
    // output_device = juce::MidiOutput::createNewDevice("Envelope Follower Midi Device");
    if (!output_device) {
        std::cout<<"Unable to create output device\n";
    }*/
    
    updateMathParams();
    midiMessages.clear();
    
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear all output channels that aren't also an input channel as
    // they may contain garbage data we don't want getting fed into downstream
    // plugins or the audio output.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    const int num_samples = buffer.getNumSamples();
    float sample;

    // Build the buffer for storing the envelope waveform.
    juce::AudioBuffer<float> vis_samples;
    vis_samples.setSize(1, num_samples); // The buffer must be large enough to hold all of the MIDI message values produced. There are at most as many MIDI messages as input audio samples.
    vis_samples.clear(); // Clear any junk data that may have populated the buffer.
    // 
    int vis_index = 0;

    // Iterate over each sample in the audio buffers:
    for (int index = 0; index < num_samples; index++) {
        // Calculate the average sample value across the input channels.
        sample = 0;
        // - sum over all of the input channels to get the total
        for (int channel = 0; channel < totalNumInputChannels; ++channel)
        {
            sample += buffer.getSample(channel, index);
            // note: it may be faster to use buffer.getReadPointer outside the loop,
            // and loop through that, instead of using getSample every time.
        }
        // - divide by the number of input channels to rescale back to the average
        sample /= totalNumInputChannels;

        // Feed the sample into the audio procesing pipeline.
        signalProcessor.takeInSample(sample);

        // Increment the number of samples that have been processde since the last produced MIDI message and GUI update.
        elapsed_since_midi++;
        elapsed_since_drawer++; // (depricated)
        // If we have processed enough samples for another MIDI output...
        if (elapsed_since_midi == samples_per_midi_message) {
            // Start counting to the next message.
            elapsed_since_midi = 0;
            // Fetch the value of the output MIDI message from the signal processing component.
            midi_value = signalProcessor.getEnvelopePosition();
            // Post the new MIDI meesage to the network interface.
            sendCCMessage(index);
            // Update the MIDI descriprion string for the GUI.
            midi_info = std::to_string(midi_channel) + " " +
                        std::to_string(midi_controller_type) + " " +
                        std::to_string(midi_value);
        }
        // Update the MIDI waveform buffer with the new MIDI value from the audio processing pipeline
        float mappedValue = juce::jmap((float)signalProcessor.getEnvelopePosition(), 0.0f, 127.0f, 0.0f, 1.0f);
        vis_samples.setSample(0, vis_index++, mappedValue);
    }

    // Update the GUI elements that display the input waveform and output envelope.
    EnvVisualiser.pushBuffer(vis_samples);
    AudioVisualiser.pushBuffer(buffer);

}

/**
 * Posts a produced MIDI message to one of the hardware's output ports.
 *
 * Arguments
 * ---------
 * int sample_number: The index of the audio sample that prompted the message to be produced. (unused)
 */
void EnvelopeFollowerAudioProcessor::sendCCMessage(int sample_number)
{
    /* Instead of adding midi messages to the buffer, what we need to do is send
     the midi messages out into hardware-land.*/
    juce::MidiMessage message;
    //std::cout<<"sending midi message "<<value<<"\n";
    // https://www.songstuff.com/recording/article/midi_message_format/
    message = juce::MidiMessage::controllerEvent(midi_channel, midi_controller_type, midi_value);
    // Post the MIDI message to the attatched MIDI device (IAC driver bus).
    output_device->sendMessageNow(message);
    
}

/**
 * Returns whether or not this plugin should have a GUI.
 *
 * Always returns True for this implementation as we want a GUI.
 *
 * Returns
 * -------
 * bool: True if this plugin should have a GUI, False otherwise.
 */
bool EnvelopeFollowerAudioProcessor::hasEditor() const
{
    return true; // A GUI has been implemented. Return true to make sure it gets used.
}

/**
 * Creates and returns a reference to the GUI management component for this plugin.
 *
 * Used to create the EnvelopeFollowerAudioProcessorEditor component for this plugin.
 *
 * Returns
 * -------
 * juce::AudioProcessorEditor*: A pointer to the newly created GUI management component.
 */
juce::AudioProcessorEditor* EnvelopeFollowerAudioProcessor::createEditor()
{
    return new EnvelopeFollowerAudioProcessorEditor (*this); // Construct and return a new GUI management component.
}

/**
 * Compiles and outputs all user-visible parameters to binary encoded XML.
 *
 * Arguments
 * ---------
 * juce::MemoryBlock& destData: The section of memory to write the compiled data to.
 */
void EnvelopeFollowerAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // Create an XML management struct to make formatting easier.
    std::unique_ptr<juce::XmlElement> xml (new juce::XmlElement ("sliderParams"));
    // Store the user managed parameters as XML.
    xml->setAttribute("gain", (double) gain_user_param->get());
    xml->setAttribute("min", (double) min_pos_user_param->get());
    xml->setAttribute("max", (double) max_pos_user_param->get());
    xml->setAttribute("lo", (double) low_pass_user_param->get());
    xml->setAttribute("hi", (double) hi_pass_user_param->get());
    xml->setAttribute("recovery", (double) recovery_user_param->get());
    xml->setAttribute("channel", midi_channel);
    xml->setAttribute("type", midi_controller_type);
    // Write the XML data to a block of RAM.
    copyXmlToBinary(*xml, destData);
}

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
void EnvelopeFollowerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // Read some XML data from the targeted memory block.
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data,
     sizeInBytes));
    // If we retreived any XML data update the parameters accordingly.
    if (xmlState.get() != nullptr) {
        if (xmlState->hasTagName("gain")) {
            *gain_user_param = xmlState->getDoubleAttribute("gain");
        }
        if (xmlState->hasTagName("min")) {
            *min_pos_user_param = xmlState->getDoubleAttribute("min");
        }
        if (xmlState->hasTagName("max")) {
            *max_pos_user_param = xmlState->getDoubleAttribute("max");
        }
        if (xmlState->hasTagName("lo")) {
            *low_pass_user_param = xmlState->getDoubleAttribute("lo");
        }
        if (xmlState->hasTagName("hi")) {
            *hi_pass_user_param = xmlState->getDoubleAttribute("hi");
        }
        if (xmlState->hasTagName("recovery")) {
            *recovery_user_param = xmlState->getDoubleAttribute("recovery");
        }
        if (xmlState->hasTagName("channel")) {
            midi_channel = xmlState->getIntAttribute("channel");
        }
        if (xmlState->hasTagName("type")) {
            midi_controller_type = xmlState->getIntAttribute("type");
        }
    }
}

/**
 * Sets the index of the MIDI channel used by this plugin.
 *
 * Arguments
 * ---------
 * int new_channel: The new index of the MIDI channel to be used by this plugin.
 */
void EnvelopeFollowerAudioProcessor::setMidiChannel(int new_channel)
{
    midi_channel = new_channel; // Set the target midi channel.
}

/**
 * Sets the type of MIDI message this processor is producing.
 *
 * Arguments
 * ---------
 * int new_type:    The type of MIDI message this processor will produce.
 *                  See https://www.paulcecchettimusic.com/full-list-of-midi-cc-numbers/ for more detail.
 */
void EnvelopeFollowerAudioProcessor::setMidiType(int new_type)
{
    midi_controller_type = new_type; // Set the target MIDI CC format type.
}

/**
 * Returns the index of the MIDI channel used by this plugin.
 *
 * Returns
 * -------
 * int: The index of the MIDI channel used by this plugin.
 */
int EnvelopeFollowerAudioProcessor::getMidiChannel()
{
    return midi_channel; // Fetch the target MIDI channel.
}

/**
 * Gets the type of MIDI message this processor is producing.
 *
 * Returns
 * -------
 * int: The type of MIDI message this processor is producing.
 */
int EnvelopeFollowerAudioProcessor::getMidiType()
{
    return midi_controller_type; // Fetch the target MIDI CC format type.
}

//==============================================================================
/**
 * Global JUCE framework function responsible for creating the EnvelopeFollowerAudioProcessor component when setting up the plugin.
 * 
 * Returns
 * -------
 * juce::AudioProcessor*: The new EnvelopeFollowerAudioProcessor component for this plugin.
 */
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new EnvelopeFollowerAudioProcessor(); // Create and return a new EnvelopeFollowerAudioProcessor component.
}
