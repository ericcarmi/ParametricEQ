#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PEQAudioProcessor::PEQAudioProcessor()
    : lastUIWidth (400),
      lastUIHeight (600),
      A1param(nullptr),
      Q1param (nullptr),
      F1param (nullptr),
      A2param(nullptr),
      Q2param (nullptr),
      F2param (nullptr),
      A3param(nullptr),
      Q3param (nullptr),
      F3param (nullptr),
      Gainparam (nullptr),
      powerParam(nullptr),
      LPenableParam(nullptr), HPenableParam(nullptr), LSenableParam(nullptr), HSenableParam(nullptr),
      LPfreqParam(nullptr), LPgainParam(nullptr), LPQParam(nullptr),
      HPfreqParam(nullptr), HPgainParam(nullptr), HPQParam(nullptr),
      hightypeParam(nullptr),highpassParam(nullptr),
      lowtypeParam(nullptr),lowpassParam(nullptr),
      LE(1.0f),LX(1.0f),RE(1.0f),RX(1.0f),Lgain(1.0f),Rgain(1.0f)

{

    addParameter (A1param = new AudioParameterFloat ("Gain1", "gain1", -12.0f, 12.0f, 0.0f));
    addParameter (Q1param = new AudioParameterFloat ("Q1", "q1", 0.01f, 0.99f, 0.5f));
  //  addParameter (Q1param = new AudioParameterFloat ("BW", "BW", 0.5, 5.0, 2.0));
    addParameter (F1param = new AudioParameterFloat ("Freq1", "freq1", 100.0f, 10000.0f, 500.0f));

    addParameter (A2param = new AudioParameterFloat ("Gain2", "gain2", -12.0f, 12.0f, 0.0f));
    addParameter (Q2param = new AudioParameterFloat ("Q2", "q2", 0.01f, 0.99f, 0.5f));
   // addParameter (Q2param = new AudioParameterFloat ("BW", "BW", 0.5, 5.0, 2.0));
    addParameter (F2param = new AudioParameterFloat ("Freq2", "freq2", 100.0f, 10000.0f, 2000.0f));

    addParameter (A3param = new AudioParameterFloat ("Gain3", "gain3", -12.0f, 12.0f, 0.0f));
    addParameter (Q3param = new AudioParameterFloat ("Q3", "q3", 0.01f, 0.99f, 0.5f));
   // addParameter (Q3param = new AudioParameterFloat ("BW", "BW", 0.5, 5.0, 2.0));
    addParameter (F3param = new AudioParameterFloat ("Freq3", "freq3", 100.0f, 10000.0f, 5000.0f));

    addParameter(Gainparam = new AudioParameterFloat("Gain", "gain", -12.0f, 12.0f, 0.0f));
    addParameter(powerParam = new AudioParameterBool("Power", "power", true));

    addParameter(LPenableParam = new AudioParameterBool("LPEnable", "LP enable", false));
    addParameter(HPenableParam = new AudioParameterBool("HPEnable", "HP enable", false));
    addParameter(LSenableParam = new AudioParameterBool("LSEnable", "LS enable", false));
    addParameter(HSenableParam = new AudioParameterBool("HSEnable", "HS enable", false));
    addParameter(BP1enableParam = new AudioParameterBool("BP1Enable", "BP1 enable", false));
    addParameter(BP2enableParam = new AudioParameterBool("BP2Enable", "BP2 enable", false));
    addParameter(BP3enableParam = new AudioParameterBool("BP3Enable", "BP3 enable", false));

    addParameter(highpassParam = new AudioParameterFloat("Highpassfreq", "highpassfreq", 1.0f, 600.0f, 200.0f));
    addParameter(lowpassParam = new AudioParameterFloat ("Lowpassfreq", "lowpassfreq", 500.0f, 10000.0f, 5000.0f));
    addParameter(lowtypeParam = new AudioParameterBool("Lowtypetoggle", "lowtypetoggle", true));
    addParameter(hightypeParam = new AudioParameterBool("Hightypetoggle", "hightypetoggle", true));

    addParameter (LPgainParam = new AudioParameterFloat ("LSgain", "lsgain", -12.0f, 12.0f, 0.0f));
    addParameter (LPQParam = new AudioParameterFloat ("LSQ", "lsq", 0.01f, 0.99f, 0.1f));
    addParameter (LPfreqParam = new AudioParameterFloat ("LSfreq", "lsfreq", 200.0f, 5000.0f, 1000.0f));

    addParameter (HPgainParam = new AudioParameterFloat ("HSgain", "hsgain", -12.0f, 12.0f, 0.0f));
    addParameter (HPQParam = new AudioParameterFloat ("HSQ", "hsq", 0.01f, 0.99f, 0.1f));
    addParameter (HPfreqParam = new AudioParameterFloat ("HSfreq", "hsfreq", 1.0f, 600.0f, 200.0f));

}

