/*
  This EQ has 3 bandpass filters, a LP/LS, a HP/HS, and automatic gain control

*/
#ifndef PLUGINPROCESSOR_H_INCLUDED
#define PLUGINPROCESSOR_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"

class PEQAudioProcessor  :            public AudioProcessor,
                                      public IIRFilter,
                                      public IIRCoefficients
{
public:
    //==============================================================================
    PEQAudioProcessor();
    ~PEQAudioProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void reset() override;

    //==============================================================================
    void processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages) override
    {
        jassert (! isUsingDoublePrecision());
        process (buffer, midiMessages);
    }

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;


    #ifndef JucePlugin_PreferredChannelConfigurations
        bool setPreferredBusArrangement (bool isInput, int bus, const AudioChannelSet& preferredSet) ;
    #endif

    //==============================================================================
    bool hasEditor() const override                                             { return true; }
    AudioProcessorEditor* createEditor() override;

    //==============================================================================
    const String getName() const override                                       { return JucePlugin_Name ; }

    bool acceptsMidi() const override;
    bool producesMidi() const override;

    double getTailLengthSeconds() const override                                { return 0.0; }

    //==============================================================================
    int getNumPrograms() override                                               { return 1; }
    int getCurrentProgram() override                                            { return 0; }
    void setCurrentProgram (int /*index*/) override                             {}
    const String getProgramName (int /*index*/) override                        { return String(); }
    void changeProgramName (int /*index*/, const String& /*name*/) override     {}

    //==============================================================================
    void getStateInformation (MemoryBlock&) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    bool updateCurrentTimeInfoFromHost (AudioPlayHead::CurrentPositionInfo&);

    int lastUIWidth, lastUIHeight;

    AudioParameterFloat* A1param;
    AudioParameterFloat* Q1param;
    AudioParameterFloat* F1param;
    AudioParameterFloat* A2param;
    AudioParameterFloat* Q2param;
    AudioParameterFloat* F2param;
    AudioParameterFloat* A3param;
    AudioParameterFloat* Q3param;
    AudioParameterFloat* F3param;

    AudioParameterFloat* Gainparam;
    AudioParameterBool* powerParam;

    AudioParameterBool* LPenableParam;
    AudioParameterBool* HPenableParam;
    AudioParameterBool* LSenableParam;
    AudioParameterBool* HSenableParam;
    AudioParameterBool* BP1enableParam;
    AudioParameterBool* BP2enableParam;
    AudioParameterBool* BP3enableParam;

    AudioParameterFloat* LPfreqParam;
    AudioParameterFloat* LPgainParam;
    AudioParameterFloat* LPQParam;
    AudioParameterFloat* HPfreqParam;
    AudioParameterFloat* HPgainParam;
    AudioParameterFloat* HPQParam;

    AudioParameterBool* hightypeParam;
    AudioParameterFloat* highpassParam;
    AudioParameterBool* lowtypeParam;
    AudioParameterFloat* lowpassParam;

    IIRFilter LPFleft, LPFright, HPFleft, HPFright, LSFleft, LSFright, HSFleft, HSFright;
    IIRFilter HPAAleft, HPAAright;
    IIRFilter filter1L, filter1R, filter2L, filter3L, filter2R, filter3R;
    IIRCoefficients coeff1L, coeff1R, coeff2L, coeff2R, coeff3L, coeff3R, LScoeffleft, LScoeffright, HScoeffleft, HScoeffright;

private:
    //==============================================================================
    void process (AudioBuffer<float>& buffer, MidiBuffer& midiMessages);
    void mainEffectLoop (AudioBuffer<float>& buffer);
    AudioBuffer<float> smoothingBuffer;

    float Lpresum, Rpresum, Lpostsum, Rpostsum;
    float LE, LX, RE, RX;
    const float log2ov2 = log(2)/2;
    const float alpha = 0.9f;

    LinearSmoothedValue<double> A1, F1, Q1, A2, F2, Q2, A3, F3, Q3, SMLPgain, SMLPQ, SMLPfreq, SMHPgain, SMHPQ, SMHPfreq, lowfreq, highfreq;
    LinearSmoothedValue<double> Lgain, Rgain, gainout;

    bool LPenable, LPenableZ;
    bool LSenable, LSenableZ;
    bool HPenable, HPenableZ;
    bool HSenable, HSenableZ;
    bool BP1enable, BP1enableZ;
    bool BP2enable, BP2enableZ;
    bool BP3enable, BP3enableZ;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PEQAudioProcessor)
};

#endif  // PLUGINPROCESSOR_H_INCLUDED
