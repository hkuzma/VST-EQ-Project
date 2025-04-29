/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


SliderValue::SliderValue(juce::String name, int value){
    this->name = name;
    this->value = value;
}

void LookAndFeel::drawRotarySlider(juce::Graphics& g,
                                   int x,
                                   int y,
                                   int width,
                                   int height,
                                   float sliderPosProportional,
                                   float rotaryStartAngle,
                                   float rotaryEndAngle,
                                   juce::Slider& slider
                                   /*SliderValue& LFV,
                                   SliderValue& LMV, SliderValue& MV, SliderValue& HMV, SliderValue& HFV, SliderValue& LCV, SliderValue& HCV*/) {

    using namespace juce;

    auto bounds = Rectangle<float>(x, y, width, height);
    
    //Color for circles
    ColourGradient gradient = ColourGradient(Colour(177u, 174u, 184u), x, y,
                                             Colour(74u, 74u, 77u), width, height, true);

    //Set background color
    g.setGradientFill(gradient);
    g.fillEllipse(bounds);


    //Set border color
    g.setColour(Colour(40u, 40u, 41u));
    g.drawEllipse(bounds, 1.f);

    auto center = bounds.getCentre();



    //RotarySliderWithLabels rswl represents the current slider
    if (auto* rswl = dynamic_cast<RotarySliderWithLabels*>(&slider)) {

        auto center = bounds.getCentre();


        //Draw level for dials
        Path p;

        Rectangle<float> r;
        r.setLeft(center.getX() - 2);
        r.setRight(center.getX() + 2);
        r.setTop(bounds.getY());
        r.setBottom(center.getY()/* - rswl->getTextHeight() * 1.5*/);


        p.addRoundedRectangle(r, 2.f);

        jassert(rotaryStartAngle < rotaryEndAngle);

        //rotate rectangle when dial turned (
        auto sliderAngRad = jmap(sliderPosProportional, 0.f, 1.f, rotaryStartAngle, rotaryEndAngle);

        p.applyTransform(AffineTransform().rotated(sliderAngRad, center.getX(), center.getY()));


        g.fillPath(p);


        g.setFont(rswl->getTextHeight());
        auto text = rswl->getDisplayString();
        int num = rswl->getValue();

        auto name = rswl->getGivenName();

        /*if (name == "LF") {


        }*/

        auto strWidth = g.getCurrentFont().getStringWidth(text);

        r.setSize(strWidth-4, rswl->getTextHeight() + 2);
        r.setCentre(center.getX(), center.getY() + rswl->getTextHeight()*2);

        g.setColour(Colours::black);

        g.fillRect(r);

        g.setColour(Colours::white);
        g.drawFittedText(text, r.toNearestInt(), juce::Justification::centred, 1);

    }
}

void RotarySliderWithLabels::paint(juce::Graphics& g) {

    using namespace juce;

    auto startAng = degreesToRadians(180.f + 45.f);
    auto endAng = degreesToRadians(180.f - 45.f) + MathConstants<float>::twoPi;

    auto range = getRange();

    auto sliderBounds = getSliderBounds();

    /*g.setColour(Colours::red);
    g.drawRect(getLocalBounds());
    g.setColour(Colours::yellow); 
    g.drawRect(sliderBounds);*/

    getLookAndFeel().drawRotarySlider(g, 
                                      sliderBounds.getX(), 
                                      sliderBounds.getY(), 
                                      sliderBounds.getWidth(),
                                      sliderBounds.getHeight(), 
                                      jmap(getValue(), range.getStart(),range.getEnd(),0.0,1.0),    //Slider Value
                                      startAng,
                                      endAng, 
                                      *this);

    //Draw a number of labels around the dials

    auto center = sliderBounds.toFloat().getCentre();
    auto radius = sliderBounds.getWidth() * .05f;

    g.setColour(Colour(0u, 172u, 1u));
    g.setFont(getTextHeight()*.8);
    
    auto numChoices = labels.size();
    for (int i = 0; i < numChoices; i++) {

        auto label = labels[i];
        auto pos = labels[i].pos;
        jassert(0.f <= pos);
        jassert(pos <= 1.f);

        auto ang  = jmap(pos, 0.f, 1.f, startAng, endAng);


        auto c = center.getPointOnCircumference(radius + getTextHeight() *.5, ang);

        Rectangle<float> r;
        auto str = labels[i].label;
        r.setSize(g.getCurrentFont().getStringWidth(str)*.8, float(getTextHeight()));
        r.setCentre(c);
        r.setY(r.getY() + getTextHeight()*.6);
        if (i == 0)
            r.setX(r.getX() - sliderBounds.getWidth()*.6); 
        else
            r.setX(r.getX() + sliderBounds.getWidth() *.6);



        g.drawFittedText(str, r.toNearestInt(), juce::Justification::centred, 1);



    }



}

