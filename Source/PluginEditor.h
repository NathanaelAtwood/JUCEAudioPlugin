#pragma once

#include "PluginProcessor.h"

//==============================================================================
class AudioPluginAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor&);
    ~AudioPluginAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    AudioPluginAudioProcessor& processorRef;
    
    juce::Label lowCutFreqLabel;
    juce::Slider lowCutFreqSlider;
    juce::AudioProcessorValueTreeState::SliderAttachment
        lowCutFreqAttachment;

    juce::Label highCutFreqLabel;
    juce::Slider highCutFreqSlider;
    juce::AudioProcessorValueTreeState::SliderAttachment
        highCutFreqAttachment;

    juce::Label peakFreqLabel;
    juce::Slider peakFreqSlider;
    juce::AudioProcessorValueTreeState::SliderAttachment
        peakFreqAttachment;

    juce::Label peakGainLabel;
    juce::Slider peakGainSlider;
    juce::AudioProcessorValueTreeState::SliderAttachment
        peakGainAttachment;

    juce::Label peakQualityLabel;
    juce::Slider peakQualitySlider;
    juce::AudioProcessorValueTreeState::SliderAttachment
        peakQualityAttachment;

    juce::Label lowCutSlopeLabel;
    juce::Slider lowCutSlopeSlider;
    juce::AudioProcessorValueTreeState::SliderAttachment
        lowCutSlopeAttachment;

    juce::Label highCutSlopeLabel;
    juce::Slider highCutSlopeSlider;
    juce::AudioProcessorValueTreeState::SliderAttachment
        highCutSlopeAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessorEditor)
};
