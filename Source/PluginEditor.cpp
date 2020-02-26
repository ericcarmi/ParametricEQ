#include "PluginProcessor.h"
#include "PluginEditor.h"

class PEQAudioProcessorEditor::ParameterSlider   :            public Slider,
                                                              private Timer
{
public:
    ParameterSlider (AudioProcessorParameter& p)
        : Slider (p.getName (256)), param (p)
    {
        setRange (0.0, 1.0, 0.0);
        startTimerHz (30);
        updateSliderPos();

    }
    void valueChanged() override
    {
        param.setValueNotifyingHost ((float) Slider::getValue());

    }
    void timerCallback() override       { updateSliderPos(); }
    void startedDragging() override     { param.beginChangeGesture(); }
    void stoppedDragging() override     { param.endChangeGesture();   }

    double getValueFromText (const String& text) override   { return param.getValueForText (text); }
    String getTextFromValue (double value) override         { return param.getText ((float) value, 1024) + getTextValueSuffix(); }

    void updateSliderPos()
    {
        const float newValue = param.getValue();

        if (newValue != (float) Slider::getValue() && ! isMouseButtonDown())
            Slider::setValue (newValue);
    }

    AudioProcessorParameter& param;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParameterSlider)
};

struct PEQAudioProcessorEditor::CustomLookAndFeel    : public LookAndFeel_V3
{
    void drawRotarySlider (Graphics& g, int x, int y, int width, int height, float sliderPos,
                           float rotaryStartAngle, float rotaryEndAngle, Slider& slider) override
    {
        const float radius = jmin (width / 2, height / 2) - 2.0f;
        const float centreX = x + width * 0.5f;
        const float centreY = y + height * 0.5f;
        const float rx = centreX - radius;
        const float ry = centreY - radius;
        const float rw = radius * 2.0f;
        const float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        const bool isMouseOver = slider.isMouseOverOrDragging() && slider.isEnabled();

        if (slider.isEnabled())
            g.setColour (slider.findColour (Slider::rotarySliderFillColourId).withAlpha (isMouseOver ? 1.0f : 0.7f));
        else
            g.setColour (Colour (0x80808080));

        {
            Rectangle<float> r (rx, ry, rw, rw);

            Path filledArc;
            filledArc.addPieSegment (rx, ry, rw, rw, rotaryStartAngle, angle, 0.8);
            g.fillPath (filledArc);

            Path knob;
            knob.addEllipse(rx, ry, rw, rw);
            knob.applyTransform(AffineTransform::scale(0.8, 0.8, centreX, centreY));
            g.setColour(Colours::black);
            g.setGradientFill(ColourGradient(Colours::lightgrey, centreX*.4, centreY*.4, Colours::black, centreX, centreY, true));
            g.fillPath(knob);

            Path needle;
            const double mangle = angle - double_Pi/2.0;
            needle.addLineSegment(Line<float>(centreX,centreY,centreX+0.4*rw*cos(mangle),centreY+0.4*rw*sin(mangle)), 2.0);
            g.setColour(Colours::white);
            g.fillPath(needle);
        }
        {
            g.setColour(Colours::black);
            const float lineThickness = jmin (15.0f, jmin (width, height) * 0.45f) * 0.1f;
            Path outlineArc;
            outlineArc.addPieSegment (rx, ry, rw, rw, rotaryStartAngle, rotaryEndAngle, 0.8);
            g.strokePath (outlineArc, PathStrokeType (lineThickness));

        }
    }
};

void PEQAudioProcessorEditor::setupCustomLookAndFeelColours (LookAndFeel& laf, Colour color)
{
    laf.setColour (Slider::thumbColourId, Colour::greyLevel (0.95f));
    laf.setColour (Slider::textBoxOutlineColourId, Colours::transparentWhite);
    laf.setColour (Slider::rotarySliderFillColourId, color);
    laf.setColour (Slider::rotarySliderOutlineColourId, Colours::black);
    laf.setColour (Slider::textBoxTextColourId, Colours::white);
    laf.setColour (Slider::textBoxBackgroundColourId, Colours::black);
    laf.setColour (Slider::textBoxOutlineColourId, Colours::black);
    laf.setColour (Slider::textBoxHighlightColourId, Colours::grey);
}