PEQAudioProcessor::~PEQAudioProcessor()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool PEQAudioProcessor::setPreferredBusArrangement (bool isInput, int bus, const AudioChannelSet& preferredSet)
{
    const int numChannels = preferredSet.size();

#if JucePlugin_IsMidiEffect
    if (numChannels != 0)
        return false;
#elif JucePlugin_IsSynth
    if (isInput || (numChannels != 1 && numChannels != 2))
        return false;
#else
    if (numChannels != 1 && numChannels != 2)
        return false;

    if (! setPreferredBusArrangement (! isInput, bus, preferredSet))
        return false;
#endif

    return setPreferredBusArrangement (isInput, bus, preferredSet);
}
#endif

bool PEQAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool PEQAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool PEQAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainInputChannelSet() != AudioChannelSet::stereo())
        return false;

    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}

//==============================================================================
void PEQAudioProcessor::prepareToPlay (double newSampleRate, int samplesPerBlock)
{
    smoothingBuffer.setSize(2, samplesPerBlock);
    reset();
}

void PEQAudioProcessor::releaseResources()
{

}

void PEQAudioProcessor::reset()
{
    const double SR = getSampleRate();

    A1.reset(SR, 0.1);
    Q1.reset(SR, 0.1);
    F1.reset(SR, 0.1);
    A2.reset(SR, 0.1);
    Q2.reset(SR, 0.1);
    F2.reset(SR, 0.1);
    A3.reset(SR, 0.1);
    Q3.reset(SR, 0.1);
    F3.reset(SR, 0.1);

    lowfreq.reset(SR,0.1);
    highfreq.reset(SR,0.1);

    SMHPgain.reset(SR, 0.1);
    SMHPQ.reset(SR, 0.1);
    SMHPfreq.reset(SR, 0.1);

    SMLPgain.reset(SR, 0.1);
    SMLPQ.reset(SR, 0.1);
    SMLPfreq.reset(SR, 0.1);

    gainout.reset(SR, 0.1);

}

