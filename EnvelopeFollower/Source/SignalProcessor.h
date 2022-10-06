/*
  ==============================================================================

    SignalProcessor.h
    Created: 19 Feb 2022 2:29pm
    Last Updated: 5 Mar 2022 9:00pm PST
    Authors:
    - Arden Butterfield
    - Patrick Thomasma
    - Ethan Killen (documentation)

    Description: Contains the API definition for the SignalProcessor component class and the implementation of the Filter component classes.
    Dependencies:
    - algorithm
    - math.h

  ==============================================================================
*/

// Import the dependencies for the contents of this file.
#pragma once

// Import the dependencies for the contents of this file.
#include <algorithm> // Imports the c++ stdlib algorithm libarary.
#include <math.h> // Imports the basic c stdlib math library

/**
 * A biquad lowpass and highpass filter.
 * 
 * Functionality is based off of the equations in
 * - https://www.st.com/resource/en/application_note/an2874-bqd-filter-design-equations-stmicroelectronics.pdf
 * - https://en.wikipedia.org/wiki/Digital_biquad_filter
 * 
 * 
 * Attributes
 * ----------
 * private const double pi: An approximation of the irrational consant pi used for calculating the filter coefficients.
 * private double prev_input: The previous input audio sample value. Defaults to 0 before input.
 * private double prev_output: The previous output audio sample value. Defaults to 0 before input.
 * private double cutoff_frequency: The audio frequency used as the cutoff threshold by the lowpass and highpass filter calculations.
 * private double sampling_frequency: The number of audio samples this filter should expect per second of audio input.
 * private double theta_c: An angle used to calculate the other filter coefficients.
 * private double k: Part of a scaling factor applied to the previous output value during filter calculations.
 * private double alpha: A divisor used to scale all input values for the filter calculations.
 * 
 * Methods
 * -------
 * public void set_sampling_frequency(double new_freq): Sets the number of audio samples this filter should expect per second of audio input.
 * public void set_cutoff_frequency(double new_freq): Sets the audio frequency used as the cutoff threshold by the lowpass and highpass filter calculations.
 * public double calculate_lpf(double new_sample): Calculates and returns the next output value as a lowpass filter.
 * public double calculate_hpf(double new_sample): Calculates and returns the next output value as a highpass filter.
 * public void calc_coeff(): Updates the coefficients used for the lowpass and highpass filter calculations.
 * 
 * Owned by
 * - SignalProcessor
 */
struct Filter {
    /**
     * Sets the number of input samples this filer expects to receive per second of audio stream.
     * 
     * Arguments
     * ---------
     * double new freq: The new number of input samples this filter should expect to recieve per second of input audio stream.
     */
    void set_sampling_frequency(double new_freq) {
        // Sets the input sample frequency for this filter.
        sampling_frequency = new_freq;
        // Updates the coefficients used by the filter.
        calc_coeff();
    };

    /**
     * Sets the cutoff frequency threshold for the lowpass and highpass filter iteration.
     * 
     * Input frequencies above or below this will be removed from the output depending on operating mode.
     * 
     * Arguments
     * ---------
     * double new freq: The new cutoff frequency threshold for this filter.
     */
    void set_cutoff_frequency(double new_freq) {
        // Sets the cutoff frequency threshold for this filter.
        cutoff_frequency = new_freq;
        // Updates the coefficients used by the filter.
        calc_coeff();
    };

    /**
     * Returns the next output value for a low pass filter given a new input sample.
     * 
     * Arguments
     * ---------
     * double new_sample: The next input value.
     * 
     * Returns
     * -------
     * double: The next output value for a high pass filter. 
     */
    double calculate_lpf(double new_sample) {
        // The inverse scaling coefficient for the new output value.
        double a0 = 1.0;
        // The scaling coefficient for the previous output value.
        double a1 = -(1.0 - k) / alpha;
        // The scaling coefficent for the new input value.
        double b0 = k / alpha;
        // The scaling coefficient for the prevous input value.
        double b1 = k / alpha;

        // The new output value.
        double output = (1 / a0) * (b0 * new_sample + b1 * prev_input - a1 * prev_output);
        prev_input = new_sample; // Update the previous input and output values.
        prev_output = output;
        return output; // Return the new output value.
    };