//==============================================================================
PEQAudioProcessorEditor::PEQAudioProcessorEditor(PEQAudioProcessor& owner)
    : AudioProcessorEditor (owner),
      companyLabel(String::empty, "PEQ"),
      lowShelfPassSwitch("LowSelect", buttonOnColor2, buttonOnColor2, buttonOffColor2),
      highShelfPassSwitch("HighSelect", buttonOnColor2, buttonOnColor2, buttonOffColor2),
      BP1switch("BP1switch", buttonOffColor, buttonOffColor, buttonOnColor),
      BP2switch("BP2switch", buttonOffColor, buttonOffColor, buttonOnColor),
      BP3switch("BP3switch", buttonOffColor, buttonOffColor, buttonOnColor),
      LPswitch("LPswitch", buttonOffColor, buttonOffColor, buttonOnColor),
      HPswitch("HPswitch", buttonOffColor, buttonOffColor, buttonOnColor),
      powerButton("Power", buttonOnColor, buttonOnColor, buttonOffColor)
{

    Path circleButtonPow;
    circleButtonPow.addEllipse(0, 0, 30, 30);
    powerButton.setShape(circleButtonPow, true, true, true);
    bool onoff = *getProcessor().powerParam;
    addAndMakeVisible(powerButton);
    powerButton.addListener(this);
    if(onoff)
        powerButton.setColours(buttonOnColor, buttonOnColor, buttonOffColor);
    else
        powerButton.setColours(buttonOffColor, buttonOffColor, buttonOnColor);

    Path circleButton;
    circleButton.addEllipse(0, 0, 20, 20);
    BP1switch.setShape(circleButton, true, true, true);
    addAndMakeVisible(BP1switch);
    BP1switch.addListener(this);

    onoff = *getProcessor().BP1enableParam;
    if(onoff)
        BP1switch.setColours(buttonOnColor, buttonOnColor, buttonOffColor);
    else
        BP1switch.setColours(buttonOffColor, buttonOffColor, buttonOnColor);

    BP2switch.setShape(circleButton, true, true, true);
    addAndMakeVisible(BP2switch);
    BP2switch.addListener(this);

    onoff = *getProcessor().BP2enableParam;
    if(onoff)
        BP2switch.setColours(buttonOnColor, buttonOnColor, buttonOffColor);
    else
        BP2switch.setColours(buttonOffColor, buttonOffColor, buttonOnColor);

    BP3switch.setShape(circleButton, true, true, true);
    addAndMakeVisible(BP3switch);
    BP3switch.addListener(this);

    onoff = *getProcessor().BP3enableParam;
    if(onoff)
        BP3switch.setColours(buttonOnColor, buttonOnColor, buttonOffColor);
    else
        BP3switch.setColours(buttonOffColor, buttonOffColor, buttonOnColor);

    addAndMakeVisible(lowShelfPassSwitch);
    lowShelfPassSwitch.addListener(this);

    addAndMakeVisible(highShelfPassSwitch);
    highShelfPassSwitch.addListener(this);

    lowShelfPassSwitch.setShape(circleButton, true, false, true);
    highShelfPassSwitch.setShape(circleButton, true, false, true);

    LPswitch.setShape(circleButton, true, true, true);
    addAndMakeVisible(LPswitch);
    LPswitch.addListener(this);

    // addAndMakeVisible(InfoButton);
    // InfoButton.setButtonText(String("Info"));
    // InfoButton.addListener(this);
    // InfoButton.setColour(TextButton::buttonColourId, Colours::white);
    // InfoButton.setConnectedEdges(Button::ConnectedOnLeft + Button::ConnectedOnRight);

    claf = new CustomLookAndFeel();
    setupCustomLookAndFeelColours (*claf,gainColor);

    claf2 = new CustomLookAndFeel();
    setupCustomLookAndFeelColours (*claf2,qColor);

    claf3 = new CustomLookAndFeel();
    setupCustomLookAndFeelColours (*claf3,freqColor);

    claf4 = new CustomLookAndFeel();
    setupCustomLookAndFeelColours (*claf4,outputColor);

    addAndMakeVisible(LPAslider = new ParameterSlider (*owner.LPgainParam));
    LPAslider->setSliderStyle(Slider::Rotary);
    LPAslider->setTextBoxStyle(Slider::TextBoxBelow, false, 60, 20);
    LPAslider->setLookAndFeel(claf);
    LPAslider->setTextValueSuffix(" dB");

    addAndMakeVisible(LPQslider = new ParameterSlider (*owner.LPQParam));
    LPQslider->setSliderStyle(Slider::Rotary);
    LPQslider->setTextBoxStyle(Slider::TextBoxBelow, false, 60, 20);
    LPQslider->setLookAndFeel(claf2);
    LPQslider->setTextValueSuffix(" Q");

    addAndMakeVisible(LPFslider = new ParameterSlider (*owner.LPfreqParam));
    LPFslider->setSliderStyle(Slider::Rotary);
    LPFslider->setTextBoxStyle(Slider::TextBoxBelow, false, 80, 20);
    LPFslider->setLookAndFeel(claf3);
    LPFslider->setTextValueSuffix(" Hz");

    addAndMakeVisible(HPAslider = new ParameterSlider (*owner.HPgainParam));
    HPAslider->setSliderStyle(Slider::Rotary);
    HPAslider->setTextBoxStyle(Slider::TextBoxBelow, false, 60, 20);
    HPAslider->setLookAndFeel(claf);
    HPAslider->setTextValueSuffix(" dB");

    addAndMakeVisible(HPQslider = new ParameterSlider (*owner.HPQParam));
    HPQslider->setSliderStyle(Slider::Rotary);
    HPQslider->setTextBoxStyle(Slider::TextBoxBelow, false, 60, 20);
    HPQslider->setLookAndFeel(claf2);
    HPQslider->setTextValueSuffix(" Q");

    addAndMakeVisible(HPFslider = new ParameterSlider (*owner.HPfreqParam));
    HPFslider->setSliderStyle(Slider::Rotary);
    HPFslider->setTextBoxStyle(Slider::TextBoxBelow, false, 80, 20);
    HPFslider->setLookAndFeel(claf3);
    HPFslider->setTextValueSuffix(String(" Hz"));

    addAndMakeVisible(lowpassSlider = new ParameterSlider (*owner.lowpassParam));
    lowpassSlider->setSliderStyle(Slider::Rotary);
    lowpassSlider->setTextBoxStyle(Slider::TextBoxBelow, false, 80, 20);
    lowpassSlider->setLookAndFeel(claf3);
    lowpassSlider->setTextValueSuffix(" Hz");
    lowpassSlider->setVisible(false);

    addAndMakeVisible(highpassSlider = new ParameterSlider (*owner.highpassParam));
    highpassSlider->setSliderStyle(Slider::Rotary);
    highpassSlider->setTextBoxStyle(Slider::TextBoxBelow, false, 80, 20);
    highpassSlider->setLookAndFeel(claf3);
    highpassSlider->setTextValueSuffix(" Hz");

    lowpassSlider->setVisible(false);
    highpassSlider->setVisible(false);

    addAndMakeVisible(A1slider = new ParameterSlider (*owner.A1param));
    A1slider->setSliderStyle(Slider::Rotary);
    A1slider->setTextBoxStyle(Slider::TextBoxBelow, false, 60, 20);
    A1slider->setLookAndFeel(claf);
    A1slider->setTextValueSuffix(" dB");

    addAndMakeVisible(Q1slider = new ParameterSlider (*owner.Q1param));
    Q1slider->setSliderStyle(Slider::Rotary);
    Q1slider->setTextBoxStyle(Slider::TextBoxBelow, false, 60, 20);
    Q1slider->setLookAndFeel(claf2);
    Q1slider->setTextValueSuffix(" Q");

    addAndMakeVisible(F1slider = new ParameterSlider (*owner.F1param));
    F1slider->setSliderStyle(Slider::Rotary);
    F1slider->setTextBoxStyle(Slider::TextBoxBelow, false, 80, 20);
    F1slider->setLookAndFeel(claf3);
    F1slider->setTextValueSuffix(" Hz");

    addAndMakeVisible(A2slider = new ParameterSlider (*owner.A2param));
    A2slider->setSliderStyle(Slider::Rotary);
    A2slider->setTextBoxStyle(Slider::TextBoxBelow, false, 60, 20);
    A2slider->setLookAndFeel(claf);
    A2slider->setTextValueSuffix(" dB");

    addAndMakeVisible(Q2slider = new ParameterSlider (*owner.Q2param));
    Q2slider->setSliderStyle(Slider::Rotary);
    Q2slider->setTextBoxStyle(Slider::TextBoxBelow, false, 60, 20);
    Q2slider->setLookAndFeel(claf2);
    Q2slider->setTextValueSuffix(" Q");

    addAndMakeVisible(F2slider = new ParameterSlider (*owner.F2param));
    F2slider->setSliderStyle(Slider::Rotary);
    F2slider->setTextBoxStyle(Slider::TextBoxBelow, false, 80, 20);
    F2slider->setLookAndFeel(claf3);
    F2slider->setTextValueSuffix(" Hz");

    addAndMakeVisible(A3slider = new ParameterSlider (*owner.A3param));
    A3slider->setSliderStyle(Slider::Rotary);
    A3slider->setTextBoxStyle(Slider::TextBoxBelow, false, 60, 20);
    A3slider->setLookAndFeel(claf);
    A3slider->setTextValueSuffix(" dB");

    addAndMakeVisible(Q3slider = new ParameterSlider (*owner.Q3param));
    Q3slider->setSliderStyle(Slider::Rotary);
    Q3slider->setTextBoxStyle(Slider::TextBoxBelow, false, 60, 20);
    Q3slider->setLookAndFeel(claf2);
    Q3slider->setTextValueSuffix(" Q");

    addAndMakeVisible(F3slider = new ParameterSlider (*owner.F3param));
    F3slider->setTextValueSuffix(" Hz");
    F3slider->setSliderStyle(Slider::Rotary);
    F3slider->setTextBoxStyle(Slider::TextBoxBelow, false, 80, 20);
    F3slider->setLookAndFeel(claf3);

    addAndMakeVisible(gainSlider = new ParameterSlider (*owner.Gainparam));
    gainSlider->setSliderStyle(Slider::Rotary);
    gainSlider->setTextBoxStyle(Slider::TextBoxBelow, false, 60, 20);
    gainSlider->setLookAndFeel(claf4);
    gainSlider->setTextValueSuffix(" dB");

    addAndMakeVisible(companyLabel);
    companyLabel.setFont (Font(String("Arial"), 25.0f, Font::bold));
    companyLabel.setColour(Label::textColourId, Colours::goldenrod);

    // Check based on settings?
    const bool whichlow = *getProcessor().lowtypeParam;
    if(whichlow) // If shelf
    {
        lowShelfPassSwitch.setColours(buttonOnColor2, buttonOnColor2, buttonOffColor2);
        LPAslider->setVisible(true);
        LPQslider->setVisible(true);
        LPFslider->setVisible(true);
        lowpassSlider->setVisible(false);

        onoff = *getProcessor().LSenableParam;

        if(onoff)
            LPswitch.setColours(buttonOnColor, buttonOnColor, buttonOffColor);
        else
            LPswitch.setColours(buttonOffColor, buttonOffColor, buttonOnColor);

    }
    else
    {
        lowShelfPassSwitch.setColours(buttonOffColor2, buttonOffColor2, buttonOnColor2);

        onoff = *getProcessor().LPenableParam;
                LPAslider->setVisible(false);
                LPQslider->setVisible(false);
                LPFslider->setVisible(false);
                lowpassSlider->setVisible(true);

        if(onoff)
            LPswitch.setColours(buttonOnColor, buttonOnColor, buttonOffColor);
        else
            LPswitch.setColours(buttonOffColor, buttonOffColor, buttonOnColor);

    }

    HPswitch.setShape(circleButton, true, false, true);
    addAndMakeVisible(HPswitch);
    HPswitch.addListener(this);
    const bool whichhigh = *getProcessor().hightypeParam;
    if(whichhigh) // If shelf
    {
        highShelfPassSwitch.setColours(buttonOnColor2, buttonOnColor2, buttonOffColor2);

        onoff = *getProcessor().HSenableParam;

        HPAslider->setVisible(true);
        HPQslider->setVisible(true);
        HPFslider->setVisible(true);
        highpassSlider->setVisible(false);
        if(onoff)
            HPswitch.setColours(buttonOnColor, buttonOnColor, buttonOffColor);
        else
            HPswitch.setColours(buttonOffColor, buttonOffColor, buttonOnColor);

    }
    else
    {
        onoff = *getProcessor().HPenableParam;
        highShelfPassSwitch.setColours(buttonOffColor2, buttonOffColor2, buttonOnColor2);

                HPAslider->setVisible(false);
                HPQslider->setVisible(false);
                HPFslider->setVisible(false);
                highpassSlider->setVisible(true);

        if(onoff)
            HPswitch.setColours(buttonOnColor, buttonOnColor, buttonOffColor);
        else
            HPswitch.setColours(buttonOffColor, buttonOffColor, buttonOnColor);
    }

    setSize (owner.lastUIWidth,owner.lastUIHeight);

    bubbleMessage = new BubbleMessageComponent(500);


}

