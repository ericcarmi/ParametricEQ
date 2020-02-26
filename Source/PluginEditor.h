#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"

class PEQAudioProcessorEditor  : public AudioProcessorEditor,
                                 private LookAndFeel_V3, private Button::Listener
{
public:
    PEQAudioProcessorEditor (PEQAudioProcessor&);
    ~PEQAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;
    void setupCustomLookAndFeelColours  (LookAndFeel& laf,const Colour color);
    void showBubbleMessage (Component* targetComponent, String textToShow);

private:
    class ParameterSlider;
    struct CustomLookAndFeel;

    TextButton InfoButton;
    Label companyLabel;
    ScopedPointer<ParameterSlider> A1slider, Q1slider, F1slider, A2slider, Q2slider, F2slider, A3slider, Q3slider, F3slider, A4slider, Q4slider, F4slider, A5slider, Q5slider, F5slider, A6slider, Q6slider, F6slider, A7slider, Q7slider, F7slider, A8slider, Q8slider, F8slider, gainSlider, LPAslider, LPQslider, LPFslider, HPAslider, HPQslider, HPFslider, lowpassSlider, highpassSlider;

    const Colour buttonOnColor = Colours::green;
    const Colour buttonOffColor = Colours::red;
    const Colour buttonOnColor2 = Colours::cyan;
    const Colour buttonOffColor2 = Colours::magenta;

    const Colour gainColor = Colours::red;
    const Colour freqColor = Colours::blue;
    const Colour qColor = Colours::green;
    const Colour outputColor = Colours::goldenrod;

    void myButtonClicked(Button* button);
    void buttonClicked(Button* button) override
    {
        myButtonClicked(button);
    }

    ScopedPointer<BubbleMessageComponent> bubbleMessage;

    ShapeButton lowShelfPassSwitch, highShelfPassSwitch;
    ShapeButton powerButton;
    ShapeButton BP1switch, BP2switch, BP3switch, LPswitch, HPswitch;

    CustomLookAndFeel *claf, *claf2, *claf3, *claf4;

    //==============================================================================
    PEQAudioProcessor& getProcessor() const
    {
        return static_cast<PEQAudioProcessor&> (processor);
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PEQAudioProcessorEditor)

};
