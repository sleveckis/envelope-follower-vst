/*
  ==============================================================================

    PluginEditor.cpp
    Created: 15 Feb 2022 10:52pm PST
    Last Updated: 5 Mar 2022 9:00pm PST
    Authors:
    - Arden Butterfield
    - Stephen Leveckis
    - Zacree Carroll
    - Ethan Killen (documentation)

    Description: Contains the implementation of the methods for the EnvelopeFollowerAudioProcessorEditor component class.
    Dependencies:
    - PluginProcessor.h
    - PluginEditor.h

  ==============================================================================
*/

// Import the depencencies for this file.
#include "PluginProcessor.h" // Import the interface definition for the EnvelopeFollowerAudioprocessor for usage in this implementation.
#include "PluginEditor.h" // Import the interface definition for the EnvelopeFollowerAudioProcessorEditor for implementation.

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
EnvelopeFollowerAudioProcessorEditor::EnvelopeFollowerAudioProcessorEditor (EnvelopeFollowerAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Load the background image from memory.
    bg = juce::ImageCache::getFromMemory(BinaryData::coolBackground_png, BinaryData::coolBackground_pngSize);

    // Sets the size of the GUI window. To 500px by 450px.
    setSize (500, 450); // horizontal, vertical
    
    // Starts the timer loop responsible for calling EnvelopeFollowerAudioProcessorEditor::timerCallback every 100 seconds.
    startTimer(100);
   
    /// <summary>
    ///     Prepares the rotary slider responsible for managing the gain of the input audio signal for display.
    /// </summary>
    gain.setLookAndFeel(&k1); // Sets the component responsible for drawing this knob.
    gain.setSliderStyle(juce::Slider::RotaryVerticalDrag); // Sets this knob to be drawn as a rotary knob.
    gain.setRange(-30.0, 30.0, 0.01); // Sets the minimum value, maximum value, and step interval for this knob.
    gain.setTextValueSuffix(" dB"); // Sets the value suffix (decibels)
    gain.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20); // Sets how the attached text box for this knob is drawn. Used to display the current value of the knob.
    gain.addListener(this); // Registers the GUI to listen for changes to the value of this knob.
    addAndMakeVisible(gain); // Registers the knob to be rendered to the GUI.
    gain.setTooltip(gain_desc); // Sets the mouseover tooltip text for this knob.

    /// <summary>
    ///     As above for the knob respinsible for managing the minimum value of the output MIDI signal.
    /// </summary>
    min_pos.setLookAndFeel(&k2);
    min_pos.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    min_pos.setRange(0.0, 100.0, 0.1);
    min_pos.setTextValueSuffix(" %");
    min_pos.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    min_pos.addListener(this);
    addAndMakeVisible(min_pos);
    min_pos.setTooltip(min_pos_desc);

    /// <summary>
    ///     As above for the knob respinsible for managing the maximum value of the output MIDI signal.
    /// </summary>
    max_pos.setLookAndFeel(&k3);
    max_pos.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    max_pos.setRange(0.0, 100.0, 0.1);
    max_pos.setTextValueSuffix(" %");
    max_pos.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    max_pos.addListener(this);
    addAndMakeVisible(max_pos);
    max_pos.setTooltip(max_pos_desc);

    /// <summary>
    ///     As above for the knob respinsible for managing the maximum considered frequencey of the input audio signal.
    /// </summary>
    lowpass.setLookAndFeel(&k4);
    lowpass.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    lowpass.setRange(0.0, 20000.0, 1.0);
    lowpass.setSkewFactorFromMidPoint(500);
    lowpass.setTextValueSuffix(" Hz");
    lowpass.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    lowpass.addListener(this);
    addAndMakeVisible(lowpass);
    lowpass.setTooltip(lowpass_desc);

    /// <summary>
    ///     As above for the knob respinsible for managing the minimum considered frequencey of the input audio signal.
    /// </summary>
    hipass.setLookAndFeel(&k5);
	hipass.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    hipass.setRange(0.0, 20000.0, 1.0);
    hipass.setSkewFactorFromMidPoint(500);
    hipass.setTextValueSuffix(" Hz");
    hipass.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    hipass.addListener(this);
    addAndMakeVisible(hipass);
    hipass.setTooltip(hipass_desc);

    /// <summary>
    ///     As above for the knob respinsible for managing the rate at which the envelope recovers after an input amplitude spike spike.
    /// </summary>
    recovery.setLookAndFeel(&k6);
	recovery.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    recovery.setRange(0.0, 1.0, 0.01);
    recovery.setTextValueSuffix(" ");
    recovery.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    recovery.addListener(this);
    addAndMakeVisible(recovery);
    recovery.setTooltip(recovery_desc);

    /// <summary>
    ///     Prepares the input selection box for usage and GUI display.
    /// </summary>
    // Sets the possible value of the selection box.
    // Midi channels are 1 - 16, inclusive.
    // https://docs.juce.com/master/classMidiMessage.html#a7c776a7b6bfa0e4216e95ff236ff250c
    for (int i = 1; i<=16; i++) { // Iterate over all possible MIDI output channels.
        channel_selector.addItem(std::to_string(i), i); // Add them as a selectable option.
    }
    channel_selector.onChange = [this] {channelSelectorChanged(); }; // Registers this GUI manager to receive updates when the value of this selection box changes.
    channel_selector.setSelectedId (audioProcessor.getMidiChannel()); // Sets the default value of the selection box.
    addAndMakeVisible(channel_selector); // Registers the selection box to be rendered to the GUI.
    channel_selector.setTooltip(channel_desc); // Sets the mouseover tooltip text for this selection box.
    

    /// <summary>
    ///     As above for the input selection box for the type of MIDI messages output by the plugin.
    /// </summary>
    // Controller type can be 1-127, inclusive https://www.paulcecchettimusic.com/full-list-of-midi-cc-numbers/
    for (int i = 1; i <= 127; i++) { // Iterate over all possible MIDI output types.
        type_selector.addItem(std::to_string(i), i); // Add them as a selectable option.
    }
    type_selector.onChange = [this] {typeSelectorChanged(); };
    type_selector.setSelectedId(audioProcessor.getMidiType());
    addAndMakeVisible(type_selector);
    type_selector.setTooltip(type_desc);
    
    /// <summary>
    ///     Registers the GUI elements responsible for rendering the output enevelope and input audio waveform for rendering by this GUI.
    ///     Also sets their mouseover tooltip text.
    /// </summary>
    addAndMakeVisible(audioProcessor.EnvVisualiser);
    addAndMakeVisible(audioProcessor.AudioVisualiser);
    audioProcessor.EnvVisualiser.setTooltip(envelope_vis_desc);
    audioProcessor.AudioVisualiser.setTooltip(audio_in_vis_desc);
    
    // The channel, bank, and current value feedback font settings
    /// <summary>
    ///     Prepares the text box responsible for rendering the current state of the output MIDI for rendring on the GUI.
    /// </summary>
    addAndMakeVisible(sending_label); // Registers the text box for rendering to the GUI.
    sending_label.setTooltip(sending_desc); // Sets the mouseover tooltip text for the text box.
    sending_label.setColour(juce::Label::textColourId, juce::Colours::lightgreen); // Sets the text color used by the text box to light green.
    sending_label.setJustificationType(juce::Justification::centred); // Sets the text alignment for the text box to be centered.
    
    // The font that the text box should use as fetched from this GUI manager.
    const juce::Font myFont = getFont();
    // Sets the font used by the text box.
    sending_label.setFont(juce::Font (myFont));
    sending_label.setText("", juce::dontSendNotification); // Sets the text displayed by the text box by default.
    
    /// <summary>
    ///     Registers the GUI element responsible for rendering mouseover tooltip text to be rendered by this GUI.
    /// </summary>
    addAndMakeVisible(tooltip_window);
    
}