    /**
     * Returns the next output value for a high pass filter given the next input value. 
     * 
     * Arguments
     * ---------
     * double new_sample: The next input value.
     * 
     * Returns
     * -------
     * double: The next output value for a low pass filter. 
     */
    double calculate_hpf(double new_sample) {
        // The inverse scaling coefficient for the new output value.
        double a0 = 1.0;
        // The scaling coefficient for the previous output value.
        double a1 = -(1.0 - k) / alpha;
        // The scaling coefficent for the new input value.
        double b0 = 1.0 / alpha;
        // The scaling coefficient for the prevous input value.
        double b1 = -1.0 / alpha;

        // The new output value.
        double output = (1 / a0) * (b0 * new_sample + b1 * prev_input - a1 * prev_output);
        prev_input = new_sample; // Update the previous input and output values.
        prev_output = output;
        return output; // Return the new output value.
    }

    /**
     * Updates the coefficients used to calculate the output values.
     */
    void calc_coeff() {
        // Recalculate the filter coefficients from the cutoff and sampling frequencies.
        theta_c = 2.0 * pi * cutoff_frequency / sampling_frequency;
        k = tan(theta_c / 2.0);
        alpha = 1 + k;
    };
    
private:
    /// <summary>
    ///     An approximation of the pi constant for usage in calculations.
    /// </summary>
    const double pi = 3.1415926535;

    /// <summary>
    ///     The previous input audio sample value.
    /// </summary>
    double prev_input = 0;
    /// <summary>
    ///     The previous output audio sample value.
    /// </summary>
    double prev_output = 0;

    /// <summary>
    ///     The minimum or maximum allowed frequency depending on whether this is being used as a lowpass or highpass filter.
    /// </summary>
    double cutoff_frequency = 1000;
    /// <summary>
    ///     The predicted number of audio samples input per second.
    /// </summary>
    double sampling_frequency = 44100;

    /// <summary>
    ///     An angle used to calculate the following constants.
    /// </summary>
    double theta_c = 0;
    /// <summary>
    ///     Part of a scaling constant applied to preceding output values when calculating the succeeding output value.
    ///     Also used to calculate Filter::alpha.
    /// </summary>
    double k = 0;
    /// <summary>
    ///     A divisor used to rescale input values when calculating the succeeding output value.
    /// </summary>
    double alpha = 0;
};

/**
 * The component of the plugin responsible for deriving the values of the MIDI envelope from an input audio stream.
 * 
 * Attributes
 * ----------
 * private float current_envelope_position: The current value of the waveform envelope normalized to between 0 and 1.
 * private float min_val: The minimum output MIDI value.
 * private float max_val: The maximum output MIDI value.
 * private float gain: A scaling factor applied to input audio samples.
 * private float sampling_frequency: The number of input audio samples the component expects to receive per second of audio.
 * private float decay: A scaling factory applied to the current_envelope_position whenever it's updated to make it decay over time.
 * private const int MIN_MIDI_VAL: The absolute minimum MIDI output value possible.
 * private const int MAX_MIDI_VAL: The absolute maximum MIDI output value possible.
 * private Filter lowFilter: A lowpass filter used to process input audio samples and filter out high frequencies.
 * private Filter highFilter: A highpass filter used to process input audio samples and filter out low frequencies.
 * 
 * Methods
 * -------
 * public SignalProcessor(): The constructor for this component. Sets up the initial parameter values.
 * public void takeInSample(double sample): Processes a new audio sample and updates the current amplitude of the envelope accordingly.
 * public int getEnvelopePosition(): Returns the current position of the waveform envelope as a valid MIDI value between 0 and 127.
 * public void setMinValue(float min): Sets the minimum possible MIDI output value.
 * public void setMaxValue(float max): Sets the maximum possible MIDI output value.
 * public void setGainValue(float gain): Sets the amplitude scaling coefficent applied to input audio samples before processing.
 * public void setLowpassValue(float gain): Sets the frequency cutoff threshold for the internal lowpass filter.
 * public void setHighpassValue(float gain): Sets the frequency cutoff threshold for the internal highpass filter.
 * public void setRecoveryTimeValue(float gain): Sets the amount of time it takes for the waveform envelope to decay to half its value given sufficiently small input audio samples.
 * public void setSamplingFrequency(float gain): Sets the number of input audio samples the component should expect to receive per second of audio.
 * private void updateEnvelopePosition(float sample): Updates the current amplitude of the waveform envelope given a new audio sample.
 * 
 * Owns
 * - Filter
 * 
 * Owned by
 * - EnvelopeFollowerAudioProcessor
 */
