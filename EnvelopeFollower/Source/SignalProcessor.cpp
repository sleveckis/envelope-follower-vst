/*
  ==============================================================================

    SignalProcessor.cpp
    Created: 19 Feb 2022 2:29pm
    Last Updated: 5 Mar 2022 9:00pm PST
    Authors:
    - Arden Butterfield
    - Patrick Thomasma
    - Ethan Killen (documentation)

    Description: Contains the implementation of the SignalProcessor component class.
    Dependencies:
    - SignalProcessor.h

  ==============================================================================
*/

// Import the dependencies for the contents of this file.
#include "SignalProcessor.h" // Import the interface definition for the SignalProcessor component for implementation.

/**
 * The constructor for the SignalProcessor component.
 *
 * Sets the default values for the audio processing parameters.
 */
SignalProcessor::SignalProcessor()
{
    // Sets the default values for the coefficents
    min_val = 0.0;
    max_val = 127.0;
    gain = 1.0;
    current_envelope_position = 0.0;
    sampling_frequency = 44100;
}

/**
 * Updates the tentative MIDI output value from a new input audio sample.
 *
 * Rescales and filters the input audio sample before updating the output value.
 *
 * Arguments
 * ---------
 * double sample: The input audio sample to update the output value
 */
void SignalProcessor::takeInSample(double sample)
{
    // Scale the input audio sample.
    sample *= gain;
    // Apply a lowpass and highpass filter to the input audio sample.
    sample = lowFilter.calculate_lpf(sample);
    sample = highFilter.calculate_hpf(sample);
    // Update the tentative MIDI output sample.
    updateEnvelopePosition(sample);
}

/**
 * Updates the value of the output MIDI messages given an input audio sample.
 *
 * Sets the output sample to the maximum of the input audio value and the decayed output value.
 *
 * Arguments
 * ---------
 * float sample: The audio sample used to update the output MIDI value.
 */
void SignalProcessor::updateEnvelopePosition(float sample)
{
    // Decay the tentative output MIDI value.
    current_envelope_position *= decay;
    // Increases the MIDI output value up to the input audio sample.
    if (abs(sample) > current_envelope_position) {
        current_envelope_position = abs(sample);
    }
}

/**
 * Gets the next output MIDI value.
 *
 * This is the rolling envelope value rescaled and clamped between the minimum and maximum output bounds.
 *
 * Returns
 * -------
 * int: The next output MIDI value.
 */
int SignalProcessor::getEnvelopePosition()
{
    // Before scaling, the envelope position is between 0 and 1, since that's what
    // the audio values are between. TODO: I think?
    // Scales the tentative ouput MIDI value  
    float scaled_envelope_position = current_envelope_position * (max_val - min_val) + min_val;
    // The actual minimum output MIDI value given the selected bounds.
    int low_bound = std::min((int)max_val, (int)min_val);
    // The actual maximum output MIDI value given the selected bounds 
    int high_bound = std::max((int)min_val, (int)max_val);
    // Returns the scaled output MIDI value clamped between the minimum and maximum output values.
    return std::max(std::min((int)scaled_envelope_position, high_bound), low_bound);
}

/**
 * Sets the minimum output MIDI value.
 *
 * Arguments
 * ---------
 * float min: The new minimum output MIDI value.
 */
void SignalProcessor::setMinValue(float new_min)
{
    // Update the relevant private parameter.
    min_val = new_min;
}

/**
 * Sets the maximum output MIDI value.
 *
 * Arguments
 * ---------
 * float max: The new maximum output MIDI value.
 */
void SignalProcessor::setMaxValue(float new_max)
{
    // Update the relevant private parameter.
    max_val = new_max;
}

/**
 * Sets the scaling factor applied to all input audio samples.
 *
 * Arguments
 * ---------
 * float min: The new scaling factor.
 */
void SignalProcessor::setGainValue(float new_gain)
{
    // Update the relevant private parameter.
    gain = new_gain;
}

/**
 * Sets the maximum input audio frequency this
 *
 * Relays the update to the internal lowpass filter.
 *
 * Arguments
 * ---------
 * float lp_val: The new cutoff threshold for the internal lowpass filter.
 */
void SignalProcessor::setLowpassValue(float lp_val)
{
    // Update the cutoff frequency threshold of the internal lowpass filter.
    lowFilter.set_cutoff_frequency(lp_val);
}

/**
 * Sets the maximum input audio frequency this
 *
 * Relays the update to the internal highpass filter.
 *
 * Arguments
 * ---------
 * float hp_val: The new cutoff threshold for the internal highpass filter.
 */
void SignalProcessor::setHighpassValue(float hp_val)
{
    // Update the cutoff frequency threshold of the internal highpass filter.
    highFilter.set_cutoff_frequency(hp_val);
}

/**
 * Sets the amount of time the envelope should take to decay to half of its value given lesser input samples.
 *
 * The recovery time is ensured to be at least 1ms.
 *
 * Arguments
 * ---------
 * float recovery_time: The amount of time the envelope should take to decay to half of its value given lesser input samples in seconds.
 */
void SignalProcessor::setRecoveryTimeValue(float recovery_time)
{
    // The recovery time is how long it takes the envelope to decay to half
    // of its original value, and is calculated as such:
    // decay ^ num_samples = 0.5
    // num_samples * log2(decay) = log2(0.5) = -1
    // decay = 2 ^ (-1 / num_samples)
    
    // If the recovery time is 0, there is effectively no envelope, so we set
    // a lower bound for the recovery time. This lower bound is 1 millisecond.
    recovery_time = fmax(recovery_time, 0.001);
    
    // The number of samples that this component should expect per recovery_time interval.
    float num_samples = recovery_time * sampling_frequency;
    // Update the decay scaling constant.
    decay = pow(2, (-1 / num_samples));
}

/**
 * Sets the number of input audio samples this component should expect per second of audio input.
 *
 * Arguments
 * ---------
 * double freq: The new number of input audio samples this component should expect per second of audio input.
 */
void SignalProcessor::setSamplingFrequency(double freq)
{
    // Update the cached sampling frequency.
    sampling_frequency = freq;
    // Update the sampling frequencies for the internal lowpass and highpass filters.
    lowFilter.set_sampling_frequency(freq);
    highFilter.set_sampling_frequency(freq);
}