EnvelopeFollowerAudioProcessorEditor::~EnvelopeFollowerAudioProcessorEditor()
{
    gain.setLookAndFeel(nullptr);
    min_pos.setLookAndFeel(nullptr);
    max_pos.setLookAndFeel(nullptr);
    lowpass.setLookAndFeel(nullptr);
    hipass.setLookAndFeel(nullptr);
    recovery.setLookAndFeel(nullptr);
}

/**
 * Draws the background of the GUI and marks the MIDI output
 * detail string for redrawing on the next GUI update.
 *
 * Arguments
 * ---------
 * juce::Graphics& g: The graphics context used to draw this GUI element on the GUI.
 */
void EnvelopeFollowerAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Set base characteristics of the window such as text size and font
    // Set up the background image to be loaded in as the background
    // Set the font size used by text in the GUI.
    g.setFont(15.0f);
    // Register the text display for the state of the output MIDI stream to be updated.
    sending_label.setText(audioProcessor.midi_info, juce::dontSendNotification);
    sending_label.repaint();
    // Set the backdrop color for the GUI. Used on the off chance the background image fails to render.
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    g.setColour (juce::Colours::white);
    // Draw the background image to the GUI.
    g.drawImage(bg, 0, 0, 500, 450, 0, 0, 1000, 900);
        
}

