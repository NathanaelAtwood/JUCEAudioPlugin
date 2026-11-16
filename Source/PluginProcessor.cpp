#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================

ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts) {
    ChainSettings settings;

    settings.lowCutFreq = apvts.getRawParameterValue("LowCut Frequency")->load();
    settings.highCutFreq = apvts.getRawParameterValue("HighCut Frequency")->load();
    settings.peakFreq = apvts.getRawParameterValue("Peak Frequency")->load();
    settings.peakGainInDecibels = apvts.getRawParameterValue("Peak Gain")->load();
    settings.peakQuality = apvts.getRawParameterValue("Peak Quality")->load();
    settings.lowCutSlope = static_cast<Slope>(apvts.getRawParameterValue("LowCut Slope")->load());
    settings.highCutSlope = static_cast<Slope>(apvts.getRawParameterValue("HighCut Slope")->load());

    return settings;
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout
AudioPluginAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout params;

    //Low cut frequency
    params.add(std::make_unique<juce::AudioParameterFloat> (
    "LowCut Frequency", "LowCut Frequency",
    juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.25f),
    20.0f));

    //High cut frequency
    params.add(std::make_unique<juce::AudioParameterFloat> (
    "HighCut Frequency", "HighCut Frequency",
    juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.25f),
    20000.0f));

    //Peak frequency
    params.add(std::make_unique<juce::AudioParameterFloat> (
        "Peak Frequency", "Peak Frequency",
        juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.25f),
        1000.0f));

    //Peak gain
    params.add(std::make_unique<juce::AudioParameterFloat> (
        "Peak Gain", "Peak Gain",
        juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f),
        0.0f));

    //Peak quality
    params.add(std::make_unique<juce::AudioParameterFloat> (
        "Peak Quality", "Peak Quality",
        juce::NormalisableRange<float>(0.1f, 10.0f, 0.1f, 0.5f),
        1.0f));

    //LowCut Slope Selector
    params.add(std::make_unique<juce::AudioParameterChoice> (
        "LowCut Slope", "LowCut Slope",
        juce::StringArray {"12 dB/oct", "24 dB/oct", "36 dB/oct", "48 dB/oct"}, 0
        ));

    //HighCut Slope Selector
    params.add(std::make_unique<juce::AudioParameterChoice> (
        "HighCut Slope", "HighCut Slope",
        juce::StringArray {"12 dB/oct", "24 dB/oct", "36 dB/oct", "48 dB/oct"}, 0
        ));

    return params;

}

//==============================================================================
AudioPluginAudioProcessor::AudioPluginAudioProcessor() // Constructor
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
{

}

AudioPluginAudioProcessor::~AudioPluginAudioProcessor()  // Destructor
{

}

//==============================================================================
//Everything below here is defined by CMakeLists.txt
const juce::String AudioPluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AudioPluginAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool AudioPluginAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool AudioPluginAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}
// ================================================================
double AudioPluginAudioProcessor::getTailLengthSeconds() const // Tail length is amount of time plugin continues after stopping playback (e.x. reverb tails)
{
    return 0.0;
}

int AudioPluginAudioProcessor::getNumPrograms() //Not sure but these sound like they are presets of some sort =====================
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int AudioPluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AudioPluginAudioProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String AudioPluginAudioProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void AudioPluginAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

