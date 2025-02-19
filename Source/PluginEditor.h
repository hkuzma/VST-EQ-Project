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
struct CustomRotarySlider : juce::Slider {
    
    CustomRotarySlider() : juce::Slider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                                        juce::Slider::TextEntryBoxPosition::NoTextBox)
    {

    }
};


//==============================================================================
/**
*/
class SimpleEQAudioProcessorEditor  : public juce::AudioProcessorEditor,
    juce::AudioProcessorParameter::Listener,    //HENRY
    juce::Timer//HENRY 
{
public:
    SimpleEQAudioProcessorEditor (SimpleEQAudioProcessor&);
    ~SimpleEQAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    
    //HENRY
    //====================================================================================================
    void parameterValueChanged(int parameterIndex, float newValue) override;

    /** Indicates that a parameter change gesture has started.

        E.g. if the user is dragging a slider, this would be called with gestureIsStarting
        being true when they first press the mouse button, and it will be called again with
        gestureIsStarting being false when they release it.

        IMPORTANT NOTE: This will be called synchronously, and many audio processors will
        call it during their audio callback. This means that not only has your handler code
        got to be completely thread-safe, but it's also got to be VERY fast, and avoid
        blocking. If you need to handle this event on your message thread, use this callback
        to trigger an AsyncUpdater or ChangeBroadcaster which you can respond to later on the
        message thread.
    */
    void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override { }

    void timerCallback() override;


    //====================================================================================================

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    SimpleEQAudioProcessor& audioProcessor;

    juce::Atomic<bool> parametersChanged{ false };


    //HENRY
    CustomRotarySlider LF_FreqSlider,
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

    MonoChain monoChain;



  /*  void setFilterBounds();
    void setPeakBounds();
    void setBounds();*/

        

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleEQAudioProcessorEditor)
};
