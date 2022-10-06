/*
  ==============================================================================

    PluginEditor.h
    Created: 15 Feb 2022 10:52pm PST
    Last Updated: 5 Mar 2022 9:00pm PST
    Authors:
    - Arden Butterfield
    - Stephen Leveckis
    - Ethan Killen (documentation)

    Description: Contains the API definition for the EnvelopeFollowerAudioProcessorEditor component class.
    Dependencies:
    - JuceHeader.h
    - PluginProcessor.h
    - BinaryData.h
    - CustomKnobs.h

  ==============================================================================
*/

// Ensure that this is only imported at most once. Ignore duplicate imports.
#pragma once

// Import the dependencies for the contents of this file.
#include <JuceHeader.h> // Import the JUCE framework dependencies necessary for this plugin to run.
#include "PluginProcessor.h" // Import the interface definition for the EnvelopeFollowerAudioProcessor and the envelope and waveform display GUI elements.
#include "BinaryData.h" // Import the assets the projucer IDE has embedded into the source code.
#include "CustomKnobs.h" // Import the interface definition for the CustomKnobs component.


/**
 * The component responsible for managing the GUI for the plugin.
 * 
 * Handles coordinating responses to user input.
 * 
 * Attributes
 * ----------
 * private EnvelopeFollowerAudioProcessor& audioProcessor: The audio processor component this component is managing a GUI for.
 * private juce::Slider gain: The rotary slider that displays and allows user modification of the input audio scaling.
 * private juce::Slider min_pos: The rotary slider that displays and allows user modification of the minimum output MIDI value.
 * private juce::Slider max_pos: The rotary slider that displays and allows user modification of the maximum output MIDI value.
 * private juce::Slider lowpass: The rotary slider that displays and allows user modification of the maximum considered input audio frequency.
 * private juce::Slider hipass: The rotary slider that displays and allows user modification of the minimum considered input audio frequency.
 * private juce::Slider recovery: The rotary slider that displays and allows user modification of how quickly the envelope decays after a large amplitude spike in the input audio samples.
 * private juce::ComboBox channel_selector: The GUI element that displays and allows the user to modify the MIDI channel the plugin outputs on.
 * private juce::ComboBox type_selector: The GUI element that displays and allows the user to modify the type of MIDI output the plugin produces.
 * private juce::Image bg: The background image for the GUI.
 * private juce::Label sending_label: The text box used to display the current state of the output envelope.
 * private const int KNOB_WIDTH: The horizontal size of the rotary sliders (knobs) displayed in the GUI.
 * private const int KNOB_HEIGHT: The vertical size of the rotary sliders (knobs) displayed in the GUI.
 * private juce::TooltipWindow tooltip_window: The GUI element responsible for rendering mouseover tooltips.
 * private CustomKnobs k1: The component used to draw the gain knob to the GUI.
 * private CustomKnobs k2: The component used to draw the min_pos knob to the GUI.
 * private CustomKnobs k3: The component used to draw the max_pos knob to the GUI.
 * private CustomKnobs k4: The component used to draw the lowpass threshold knob to the GUI.
 * private CustomKnobs k5: The component used to draw the hipass threshold knob to the GUI.
 * private CustomKnobs k6: The component used to draw the recovery time knob to the GUI.
 * private juce::Rectangle<int> gain_rect: The bounding box used to position the gain knob in the GUI.
 * private juce::Rectangle<int> hipass_rect: The bounding box used to position the high pass threshold knob in the GUI.
 * private juce::Rectangle<int> lowpass_rect: The bounding box used to position the low pass threshold knob in the GUI.
 * private juce::Rectangle<int> min_pos_rect: The bounding box used to position the minimum output value knob in the GUI.
 * private juce::Rectangle<int> max_pos_rect: The bounding box used to position the maximum output value knob in the GUI.
 * private juce::Rectangle<int> recovery_rect: The bounding box used to position the recovery time knob in the GUI.
 * private const std::string gain_desc: The mouseover tooltip text for the gain knob.
 * private const std::string min_pos_desc: The mouseover tooltip text for the min output value knob.
 * private const std::string max_pos_desc: The mouseover tooltip text for the max output value knob.
 * private const std::string lowpass_desc: The mouseover tooltip text for the low pass threshold knob.
 * private const std::string hipass_desc: The mouseover tooltip text for the high pass threshold knob.
 * private const std::string recovery_desc: The mouseover tooltip text for the recovery time knob.
 * private const std::string channel_desc: The mouseover tooltip text for the output channel selection box.
 * private const std::string type_desc: The mouseover tooltip text for the min output MIDI type selection box.
 * private const std::string audio_in_vis_desc: The mouseover tooltip text for the input waveform visualizer GUI element.
 * private const std::string envelope_vis_desc: The mouseover tooltip text for the output envelope visualzer GUI element.
 * private const std::string sending_desc: The mouseover tooltip text for the min output value knob.
 * 
 * Methods
 * -------
 * public EnvelopeFollowerAudioProcessorEditor(): The constructor for the EnvelopeFollowerAudioprocessorEditor component.
 * public void paint(juce::Graphics& g): Renders the backdrop and background for the GUI.
 * public void resized(): Handles positioning and rescaling all of the GUI elements when the window is initialized and whenever the windows size changes.
 * public void sliderValueChanged(juce::Slider* slider): Handles relaying changes to the values of any of the attached sliders to the EnvelopeFollowerAudioProcessor component.
 * public void timerCallback(): Handles periodically updating the value of the GUI from the state of the EnvelopeFollowerAudioProcessor component.
 * public static const juce::Font& getFont(): Returns the text font used by the rendered GUI.
 * private void channelSelectorChanged(): Handles changes to the value of the MIDI output channel selection box by relaying the changed value to the EnvelopeFollowerAudioProcessor component.
 * private void typeSelectorChanged(): Handles changes to the value of the MIDI output type selection box by relaying the changed value to the EnvelopeFollowerAudioProcessor component.
 * 
 * Inherits
 * - juce::AudioProcessorEditor
 * - juce::Slider::Listener
 * - juce::Timer
 * 
 * Owns
 * - CustomKnobs (multiple instances)
 * 
 * Owned by
 * - EnvelopeFollowerAudioProcessor
 */