class SignalProcessor
{
public:
    /**
     * The constructor for the SignalProcessor component.
     * 
     * Sets the default values for the audio processing parameters.
     */
    SignalProcessor();

    /**
     * Updates the tentative MIDI output value from a new input audio sample.
     * 
     * Rescales and filters the input audio sample before updating the output value.
     * 
     * Arguments
     * ---------
     * double sample: The input audio sample to update the output value 
     */
    void takeInSample(double sample);

    /**
     * Gets the next output MIDI value.
     * 
     * This is the rolling envelope value rescaled and clamped between the minimum and maximum output bounds.
     * 
     * Returns
     * -------
     * int: The next output MIDI value.
     */
    int getEnvelopePosition();

    /**
     * Sets the minimum output MIDI value.
     * 
     * Arguments
     * ---------
     * float min: The new minimum output MIDI value.
     */
    void setMinValue(float min);

    /**
     * Sets the maximum output MIDI value.
     * 
     * Arguments
     * ---------
     * float max: The new maximum output MIDI value.
     */
    void setMaxValue(float max);

    /**
     * Sets the scaling factor applied to all input audio samples.
     * 
     * Arguments
     * ---------
     * float min: The new scaling factor.
     */
    void setGainValue(float gain);

    /**
     * Sets the maximum input audio frequency this 
     * 
     * Relays the update to the internal lowpass filter.
     * 
     * Arguments
     * ---------
     * float lp_val: The new cutoff threshold for the internal lowpass filter.
     */
    void setLowpassValue(float lp_val);

    /**
     * Sets the maximum input audio frequency this 
     * 
     * Relays the update to the internal highpass filter.
     * 
     * Arguments
     * ---------
     * float hp_val: The new cutoff threshold for the internal highpass filter.
     */
    void setHighpassValue(float hp_val);

    /**
     * Sets the amount of time the envelope should take to decay to half of its value given lesser input samples.
     * 
     * The recovery time is ensured to be at least 1ms.
     * 
     * Arguments
     * ---------
     * float recovery_time: The amount of time the envelope should take to decay to half of its value given lesser input samples in seconds.
     */
    void setRecoveryTimeValue(float recovery_time);

    /**
     * Sets the number of input audio samples this component should expect per second of audio input.
     * 
     * Arguments
     * ---------
     * double freq: The new number of input audio samples this component should expect per second of audio input.
     */
    void setSamplingFrequency(double freq);

private:
    /// <summary>
    ///     The current rolling MIDI output value.
    /// </summary>
    float current_envelope_position;

    /// <summary>
    ///     The minimum MIDI output value.
    /// </summary>
    float min_val;

    /// <summary>
    ///     The maximum MIDI output value.
    /// </summary>
    float max_val;

    /// <summary>
    ///     A scaling coefficient applied to the input audio samples before updating the envelope position.
    /// </summary>
    float gain;

    /// <summary>
    ///     The number of audio samples that this component should expect to recieve for each second of audio input.
    /// </summary>
    double sampling_frequency;

    /// <summary>
    ///     A multiplier applied to the tentative envelope position for every input audio sample.
    ///     The lower this is the faster the tentative envelope position approaches 0 given lower audio input.
    /// </summary>
    float decay = 0.99;

    /// <summary>
    ///     The minimum value of the MIDI output messages.
    /// </summary>
    const int MIN_MIDI_VAL = 0;
    /// <summary>
    ///     The maximum value of the MIDI output messages.
    /// </summary>
    const int MAX_MIDI_VAL = 127;
    
    /// <summary>
    ///     The lowpass filter applied to the input audio samples before updating the MIDI output value.
    /// 
    ///     Disposed of when this component is disposed of.
    /// </summary>
    Filter lowFilter;
    /// <summary>
    ///     The highpass filter applied to the input audio samples before updating the MIDI output value.
    /// 
    ///     Disposed of when this component is disposed of.
    /// </summary>
    Filter highFilter;

    /**
     * Updates the value of the output MIDI messages given an input audio sample.
     *
     * Sets the output sample to the maximum of the input audio value and the decayed output value.
     *
     * Arguments
     * ---------
     * float sample: The audio sample used to update the output MIDI value.
     */
    void updateEnvelopePosition(float sample);
};
