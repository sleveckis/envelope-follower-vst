/*
  ==============================================================================

    CustomKnobs.h
    Created: 28 Feb 2022 4:03pm PST
    Last Updated: 5 Mar 2022 9:00pm PST
    Authors:
    - Stephen Leveckis
    - Ethan Killen (documentation)

    Description: Contains the API definition for the CustomKnobs component class.
    Dependencies:
    - JuceHeader.h

  ==============================================================================
*/

// Ensure that this is only imported at most once. Ignore duplicate imports.
#pragma once

// Import the dependencies for the contents of this file.
#include <JuceHeader.h> // Import the JUCE framework dependencies necessary for this plugin to run.

/**
 * A component responsible for drawing a rotary slider to the GUI.
 * 
 * Attributes
 * ----------
 * private juce::Image img1: A sprite sheet containing the visible states of the rotary slider this draws.
 * 
 * Methods
 * -------
 * public CustomKnobs(): The constructor for this component.
 * public void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos, float rotaryStartAngle, float rotaryEndAngle, juce::Slider& slider): Draws the given rotary slider to the GUI at the given position, size, and rotary position.
 * 
 * Inherits
 * - juce::LookAndFeel_V4
 * 
 * Owned by
 * - EnvelopeFollowerAudioProcessorEditor
 */
class CustomKnobs : public juce::LookAndFeel_V4
{
public:
    /**
     * The constructor for the CustomKnobs component. 
     * 
     * Fetches the possible knob positions as an image from disk and caches it for use.
     */
    CustomKnobs();

    /**
     * The method responsible for drawing a dial (rotary slider) on the GUI.
     * 
     * Arguments
     * ---------
     * juce::Graphics& g: The graphics context the will be drawn in.
     * int x: The horizontal offset of the dial right from the left edge of the graphics context.
     * int y: The vertical offset of the dial down from the top edge of the graphics context.
     * int width: The horizontal size of the dial to draw to the GUI.
     * int height: The vertical size of the dial to draw tothe GUI.
     * float sliderPos: The angle the dial is set to.
     * float rotaryStartAngle: The angle representing the minimum selectable value of the dial.
     * float rotaryEndAngle: The angle representing the maximum selectable value of the dial.
     * juce::Slider &slider: The dial that to draw to the GUI.
     */
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, 
        float sliderPos, float rotaryStartAngle, float rotaryEndAngle, juce::Slider& slider) override;

private:
    /// <summary>
    ///     A sprite sheet containing the visible states of the rotary slider.
    /// 
    ///     Disposed of when this component is disposed of.
    /// </summary>
    juce::Image img1;
};
