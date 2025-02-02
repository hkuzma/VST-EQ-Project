/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SimpleEQAudioProcessor::SimpleEQAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

SimpleEQAudioProcessor::~SimpleEQAudioProcessor()
{
}

//==============================================================================
const juce::String SimpleEQAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SimpleEQAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SimpleEQAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SimpleEQAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SimpleEQAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SimpleEQAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SimpleEQAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SimpleEQAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SimpleEQAudioProcessor::getProgramName (int index)
{
    return {};
}

void SimpleEQAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SimpleEQAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..


    //HENRY
    //Object to be passed to filter chains
    juce::dsp::ProcessSpec spec;

    //maximum number of samples processed at a time
    spec.maximumBlockSize = samplesPerBlock;

    //number of channels
    spec.numChannels = 1;

    //Sample Rate
    spec.sampleRate = sampleRate;

    leftChain.prepare(spec);
    rightChain.prepare(spec);

    //Call Updater for param values
    auto chainSettings = getChainSettings(apvts);
    //Have to convert gain from decibels to "Gain Units"
    //========================================================================================================================================

    auto lfCoefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, 
                                                                              chainSettings.lfFreq, 
                                                                              chainSettings.lfQuality,
                                                                              juce::Decibels::decibelsToGain(chainSettings.lfGainInDecibels));
    //Copy values from lf Coefficients object -- Wrapper around array allocated on the heap
   //Allocation on the heap is not good for Audio software --> WHY???
    *leftChain.get<ChainPositions::LF>().coefficients = *lfCoefficients;
    *rightChain.get<ChainPositions::LF>().coefficients = *lfCoefficients;


    auto lmCoefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, 
                                                                              chainSettings.lmFreq, 
                                                                              chainSettings.lmQuality,
                                                                              juce::Decibels::decibelsToGain(chainSettings.lmGainInDecibels));
    *leftChain.get<ChainPositions::LM>().coefficients = *lmCoefficients;
    *rightChain.get<ChainPositions::LM>().coefficients = *lmCoefficients;

    auto mCoefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, 
                                                                             chainSettings.mFreq, 
                                                                             chainSettings.mQuality,
                                                                             juce::Decibels::decibelsToGain(chainSettings.mGainInDecibels));
    *leftChain.get<ChainPositions::M>().coefficients = *mCoefficients;
    *rightChain.get<ChainPositions::M>().coefficients = *mCoefficients;

    auto hmCoefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, 
                                                                              chainSettings.hmFreq, 
                                                                              chainSettings.hmQuality,
                                                                              juce::Decibels::decibelsToGain(chainSettings.hmGainInDecibels));
    *leftChain.get<ChainPositions::HM>().coefficients = *hmCoefficients;
    *rightChain.get<ChainPositions::HM>().coefficients = *hmCoefficients;

    auto hfCoefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, 
                                                                               chainSettings.hfFreq, 
                                                                               chainSettings.hfQuality,
                                                                               juce::Decibels::decibelsToGain(chainSettings.hfGainInDecibels));
    *leftChain.get<ChainPositions::HF>().coefficients = *hfCoefficients;
    *rightChain.get<ChainPositions::HF>().coefficients = *hfCoefficients;

}

void SimpleEQAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SimpleEQAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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
#endif

void SimpleEQAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
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

    //ALWAYS UPDATE PARAMETERS BEFORE YOU RUN AUDIO THROUGH IT
    //=======================================================================================================================

    auto chainSettings = getChainSettings(apvts);

    auto lfCoefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(),
                                                                              chainSettings.lfFreq,
                                                                              chainSettings.lfQuality,
                                                                              juce::Decibels::decibelsToGain(chainSettings.lfGainInDecibels));
    //Copy values from lf Coefficients object -- Wrapper around array allocated on the heap
   //Allocation on the heap is not good for Audio software --> WHY???
    *leftChain.get<ChainPositions::LF>().coefficients = *lfCoefficients;
    *rightChain.get<ChainPositions::LF>().coefficients = *lfCoefficients;


    auto lmCoefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(),
                                                                              chainSettings.lmFreq,
                                                                              chainSettings.lmQuality,
                                                                              juce::Decibels::decibelsToGain(chainSettings.lmGainInDecibels));
    *leftChain.get<ChainPositions::LM>().coefficients = *lmCoefficients;
    *rightChain.get<ChainPositions::LM>().coefficients = *lmCoefficients;

    auto mCoefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(),
                                                                             chainSettings.mFreq,
                                                                             chainSettings.mQuality,
                                                                             juce::Decibels::decibelsToGain(chainSettings.mGainInDecibels));
    *leftChain.get<ChainPositions::M>().coefficients = *mCoefficients;
    *rightChain.get<ChainPositions::M>().coefficients = *mCoefficients;

    auto hmCoefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(),
                                                                              chainSettings.hmFreq,
                                                                              chainSettings.hmQuality,
                                                                              juce::Decibels::decibelsToGain(chainSettings.hmGainInDecibels));
    *leftChain.get<ChainPositions::HM>().coefficients = *hmCoefficients;
    *rightChain.get<ChainPositions::HM>().coefficients = *hmCoefficients;

    auto hfCoefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(),
                                                                              chainSettings.hfFreq,
                                                                              chainSettings.hfQuality,
                                                                              juce::Decibels::decibelsToGain(chainSettings.hfGainInDecibels));
    *leftChain.get<ChainPositions::HF>().coefficients = *hfCoefficients;
    *rightChain.get<ChainPositions::HF>().coefficients = *hfCoefficients;


    //=======================================================================================================================

    //HENRY
    //Extract Left and Right channels from buffer
    juce::dsp::AudioBlock<float> block(buffer);

    //Left channel is channel 0 and Right Channel is 1
    auto leftBlock = block.getSingleChannelBlock(0);
    auto rightBlock = block.getSingleChannelBlock(1);

    //Create context for audio processing
    juce::dsp::ProcessContextReplacing<float> leftContext(leftBlock);
    juce::dsp::ProcessContextReplacing<float> rightContext(rightBlock); 

    leftChain.process(leftContext);
    rightChain.process(rightContext);

}

//==============================================================================
bool SimpleEQAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SimpleEQAudioProcessor::createEditor()
{
    //return new SimpleEQAudioProcessorEditor (*this);

    return new juce::GenericAudioProcessorEditor(*this);  //HENRY

}

//==============================================================================
void SimpleEQAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void SimpleEQAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//helper function for data structure storign param settings
//Update Values for each parameter
ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts) {
    
    ChainSettings settings;

    //Returns a normalized value, we need a real world value
        /*apvts.getParameter("LowCut Freq")->getValue();*/

    //returns units in correctly formatted range
    settings.lowCutFreq = apvts.getRawParameterValue("LowCut Freq")->load();
    settings.lowCutSlope = apvts.getRawParameterValue("LowCut Slope")->load();
    

    settings.highCutFreq = apvts.getRawParameterValue("HighCut Freq")->load();  
    settings.highCutSlope = apvts.getRawParameterValue("HighCut Slope")->load();



    settings.lfFreq = apvts.getRawParameterValue("LF")->load();
    settings.lfGainInDecibels = apvts.getRawParameterValue("LF Gain")->load();
    settings.lfQuality = apvts.getRawParameterValue("LF Q")->load();

    settings.lmFreq = apvts.getRawParameterValue("LM")->load();
    settings.lmGainInDecibels = apvts.getRawParameterValue("LM Gain")->load();
    settings.lmQuality = apvts.getRawParameterValue("LM Q")->load();

    settings.mFreq = apvts.getRawParameterValue("M")->load();
    settings.mGainInDecibels = apvts.getRawParameterValue("M Gain")->load();
    settings.mQuality = apvts.getRawParameterValue("M Q")->load();

    settings.hmFreq = apvts.getRawParameterValue("HM")->load();
    settings.hmGainInDecibels = apvts.getRawParameterValue("HM Gain")->load();
    settings.hmQuality = apvts.getRawParameterValue("HM Q")->load();

    settings.hfFreq = apvts.getRawParameterValue("HF")->load();
    settings.hfGainInDecibels = apvts.getRawParameterValue("HF Gain")->load();
    settings.hfQuality = apvts.getRawParameterValue("HF Q")->load();



    return settings;

}


