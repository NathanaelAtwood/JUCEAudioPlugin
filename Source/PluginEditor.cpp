#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor& p)
    : AudioProcessorEditor (&p),
    processorRef (p),
    lowCutFreqAttachment (processorRef.apvts, "LowCut Frequency", lowCutFreqSlider),
    highCutFreqAttachment (processorRef.apvts, "HighCut Frequency", highCutFreqSlider),
    peakFreqAttachment (processorRef.apvts, "Peak Frequency", peakFreqSlider),
    peakGainAttachment (processorRef.apvts, "Peak Gain", peakGainSlider),
    peakQualityAttachment (processorRef.apvts, "Peak Quality", peakQualitySlider),
    lowCutSlopeAttachment (processorRef.apvts, "LowCut Slope", lowCutSlopeSlider),
    highCutSlopeAttachment (processorRef.apvts, "HighCut Slope", highCutSlopeSlider)
{
    //LCF
    lowCutFreqLabel.setText(
        "LC Freq", juce::dontSendNotification);
    lowCutFreqLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible (lowCutFreqLabel);
    
    lowCutFreqSlider.setSliderStyle(
        juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    lowCutFreqSlider.setTextBoxStyle(
        juce::Slider::TextBoxBelow, false, 80, 20);
    addAndMakeVisible (lowCutFreqSlider);

    //HCF
    highCutFreqLabel.setText(
        "HC Freq", juce::dontSendNotification);
    highCutFreqLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible (highCutFreqLabel);
    
    highCutFreqSlider.setSliderStyle(
        juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    highCutFreqSlider.setTextBoxStyle(
        juce::Slider::TextBoxBelow, false, 80, 20);
    addAndMakeVisible (highCutFreqSlider);

    //Peak Freq
    peakFreqLabel.setText(
        "Peak Freq", juce::dontSendNotification);
    peakFreqLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible (peakFreqLabel);

    peakFreqSlider.setSliderStyle(
        juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    peakFreqSlider.setTextBoxStyle(
        juce::Slider::TextBoxBelow, false, 80, 20);
    addAndMakeVisible (peakFreqSlider);

    //Peak Gain
    peakGainLabel.setText(
        "Peak Gain", juce::dontSendNotification);
    peakGainLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible (peakGainLabel);
    
    peakGainSlider.setSliderStyle(
        juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    peakGainSlider.setTextBoxStyle(
        juce::Slider::TextBoxBelow, false, 80, 20);
    addAndMakeVisible (peakGainSlider);

    //Peak Q
    peakQualityLabel.setText(
        "Peak Q", juce::dontSendNotification);
    peakQualityLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible (peakQualityLabel);
    peakQualitySlider.setSliderStyle(
        juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    peakQualitySlider.setTextBoxStyle(
        juce::Slider::TextBoxBelow, false, 80, 20);
    addAndMakeVisible (peakQualitySlider);

    //LCQ
    lowCutSlopeLabel.setText(
        "LC Slope", juce::dontSendNotification);
    lowCutSlopeLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible (lowCutSlopeLabel);

    lowCutSlopeSlider.setSliderStyle(
        juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    lowCutSlopeSlider.setTextBoxStyle(
        juce::Slider::TextBoxBelow, false, 80, 20);
    addAndMakeVisible (lowCutSlopeSlider);


    //HCQ
    highCutSlopeLabel.setText(
        "HC Slope", juce::dontSendNotification);
    highCutSlopeLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible (highCutSlopeLabel);
    
    highCutSlopeSlider.setSliderStyle(
        juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    highCutSlopeSlider.setTextBoxStyle(
        juce::Slider::TextBoxBelow, false, 80, 20);
    addAndMakeVisible (highCutSlopeSlider);

    setSize (1000, 600);
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
{
}

//==============================================================================
void AudioPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

}

void AudioPluginAudioProcessorEditor::resized() //random positions for now, not indicative of final layout
{
    lowCutFreqLabel.setBounds (200, 50, 100, 100);
    lowCutFreqSlider.setBounds (200, 100, 100, 100);

    highCutFreqLabel.setBounds (200, 250, 100, 100);
    highCutFreqSlider.setBounds (200, 300, 100, 100);

    peakFreqLabel.setBounds (500, 50, 100, 100);
    peakFreqSlider.setBounds (500, 100, 100, 100);

    peakGainLabel.setBounds (500, 250, 100, 100);
    peakGainSlider.setBounds (500, 300, 100, 100);

    peakQualityLabel.setBounds (500, 450, 100, 100);
    peakQualitySlider.setBounds (500, 500, 100, 100);

    lowCutSlopeLabel.setBounds (800, 50, 100, 100);
    lowCutSlopeSlider.setBounds (800, 100, 100, 100);

    highCutSlopeLabel.setBounds (800, 250, 100, 100);
    highCutSlopeSlider.setBounds (800, 300, 100, 100);

}
