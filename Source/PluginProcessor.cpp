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


    //PEAKS
    //========================================================================================================================================
    
    //Call Updater for param values
    auto chainSettings = getChainSettings(apvts);

    updatePeakFilter(chainSettings);

    //HIGH PASS
    //=========================================================================================================================================
    
    //Takes Frequency (float), sampleRate(float), Order // Slope (float)
    //lowCutSlope = 0,1,2, or 3 depending on choice. To convert to order, add 1 and multiply by 2
    auto lowCutCoefficients = juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(chainSettings.lowCutFreq,
        sampleRate,
        2 * (chainSettings.lowCutSlope + 1));
    auto& leftLowCut = leftChain.get<ChainPositions::LowCut>();
    auto& rightLowCut = rightChain.get<ChainPositions::LowCut>();

    updateCutFilter(leftLowCut, lowCutCoefficients, chainSettings.lowCutSlope);
    updateCutFilter(rightLowCut, lowCutCoefficients, chainSettings.lowCutSlope);
    
    //LOWPASS
    //=======================================================================================================================================
    
    auto highCutCoefficients = juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(chainSettings.highCutFreq,
        sampleRate,
        2 * (chainSettings.highCutSlope + 1));

    //L
    auto& leftHighCut = leftChain.get<ChainPositions::LowCut>();
    leftHighCut.setBypassed<0>(true);
    leftHighCut.setBypassed<1>(true);
    leftHighCut.setBypassed<2>(true);
    leftHighCut.setBypassed<3>(true);

    //R
    auto& rightHighCut = rightChain.get<ChainPositions::LowCut>();
    rightHighCut.setBypassed<0>(true); 
    rightHighCut.setBypassed<1>(true); 
    rightHighCut.setBypassed<2>(true); 
    rightHighCut.setBypassed<3>(true); 

    switch (chainSettings.highCutSlope)
    {
        case Slope_12:
        {
            //L
            *leftHighCut.get<0>().coefficients = *highCutCoefficients[0]; 
            leftHighCut.setBypassed<0>(false); 

            //R
            *rightHighCut.get<0>().coefficients = *highCutCoefficients[0]; 
            rightHighCut.setBypassed<0>(false);
            break;
        }
        case Slope_24:
        {
            //L
            *leftHighCut.get<0>().coefficients = *highCutCoefficients[0]; 
            leftHighCut.setBypassed<0>(false);
            *leftHighCut.get<1>().coefficients = *highCutCoefficients[1]; 
            leftHighCut.setBypassed<1>(false);

            //R
            *rightHighCut.get<0>().coefficients = *highCutCoefficients[0]; 
            rightHighCut.setBypassed<0>(false);
            *rightHighCut.get<1>().coefficients = *highCutCoefficients[1]; 
            rightHighCut.setBypassed<1>(false);
            break;
        }
        case Slope_36:
        {
            //L
            *leftHighCut.get<0>().coefficients = *highCutCoefficients[0]; 
            leftHighCut.setBypassed<0>(false);
            *leftHighCut.get<1>().coefficients = *highCutCoefficients[1]; 
            leftHighCut.setBypassed<1>(false); 
            *leftHighCut.get<2>().coefficients = *highCutCoefficients[2]; 
            leftHighCut.setBypassed<2>(false); 

            //R
            *rightHighCut.get<0>().coefficients = *highCutCoefficients[0]; 
            rightHighCut.setBypassed<0>(false); 
            *rightHighCut.get<1>().coefficients = *highCutCoefficients[1]; 
            rightHighCut.setBypassed<1>(false);
            *rightHighCut.get<2>().coefficients = *highCutCoefficients[2]; 
            rightHighCut.setBypassed<2>(false); 
            break;
        }
        case Slope_48:
        {
            //L
            *leftHighCut.get<0>().coefficients = *highCutCoefficients[0];
            leftHighCut.setBypassed<0>(false);
            *leftHighCut.get<1>().coefficients = *highCutCoefficients[1];
            leftHighCut.setBypassed<1>(false);
            *leftHighCut.get<2>().coefficients = *highCutCoefficients[2];
            leftHighCut.setBypassed<2>(false);
            *leftHighCut.get<3>().coefficients = *highCutCoefficients[3];
            leftHighCut.setBypassed<3>(false);

            //R
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
}
//=============================================================================================================================

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
    auto chainSettings = getChainSettings(apvts);

    updatePeakFilter(chainSettings);



    //HPF
    //========================================================================================================================================
    
    auto cutCoefficients = juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(chainSettings.lowCutFreq,
                                                                                                       getSampleRate(),
                                                                                                       2 * (chainSettings.lowCutSlope + 1));
    auto& leftLowCut = leftChain.get<ChainPositions::LowCut>();
    auto& rightLowCut = rightChain.get<ChainPositions::LowCut>();

    
    std::cout << "Low Cut Slope: " << chainSettings.lowCutSlope;

    updateCutFilter(leftLowCut, cutCoefficients,chainSettings.lowCutSlope); 
    updateCutFilter(rightLowCut, cutCoefficients,chainSettings.lowCutSlope);   





    //HIGHCUT
    //===========================================================================================================================
    auto highCutCoefficients = juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod(chainSettings.highCutFreq,
        getSampleRate(),
        2 * (chainSettings.highCutSlope + 1));

    //L
    auto& leftHighCut = leftChain.get<ChainPositions::LowCut>();
    leftHighCut.setBypassed<0>(true);
    leftHighCut.setBypassed<1>(true);
    leftHighCut.setBypassed<2>(true);
    leftHighCut.setBypassed<3>(true);

    //R
    auto& rightHighCut = rightChain.get<ChainPositions::LowCut>();
    rightHighCut.setBypassed<0>(true);
    rightHighCut.setBypassed<1>(true);
    rightHighCut.setBypassed<2>(true);
    rightHighCut.setBypassed<3>(true);

    switch (chainSettings.highCutSlope)
    {
        case Slope_12:
        {
            //L
            *leftHighCut.get<0>().coefficients = *highCutCoefficients[0];
            leftHighCut.setBypassed<0>(false);

            //R
            *rightHighCut.get<0>().coefficients = *highCutCoefficients[0];
            rightHighCut.setBypassed<0>(false);
            break;
        }
        case Slope_24:
        {
            //L
            *leftHighCut.get<0>().coefficients = *highCutCoefficients[0];
            leftHighCut.setBypassed<0>(false);
            *leftHighCut.get<1>().coefficients = *highCutCoefficients[1];
            leftHighCut.setBypassed<1>(false);

            //R
            *rightHighCut.get<0>().coefficients = *highCutCoefficients[0];
            rightHighCut.setBypassed<0>(false);
            *rightHighCut.get<1>().coefficients = *highCutCoefficients[1];
            rightHighCut.setBypassed<1>(false);
            break;
        }
        case Slope_36:
        {
            //L
            *leftHighCut.get<0>().coefficients = *highCutCoefficients[0];
            leftHighCut.setBypassed<0>(false);
            *leftHighCut.get<1>().coefficients = *highCutCoefficients[1];
            leftHighCut.setBypassed<1>(false);
            *leftHighCut.get<2>().coefficients = *highCutCoefficients[2];
            leftHighCut.setBypassed<2>(false);

            //R
            *rightHighCut.get<0>().coefficients = *highCutCoefficients[0];
            rightHighCut.setBypassed<0>(false);
            *rightHighCut.get<1>().coefficients = *highCutCoefficients[1];
            rightHighCut.setBypassed<1>(false);
            *rightHighCut.get<2>().coefficients = *highCutCoefficients[2];
            rightHighCut.setBypassed<2>(false);
            break;
        }
        case Slope_48:
        {
            //L
            *leftHighCut.get<0>().coefficients = *highCutCoefficients[0];
            leftHighCut.setBypassed<0>(false);
            *leftHighCut.get<1>().coefficients = *highCutCoefficients[1];
            leftHighCut.setBypassed<1>(false);
            *leftHighCut.get<2>().coefficients = *highCutCoefficients[2];
            leftHighCut.setBypassed<2>(false);
            *leftHighCut.get<3>().coefficients = *highCutCoefficients[3];
            leftHighCut.setBypassed<3>(false);

            //R
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
    //Because we are using an ENUM for slope, we have to cast to that type.
    settings.lowCutSlope = static_cast<Slope>(apvts.getRawParameterValue("LowCut Slope")->load());
    

    settings.highCutFreq = apvts.getRawParameterValue("HighCut Freq")->load();  
    settings.highCutSlope = static_cast<Slope>(apvts.getRawParameterValue("HighCut Slope")->load());



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

void SimpleEQAudioProcessor::updatePeakFilter(const ChainSettings& chainSettings) {

    auto lfCoefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(),
                                                                              chainSettings.lfFreq,
                                                                              chainSettings.lfQuality,
                                                                              juce::Decibels::decibelsToGain(chainSettings.lfGainInDecibels));
    *leftChain.get<ChainPositions::LF>().coefficients = *lfCoefficients;    //Copy values from lf Coefficients object -- Wrapper around array allocated on the heap
    *rightChain.get<ChainPositions::LF>().coefficients = *lfCoefficients;   //Allocation on the heap is not good for Audio software --> WHY???

    updateCoefficients(leftChain.get<ChainPositions::LF>().coefficients, lfCoefficients);
    updateCoefficients(rightChain.get<ChainPositions::LF>().coefficients, lfCoefficients);

    auto lmCoefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(),
                                                                              chainSettings.lmFreq,
                                                                              chainSettings.lmQuality,
                                                                              juce::Decibels::decibelsToGain(chainSettings.lmGainInDecibels));

    updateCoefficients(leftChain.get<ChainPositions::LM>().coefficients, lmCoefficients);
    updateCoefficients(rightChain.get<ChainPositions::LM>().coefficients, lmCoefficients);

    auto mCoefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(),
                                                                             chainSettings.mFreq,
                                                                             chainSettings.mQuality,
                                                                             juce::Decibels::decibelsToGain(chainSettings.mGainInDecibels));

    updateCoefficients(leftChain.get<ChainPositions::M>().coefficients, mCoefficients);
    updateCoefficients(rightChain.get<ChainPositions::M>().coefficients, mCoefficients);

    auto hmCoefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(),
                                                                              chainSettings.hmFreq,
                                                                              chainSettings.hmQuality,
                                                                              juce::Decibels::decibelsToGain(chainSettings.hmGainInDecibels));


    updateCoefficients(leftChain.get<ChainPositions::HM>().coefficients, hmCoefficients);
    updateCoefficients(rightChain.get<ChainPositions::HM>().coefficients, hmCoefficients);


    auto hfCoefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(),
                                                                              chainSettings.hfFreq,
                                                                              chainSettings.hfQuality,
                                                                              juce::Decibels::decibelsToGain(chainSettings.hfGainInDecibels));
    
     //WHAT THE HECK IS THE POINT OF THIS
     updateCoefficients(leftChain.get<ChainPositions::HF>().coefficients, hfCoefficients);
     updateCoefficients(rightChain.get<ChainPositions::HF>().coefficients, hfCoefficients);


}

//WHAT THE HECK IS THE POINT OF THIS
void SimpleEQAudioProcessor::updateCoefficients(Coefficients& old, const Coefficients& replacements) {    
    *old = *replacements;
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
                                                           juce::NormalisableRange<float>(20.f, 20000.f, 1.f, .3f), 
                                                           20.f));

    //HIGH CUT
    //=============================================================================================================
    layout.add(std::make_unique<juce::AudioParameterFloat>("HighCut Freq",
                                                           "HighCut Freq", 
                                                           juce::NormalisableRange<float>(20.f, 20000.f, 1.f, .2f), 
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

    //Slope choice 0 (12 db/oct) == order: 2
    //Slope choice 1 (24 db/oct) == order: 4
    //Slope choice 2 (36 db/oct) == order: 6
    //Slope choice 3 (48 db/oct) == order: 8

    //Order = 2*(Slope choice + 1)



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
