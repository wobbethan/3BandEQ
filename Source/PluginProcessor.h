/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

enum Slope {
    Slope_12,
    Slope_24,
    Slope_36,
    Slope_48
};


struct ChainSettings {
    float peakFreq{ 0 }, peakGainInDecibels{ 0 }, peakQuality{ 1.f };
    float lowCutFreq{ 0 }, highCutFreq{ 0 };
    int lowCutSlope{ 0 }, highCutSlope{ 0 };
};

ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts);

//==============================================================================
/**
*/
class HackAThonAudioProcessor : public juce::AudioProcessor
#if JucePlugin_Enable_ARA
    , public juce::AudioProcessorARAExtension
#endif
{
public:
    //==============================================================================
    HackAThonAudioProcessor();
    ~HackAThonAudioProcessor() override;

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

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
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState apvts{ *this, nullptr, "Parameters", createParameterLayout()};

private:
    using Filter = juce::dsp::IIR::Filter<float>;
    using CutFilter = juce::dsp::ProcessorChain<Filter, Filter, Filter, Filter>;
    using MonoChain = juce::dsp::ProcessorChain<CutFilter, Filter, CutFilter>;

    MonoChain leftChain, rightChain;
    enum ChainPositions {
        LowCut,
        Peak,
        HighCut
    };
    void updatePeakFilter(const ChainSettings& chainSettings);
    using Coefficients = Filter::CoefficientsPtr;
    static void updateCoefficients(Coefficients old, const Coefficients replacement);

    template<typename ChainType, typename CoefficientType>
    void updateLowCutFilter(ChainType& rightLowCut, const CoefficientType& cutCoefficients, const ChainSettings& chainSettings) {

             rightLowCut.setBypassed<0>(true);
             rightLowCut.setBypassed<1>(true);
             rightLowCut.setBypassed<2>(true);
             rightLowCut.setBypassed<3>(true);

             switch (chainSettings.lowCutSlope) {

             case Slope_12:
             {
                 *rightLowCut.template get<0>().coefficients = *cutCoefficients[0];
                 rightLowCut.template setBypassed<0>(false);
                 break;
             }
             case Slope_24:
             {
                 *rightLowCut.template get<0>().coefficients = *cutCoefficients[0];
                 rightLowCut.template setBypassed<0>(false);
                 *rightLowCut.template get<1>().coefficients = *cutCoefficients[0];
                 rightLowCut.setBypassed<1>(false);
                 break;
             }
             case Slope_36: {
                 *rightLowCut.template get<0>().coefficients = *cutCoefficients[0];
                 rightLowCut.template setBypassed<0>(false);
                 *rightLowCut.template get<1>().coefficients = *cutCoefficients[0];
                 rightLowCut.template setBypassed<1>(false);
                 *rightLowCut.template get<2>().coefficients = *cutCoefficients[0];
                 rightLowCut.template setBypassed<2>(false);

                 break;
             }
             case Slope_48: {
                 *rightLowCut.template get<0>().coefficients = *cutCoefficients[0];
                 rightLowCut.template setBypassed<0>(false);
                 *rightLowCut.template get<1>().coefficients = *cutCoefficients[0];
                 rightLowCut.template setBypassed<1>(false);
                 *rightLowCut.template get<2>().coefficients = *cutCoefficients[0];
                 rightLowCut.template setBypassed<2>(false);
                 *rightLowCut.template get<3>().coefficients = *cutCoefficients[0];
                 rightLowCut.template setBypassed<3>(false);

                 break;
               }
             }
    }
    template<typename ChainType, typename CoefficientType>
    void updateHighCutFilter(ChainType& rightHighCut, const CoefficientType& cutCoefficients, const ChainSettings& chainSettings) {

        rightHighCut.setBypassed<0>(true);
        rightHighCut.setBypassed<1>(true);
        rightHighCut.setBypassed<2>(true);
        rightHighCut.setBypassed<3>(true);

        switch (chainSettings.highCutSlope) {

        case Slope_48:
        {
            *rightHighCut.template get<0>().coefficients = *cutCoefficients[0];
            rightHighCut.template setBypassed<0>(false);
            break;
        }
        case Slope_36:
        {
            *rightHighCut.template get<0>().coefficients = *cutCoefficients[0];
            rightHighCut.template setBypassed<0>(false);
            *rightHighCut.template get<1>().coefficients = *cutCoefficients[0];
            rightHighCut.setBypassed<1>(false);
            break;
        }
        case Slope_24: {
            *rightHighCut.template get<0>().coefficients = *cutCoefficients[0];
            rightHighCut.template setBypassed<0>(false);
            *rightHighCut.template get<1>().coefficients = *cutCoefficients[0];
            rightHighCut.template setBypassed<1>(false);
            *rightHighCut.template get<2>().coefficients = *cutCoefficients[0];
            rightHighCut.template setBypassed<2>(false);

            break;
        }
        case Slope_12: {
            *rightHighCut.template get<0>().coefficients = *cutCoefficients[0];
            rightHighCut.template setBypassed<0>(false);
            *rightHighCut.template get<1>().coefficients = *cutCoefficients[0];
            rightHighCut.template setBypassed<1>(false);
            *rightHighCut.template get<2>().coefficients = *cutCoefficients[0];
            rightHighCut.template setBypassed<2>(false);
            *rightHighCut.template get<3>().coefficients = *cutCoefficients[0];
            rightHighCut.template setBypassed<3>(false);

            break;
        }
        }
    }
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HackAThonAudioProcessor)
};