/**
 * Sets the position and size of all GUI elements.
 *
 * Called when the GUI is first initialized and then whenever it is resized.
 * Would be called whenever the the GUI is resized but the GUI is currently staticly sized.
 * Update this if the GUI is changed to allow resizing.
 */
void EnvelopeFollowerAudioProcessorEditor::resized()
{
    // Lays out the 
    
    // Set the offsets and sizes of all of the knobs on the GUI.
    gain.setBounds(gain_rect);
    hipass.setBounds(hipass_rect);
    lowpass.setBounds(lowpass_rect);
    min_pos.setBounds(min_pos_rect);
    max_pos.setBounds(max_pos_rect);
    recovery.setBounds(recovery_rect);

    // Set the offsets and sizes of all of the selection boxes in the GUI.
    channel_selector.setBounds(10, 400, 100, 25); // horizontal offset, veritcal offset, horizontal size, vertical size
    type_selector.setBounds(292, 400, 100, 25);

    // Both halves of the visualizezr
    // Set the offsets and sizes of both of the waveform visualizers in the GUI.
    audioProcessor.EnvVisualiser.setBounds(56, 305, 398, 70); // horizontal offset, veritcal offset, horizontal size, vertical size
    audioProcessor.AudioVisualiser.setBounds(56, 235, 398, 70);
    
    // The user feedback menu at the top
    // Set the offset and size of the text display for the current value  of the MIDI output stream to the top of the GUI.
    sending_label.setBounds(338, 25, 200, 25); // horizontal offset, veritcal offset, horizontal size, vertical size
}

/**
 * Relays changes in the value of a juce::Slider derived GUI element to the EnvelopeFollowerAudioProcessor.
 *
 * Hooks into the value of a juce::Slider derived GUI element changing through one of JUCEs event/signal systems.
 *
 * Arguments
 * ---------
 * juce::Slider: The GUI element that changed in value.
 */
void EnvelopeFollowerAudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{
    /// <summary>
    ///     Respond to the value of any of the input knobs changing by relaying the changed value to the EnvelopeFollowerAudioProcessor.
    /// </summary>
    if (slider == &gain) { // Handle the gain-control knob changing.
        audioProcessor.gain_user_param->beginChangeGesture();
        *audioProcessor.gain_user_param = (float) gain.getValue();
        audioProcessor.gain_user_param->endChangeGesture();
    } else if (slider == &min_pos) { // Handle the min-value knob changing.
        audioProcessor.min_pos_user_param->beginChangeGesture();
        *audioProcessor.min_pos_user_param = (float) min_pos.getValue();
        audioProcessor.min_pos_user_param->endChangeGesture();
    } else if (slider == &max_pos) { // Handle the max-value knob changing.
        audioProcessor.max_pos_user_param->beginChangeGesture();
        *audioProcessor.max_pos_user_param = (float) max_pos.getValue();
        audioProcessor.max_pos_user_param->endChangeGesture();
    } else if (slider == &lowpass) { // Handle the maximum frequency knob changing.
        audioProcessor.low_pass_user_param->beginChangeGesture();
        *audioProcessor.low_pass_user_param = (float) lowpass.getValue();
        audioProcessor.low_pass_user_param->endChangeGesture();
    } else if (slider == &hipass) { // Handle the minimum frequency knob changing.
        audioProcessor.hi_pass_user_param->beginChangeGesture();
        *audioProcessor.hi_pass_user_param = (float) hipass.getValue();
        audioProcessor.hi_pass_user_param->endChangeGesture();
    } else if (slider == &recovery) { // handle the recovery rate knob changing.
        audioProcessor.recovery_user_param->beginChangeGesture();
        *audioProcessor.recovery_user_param = (float) recovery.getValue();
        audioProcessor.recovery_user_param->endChangeGesture();
    }
}

/**
 * Updates the value of all the primary user input GUI elements to their corresponding value
 * as held by the EnvelopeFollowerAudioProcessor component.
 *
 * Called every 100 milliseconds while this component exists in the current implementation.
 */
void EnvelopeFollowerAudioProcessorEditor::timerCallback()
{
    // If a parameter is automated by the DAW, that will change the parameters
    // value in the PluginProcessor, but not in the PluginEditor. To update
    // the knobs when these changes happen, this fetchs the current values of
    // the parameters from the PluginProcessor and updates the respective GUI
    // elements every 100 milliseconds.
    gain.setValue(*audioProcessor.gain_user_param, juce::dontSendNotification);
    min_pos.setValue(*audioProcessor.min_pos_user_param, juce::dontSendNotification);
    max_pos.setValue(*audioProcessor.max_pos_user_param, juce::dontSendNotification);
    lowpass.setValue(*audioProcessor.low_pass_user_param, juce::dontSendNotification);
    hipass.setValue(*audioProcessor.hi_pass_user_param, juce::dontSendNotification);
    recovery.setValue(*audioProcessor.recovery_user_param, juce::dontSendNotification);
    channel_selector.setSelectedId(audioProcessor.getMidiChannel());
    type_selector.setSelectedId(audioProcessor.getMidiType());
}

/**
 * Returns the custom text font used by this GUI.
 *
 * Returns
 * -------
 * static const juce::Font&: The font that this GUI should use.
 */
const juce::Font& EnvelopeFollowerAudioProcessorEditor::getFont()
{
    // The JUCE text font that this GUI and its elements should use. Generated as a static var to prevent it being regenerated evey time this is called.
    static juce::Font customFont(juce::Font(juce::Typeface::createSystemTypefaceFor(BinaryData::Inversionz_otf,
        BinaryData::Inversionz_otfSize)));
    // Returns the above JUCE text font.
    return customFont;
}

/**
 * Relays a change to the value of the GUI element representing the current output MIDI channel
 * to the EnvelopeFollowerAudioProcessor.
 */
void EnvelopeFollowerAudioProcessorEditor::channelSelectorChanged()
{
    // The index of the output MIDI channel that has been selected.
    int id = channel_selector.getSelectedId();
    // Relays the change to the selected MIDI output channel to the EnvelopeFollowerAudioProcessor component.
    audioProcessor.setMidiChannel(id);
}

/**
 * Relays a change to the value of the GUI element representing the current output MIDI type
 * to the EnvelopeFollowerAudioProcessor.
 */
void EnvelopeFollowerAudioProcessorEditor::typeSelectorChanged()
{
    // The MIDI output type that has been selected.
    int id = type_selector.getSelectedId();
    // Relays the change to the selected MIDI output type to the EnvelopeFollowerAudioProcessor component.
    audioProcessor.setMidiType(id);
}