PEQAudioProcessorEditor::~PEQAudioProcessorEditor()
{

}

void PEQAudioProcessorEditor::showBubbleMessage (Component* targetComponent, String textToShow)
{

    if (Desktop::canUseSemiTransparentWindows())
    {
        bubbleMessage->setAlwaysOnTop (true);
        bubbleMessage->addToDesktop (0);
    }
    else
    {
        targetComponent->getTopLevelComponent()->addChildComponent (bubbleMessage);
    }

    AttributedString text (textToShow);
    text.setJustification (Justification::centred);
    text.setColour (targetComponent->findColour (TextButton::textColourOffId));

  //  bubbleMessage->showAt (targetComponent, text, 10000, false, false);
}

void PEQAudioProcessorEditor::myButtonClicked(Button* button)
{
    if(button == &powerButton)
    {
        if( *getProcessor().powerParam  )
        {
            *getProcessor().powerParam = false;
            powerButton.setColours(buttonOffColor, buttonOffColor, buttonOnColor);
        }
        else
        {
            *getProcessor().powerParam = true;
            powerButton.setColours(buttonOnColor, buttonOnColor, buttonOffColor);
        }
    }
    // Functions as the on/off switch for both lowpass and lowshelf filters
    if( button == &LPswitch)
    {
        const bool which = *getProcessor().lowtypeParam;

        // If which, type is lowshelf
        if(which)
        {
            const bool huh = *getProcessor().LSenableParam;
            if(huh)
            {
                *getProcessor().LSenableParam = false;
                LPswitch.setColours(buttonOffColor, buttonOffColor, buttonOnColor);

            }
            else
            {
                *getProcessor().LSenableParam = true;
                LPswitch.setColours(buttonOnColor, buttonOnColor, buttonOffColor);

            }

        }

        // If notwhich, type is lowpass
        else
        {
            const bool huh = *getProcessor().LPenableParam;
            if(huh)
            {
                *getProcessor().LPenableParam = false;
                LPswitch.setColours(buttonOffColor, buttonOffColor, buttonOnColor);

            }
            else
            {
                *getProcessor().LPenableParam = true;
                LPswitch.setColours(buttonOnColor, buttonOnColor, buttonOffColor);

            }
        }

    }

    if( button == &HPswitch)
    {
        const bool which = *getProcessor().hightypeParam;
        // If which, type is highshelf
        if(which)
        {
            const bool huh = *getProcessor().HSenableParam;
            if(huh) // If true/on, change to false/off
            {
                *getProcessor().HSenableParam = false;
                HPswitch.setColours(buttonOffColor, buttonOffColor, buttonOnColor);

            }
            else
            {
                *getProcessor().HSenableParam = true;
                HPswitch.setColours(buttonOnColor, buttonOnColor, buttonOffColor);

            }

        }

        // If notwhich, type is lowshelf
        else
        {
            const bool huh = *getProcessor().HPenableParam;
            if(huh)
            {
                *getProcessor().HPenableParam = false;
                HPswitch.setColours(buttonOffColor, buttonOffColor, buttonOnColor);

            }
            else
            {
                *getProcessor().HPenableParam = true;
                HPswitch.setColours(buttonOnColor, buttonOnColor, buttonOffColor);

            }
        }
    }

    if( button == &BP1switch)
    {

        const bool huh = *getProcessor().BP1enableParam;

        if(huh)
        {
            *getProcessor().BP1enableParam = false;
            BP1switch.setColours(buttonOffColor, buttonOffColor, buttonOnColor);

        }
        else
        {
            *getProcessor().BP1enableParam = true;
            BP1switch.setColours(buttonOnColor, buttonOnColor, buttonOffColor);
        }
    }
    if( button == &BP2switch)
    {
        const bool huh = *getProcessor().BP2enableParam;

        if(huh)
        {
            *getProcessor().BP2enableParam = false;
            BP2switch.setColours(buttonOffColor, buttonOffColor, buttonOnColor);


        }
        else
        {
            *getProcessor().BP2enableParam = true;
            BP2switch.setColours(buttonOnColor, buttonOnColor, buttonOffColor);

        }
    }
    if( button == &BP3switch)
    {
        const bool huh = *getProcessor().BP3enableParam;

        if(huh)
        {
            *getProcessor().BP3enableParam = false;
            BP3switch.setColours(buttonOffColor, buttonOffColor, buttonOnColor);
        }
        else
        {
            *getProcessor().BP3enableParam = true;
            BP3switch.setColours(buttonOnColor, buttonOnColor, buttonOffColor);


        }
    }

    if(button == &InfoButton)
    {
        // showBubbleMessage (button,
        //                    "Filters in this EQ: 1 lowpass, 1 lowshelf\n 1 highpass, 1 highshelf\n 3 bandpass.\n\n "
        //                    "Red : Gain       \nGreen : Q factor \n Blue : Frequency\n "
        //                    "Green/Red switches toggle filters on/off. \n Cyan/Magenta toggles between shelf/pass");
    }

    if(button == &lowShelfPassSwitch)
    {
        const bool which = *getProcessor().lowtypeParam;

        // If notwhich, low type is lowpass, so switch it to lowshelf
        if(!which)
        {
            *getProcessor().lowtypeParam = true;
            lowShelfPassSwitch.setColours(buttonOnColor2, buttonOnColor2, buttonOffColor2);

            LPAslider->setVisible(true);
            LPQslider->setVisible(true);
            LPFslider->setVisible(true);
            lowpassSlider->setVisible(false);

            // not working, need another slider
            const bool huh = *getProcessor().LSenableParam;

            if(huh) // Need to change LPswitch based on whether or not this one is enabled
            {
                LPswitch.setColours(buttonOnColor, buttonOnColor, buttonOffColor);
            }
            else
            {
                LPswitch.setColours(buttonOffColor, buttonOffColor, buttonOnColor);
            }
        }

        else
        {
            // Switch to lowpass
            *getProcessor().lowtypeParam = false;
            lowShelfPassSwitch.setColours(buttonOffColor2, buttonOffColor2, buttonOnColor2);

            LPAslider->setVisible(false);
            LPQslider->setVisible(false);
            LPFslider->setVisible(false);
            lowpassSlider->setVisible(true);

            const bool huh = *getProcessor().LPenableParam;

            if(huh) // Need to change LPswitch based on whether or not this one is enabled
            {
                LPswitch.setColours(buttonOnColor, buttonOnColor, buttonOffColor);
            }
            else
            {
                LPswitch.setColours(buttonOffColor, buttonOffColor, buttonOnColor);

            }


        }
    }

    if(button == &highShelfPassSwitch)
    {

        const bool which = *getProcessor().hightypeParam;

        // If notwhich, low type is lowpass, so switch it to lowshelf
        if(!which)
        {
            *getProcessor().hightypeParam = true;
            highShelfPassSwitch.setColours(buttonOnColor2, buttonOnColor2, buttonOffColor2);


            HPAslider->setVisible(true);
            HPQslider->setVisible(true);
            HPFslider->setVisible(true);
            highpassSlider->setVisible(false);

            // not working, need another slider
            const bool huh = *getProcessor().HSenableParam;

            if(huh) // Need to change LPswitch based on whether or not this one is enabled
            {
                HPswitch.setColours(buttonOnColor, buttonOnColor, buttonOffColor);
            }
            else
            {
                HPswitch.setColours(buttonOffColor, buttonOffColor, buttonOnColor);
            }
        }

        else
        {
            // Switch to pass
            *getProcessor().hightypeParam = false;
            highShelfPassSwitch.setColours(buttonOffColor2, buttonOffColor2, buttonOnColor2);

            HPAslider->setVisible(false);
            HPQslider->setVisible(false);
            HPFslider->setVisible(false);
            highpassSlider->setVisible(true);

            const bool huh2 = *getProcessor().HPenableParam;

            if(huh2) // Need to change LPswitch based on whether or not this one is enabled
            {
                HPswitch.setColours(buttonOnColor, buttonOnColor, buttonOffColor);

            }
            else
            {
                HPswitch.setColours(buttonOffColor, buttonOffColor, buttonOnColor);

            }
        }
    }
    repaint();
}