class EnvelopeFollowerAudioProcessorEditor : public juce::AudioProcessorEditor, public juce::Slider::Listener, public juce::Timer
{
public:
    /**
     * The constructor for the EnvelopeFollowerAudioProcessorEditor component.
     * 
     * Loads the background image for the GUI from memory.
     * Sets the size of the GUI window.
     * Starts the 100 milliszecond internal timer loop responsible for calling EnvelopeFollowerAudioProcessorEditor::timerCallback.
     * Constructs all child GUI elements and registers them to be displayed in this GUI. 
     * 
     * Arguments
     * EnvelopeFollowerAudioProcessorEditor&: A reference to the EnvelopeFollowerAudioProcessorEditor component being constructed.
     */
    EnvelopeFollowerAudioProcessorEditor(EnvelopeFollowerAudioProcessor&);
    
    ~EnvelopeFollowerAudioProcessorEditor() override;

    //

    /**
     * Draws the background of the GUI and marks the MIDI output
     * detail string for redrawing on the next GUI update.
     * 
     * Arguments
     * ---------
     * juce::Graphics& g: The graphics context used to draw this GUI element on the GUI.
     */
    void paint(juce::Graphics&) override;

    /**
     * Sets the position and size of all GUI elements.
     * 
     * Called when the GUI is first initialized and then whenever it is resized.
     * Would be called whenever the the GUI is resized but the GUI is currently staticly sized.
     * Update this if the GUI is changed to allow resizing.
     */
    void resized() override;

    /**
     * Relays changes in the value of a juce::Slider derived GUI element to the EnvelopeFollowerAudioProcessor.
     * 
     * Hooks into the value of a juce::Slider derived GUI element changing through one of JUCEs event/signal systems.
     * 
     * Arguments
     * ---------
     * juce::Slider: The GUI element that changed in value.
     */
    void sliderValueChanged(juce::Slider*) override;

    /**
     * Updates the value of all the primary user input GUI elements to their corresponding value
     * as held by the EnvelopeFollowerAudioProcessor component.
     * 
     * Called every 100 milliseconds while this component exists in the current implementation.
     */
    void timerCallback() override;

    /**
     * Returns the custom text font used by this GUI.
     * 
     * Returns
     * -------
     * static const juce::Font&: The font that this GUI should use.
     */
    static const juce::Font& getFont();

private:
    /// <summary>
    ///     The EnvelopeFollowerAudioProcessor component that this component is managing a GUI for.
    /// 
    ///     The referenced component owns this component and will outlive this component.
    /// </summary>
    EnvelopeFollowerAudioProcessor& audioProcessor;

    // All the custom Knobs objects which we use as input for 
    // "SetLookAndFeel" in PluginEditor.cpp

    /// <summary>
    ///     The rotary slider responsible for managing the gain applied to the input audio signal.
    /// 
    ///     Disposed of when this component is disposed of.
    /// </summary>
    juce::Slider gain;
    /// <summary>
    ///     The rotary slider responsible for managing the minimum value of the output MIDI produced by the EnvelopeFollowerAudioProcessor component.
    /// 
    ///     Disposed of when this component is disposed of.
    /// </summary>
    juce::Slider min_pos;
    /// <summary>
    ///     The rotary slider responsible for managing the maximum value of the output MIDI produced by the EnvelopeFollowerAudioProcessor component.
    /// 
    ///     Disposed of when this component is disposed of.
    /// </summary>
    juce::Slider max_pos;
    /// <summary>
    ///     The rotary slider responsible for managing the maximum input audio frequency considered by the EnvelopeFollowerAudioProcessor when producing the output MIDI.
    /// 
    ///     Disposed of when this component is disposed of.
    /// </summary>
    juce::Slider lowpass;
    /// <summary>
    ///     The rotary slider responsible for managing the minimum input audio frequency condidered by the EnvelopeFollowerAudioProcessor when producing the output MIDI.
    /// 
    ///     Disposed of when this component is disposed of.
    /// </summary>
    juce::Slider hipass;
    /// <summary>
    ///     The rotary slider responsible for managing the rate at which the produced envelope recovers after a spike in input amplitude.
    /// 
    ///     Disposed of when this component is disposed of.
    /// </summary>
    juce::Slider recovery;
    /// <summary>
    ///     The GUI element used to select the MIDI channel the output stream is sent to.
    /// 
    ///     Disposed of when this component is disposed of.
    /// </summary>
    juce::ComboBox channel_selector;
    /// <summary>
    ///     The GUI element used to select the type of MIDI messages output by this plugin.
    /// 
    ///     Disposed of when this component is disposed of.
    /// </summary>
    juce::ComboBox type_selector;

