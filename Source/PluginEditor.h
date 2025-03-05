/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//HENRY

//Custom Struct for circular slider // Dials
struct LookAndFeel : juce::LookAndFeel_V4 {
    void drawRotarySlider(juce::Graphics&,
        int x, int y, int width, int height,
        float sliderPosProportional,
        float rotaryStartAngle,
        float rotaryEndAngle,
        juce::Slider&) override;


};

struct RotarySliderWithLabels : juce::Slider {
    
    RotarySliderWithLabels(juce::RangedAudioParameter& rap, const juce::String& unitSuffix) : 
    juce::Slider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                 juce::Slider::TextEntryBoxPosition::NoTextBox),
    param(&rap),
    suffix(unitSuffix)
    {
        setLookAndFeel(&lnf);
    }

    ~RotarySliderWithLabels() {
        setLookAndFeel(nullptr);
    }

    void paint(juce::Graphics& g) override;
    juce::Rectangle<int> getSliderBounds() const;
    int getTextHeight() const { return 14; }
    juce::String getDisplayString() const;

private:
    LookAndFeel lnf;
    
    juce::RangedAudioParameter* param;
    juce::String suffix;


};

//RESPONSE CURVE STRUCT
//=============================================================================================================================
struct ResponseCurveComponent : juce::Component,
    juce::AudioProcessorParameter::Listener,
    juce::Timer
{
    ResponseCurveComponent(SimpleEQAudioProcessor&);
    ~ResponseCurveComponent();
    void parameterValueChanged(int parameterIndex, float newValue) override;

    void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override { }

    void timerCallback() override;

    void paint(juce::Graphics& g) override;

private:
    SimpleEQAudioProcessor& audioProcessor;
    juce::Atomic<bool> parametersChanged{ false };

    MonoChain monoChain;


};

//CLASS
//==============================================================================
class SimpleEQAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    SimpleEQAudioProcessorEditor (SimpleEQAudioProcessor&);
    ~SimpleEQAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    SimpleEQAudioProcessor& audioProcessor;



    //HENRY
    RotarySliderWithLabels LF_FreqSlider,
                       LF_GainSlider,
                       LF_QSlider,
                       LM_FreqSlider,
                       LM_GainSlider,
                       LM_QSlider,
                       M_FreqSlider,
                       M_GainSlider,
                       M_QSlider,
                       HM_FreqSlider,
                       HM_GainSlider,
                       HM_QSlider,
                       HF_FreqSlider,
                       HF_GainSlider,
                       HF_QSlider,
                       lowCutFreqSlider,
                       highCutFreqSlider,
                       lowCutSlopeSlider,
                       highCutSlopeSlider;

    //Instance of Response Curve
    ResponseCurveComponent responseCurveComponent;

    using APVTS = juce::AudioProcessorValueTreeState;
    using Attachment = APVTS::SliderAttachment;

    Attachment LF_FreqSliderAttachment,
               LF_GainSliderAttachment,
               LF_QSliderAttachment,
               LM_FreqSliderAttachment,
               LM_GainSliderAttachment,
               LM_QSliderAttachment,
               M_FreqSliderAttachment,
               M_GainSliderAttachment,
               M_QSliderAttachment,
               HM_FreqSliderAttachment,
               HM_GainSliderAttachment,
               HM_QSliderAttachment,
               HF_FreqSliderAttachment,
               HF_GainSliderAttachment,
               HF_QSliderAttachment,
               lowCutFreqSliderAttachment,
               highCutFreqSliderAttachment,
               lowCutSlopeSliderAttachment,
               highCutSlopeSliderAttachment;


    //It's Helpful to put identical components in a vector

    std::vector<juce::Component*> getComps();


        

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleEQAudioProcessorEditor)
};