juce::Rectangle<int> RotarySliderWithLabels::getSliderBounds() const {

    auto bounds = getLocalBounds();
    //Set bounds to square
    auto size = juce::jmin(bounds.getWidth(), bounds.getHeight());

    size -= getTextHeight() * 2;

    juce::Rectangle<int> r;
    r.setSize(size, size);
    r.setCentre(bounds.getCentreX(), 0);
    r.setY(2);

    return r;
}


juce::String RotarySliderWithLabels::getDisplayString() const {
    
    //return juce::String(getValue());

    if (auto* choiceParam = dynamic_cast<juce::AudioParameterChoice*>(param)) {
        return choiceParam->getCurrentChoiceName();
        std::cout << choiceParam;
    }
  /*  auto* choiceParam = dynamic_cast<juce::AudioParameterChoice*>(param);
    auto New = choiceParam->getCurrentChoiceName();*/



    juce::String str;
    bool addK = false;

    if (auto* floatParam = dynamic_cast<juce::AudioParameterFloat*>(param)) {
         
        float val = getValue();

        if (val > 999) {
            val = val / 1000.f;
            addK = true;
        }

        str = juce::String(val, (addK ? 2 : 0));
    }
    else {          //THIS STATEMENT SHOULD NOT TRIGGER BUT SEEMS TO TRIGGER OFF OF THE LOW CUT SLOPES
        float val = getValue();
        str << 12 + val*12.0f;
        NULL;
        //str = suffix;
        //jassertfalse;
    }

    if (suffix.isNotEmpty()) {
        str << " ";
        if (addK)
            str << "k";

        str << suffix;
    }
    return str;
}


//=================================================================================================================

//SEPARATE COMPONENT FOR RESPONSE CURVE
ResponseCurveComponent::ResponseCurveComponent(SimpleEQAudioProcessor& p) : audioProcessor(p) {
    //listen for when parameters change --> Returns array of pointers
    const auto& params = audioProcessor.getParameters();
    for (auto param : params) {
        param->addListener(this);
    }

    //Start timer
    startTimerHz(60);

}

ResponseCurveComponent::~ResponseCurveComponent() {
    
    const auto& params = audioProcessor.getParameters();
    for (auto param : params) {
        param->removeListener(this);
    }

}

void ResponseCurveComponent::parameterValueChanged(int parameterIndex, float newValue) {

    parametersChanged.set(true);
}

