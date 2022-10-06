/*
  ==============================================================================

    CustomKnobs.cpp
    Created: 28 Feb 2022 4:03pm PST
    Last Updated: 5 Mar 2022 9:00pm PST
    Authors:
    - Stephen Leveckis
    - Arden Butterfield
    - Ethan Killen (documentation)

    Description: Contains the implementation of the methods for the CustomKnobs component class.
    Dependencies:
    - CustomKnobs.h

  ==============================================================================
*/

// Import the dependencies for the contents of this file.
#include "CustomKnobs.h" // Import the interface definition for the CustomKnobs component class for implementation.

/**
 * The constructor for the CustomKnobs component.
 *
 * Fetches the possible knob positions as an image from disk and caches it for use.
 */
CustomKnobs::CustomKnobs() {
    /// Loads the image used to render the state of the knob from memory.
    img1 = juce::ImageCache::getFromMemory(BinaryData::knob1_png, BinaryData::knob1_pngSize);
}

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
void CustomKnobs::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
    float rotaryStartAngle, float rotaryEndAngle, juce::Slider& slider) {

    if (img1.isValid()) { // If we have a valid image to use to display the knob state use it.
        // The 
        const double rotation = (slider.getValue() - slider.getMinimum()) /
            (slider.getMaximum() - slider.getMinimum());

        // The number of knob sprites in the knob sprite sheet that is the loaded knob image.
        const int frames = img1.getHeight() / img1.getWidth();
        // The index of the knob sprite we want from the sprite sheet that is the loaded knob image.
        const int frameID = (int)ceil(rotation * ((double)frames - 1.0));
        // The radius of the knob image to display.
        const float radius = juce::jmin(width / 2.0f, height / 2.0f);
        // The horizontal position of the center of the knob image to display.
        const float centerX = x + width * 0.5f;
        // The vertical position of the center of the knob image to display.
        const float centerY = y + height * 0.5f;
        // The horizontal position of the top left corner of the knob image to display.
        const float rx = centerX - radius - 1.0f;
        // The vertical position of the top left corner of the knob image to display.
        const float ry = centerY - radius;

        // Render the knob sprite to the GUI at the corrent offset.
        g.juce::Graphics::drawImage(img1, (int)rx, (int)ry, 2 * (int)radius, 2 * (int)radius, 0,
            frameID * img1.getWidth(), img1.getWidth(), img1.getWidth());

    }
    else { // Otherwise display some simple error graphics.
        // Display an error on the VST interface if the graphics file
        // is missing or corrupted.

        // The percentage of the width that the dial would be to render the error text into.
        static const float textPpercent = 0.35f;
        // A rectangle to render the error text into.
        juce::Rectangle<float> text_bounds(1.0f + width * (1.0f - textPpercent) / 2.0f, 0.5f * height, width * textPpercent, 0.5f * height);

        // Set the color of the error text and render it into the given bounding box.
        g.setColour(juce::Colours::white);
        g.juce::Graphics::drawFittedText(juce::String("NOIMG"),
            text_bounds.getSmallestIntegerContainer(), juce::Justification::horizontallyCentred | juce::Justification::centred, 1);

    }
}