    /// <summary>
    ///     The background image used by the GUI.
    /// 
    ///     Disposed of when this component is disposed of.
    /// </summary>
    juce::Image bg;
    
    /// <summary>
    ///     The text output for the current state of the output MIDI stream.
    /// 
    ///     Disposed of when this component is disposed of.
    /// </summary>
    juce::Label sending_label;
    
    /// <summary>
    ///     The horizontal size of the rotary sliders in the GUI in pixels.
    /// </summary>
    const int KNOB_WIDTH = 100;
    /// <summary>
    ///     The vertical size of the rotary sliders in the GUI in pixels.
    /// </summary>
    const int KNOB_HEIGHT = 100;

    /// <summary>
    ///     The GUI element which renders the mouseover text for the rotary sliders.
    /// </summary>
    juce::TooltipWindow tooltip_window;

    /// <summary>
    ///     All of the CustomKnob components used to draw the rotary sliders on the GUI.
    /// 
    ///     Disposed of when this component is disposed of.
    /// </summary>
    CustomKnobs k1; // gain
    CustomKnobs k2; // min_pos
    CustomKnobs k3; // max_pos
    CustomKnobs k4; // lowpass
    CustomKnobs k5; // highpass
    CustomKnobs k6; // recovery
    
    /// <summary>
    ///     The bounding box for each of the rotary sliders as drawn on the GUI.
    /// 
    ///     x, y, width, height
    ///     Width placement are all stepped by 1/6 of 460 (500 - 20 - 20),
    ///     with a border of 20 on the left and right side of the screen
    /// </summary>
    juce::Rectangle<int> gain_rect = juce::Rectangle<int>(5, 79, KNOB_WIDTH, KNOB_HEIGHT);
    juce::Rectangle<int> hipass_rect = juce::Rectangle<int>(83, 80, KNOB_WIDTH, KNOB_HEIGHT);
    juce::Rectangle<int> lowpass_rect = juce::Rectangle<int>(161, 80, KNOB_WIDTH, KNOB_HEIGHT);
    juce::Rectangle<int> min_pos_rect = juce::Rectangle<int>(238, 80, KNOB_WIDTH, KNOB_HEIGHT);
    juce::Rectangle<int> max_pos_rect = juce::Rectangle<int>(312, 80, KNOB_WIDTH, KNOB_HEIGHT);
    juce::Rectangle<int> recovery_rect = juce::Rectangle<int>(388, 80, KNOB_WIDTH, KNOB_HEIGHT);
    
    /// <summary>
    ///     The mouseover description text for each of the GUI elements.
    /// </summary>
    const std::string gain_desc = "Gain applied to the incoming signal before calculating of the envelope";
    const std::string min_pos_desc = "Knob position to be sent when the incoming signal has no amplitude";
    const std::string max_pos_desc = "Knob position to be sent when the incoming signal is at maximum amplitude";
    const std::string lowpass_desc = "Cutoff frequency of lowpass filter applied to incoming signal before calculating the envelope";
    const std::string hipass_desc = "Cutoff frequency of highpass filter applied to incoming signal before calculating the envelope";
    const std::string recovery_desc = "Time in seconds it takes the tracked envelope to return to half of its present value, when no audio is coming in";
    
    const std::string channel_desc = "Midi channel where the messages will be sent to";
    const std::string type_desc = "Midi CC number of the midi messages (For instance, 1 = modulation wheel, 7 = volume)";
    
    const std::string audio_in_vis_desc = "Waveform of raw input audio, in red";
    const std::string envelope_vis_desc = "Envelope after processing, to be sent as midi, in green";
    
    const std::string sending_desc = "The channel, bank, and current value of the most recently sent midi message";
    
    
    /**
     * Relays a change to the value of the GUI element representing the current output MIDI channel
     * to the EnvelopeFollowerAudioProcessor.
     */
    void channelSelectorChanged();

    /**
     * Relays a change to the value of the GUI element representing the current output MIDI type
     * to the EnvelopeFollowerAudioProcessor.
     */
    void typeSelectorChanged();
    
    // A macro that prevents memory leaks and by-value copying of this component from the JUCE framework.
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EnvelopeFollowerAudioProcessorEditor)
};
