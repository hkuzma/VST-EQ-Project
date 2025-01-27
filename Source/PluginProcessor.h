/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/
class SimpleEQAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    SimpleEQAudioProcessor();
    ~SimpleEQAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;



    //Function to Create Parameter Layout for apvts -- HENRY
    //================================================================================
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    //HENRY
    /*PARAMS: AudioProcessor --> AUDIO PROCESSOR TO CONNECT TO -- Passed by reference
    *         UndoManager --> THE UNDO MANAGER BEING USED -- Ptr
    *         Identifier --> THE TYPE OF VALUE TREE -- Passed by reference --> String
    *         ParameterLayour --> The layout of plugin parameters
    */
    juce::AudioProcessorValueTreeState apvts{ *this,nullptr, "Parameters", createParameterLayout() };


private:


    //Each Filter has a response of 12db per octave for lowpass//highpass filter
    using Filter = juce::dsp::IIR::Filter<float>;

    //Put 4 filters in a processing chain for cut filters 
    using CutFilter = juce::dsp::ProcessorChain<Filter, Filter, Filter, Filter>;


    //create 2 mono chains to represent 1 stereochain
    using MonoChain = juce::dsp::ProcessorChain<CutFilter, Filter, Filter, Filter, Filter, Filter, CutFilter>;

    MonoChain leftChain, rightChain;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleEQAudioProcessor)
};