//==============================================================================
void AudioPluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock) //IMPORTANT called prepare,reset, or preparetoplay
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..

    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = 1;
    spec.sampleRate = sampleRate;

    leftChain.prepare(spec);
    rightChain.prepare(spec);

    auto chainSettings = getChainSettings(apvts);

    auto peakCoefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, chainSettings.peakFreq, chainSettings.peakQuality,
        juce::Decibels::decibelsToGain(chainSettings.peakGainInDecibels));

    *leftChain.get<ChainPositions::Peak>().coefficients = *peakCoefficients;
    *rightChain.get<ChainPositions::Peak>().coefficients = *peakCoefficients;

    auto lowCutCoefficients = juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(chainSettings.lowCutFreq, sampleRate,
        (chainSettings.lowCutSlope + 1) * 2); //Filter order 2 for 12db/oct, 4 for 24 db/oct, etc.
    
    auto highCutCoefficients = juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod(chainSettings.highCutFreq, sampleRate,
        (chainSettings.highCutSlope + 1) * 2);
    //TODO: Refactor low/highcut coefficients
    
    //==============================================================================
    //Low Cut coefficient setup
    auto& leftLowCut = leftChain.get<ChainPositions::LowCut>();
    
    leftLowCut.setBypassed<0>(true);
    leftLowCut.setBypassed<1>(true);
    leftLowCut.setBypassed<2>(true);
    leftLowCut.setBypassed<3>(true);

    //1 coefficient per 2 * n order, unbypass accordingly
    switch (chainSettings.lowCutSlope) {
        case Slope_12:
            *leftLowCut.get<0>().coefficients = *lowCutCoefficients[0];
            leftLowCut.setBypassed<0>(false);
            break;
        case Slope_24:
            *leftLowCut.get<0>().coefficients = *lowCutCoefficients[0];
            leftLowCut.setBypassed<0>(false);
            *leftLowCut.get<1>().coefficients = *lowCutCoefficients[1];
            leftLowCut.setBypassed<1>(false);
            break;
        case Slope_36:
            leftLowCut.get<0>().coefficients = *lowCutCoefficients[0];
            leftLowCut.setBypassed<0>(false);
            *leftLowCut.get<1>().coefficients = *lowCutCoefficients[1];
            leftLowCut.setBypassed<1>(false);
            *leftLowCut.get<2>().coefficients = *lowCutCoefficients[2];
            leftLowCut.setBypassed<2>(false);
            break;
        case Slope_48:
            leftLowCut.get<0>().coefficients = *lowCutCoefficients[0];
            leftLowCut.setBypassed<0>(false);
            *leftLowCut.get<1>().coefficients = *lowCutCoefficients[1];
            leftLowCut.setBypassed<1>(false);
            *leftLowCut.get<2>().coefficients = *lowCutCoefficients[2];
            leftLowCut.setBypassed<2>(false);
            *leftLowCut.get<3>().coefficients = *lowCutCoefficients[3];
            leftLowCut.setBypassed<3>(false);
            break;

    }

    auto& rightLowCut = rightChain.get<ChainPositions::LowCut>();

    rightLowCut.setBypassed<0>(true);
    rightLowCut.setBypassed<1>(true);
    rightLowCut.setBypassed<2>(true);
    rightLowCut.setBypassed<3>(true);

    //1 coefficient per 2 * n order, unbypass accordingly
    switch (chainSettings.lowCutSlope) {
        case Slope_12:
            *rightLowCut.get<0>().coefficients = *lowCutCoefficients[0];
            rightLowCut.setBypassed<0>(false);
            break;
        case Slope_24:
            *rightLowCut.get<0>().coefficients = *lowCutCoefficients[0];
            rightLowCut.setBypassed<0>(false);
            *rightLowCut.get<1>().coefficients = *lowCutCoefficients[1];
            rightLowCut.setBypassed<1>(false);
            break;
        case Slope_36:
            rightLowCut.get<0>().coefficients = *lowCutCoefficients[0];
            rightLowCut.setBypassed<0>(false);
            *rightLowCut.get<1>().coefficients = *lowCutCoefficients[1];
            rightLowCut.setBypassed<1>(false);
            *rightLowCut.get<2>().coefficients = *lowCutCoefficients[2];
            rightLowCut.setBypassed<2>(false);
            break;
        case Slope_48:
            rightLowCut.get<0>().coefficients = *lowCutCoefficients[0];
            rightLowCut.setBypassed<0>(false);
            *rightLowCut.get<1>().coefficients = *lowCutCoefficients[1];
            rightLowCut.setBypassed<1>(false);
            *rightLowCut.get<2>().coefficients = *lowCutCoefficients[2];
            rightLowCut.setBypassed<2>(false);
            *rightLowCut.get<3>().coefficients = *lowCutCoefficients[3];
            rightLowCut.setBypassed<3>(false);
            break;

    }

    //==============================================================================
    //High Cut coefficient setup

    auto& leftHighCut = leftChain.get<ChainPositions::HighCut>();
    
    leftHighCut.setBypassed<0>(true);
    leftHighCut.setBypassed<1>(true);
    leftHighCut.setBypassed<2>(true);
    leftHighCut.setBypassed<3>(true);

    //1 coefficient per 2 * n order, unbypass accordingly
    switch (chainSettings.highCutSlope) {
        case Slope_12:
            *leftHighCut.get<0>().coefficients = *highCutCoefficients[0];
            leftHighCut.setBypassed<0>(false);
            break;
        case Slope_24:
            *leftHighCut.get<0>().coefficients = *highCutCoefficients[0];
            leftHighCut.setBypassed<0>(false);
            *leftHighCut.get<1>().coefficients = *highCutCoefficients[1];
            leftHighCut.setBypassed<1>(false);
            break;
        case Slope_36:
            *leftHighCut.get<0>().coefficients = *highCutCoefficients[0];
            leftHighCut.setBypassed<0>(false);
            *leftHighCut.get<1>().coefficients = *highCutCoefficients[1];
            leftHighCut.setBypassed<1>(false);
            *leftHighCut.get<2>().coefficients = *highCutCoefficients[2];
            leftHighCut.setBypassed<2>(false);
            break;
        case Slope_48:
            *leftHighCut.get<0>().coefficients = *highCutCoefficients[0];
            leftHighCut.setBypassed<0>(false);
            *leftHighCut.get<1>().coefficients = *highCutCoefficients[1];
            leftHighCut.setBypassed<1>(false);
            *leftHighCut.get<2>().coefficients = *highCutCoefficients[2];
            leftHighCut.setBypassed<2>(false);
            *leftHighCut.get<3>().coefficients = *highCutCoefficients[3];
            leftHighCut.setBypassed<3>(false);
            break;

    }

    auto& rightHighCut = rightChain.get<ChainPositions::HighCut>();

    rightHighCut.setBypassed<0>(true);
    rightHighCut.setBypassed<1>(true);
    rightHighCut.setBypassed<2>(true);
    rightHighCut.setBypassed<3>(true);

    //1 coefficient per 2 * n order, unbypass accordingly
    switch (chainSettings.highCutSlope) {
        case Slope_12:
            *rightHighCut.get<0>().coefficients = *highCutCoefficients[0];
            rightHighCut.setBypassed<0>(false);
            break;
        case Slope_24:
            *rightHighCut.get<0>().coefficients = *highCutCoefficients[0];
            rightHighCut.setBypassed<0>(false);
            *rightHighCut.get<1>().coefficients = *highCutCoefficients[1];
            rightHighCut.setBypassed<1>(false);
            break;
        case Slope_36:
            *rightHighCut.get<0>().coefficients = *highCutCoefficients[0];
            rightHighCut.setBypassed<0>(false);
            *rightHighCut.get<1>().coefficients = *highCutCoefficients[1];
            rightHighCut.setBypassed<1>(false);
            *rightHighCut.get<2>().coefficients = *highCutCoefficients[2];
            rightHighCut.setBypassed<2>(false);
            break;
        case Slope_48:
            *rightHighCut.get<0>().coefficients = *highCutCoefficients[0];
            rightHighCut.setBypassed<0>(false);
            *rightHighCut.get<1>().coefficients = *highCutCoefficients[1];
            rightHighCut.setBypassed<1>(false);
            *rightHighCut.get<2>().coefficients = *highCutCoefficients[2];
            rightHighCut.setBypassed<2>(false);
            *rightHighCut.get<3>().coefficients = *highCutCoefficients[3];
            rightHighCut.setBypassed<3>(false);
            break;

    }
}

void AudioPluginAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool AudioPluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const //daw calls to match config of plugin, ex cant match 5.1 to stereo plugin, returns false
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}

void AudioPluginAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, // this is like a function block, input to function block returns an output
                                              juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);

    juce::ScopedNoDenormals noDenormals; //optimization, Denormals process floating point like .000004, processing gets expensive so noDenormals zeroes those out
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());


    auto chainSettings = getChainSettings(apvts);

    auto peakCoefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(), chainSettings.peakFreq, chainSettings.peakQuality,
        juce::Decibels::decibelsToGain(chainSettings.peakGainInDecibels));

    *leftChain.get<ChainPositions::Peak>().coefficients = *peakCoefficients;
    *rightChain.get<ChainPositions::Peak>().coefficients = *peakCoefficients;

    auto lowCutCoefficients = juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(chainSettings.lowCutFreq, getSampleRate(),
        (chainSettings.lowCutSlope + 1) * 2); //Filter order 2 for 12db/oct, 4 for 24 db/oct, etc.

    auto highCutCoefficients = juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod(chainSettings.highCutFreq, getSampleRate(),
        (chainSettings.highCutSlope + 1) * 2);

    //TODO: Refactor

    auto& leftLowCut = leftChain.get<ChainPositions::LowCut>();

    leftLowCut.setBypassed<0>(true);
    leftLowCut.setBypassed<1>(true);
    leftLowCut.setBypassed<2>(true);
    leftLowCut.setBypassed<3>(true);

    //1 coefficient per 2 * n order, unbypass accordingly
    switch (chainSettings.lowCutSlope) {
        case Slope_12:
            *leftLowCut.get<0>().coefficients = *lowCutCoefficients[0];
            leftLowCut.setBypassed<0>(false);
            break;
        case Slope_24:
            *leftLowCut.get<0>().coefficients = *lowCutCoefficients[0];
            leftLowCut.setBypassed<0>(false);
            *leftLowCut.get<1>().coefficients = *lowCutCoefficients[1];
            leftLowCut.setBypassed<1>(false);
            break;
        case Slope_36:
            *leftLowCut.get<0>().coefficients = *lowCutCoefficients[0];
            leftLowCut.setBypassed<0>(false);
            *leftLowCut.get<1>().coefficients = *lowCutCoefficients[1];
            leftLowCut.setBypassed<1>(false);
            *leftLowCut.get<2>().coefficients = *lowCutCoefficients[2];
            leftLowCut.setBypassed<2>(false);
            break;
        case Slope_48:
            *leftLowCut.get<0>().coefficients = *lowCutCoefficients[0];
            leftLowCut.setBypassed<0>(false);
            *leftLowCut.get<1>().coefficients = *lowCutCoefficients[1];
            leftLowCut.setBypassed<1>(false);
            *leftLowCut.get<2>().coefficients = *lowCutCoefficients[2];
            leftLowCut.setBypassed<2>(false);
            *leftLowCut.get<3>().coefficients = *lowCutCoefficients[3];
            leftLowCut.setBypassed<3>(false);
            break;

    }

    auto& rightLowCut = rightChain.get<ChainPositions::LowCut>();

    rightLowCut.setBypassed<0>(true);
    rightLowCut.setBypassed<1>(true);
    rightLowCut.setBypassed<2>(true);
    rightLowCut.setBypassed<3>(true);

    //1 coefficient per 2 * n order, unbypass accordingly
    switch (chainSettings.lowCutSlope) {
        case Slope_12:
            *rightLowCut.get<0>().coefficients = *lowCutCoefficients[0];
            rightLowCut.setBypassed<0>(false);
            break;
        case Slope_24:
            *rightLowCut.get<0>().coefficients = *lowCutCoefficients[0];
            rightLowCut.setBypassed<0>(false);
            *rightLowCut.get<1>().coefficients = *lowCutCoefficients[1];
            rightLowCut.setBypassed<1>(false);
            break;
        case Slope_36:
            *rightLowCut.get<0>().coefficients = *lowCutCoefficients[0];
            rightLowCut.setBypassed<0>(false);
            *rightLowCut.get<1>().coefficients = *lowCutCoefficients[1];
            rightLowCut.setBypassed<1>(false);
            *rightLowCut.get<2>().coefficients = *lowCutCoefficients[2];
            rightLowCut.setBypassed<2>(false);
            break;
        case Slope_48:
            *rightLowCut.get<0>().coefficients = *lowCutCoefficients[0];
            rightLowCut.setBypassed<0>(false);
            *rightLowCut.get<1>().coefficients = *lowCutCoefficients[1];
            rightLowCut.setBypassed<1>(false);
            *rightLowCut.get<2>().coefficients = *lowCutCoefficients[2];
            rightLowCut.setBypassed<2>(false);
            *rightLowCut.get<3>().coefficients = *lowCutCoefficients[3];
            rightLowCut.setBypassed<3>(false);
            break;
    }

    auto& leftHighCut = leftChain.get<ChainPositions::HighCut>();

    leftHighCut.setBypassed<0>(true);
    leftHighCut.setBypassed<1>(true);
    leftHighCut.setBypassed<2>(true);
    leftHighCut.setBypassed<3>(true);

    //1 coefficient per 2 * n order, unbypass accordingly
    switch (chainSettings.highCutSlope) {
        case Slope_12:
            *leftHighCut.get<0>().coefficients = *highCutCoefficients[0];
            leftHighCut.setBypassed<0>(false);
            break;
        case Slope_24:
            *leftHighCut.get<0>().coefficients = *highCutCoefficients[0];
            leftHighCut.setBypassed<0>(false);
            *leftHighCut.get<1>().coefficients = *highCutCoefficients[1];
            leftHighCut.setBypassed<1>(false);
            break;
        case Slope_36:
            *leftHighCut.get<0>().coefficients = *highCutCoefficients[0];
            leftHighCut.setBypassed<0>(false);
            *leftHighCut.get<1>().coefficients = *highCutCoefficients[1];
            leftHighCut.setBypassed<1>(false);
            *leftHighCut.get<2>().coefficients = *highCutCoefficients[2];
            leftHighCut.setBypassed<2>(false);
            break;
        case Slope_48:
            *leftHighCut.get<0>().coefficients = *highCutCoefficients[0];
            leftHighCut.setBypassed<0>(false);
            *leftHighCut.get<1>().coefficients = *highCutCoefficients[1];
            leftHighCut.setBypassed<1>(false);
            *leftHighCut.get<2>().coefficients = *highCutCoefficients[2];
            leftHighCut.setBypassed<2>(false);
            *leftHighCut.get<3>().coefficients = *highCutCoefficients[3];
            leftHighCut.setBypassed<3>(false);
            break;

    }

    auto& rightHighCut = rightChain.get<ChainPositions::HighCut>();

    rightHighCut.setBypassed<0>(true);
    rightHighCut.setBypassed<1>(true);
    rightHighCut.setBypassed<2>(true);
    rightHighCut.setBypassed<3>(true);

    //1 coefficient per 2 * n order, unbypass accordingly
    switch (chainSettings.highCutSlope) {
        case Slope_12:
            *rightHighCut.get<0>().coefficients = *highCutCoefficients[0];
            rightHighCut.setBypassed<0>(false);
            break;
        case Slope_24:
            *rightHighCut.get<0>().coefficients = *highCutCoefficients[0];
            rightHighCut.setBypassed<0>(false);
            *rightHighCut.get<1>().coefficients = *highCutCoefficients[1];
            rightHighCut.setBypassed<1>(false);
            break;
        case Slope_36:
            *rightHighCut.get<0>().coefficients = *highCutCoefficients[0];
            rightHighCut.setBypassed<0>(false);
            *rightHighCut.get<1>().coefficients = *highCutCoefficients[1];
            rightHighCut.setBypassed<1>(false);
            *rightHighCut.get<2>().coefficients = *highCutCoefficients[2];
            rightHighCut.setBypassed<2>(false);
            break;
        case Slope_48:
            *rightHighCut.get<0>().coefficients = *highCutCoefficients[0];
            rightHighCut.setBypassed<0>(false);
            *rightHighCut.get<1>().coefficients = *highCutCoefficients[1];
            rightHighCut.setBypassed<1>(false);
            *rightHighCut.get<2>().coefficients = *highCutCoefficients[2];
            rightHighCut.setBypassed<2>(false);
            *rightHighCut.get<3>().coefficients = *highCutCoefficients[3];
            rightHighCut.setBypassed<3>(false);
            break;
    }



    juce::dsp::AudioBlock<float> block(buffer);

    auto leftBlock = block.getSingleChannelBlock(0); //audio blocks for each channel
    auto rightBlock = block.getSingleChannelBlock(1);

    juce::dsp::ProcessContextReplacing<float> leftContext(leftBlock); //wrapper around each block
    juce::dsp::ProcessContextReplacing<float> rightContext(rightBlock);

    leftChain.process(leftContext);
    rightChain.process(rightContext);

}

//==============================================================================
bool AudioPluginAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor) // this refers to the graphic interface the user sees
}

juce::AudioProcessorEditor* AudioPluginAudioProcessor::createEditor()
{
    return new AudioPluginAudioProcessorEditor (*this); //used to edit the editor. shocking right
}

//==============================================================================
void AudioPluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    juce::ignoreUnused (destData);
}

void AudioPluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    juce::ignoreUnused (data, sizeInBytes);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AudioPluginAudioProcessor();
}