//==============================================================================
void PEQAudioProcessorEditor::paint (Graphics& g)
{
    g.setColour(Colours::lightgrey);
    g.fillAll();

    g.setColour(Colours::black);
    Path p1, p2, p3, p4;

    Line<float> L1, L2, L3, L4, L5, L6;

    L1.setStart(135, 30);
    L1.setEnd(135, 420);
    L2.setStart(0, 420);
    L2.setEnd(400, 420);
    L3.setStart(110, 420);
    L3.setEnd(200,600);
    L4.setStart(290, 420);
    L4.setEnd(200,600);
    L5.setStart(265, 30);
    L5.setEnd(265, 420);

    g.drawLine(L1, 5.0);
    g.drawLine(L2, 5.0);
    g.drawLine(L3, 5.0);
    g.drawLine(L4, 5.0);
    g.drawLine(L5, 5.0);

    g.setColour(Colours::black);
    g.fillRect(0, 0, getWidth(), 30);

}

void PEQAudioProcessorEditor::resized()
{
    // Bandpass filters
    A1slider->setBounds(25, 70, 100, 100);
    Q1slider->setBounds(25, 190, 100, 100);
    F1slider->setBounds(25, 310, 100, 100);
    A2slider->setBounds(150, 70, 100, 100);
    Q2slider->setBounds(150, 190, 100, 100);
    F2slider->setBounds(150, 310, 100, 100);
    A3slider->setBounds(275, 70, 100, 100);
    Q3slider->setBounds(275, 190, 100, 100);
    F3slider->setBounds(275, 310, 100, 100);
    // Lowpass in the lower left
    LPAslider->setBounds(50,  440, 75, 75);
    LPQslider->setBounds(100, 520, 75, 75);
    LPFslider->setBounds(0,   520, 75, 75);
    // Highpass in the lower right
    HPAslider->setBounds(275, 440, 75, 75);
    HPQslider->setBounds(225, 520, 75, 75);
    HPFslider->setBounds(325, 520, 75, 75);

    lowpassSlider->setBounds(40, 480, 100, 100);
    highpassSlider->setBounds(260, 480, 100, 100);

    gainSlider->setBounds(150, 425, 100, 100);

    companyLabel.setBounds(165, 0, 200, 30);
    //InfoButton.setBounds(0,0, 50, 30);
    powerButton.setBounds(getWidth()-30, 0, 30, 30);

    // Buttons
    BP1switch.setBounds(64, 30, 20, 20);
    BP2switch.setBounds(189, 30, 20, 20);
    BP3switch.setBounds(314, 30, 20, 20);
    LPswitch.setBounds(10, 425, 20, 20);
    HPswitch.setBounds(370, 425, 20, 20);
    lowShelfPassSwitch.setBounds(10, 475, 20, 20);
    highShelfPassSwitch.setBounds(370, 475, 20, 20);

    getProcessor().lastUIWidth = getWidth();
    getProcessor().lastUIHeight = getHeight();
}

//==============================================================================