void ResponseCurveComponent::timerCallback() {

    //If parameters changed = true, set to false AND...
    if (parametersChanged.compareAndSetBool(false, true)) {

        //update mono chain from apvts
        //signal repaint
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

void ResponseCurveComponent::paint(juce::Graphics& g)
{
    using namespace juce;
    // (Our component is opaque, so we must completely fill the background with a solid colour)

    //HENRY
    //Fill area in black
    g.fillAll(Colours::black);
    
    //int LFvalue = LF_FreqSlider.getValue();
    
    
   /* auto LF = SimpleEQAudioProcessorEditor:: getLF_Value();*/


    auto responseArea = getLocalBounds();


    auto w = responseArea.getWidth();

    auto& lowcut = monoChain.get<ChainPositions::LowCut>();
    auto& LF = monoChain.get<ChainPositions::LF>();
    auto& LM = monoChain.get<ChainPositions::LM>();
    auto& M = monoChain.get<ChainPositions::M>();
    auto& HM = monoChain.get<ChainPositions::HM>();
    auto& HF = monoChain.get<ChainPositions::HF>();
    auto& highcut = monoChain.get<ChainPositions::HighCut>();

    auto chainSettings = getChainSettings(audioProcessor.apvts);

   


    //ATTEMPTING IDEA TO DRAW DOTS ON LINE
    //auto LFfreq = LF.coefficients->getRawCoefficients();

   

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
    //FLAT LINE
    Path main;
    main.startNewSubPath(responseArea.getX(), responseArea.getCentreY());
    main.lineTo(responseArea.getRight(), responseArea.getCentreY());
    g.setColour(Colours::brown);
    g.strokePath(main, PathStrokeType(2.f));

    Path responseCurve;

    const double outputMin = responseArea.getBottom();
    const double outputMax = responseArea.getY();

    auto map = [outputMin, outputMax](double input) {
        return jmap(input, -24.0, 24.0, outputMin, outputMax);

        };

    responseCurve.startNewSubPath(responseArea.getX(), map(mags.front()));


    //CREATE PATH FOR RESPONSE CURVE
    for (size_t i = 1; i < mags.size(); i++) {
        responseCurve.lineTo(responseArea.getX() + i, map(mags[i]));
    }


    g.setColour(Colours::orange);
    g.drawRoundedRectangle(responseArea.toFloat(), 4.f, 1.f);

    g.setColour(Colours::white);
    g.strokePath(responseCurve, PathStrokeType(2.f));


    //DRAW POINTS
    auto LF_Freq = chainSettings.lfFreq;
    auto LM_Freq = chainSettings.lmFreq;
    auto M_Freq = chainSettings.mFreq;
    auto HM_Freq = chainSettings.hmFreq;
    auto HF_Freq = chainSettings.hfFreq;

    auto LF_Gain = chainSettings.lfGainInDecibels;
    auto LM_Gain = chainSettings.lmGainInDecibels;
    auto M_Gain = chainSettings.mGainInDecibels;
    auto HM_Gain = chainSettings.hmGainInDecibels;
    auto HF_Gain = chainSettings.hfGainInDecibels;




    Path LFREQ; 
    Rectangle<float> r;
    r.setSize(10,10);

    auto midpoint = 90;
    
    //Convert Frequency to Pixels
    auto x = (LF_Freq / 20000);  
    x = 600 * x;
    //Account for Skew
    x = (200 * log10(x)) + 44;


    r.setCentre(responseArea.getX() + x, responseArea.getCentreY() + map(int(mags[x])) - midpoint);

    LFREQ.addRoundedRectangle(r, 2.f);

    g.setColour(juce::Colours::red);
    g.fillPath(LFREQ);

    //DEBUGGING
   /* juce::String str;

    str << "HM: Y = " << y << "| X = " << x << "| bottom=" << responseArea.getBottom();*/




    Path LMFREQ;
    Rectangle<float> r2;
    r2.setSize(10, 10); 

    //Convert Frequency to Pixels
    x = (LM_Freq / 20000);
    x = 600 * x;
    //Account for Skew
    x = (200 * log10(x)) +  44;
    
    r2.setCentre(responseArea.getX() + x, responseArea.getCentreY() + map(int(mags[x])) - midpoint);

    LMFREQ.addRoundedRectangle(r2, 2.f); 
    g.setColour(juce::Colours::orange);  
    g.fillPath(LMFREQ); 

    Path MFREQ;
    Rectangle<float> r3;
    r3.setSize(10, 10);

   
    //Convert Frequency to Pixels
    x = (M_Freq / 20000);
    x = 600 * x;
    //Account for Skew
    x = (200 * log10(x)) + 44;

    r2.setCentre(responseArea.getX() + x, responseArea.getCentreY() + map(int(mags[x])) - midpoint);

    MFREQ.addRoundedRectangle(r2, 2.f);
    g.setColour(juce::Colours::yellow);
    g.fillPath(MFREQ);

    Path HMFREQ;
    Rectangle<float> r4;
    r4.setSize(10, 10);

    //Convert Frequency to Pixels
    x = (HM_Freq / 20000);
    x = 600 * x;
    //Account for Skew
    x = (200 * log10(x)) + 44;

    r2.setCentre(responseArea.getX() + x, responseArea.getCentreY() + map(int(mags[x])) - midpoint);

    HMFREQ.addRoundedRectangle(r2, 2.f);
    g.setColour(juce::Colours::green);
    g.fillPath(HMFREQ);

    Path HFFREQ;
    Rectangle<float> r5;
    r4.setSize(10, 10);

    //Convert Frequency to Pixels
    x = (HF_Freq / 20000);
    x = 600 * x;
    //Account for Skew
    x = (200 * log10(x)) + 44;

    r2.setCentre(responseArea.getX() + x, responseArea.getCentreY() + map(int(mags[x])) - midpoint);

    HFFREQ.addRoundedRectangle(r2, 2.f);
    g.setColour(juce::Colours::blue);
    g.fillPath(HFFREQ);

    

    //DEBUG RECT ===========================================================================================
    Path textp;
    Rectangle<float> LOWMID;

    /*juce::String str; 

    str << "HM: Y = " << y << "| X = " << x << "| bottom=" << responseArea.getBottom();*/


    LOWMID.setLeft(responseArea.getX());
    LOWMID.setRight(responseArea.getX() + 50);
    LOWMID.setBottom(responseArea.getY());
    LOWMID.setTop(responseArea.getY() + 30);

    LOWMID.setSize(350, 20);
    LOWMID.setCentre(responseArea.getX()+400, responseArea.getY()+150);

    textp.addRoundedRectangle(LOWMID, 2.f);
    /*g.setColour(Colours::aqua);
    g.fillPath(textp)*/;


    

   
    
    /*g.setColour(Colours::white);
    g.drawFittedText(str, LOWMID.toNearestInt(), juce::Justification::centred, 1);*/

  


     
   
    //==================================================================================================================

}


//==============================================================================
SimpleEQAudioProcessorEditor::SimpleEQAudioProcessorEditor(SimpleEQAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p),

    LF_FreqSlider(*audioProcessor.apvts.getParameter("LF"), "Hz", "LF"), LF_GainSlider(*audioProcessor.apvts.getParameter("LF Gain"), "dB", "LFG"), LF_QSlider(*audioProcessor.apvts.getParameter("LF Q"), "", "LFQ"),
    LM_FreqSlider(*audioProcessor.apvts.getParameter("LM"), "Hz", "LM"), LM_GainSlider(*audioProcessor.apvts.getParameter("LM Gain"), "dB", "LMG"), LM_QSlider(*audioProcessor.apvts.getParameter("LM Q"), "", "LMQ"),
    M_FreqSlider(*audioProcessor.apvts.getParameter("M"), "Hz", "M"), M_GainSlider(*audioProcessor.apvts.getParameter("M Gain"), "dB", "MG"), M_QSlider(*audioProcessor.apvts.getParameter("M Q"), "", "MQ"),
    HM_FreqSlider(*audioProcessor.apvts.getParameter("HM"), "Hz", "HM"), HM_GainSlider(*audioProcessor.apvts.getParameter("HM Gain"), "dB", "HMG"), HM_QSlider(*audioProcessor.apvts.getParameter("HM Q"), "", "HMQ"),
    HF_FreqSlider(*audioProcessor.apvts.getParameter("HF"), "Hz", "HF"), HF_GainSlider(*audioProcessor.apvts.getParameter("HF Gain"), "dB", "HFG"), HF_QSlider(*audioProcessor.apvts.getParameter("HF Q"), "", "HFQ"),
    lowCutFreqSlider(*audioProcessor.apvts.getParameter("LowCut Freq"), "Hz", "LC"), lowCutSlopeSlider(*audioProcessor.apvts.getParameter("LowCutSlope"), "dB/Oct", "LCS"),
    highCutFreqSlider(*audioProcessor.apvts.getParameter("HighCut Freq"), "Hz", "HC"), highCutSlopeSlider(*audioProcessor.apvts.getParameter("HighCutSlope"), "dB/Oct", "HCS"),


    responseCurveComponent(audioProcessor),

    lowCutFreqSliderAttachment(audioProcessor.apvts, "LowCut Freq", lowCutFreqSlider), lowCutSlopeSliderAttachment(audioProcessor.apvts, "LowCut Slope", lowCutSlopeSlider),
    highCutFreqSliderAttachment(audioProcessor.apvts, "HighCut Freq", highCutFreqSlider), highCutSlopeSliderAttachment(audioProcessor.apvts, "HighCut Slope", highCutSlopeSlider),
    LF_FreqSliderAttachment(audioProcessor.apvts, "LF", LF_FreqSlider), LF_GainSliderAttachment(audioProcessor.apvts, "LF Gain", LF_GainSlider), LF_QSliderAttachment(audioProcessor.apvts, "LF Q", LF_QSlider),
    LM_FreqSliderAttachment(audioProcessor.apvts, "LM", LM_FreqSlider), LM_GainSliderAttachment(audioProcessor.apvts, "LM Gain", LM_GainSlider), LM_QSliderAttachment(audioProcessor.apvts, "LM Q", LM_QSlider),
    M_FreqSliderAttachment(audioProcessor.apvts, "M", M_FreqSlider), M_GainSliderAttachment(audioProcessor.apvts, "M Gain", M_GainSlider), M_QSliderAttachment(audioProcessor.apvts, "M Q", M_QSlider),
    HM_FreqSliderAttachment(audioProcessor.apvts, "HM", HM_FreqSlider), HM_GainSliderAttachment(audioProcessor.apvts, "HM Gain", HM_GainSlider), HM_QSliderAttachment(audioProcessor.apvts, "HM Q", HM_QSlider),
    HF_FreqSliderAttachment(audioProcessor.apvts, "HF", HF_FreqSlider), HF_GainSliderAttachment(audioProcessor.apvts, "HF Gain", HF_GainSlider), HF_QSliderAttachment(audioProcessor.apvts, "HF Q", HF_QSlider),

    LF_Value("LFV", 0), LM_Value("LMV", 0), M_Value("MV", 0), HM_Value("HMV", 0), HF_Value("HMV", 0), LowCut_Value("LCV", 0), HighCut_Value("HCV", 0)


{


    //Initialize Labels
    LF_FreqSlider.labels.add({ 0.f, "20Hz" });
    LM_FreqSlider.labels.add({ 0.f, "20Hz" });
    M_FreqSlider.labels.add({ 0.f, "20Hz" });
    HM_FreqSlider.labels.add({ 0.f, "20Hz" });
    HF_FreqSlider.labels.add({ 0.f, "20Hz" });

    LF_FreqSlider.labels.add({ 1.f, "20kHz" });
    LM_FreqSlider.labels.add({ 1.f, "20kHz" });
    M_FreqSlider.labels.add({ 1.f, "20kHz" });
    HM_FreqSlider.labels.add({ 1.f, "20kHz" });
    HF_FreqSlider.labels.add({ 1.f, "20kHz" });


    
    HF_GainSlider.labels.add({ 0.f, "-24db" });
    LM_GainSlider.labels.add({ 0.f, "-24db" });
    M_GainSlider.labels.add({ 0.f, "-24db" });
    LF_GainSlider.labels.add({ 0.f, "-24db" });
    HM_GainSlider.labels.add({ 0.f, "-24db" });

    LF_GainSlider.labels.add({ 1.f, "+24db" });
    LM_GainSlider.labels.add({ 1.f, "+24db" });
    M_GainSlider.labels.add({ 1.f, "+24db" });
    HM_GainSlider.labels.add({ 1.f, "+24db" });
    HF_GainSlider.labels.add({ 1.f, "+24db" });

    lowCutFreqSlider.labels.add({ 0.f, "12" });
    lowCutFreqSlider.labels.add({ 1.f, "24" });
    highCutFreqSlider.labels.add({ 0.f, "12" });
    highCutFreqSlider.labels.add({ 1.f, "24" });



    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.

    for (auto* comp : getComps()) {

        addAndMakeVisible(comp);

    }



    setSize (600, 400);
}




SimpleEQAudioProcessorEditor::~SimpleEQAudioProcessorEditor()
{
}

//==============================================================================
void SimpleEQAudioProcessorEditor::paint (juce::Graphics& g)
{
    using namespace juce; 
    // (Our component is opaque, so we must completely fill the background with a solid colour)

    auto cornerRadius = 0.f;
    auto thickness = 3.f;
    auto TOPMOD = 11;
    auto BOTMOD = 3;
    auto XMOD = 2;

    //HENRY
    //Fill area in black
    g.fillAll(Colours::black);

    g.setColour(juce::Colours::red); 
    auto bounds = LF_FreqSlider.getBounds();
    auto qBounds = LF_QSlider.getBounds();
    auto left = bounds.getX() + XMOD +2;
    auto right = bounds.getX() + bounds.getWidth() - XMOD;

    auto width = bounds.getWidth();
    auto height = bounds.getHeight() * 3 + BOTMOD;

    auto top = bounds.getY() - TOPMOD;
    auto bottom = qBounds.getBottom() + BOTMOD;

    g.drawRoundedRectangle(Rectangle<float>(left, top, width - (.05 * width)-2, height + (.06 * height)), cornerRadius, thickness);

  /*  g.drawRoundedRectangle(LF_FreqSlider.getBounds().toFloat(), cornerRadius, thickness);
    g.drawRoundedRectangle(LF_GainSlider.getBounds().toFloat(), cornerRadius, thickness);
    g.drawRoundedRectangle(LF_QSlider.getBounds().toFloat(), cornerRadius, thickness);  */

    //g.drawHorizontalLine(LF_FreqSlider.getBounds().getBottom(), 4.f, 1.f);

    g.setColour(juce::Colours::orange);
    bounds = LM_FreqSlider.getBounds();
    qBounds = LM_QSlider.getBounds(); 
    left = bounds.getX() + XMOD;
    right = bounds.getX() + bounds.getWidth() - XMOD;

    width = bounds.getWidth();
    height = bounds.getHeight() * 3 + BOTMOD;

    top = bounds.getY() - TOPMOD;
    bottom = qBounds.getBottom() + BOTMOD;

    g.drawRoundedRectangle(Rectangle<float>(left, top, width - (.05 * width), height + (.06 * height)), cornerRadius, thickness); 

    juce::Colour(0, 0, 0);
    g.setColour(juce::Colours::yellow);  
    bounds = M_FreqSlider.getBounds();
    qBounds = M_QSlider.getBounds();
    left = bounds.getX() + XMOD; 
    right = bounds.getX() + bounds.getWidth() - XMOD; 

    width = bounds.getWidth();
    height = bounds.getHeight() * 3 + BOTMOD;

    top = bounds.getY() - TOPMOD;
    bottom = qBounds.getBottom() + BOTMOD;

    //g.drawRoundedRectangle(Rectangle<float>(left, top, width - (.05 * width), height + (.06 * height)), cornerRadius, thickness);
    bounds = M_FreqSlider.getBounds();
    width = std::min(bounds.getWidth(), bounds.getHeight());
    g.drawRoundedRectangle(Rectangle<float>(bounds.getX() + 25, bounds.getY(), width - 25, width - 25), 50.f, thickness);
   
    bounds = M_GainSlider.getBounds();
    width = std::min(bounds.getWidth(), bounds.getHeight());
    g.drawRoundedRectangle(Rectangle<float>(bounds.getX() + 25, bounds.getY(), width - 25, width - 25), 50.f, thickness);

    bounds = M_QSlider.getBounds();
    width = std::min(bounds.getWidth(), bounds.getHeight());
    g.drawRoundedRectangle(Rectangle<float>(bounds.getX() + 25, bounds.getY(), width - 25, width - 25), 50.f, thickness);

   /* g.drawRoundedRectangle(M_GainSlider.getBounds().toFloat()*.9, 25, thickness);
    g.drawRoundedRectangle(M_QSlider.getBounds().toFloat(), 25, thickness);*/

    g.setColour(juce::Colours::green);

    bounds = HM_FreqSlider.getBounds();
    qBounds = HM_QSlider.getBounds();
    left = bounds.getX() + XMOD;
    right = bounds.getX() + bounds.getWidth() - XMOD;

    width = bounds.getWidth();
    height = bounds.getHeight() * 3 + BOTMOD;

    top = bounds.getY() - TOPMOD;
    bottom = qBounds.getBottom() + BOTMOD;

    g.drawRoundedRectangle(Rectangle<float>(left, top, width - (.05 * width), height + (.06 * height)), cornerRadius, thickness);


    /*g.drawRoundedRectangle(HM_FreqSlider.getBounds().toFloat(), cornerRadius, thickness);
    g.drawRoundedRectangle(HM_GainSlider.getBounds().toFloat(), cornerRadius, thickness);
    g.drawRoundedRectangle(HM_QSlider.getBounds().toFloat(), cornerRadius, thickness);*/

    g.setColour(juce::Colours::blue);
    bounds = HF_FreqSlider.getBounds(); 
    qBounds = HF_QSlider.getBounds(); 
    left = bounds.getX() + XMOD;
    right = bounds.getX() + bounds.getWidth() - XMOD;

    width = bounds.getWidth();
    height = bounds.getHeight() * 3 + BOTMOD;

    top = bounds.getY() - TOPMOD;
    bottom = qBounds.getBottom() + BOTMOD;

    g.drawRoundedRectangle(Rectangle<float>(left, top, width - (.05 * width) -2, height + (.06 * height)), cornerRadius, thickness);


  /*  g.drawHorizontalLine(top,left,right);
    g.drawHorizontalLine(bottom,left,right); */
    //g.drawVerticalLine(left,top,bottom); 
    //g.drawVerticalLine(right,top,bottom);  



    //g.drawRoundedRectangle(HF_FreqSlider.getBounds().toFloat(), cornerRadius, thickness);
    //g.drawRoundedRectangle(HF_GainSlider.getBounds().toFloat(), cornerRadius, thickness);
    //g.drawRoundedRectangle(HF_QSlider.getBounds().toFloat(), cornerRadius, thickness);





    
}

void SimpleEQAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    //Reserve top third for Spectrograph
    auto bounds = getLocalBounds();
    auto responseArea = bounds.removeFromTop(bounds.getHeight() * 0.45);

    auto pad = bounds.removeFromTop(bounds.getHeight() * .05);

    responseCurveComponent.setBounds(responseArea);


    auto lowCutArea = bounds.removeFromLeft(bounds.getWidth() * 0.10);
    auto highCutArea = bounds.removeFromRight(bounds.getWidth() * 0.11111111);

    lowCutFreqSlider.setBounds(lowCutArea.removeFromTop(lowCutArea.getHeight()* .5));
    lowCutSlopeSlider.setBounds(lowCutArea);
    highCutFreqSlider.setBounds(highCutArea.removeFromTop(highCutArea.getHeight() * .5));
    highCutSlopeSlider.setBounds(highCutArea);

    auto lfArea = bounds.removeFromLeft(bounds.getWidth() * .2);

    LF_FreqSlider.setBounds(lfArea.removeFromTop(lfArea.getHeight() * 0.33)); 
    LF_GainSlider.setBounds(lfArea.removeFromTop(lfArea.getHeight() * 0.475)); 
    LF_QSlider.setBounds(lfArea.removeFromTop(lfArea.getHeight() * 0.9));

    auto lmArea = bounds.removeFromLeft(bounds.getWidth() * .25);

    LM_FreqSlider.setBounds(lmArea.removeFromTop(lmArea.getHeight() * 0.33));
    LM_GainSlider.setBounds(lmArea.removeFromTop(lmArea.getHeight() * 0.475));
    LM_QSlider.setBounds(lmArea.removeFromTop(lmArea.getHeight() * 0.9));

    auto mArea = bounds.removeFromLeft(bounds.getWidth() * .33);

    M_FreqSlider.setBounds(mArea.removeFromTop(mArea.getHeight() * 0.33));
    M_GainSlider.setBounds(mArea.removeFromTop(mArea.getHeight() * 0.475));
    M_QSlider.setBounds(mArea.removeFromTop(mArea.getHeight() * 0.9));

    //M_QSlider.setBounds(mArea);

    auto hmArea = bounds.removeFromLeft(bounds.getWidth() * .5);

    HM_FreqSlider.setBounds(hmArea.removeFromTop(hmArea.getHeight() * 0.33));
    HM_GainSlider.setBounds(hmArea.removeFromTop(hmArea.getHeight() * 0.475));
    HM_QSlider.setBounds(hmArea.removeFromTop(hmArea.getHeight()* 0.9));

    auto hfArea = bounds;

    HF_FreqSlider.setBounds(hfArea.removeFromTop(hfArea.getHeight() * 0.33));
    HF_GainSlider.setBounds(hfArea.removeFromTop(hfArea.getHeight() * 0.475));
    HF_QSlider.setBounds(hfArea.removeFromTop(hfArea.getHeight() * 0.9));

 
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
        &responseCurveComponent
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
