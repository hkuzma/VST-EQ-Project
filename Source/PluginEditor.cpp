/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SimpleEQAudioProcessorEditor::SimpleEQAudioProcessorEditor (SimpleEQAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), 

lowCutFreqSliderAttachment(audioProcessor.apvts, "LowCut Freq", lowCutFreqSlider),
lowCutSlopeSliderAttachment(audioProcessor.apvts, "LowCut Slope", lowCutSlopeSlider),

highCutFreqSliderAttachment(audioProcessor.apvts, "HighCut Freq", highCutFreqSlider),
highCutSlopeSliderAttachment(audioProcessor.apvts, "HighCut Slope", highCutSlopeSlider),

LF_FreqSliderAttachment(audioProcessor.apvts, "LF", LF_FreqSlider),
LF_GainSliderAttachment(audioProcessor.apvts, "LF Gain", LF_GainSlider),
LF_QSliderAttachment(audioProcessor.apvts, "LF Q", LF_QSlider),

LM_FreqSliderAttachment(audioProcessor.apvts, "LM", LM_FreqSlider),
LM_GainSliderAttachment(audioProcessor.apvts, "LM Gain", LM_GainSlider),
LM_QSliderAttachment(audioProcessor.apvts, "LM Q", LM_QSlider),

M_FreqSliderAttachment(audioProcessor.apvts, "M", M_FreqSlider),
M_GainSliderAttachment(audioProcessor.apvts, "M Gain", M_GainSlider),
M_QSliderAttachment(audioProcessor.apvts, "M Q", M_QSlider),

HM_FreqSliderAttachment(audioProcessor.apvts, "HM", HM_FreqSlider),
HM_GainSliderAttachment(audioProcessor.apvts, "HM Gain", HM_GainSlider),
HM_QSliderAttachment(audioProcessor.apvts, "HM Q", HM_QSlider),

HF_FreqSliderAttachment(audioProcessor.apvts, "HF", HF_FreqSlider),
HF_GainSliderAttachment(audioProcessor.apvts, "HF Gain", HF_GainSlider),
HF_QSliderAttachment(audioProcessor.apvts, "HF Q", HF_QSlider)



{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.

    for (auto* comp : getComps()) {

        addAndMakeVisible(comp);

    }

    //listen for when parameters change --> Returns array of pointers
    const auto& params = audioProcessor.getParameters();
    for (auto param : params) {
        param->addListener(this);
    }

    //Start timer
    startTimerHz(60);



    setSize (600, 400);
}

SimpleEQAudioProcessorEditor::~SimpleEQAudioProcessorEditor()
{
    const auto& params = audioProcessor.getParameters();
    for (auto param : params) {
        param->removeListener(this);
    }
}

//==============================================================================
void SimpleEQAudioProcessorEditor::paint (juce::Graphics& g)
{
    using namespace juce; 
    // (Our component is opaque, so we must completely fill the background with a solid colour)

    //HENRY
    g.fillAll (Colours::black);

    auto bounds = getLocalBounds();
    auto responseArea = bounds.removeFromTop(bounds.getHeight() * 0.5);


    auto w = responseArea.getWidth();

    auto& lowcut = monoChain.get<ChainPositions::LowCut>();
    auto& LF = monoChain.get<ChainPositions::LF>();
    auto& LM = monoChain.get<ChainPositions::LM>();
    auto& M = monoChain.get<ChainPositions::M>();
    auto& HM = monoChain.get<ChainPositions::HM>(); 
    auto& HF = monoChain.get<ChainPositions::HF>(); 
    auto& highcut = monoChain.get<ChainPositions::HighCut>();

    auto sampleRate = audioProcessor.getSampleRate();

    std::vector<double>mags;
    mags.resize(w);

    //Draw magnitude at each pixel in w
    for (int i = 0; i < w; i++) {
        
        double mag = 1.f;
        auto freq = mapToLog10(double(i) / double(w), 20.0, 20000.0);

        if (!monoChain.isBypassed<ChainPositions::LF>())
            mag *= LF.coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!monoChain.isBypassed<ChainPositions::LM>())
            mag *= LM.coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!monoChain.isBypassed<ChainPositions::M>())
            mag *= M.coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!monoChain.isBypassed<ChainPositions::HM>())
            mag *= HM.coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!monoChain.isBypassed<ChainPositions::HF>())
            mag *= HF.coefficients->getMagnitudeForFrequency(freq, sampleRate);

        if (!lowcut.isBypassed<0>())
            mag *= lowcut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!lowcut.isBypassed<1>())
            mag *= lowcut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!lowcut.isBypassed<2>())
            mag *= lowcut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!lowcut.isBypassed<3>())
            mag *= lowcut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);

        if (!highcut.isBypassed<0>()) 
            mag *= highcut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!highcut.isBypassed<1>())
            mag *= highcut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!highcut.isBypassed<2>())
            mag *= highcut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!highcut.isBypassed<3>())
            mag *= highcut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);

        mags[i] = Decibels::gainToDecibels(mag);
    }

    Path responseCurve;

    const double outputMin = responseArea.getBottom();
    const double outputMax = responseArea.getY();

    auto map = [outputMin, outputMax](double input) {
        return jmap(input, -24.0, 24.0, outputMin, outputMax);
    
    };

    responseCurve.startNewSubPath(responseArea.getX(), map(mags.front()));

    for (size_t i = 1; i < mags.size(); i++) {
        responseCurve.lineTo(responseArea.getX() + i, map(mags[i]));
    }

    g.setColour(Colours::orange);
    g.drawRoundedRectangle(responseArea.toFloat(), 4.f, 1.f);

    g.setColour(Colours::white);
    g.strokePath(responseCurve, PathStrokeType(2.f));

    //==================================================================================================================
    
}

void SimpleEQAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    //Reserve top third for Spectrograph
    auto bounds = getLocalBounds();
    auto responseArea = bounds.removeFromTop(bounds.getHeight() * 0.5);


    auto lowCutArea = bounds.removeFromLeft(bounds.getWidth() * 0.10);
    auto highCutArea = bounds.removeFromRight(bounds.getWidth() * 0.11111111);

    lowCutFreqSlider.setBounds(lowCutArea.removeFromTop(lowCutArea.getHeight()* .5));
    lowCutSlopeSlider.setBounds(lowCutArea);
    highCutFreqSlider.setBounds(highCutArea.removeFromTop(highCutArea.getHeight() * .5));
    highCutSlopeSlider.setBounds(highCutArea);

    auto lfArea = bounds.removeFromLeft(bounds.getWidth() * .2);

    LF_FreqSlider.setBounds(lfArea.removeFromTop(lfArea.getHeight() * 0.33)); 
    LF_GainSlider.setBounds(lfArea.removeFromTop(lfArea.getHeight() * 0.50)); 
    LF_QSlider.setBounds(lfArea); 

    auto lmArea = bounds.removeFromLeft(bounds.getWidth() * .25);

    LM_FreqSlider.setBounds(lmArea.removeFromTop(lmArea.getHeight() * 0.33));
    LM_GainSlider.setBounds(lmArea.removeFromTop(lmArea.getHeight() * 0.50));
    LM_QSlider.setBounds(lmArea);

    auto mArea = bounds.removeFromLeft(bounds.getWidth() * .33);

    M_FreqSlider.setBounds(mArea.removeFromTop(mArea.getHeight() * 0.33));
    M_GainSlider.setBounds(mArea.removeFromTop(mArea.getHeight() * 0.50));
    M_QSlider.setBounds(mArea);

    auto hmArea = bounds.removeFromLeft(bounds.getWidth() * .5);

    HM_FreqSlider.setBounds(hmArea.removeFromTop(hmArea.getHeight() * 0.33));
    HM_GainSlider.setBounds(hmArea.removeFromTop(hmArea.getHeight() * 0.50));
    HM_QSlider.setBounds(hmArea);

    auto hfArea = bounds;

    HF_FreqSlider.setBounds(hfArea.removeFromTop(hfArea.getHeight() * 0.33));
    HF_GainSlider.setBounds(hfArea.removeFromTop(hfArea.getHeight() * 0.50));
    HF_QSlider.setBounds(hfArea);

 
}

void SimpleEQAudioProcessorEditor::parameterValueChanged(int parameterIndex, float newValue) {

    parametersChanged.set(true);
}

void SimpleEQAudioProcessorEditor::timerCallback() {

    //If parameters changed = true, set to false AND...
    if (parametersChanged.compareAndSetBool(false, true)) {
        //update mono chain from apvts
        
        auto chainSettings = getChainSettings(audioProcessor.apvts);
        auto lfCoefficients = makePeakFilter(chainSettings, audioProcessor.getSampleRate(), "lf");
        updateCoefficients(monoChain.get<ChainPositions::LF>().coefficients, lfCoefficients);

        auto lmCoefficients = makePeakFilter(chainSettings, audioProcessor.getSampleRate(), "lm");
        updateCoefficients(monoChain.get<ChainPositions::LM>().coefficients, lmCoefficients);

        auto mCoefficients = makePeakFilter(chainSettings, audioProcessor.getSampleRate(), "m");
        updateCoefficients(monoChain.get<ChainPositions::M>().coefficients, mCoefficients);

        auto hmCoefficients = makePeakFilter(chainSettings, audioProcessor.getSampleRate(), "hm");
        updateCoefficients(monoChain.get<ChainPositions::HM>().coefficients, hmCoefficients);

        auto hfCoefficients = makePeakFilter(chainSettings, audioProcessor.getSampleRate(), "hf");
        updateCoefficients(monoChain.get<ChainPositions::HF>().coefficients, hfCoefficients);

        auto lowCutCoefficients = makeLowCutFilter(chainSettings, audioProcessor.getSampleRate());
        updateCutFilter(monoChain.get<ChainPositions::LowCut>(), lowCutCoefficients, chainSettings.lowCutSlope);

        auto highCutCoefficients = makeHighCutFilter(chainSettings, audioProcessor.getSampleRate()); 
        updateCutFilter(monoChain.get<ChainPositions::HighCut>(), highCutCoefficients, chainSettings.highCutSlope);








        //signal a repaint
        repaint();
    }
}

//HENRY
std::vector<juce::Component*> SimpleEQAudioProcessorEditor::getComps() {

    return
    {
        &LF_FreqSlider,
        &LF_GainSlider,
        &LF_QSlider,
        &LM_FreqSlider,
        &LM_GainSlider,
        &LM_QSlider,
        &M_FreqSlider,
        &M_GainSlider,
        &M_QSlider,
        &HM_FreqSlider,
        &HM_GainSlider,
        &HM_QSlider,
        &HF_FreqSlider,
        &HF_GainSlider,
        &HF_QSlider,
        &lowCutFreqSlider,
        &highCutFreqSlider,
        &lowCutSlopeSlider,
        &highCutSlopeSlider,
    };
}
//void SimpleEQAudioProcessorEditor::setFilterBounds() {
//
//}
//
//void SimpleEQAudioProcessorEditor::setPeakBounds() {
//
//}
//
//
//
//void SimpleEQAudioProcessorEditor::setBounds() {
//
//}