//DECLARATION FOR PARAMETER LAYOUT -- HENRY
//PARAMETERS TO LAYOUT -- LOW CUT, LF, LM, M, HM, HF, HIGH CUT
//FUNCTIONS OF PARAMS -- LOW CUT, HIGH CUT { ADJUSTABLE FREQUENCY AND SLOPE} LF, LM, M, HM, H { ADJUSTABLE FREQUENCY, GAIN, Q}
//DOCUMENTATION INFO @ 19:43 in video
juce::AudioProcessorValueTreeState::ParameterLayout SimpleEQAudioProcessor::createParameterLayout() {

    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    //LOW CUT
    //AudioParameterFloat Parameters: ParameterID (string), ParameterName (string), 
        //normalisableRange (NormalisableRange<float>), defaultValue (float)
        
    //NormalisableRange takes a series of parameters: Low Frequency(float), High Frequency(float), Step-Size(float), 
        //Skew Factor (float) -- alters distribution of slider
        // 
        // 
    //=============================================================================================================
    layout.add(std::make_unique<juce::AudioParameterFloat>("LowCut Freq", 
                                                           "LowCut Freq", 
                                                           juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 1.f), 
                                                           20.f));

    //HIGH CUT
    //=============================================================================================================
    layout.add(std::make_unique<juce::AudioParameterFloat>("HighCut Freq",
                                                           "HighCut Freq", 
                                                           juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 1.f), 
                                                           20000.f));
    //Peaks
    //=============================================================================================================
    layout.add(std::make_unique<juce::AudioParameterFloat>("LF",
                                                           "LF", 
                                                           juce::NormalisableRange<float>(20.f, 20000.f, 1.f, .2f), 
                                                           100.f));
   //TESTING CREATING A CHANGEABLE SKEW
    juce::NormalisableRange<float> EQRange{ 20.f, 20000.f};
    EQRange.setSkewForCentre(200.f);

    layout.add(std::make_unique<juce::AudioParameterFloat>("LM",
                                                           "LM",
                                                           juce::NormalisableRange<float>(20.f, 20000.f, 1.f, .2f),
                                                           250.f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("M",
                                                           "M",
                                                           juce::NormalisableRange<float>(20.f, 20000.f, 1.f, .2f),
                                                           1000.f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("HM",
                                                           "HM",
                                                           juce::NormalisableRange<float>(20.f, 20000.f, 1.f, .2f),
                                                           2500.f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("HF",
                                                           "HF",
                                                           juce::NormalisableRange<float>(20.f, 20000.f, 1.f, .2f),
                                                           7500.f));
    
    //GAIN
    //A GOOD RANGE FOR GAIN IS -24 --> 24db
    //=============================================================================================================
    layout.add(std::make_unique<juce::AudioParameterFloat>("LF Gain",
                                                           "LF Gain", 
                                                           juce::NormalisableRange<float>(-24.f, 24.f, 0.5f, 1.f), 
                                                           0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("LM Gain",
                                                           "LM Gain",
                                                           juce::NormalisableRange<float>(-24.f, 24.f, 0.5f, 1.f),
                                                           0.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("M Gain",
                                                           "M Gain",
                                                           juce::NormalisableRange<float>(-24.f, 24.f, 0.5f, 1.f),
                                                           0.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("HM Gain",
                                                           "HM Gain",
                                                           juce::NormalisableRange<float>(-24.f, 24.f, 0.5f, 1.f),
                                                           0.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("HF Gain",
                                                           "HF Gain",
                                                           juce::NormalisableRange<float>(-24.f, 24.f, 0.5f, 1.f),
                                                           0.0f));

    //Q
    //=============================================================================================================
    layout.add(std::make_unique<juce::AudioParameterFloat>("LF Q",
                                                           "LF Q", 
                                                           juce::NormalisableRange<float>(0.1, 10, 0.05f, 1.f), 
                                                           1.f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("LM Q",
                                                           "LM Q",
                                                           juce::NormalisableRange<float>(0.1, 10, 0.05f, 1.f),
                                                           1.f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("M Q",
                                                           "M Q",
                                                           juce::NormalisableRange<float>(0.1, 10, 0.05f, 1.f),
                                                           1.f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("HM Q",
                                                           "HM Q",
                                                           juce::NormalisableRange<float>(0.1, 10, 0.05f, 1.f),
                                                           1.f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("HF Q",
                                                           "HF Q",
                                                           juce::NormalisableRange<float>(0.1, 10, 0.05f, 1.f),
                                                           1.f));

    //STEEPNESS
    //Expressed in DB/Octave: 12db/octave, 24db/octave, 36db/octave etc :: equation expressed in multiples of 6 or 12
    //Using 12, 24, 36, 48

    //AUDIOPARAMETER CHOICE PARAMS: parameterID (string), parameterName(string), choices(StringArray), defaultItemIndex(int)
    //=======================================================================================================================

    juce::StringArray stringArray;

    //Construct 4 strings to place in array
    //Used for Dropdown menu for HIGH AND LOW CUT
    for (int i = 0; i < 4; i++) {
        juce::String str;
        str << (12 + i*12);
        str << " db/Oct";
        stringArray.add(str);
    }

    //LOWCUT OPTIONS
    layout.add(std::make_unique <juce::AudioParameterChoice>("LowCut Slope", "LowCut Slope", stringArray, 0));

    //HIGHCUT OPTIONS
    layout.add(std::make_unique <juce::AudioParameterChoice>("HighCut Slope", "HighCut Slope", stringArray, 0));
    






    return layout;


}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SimpleEQAudioProcessor();
}