void PEQAudioProcessor::process (AudioBuffer<float>& buffer,MidiBuffer& midiMessages)
{
    const int numSamples = buffer.getNumSamples();

    for (int i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
        buffer.clear (i, 0, numSamples);

    if( *powerParam )
    {
        mainEffectLoop(buffer);
    }

    LPenableZ = LPenable;
    HPenableZ = HPenable;
    LSenableZ = LSenable;
    HSenableZ = HSenable;
    BP1enableZ = BP1enable;
    BP2enableZ = BP2enable;
    BP3enableZ = BP3enable;

}

void PEQAudioProcessor::mainEffectLoop (AudioBuffer<float>& buffer)
{
    int nums = buffer.getNumSamples();
    int chans = buffer.getNumChannels();
    const double sampRate = getSampleRate();
    const double Fn = 2.0 * double_Pi / sampRate;

    A1.setValue((*A1param));
    Q1.setValue(((*Q1param)));
    F1.setValue((*F1param));
    A2.setValue((*A2param));
    Q2.setValue(((*Q2param)));
    F2.setValue((*F2param));
    A3.setValue((*A3param));
    Q3.setValue(((*Q3param)));
    F3.setValue((*F3param));

    lowfreq.setValue(*lowpassParam);
    highfreq.setValue(*highpassParam);

    SMLPQ.setValue(*LPQParam);
    SMLPfreq.setValue(*LPfreqParam);
    SMLPgain.setValue(*LPgainParam);

    SMHPQ.setValue(*HPQParam);
    SMHPfreq.setValue(*HPfreqParam);
    SMHPgain.setValue(*HPgainParam);
    gainout.setValue(std::pow(10,(*Gainparam)/20.0));

    const bool filt = LPenable | LSenable | HSenable | HPenable | BP1enable | BP2enable | BP3enable;
    Lpostsum = Lpresum = Rpostsum = Rpresum = 0.0f;

    if(filt)
    {
    if( chans == 2 )
    {

        float* leftChannel = buffer.getWritePointer (0);
        float* rightChannel = buffer.getWritePointer (1);

        for( int samp = 0; samp < nums; ++samp)
        {

            LPenable = *LPenableParam;
            HPenable = *HPenableParam;
            BP1enable = *BP1enableParam;
            BP2enable = *BP2enableParam;
            BP3enable = *BP3enableParam;
            LSenable = *LSenableParam;
            HSenable = *HSenableParam;

            const double g1 = pow(10, A1.getNextValue()/20.0f);                                  // A: gain
            const double w1 = Fn * F1.getNextValue();                                          // F: frequency
            const double a1 = sin(w1) / (2.0f * Q1.getNextValue());                            // Q: bandwidth
           // const float sw1 = sin(w1);
            //const float a1 = sw1*sinh(log2ov2 * Q1.getNextValue() * w1/sw1);

            const double g2 = pow(10, A2.getNextValue()/20.0f);
            const double w2 = Fn * F2.getNextValue();
            const double a2 = sin(w2) / (2.0f * Q2.getNextValue());
          //  const float sw2 = sin(w2);
            //const float a2 = sw2*sinh(log2ov2 * Q2.getNextValue() * w2/sw2);

            const double g3 = pow(10, A3.getNextValue()/20.0f);
            const double w3 = Fn * F3.getNextValue();
            const double a3 = sin(w3) / (2.0f * Q3.getNextValue());
         //   const float sw3 = sin(w3);
            //const float a3 = sw3*sinh(log2ov2 * Q3.getNextValue() * w3/sw3);

            // Get energy before effect is applied
            const float Ltemp = leftChannel[samp];
            const float Rtemp = rightChannel[samp];

            Lpresum += Ltemp*Ltemp;                                                        // Sum up energy before effect
            Rpresum += Rtemp*Rtemp;

            // 3 Bandpass filters
            filter1L.setCoefficients(IIRCoefficients(1.0 + a1*g1, -2.0*cos(w1), 1.0 - a1*g1, 1.0 + a1/g1, -2.0*cos(w1), 1.0 - a1/g1));
            filter1R.setCoefficients(IIRCoefficients(1.0 + a1*g1, -2.0*cos(w1), 1.0 - a1*g1, 1.0 + a1/g1, -2.0*cos(w1), 1.0 - a1/g1));
            filter2L.setCoefficients(IIRCoefficients(1.0 + a2*g2, -2.0*cos(w2), 1.0 - a2*g2, 1.0 + a2/g2, -2.0*cos(w2), 1.0 - a2/g2));
            filter2R.setCoefficients(IIRCoefficients(1.0 + a2*g2, -2.0*cos(w2), 1.0 - a2*g2, 1.0 + a2/g2, -2.0*cos(w2), 1.0 - a2/g2));
            filter3L.setCoefficients(IIRCoefficients(1.0 + a3*g3, -2.0*cos(w3), 1.0 - a3*g3, 1.0 + a3/g3, -2.0*cos(w3), 1.0 - a3/g3));
            filter3R.setCoefficients(IIRCoefficients(1.0 + a3*g3, -2.0*cos(w3), 1.0 - a3*g3, 1.0 + a3/g3, -2.0*cos(w3), 1.0 - a3/g3));

            const double LPfreq = SMLPfreq.getNextValue();
            const double LPQ = SMLPQ.getNextValue();
            const double LPgain = pow(10,SMLPgain.getNextValue()/20.0);
            const double HPfreq = SMHPfreq.getNextValue();
            const double HPQ = SMHPQ.getNextValue();
            const double HPgain = pow(10,SMHPgain.getNextValue()/20.0);
            const double lowF = lowfreq.getNextValue();
            const double highF = highfreq.getNextValue();

            LPFleft.setCoefficients(coeff1L.makeLowPass(sampRate, lowF));
            LPFright.setCoefficients(coeff1R.makeLowPass(sampRate, lowF));

            LSFleft.setCoefficients(LScoeffleft.makeLowShelf(sampRate, LPfreq, LPQ, LPgain));
            LSFright.setCoefficients(LScoeffright.makeLowShelf(sampRate, LPfreq, LPQ, LPgain));

            HPFleft.setCoefficients(coeff2L.makeHighPass(sampRate, highF));
            HPFright.setCoefficients(coeff2R.makeHighPass(sampRate, highF));

            HSFleft.setCoefficients(HScoeffleft.makeHighShelf(sampRate, HPfreq, HPQ, HPgain));
            HSFright.setCoefficients(HScoeffleft.makeHighShelf(sampRate, HPfreq, HPQ, HPgain));

            float Lout = 0.0f;
            float Rout = 0.0f;

            if( LPenable )
            {
                const float LPLeft = LPFleft.processSingleSampleRaw(Ltemp);
                const float LPRight = LPFright.processSingleSampleRaw(Rtemp);
                Lout += LPLeft;
                Rout += LPRight;
            }
            if( HPenable )
            {
                const float HPLeft = (HPFleft.processSingleSampleRaw(Ltemp));
                const float HPRight = (HPFright.processSingleSampleRaw(Rtemp));
                Lout += HPLeft;
                Rout += HPRight;
            }
            if( LSenable )
            {
                const float LSLeft = LSFleft.processSingleSampleRaw(Ltemp);
                const float LSRight = LSFright.processSingleSampleRaw(Rtemp);
                Lout += LSLeft;
                Rout += LSRight;

            }
            if( HSenable )
            {
                const float HSLeft = (HSFleft.processSingleSampleRaw(Ltemp));
                const float HSRight = (HSFright.processSingleSampleRaw(Rtemp));
                Lout += HSLeft;
                Rout += HSRight;
            }
            if(BP1enable)
            {
                const float BP1Left = filter1L.processSingleSampleRaw(Ltemp);
                const float BP1Right = filter1R.processSingleSampleRaw(Rtemp);
                Lout += BP1Left;
                Rout += BP1Right;

            }
            if(BP2enable)
            {
                const float BP2Left = filter2L.processSingleSampleRaw(Ltemp);
                const float BP2Right = filter2R.processSingleSampleRaw(Rtemp);
                Lout += BP2Left;
                Rout += BP2Right;
            }
            if(BP3enable)
            {
                const float BP3Left = filter3L.processSingleSampleRaw(Ltemp);
                const float BP3Right = filter3R.processSingleSampleRaw(Rtemp);
                Lout += BP3Left;
                Rout += BP3Right;
            }

            const float Ltempo = Lout;
            const float Rtempo = Rout;
            const double tempgain = gainout.getNextValue();

            leftChannel[samp]  = Ltempo * tempgain;                               // Apply output gain
            rightChannel[samp] = Rtempo * tempgain;

            Lpostsum += Ltempo*Ltempo * tempgain;                                 // Sum up  energy after effect
            Rpostsum += Rtempo*Rtempo * tempgain;

        }
        // Apply gain ramp based on ratio of energies, including LP filtering of the ratio for smoothness' sake
        // Check left and right individually so that it doesn't go to 0 if only one channel is used (happened in the testing graph)
        if(Lpostsum > 0.0)
        {
            LX = sqrtf(Lpresum/Lpostsum) + (1-alpha) * LE;
            buffer.applyGainRamp(0, 0, nums, LE, LX);
        }
        else
        {
            LX = (1-alpha) * LE;
            buffer.applyGainRamp(0, 0, nums, LE, LX);
        }
        if(Rpostsum > 0.0)
        {
            RX = sqrtf(Rpresum/Rpostsum) + (1-alpha) * RE;
            buffer.applyGainRamp(1, 0, nums, RE, RX);
        }
        else
        {
            RX = (1-alpha) * RE;
            buffer.applyGainRamp(1, 0, nums, RE, RX);
        }

        LE = LX;  // Delay
        RE = RX;

    }

    if( chans == 1 )
    {

        float* leftChannel = buffer.getWritePointer (0);

        for( int samp = 0; samp < nums; ++samp)
        {
            LPenable = *LPenableParam;
            HPenable = *HPenableParam;
            BP1enable = *BP1enableParam;
            BP2enable = *BP2enableParam;
            BP3enable = *BP3enableParam;
            LSenable = *LSenableParam;
            HSenable = *HSenableParam;

            // Change a_i to the equation that uses bandwidth instead of Q?
            // For GUI, make an option to switch between decades or octaves
            const double g1 = pow(10, A1.getNextValue()/20.0f);                                  // A: gain
            const double w1 = (Fn * F1.getNextValue());                                          // F: frequency
            const double a1 = sin(w1) / (2.0f * Q1.getNextValue());                              // Q: bandwidth

            const double g2 = pow(10, A2.getNextValue()/20.0f);
            const double w2 = (Fn * F2.getNextValue());
            const double a2 = sin(w2) / (2.0f * Q2.getNextValue());

            const double g3 = pow(10, A3.getNextValue()/20.0f);
            const double w3 = (Fn * F3.getNextValue());
            const double a3 = sin(w3) / (2.0f * Q3.getNextValue());

            const float Ltemp = leftChannel[samp];
            Lpresum += Ltemp*Ltemp;

            filter1L.setCoefficients(IIRCoefficients(1.0 + a1*g1, -2.0*cos(w1), 1.0 - a1*g1, 1.0 + a1/g1, -2.0*cos(w1), 1.0 - a1/g1));
            filter2L.setCoefficients(IIRCoefficients(1.0 + a2*g2, -2.0*cos(w2), 1.0 - a2*g2, 1.0 + a2/g2, -2.0*cos(w2), 1.0 - a2/g2));
            filter3L.setCoefficients(IIRCoefficients(1.0 + a3*g3, -2.0*cos(w3), 1.0 - a3*g3, 1.0 + a3/g3, -2.0*cos(w3), 1.0 - a3/g3));

            const double LPfreq = SMLPfreq.getNextValue();
            const double LPQ = SMLPQ.getNextValue();
            const double LPgain = pow(10,SMLPgain.getNextValue()/20.0);
            const double HPfreq = SMHPfreq.getNextValue();
            const double HPQ = SMHPQ.getNextValue();
            const double HPgain = pow(10,SMHPgain.getNextValue()/20.0);
            const double lowF = lowfreq.getNextValue();
            const double highF = highfreq.getNextValue();

            float Lout = 0.0f;

            if( LPenable )
            {
                LPFleft.setCoefficients(coeff1L.makeLowPass(sampRate, lowF));
                const float LPLeft = LPFleft.processSingleSampleRaw(Ltemp);
                Lout += LPLeft;
            }
            if( HPenable )
            {
                HPFleft.setCoefficients(coeff2L.makeHighPass(sampRate, highF));
                const float HPLeft = (HPFleft.processSingleSampleRaw(Ltemp));
                Lout += HPLeft;
            }
            if( LSenable )
            {
                LSFleft.setCoefficients(LScoeffleft.makeLowShelf(sampRate, LPfreq, LPQ, LPgain));
                const float LSLeft = LSFleft.processSingleSampleRaw(Ltemp);
                Lout += LSLeft;

            }
            if( HSenable )
            {
                HSFleft.setCoefficients(HScoeffleft.makeHighShelf(sampRate, HPfreq, HPQ, HPgain));
                const float HSLeft = (HSFleft.processSingleSampleRaw(Ltemp));
                Lout += HSLeft;
            }
            if(BP1enable)
            {
                const float BP1Left = filter1L.processSingleSampleRaw(Ltemp);
                Lout += BP1Left;

            }
            if(BP2enable)
            {
                const float BP2Left = filter2L.processSingleSampleRaw(Ltemp);
                Lout += BP2Left;
            }
            if(BP3enable)
            {
                const float BP3Left = filter3L.processSingleSampleRaw(Ltemp);
                Lout += BP3Left;
            }

            const float Ltempo = Lout;
            Lpostsum += Ltempo*Ltempo;
            const float tempgain = gainout.getNextValue();
            leftChannel[samp]  = Ltempo * tempgain;                               // Apply output gain

        }

        if( (Lpresum > 0.0) & (Lpostsum > 0.0))
        {
            LX = alpha * sqrtf(Lpresum/Lpostsum) + (1-alpha) * LE;
            buffer.applyGainRamp(0, 0, nums, LE, LX);
        }
        else
        {
            LX = (1-alpha) * LE;
            buffer.applyGainRamp(0, 0, nums, LE, LX);
        }

        LE = LX;
    }
    }
    else
    {

        if( chans == 2 )
        {

            float* leftChannel = buffer.getWritePointer (0);
            float* rightChannel = buffer.getWritePointer (1);

            for( int samp = 0; samp < nums; ++samp)
            {
                // Get energy before effect is applied
                const float Ltempo = leftChannel[samp];
                const float Rtempo = rightChannel[samp];

                const float tempgain = gainout.getNextValue();

                leftChannel[samp]  = Ltempo * tempgain;                               // Apply output gain
                rightChannel[samp] = Rtempo * tempgain;
            }
        }

        if( chans == 1 )
        {
            float* leftChannel = buffer.getWritePointer (0);

            for( int samp = 0; samp < nums; ++samp)
            {
                const float Ltempo = leftChannel[samp];
                const float tempgain = gainout.getNextValue();
                leftChannel[samp]  = Ltempo * tempgain;                               // Apply output gain
            }
        }
    }
}

//==============================================================================
AudioProcessorEditor* PEQAudioProcessor::createEditor()
{
    return new PEQAudioProcessorEditor (*this);
}

//==============================================================================
void PEQAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // Here's an example of how you can use XML to make it easy and more robust:

    // Create an outer XML element..
    XmlElement xml ("EqualizerSETTINGS");

    // add some attributes to it..
    xml.setAttribute ("uiWidth", lastUIWidth);
    xml.setAttribute ("uiHeight", lastUIHeight);

    // Store the values of all our parameters, using their param ID as the XML attribute
    for (int i = 0; i < getNumParameters(); ++i)
        if (AudioProcessorParameterWithID* p = dynamic_cast<AudioProcessorParameterWithID*> (getParameters().getUnchecked(i)))
            xml.setAttribute (p->paramID, p->getValue());

    // then use this helper function to stuff it into the binary blob and return it..
    copyXmlToBinary (xml, destData);
}

void PEQAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.

    // This getXmlFromBinary() helper function retrieves our XML from the binary blob..
    ScopedPointer<XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState != nullptr)
    {
        // make sure that it's actually our type of XML object..
        if (xmlState->hasTagName ("EqualizerSETTINGS"))
        {
            // ok, now pull out our last window size..
            lastUIWidth  = jmax (xmlState->getIntAttribute ("uiWidth", lastUIWidth), 400);
            lastUIHeight = jmax (xmlState->getIntAttribute ("uiHeight", lastUIHeight), 200);

            // Now reload our parameters..
            for (int i = 0; i < getNumParameters(); ++i)
                if (AudioProcessorParameterWithID* p = dynamic_cast<AudioProcessorParameterWithID*> (getParameters().getUnchecked(i)))
                    p->setValueNotifyingHost ((float) xmlState->getDoubleAttribute (p->paramID, p->getValue()));
        }
    }
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PEQAudioProcessor();
}
