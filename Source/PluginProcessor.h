#pragma once

#include <JuceHeader.h>
#include "synthesis/effects/reverb.h"
#include "synthesis/framework/poly_values.h"
#include "synthesis/framework/value.h"

class VitalReverbAudioProcessor : public juce::AudioProcessor
{
public:
    VitalReverbAudioProcessor();
    ~VitalReverbAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // Árbol de parámetros para conectar con la Interfaz Gráfica
    juce::AudioProcessorValueTreeState apvts;

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    std::unique_ptr<vital::Reverb> vitalReverb;

    // Nodos de control de Vital: Los 10 parámetros del usuario
    std::unique_ptr<vital::Value> valAudioIn;
    std::unique_ptr<vital::Value> valLowCut;    // LOW CUT
    std::unique_ptr<vital::Value> valHighCut;   // HIGH CUT
    std::unique_ptr<vital::Value> valCutoff;    // CUTTOFF
    std::unique_ptr<vital::Value> valGain;      // GAIN
    std::unique_ptr<vital::Value> valChorAmt;   // CHOR AMT
    std::unique_ptr<vital::Value> valChorFreq;  // CHOR FREQ
    std::unique_ptr<vital::Value> valDelay;     // DELAY
    std::unique_ptr<vital::Value> valSize;      // SIZE
    std::unique_ptr<vital::Value> valMix;       // MIX
    std::unique_ptr<vital::Value> valTime;      // TIME

    // Nodos Ocultos de estabilidad para el DSP
    std::unique_ptr<vital::Value> valWidthHidden;
    std::unique_ptr<vital::Value> valLowShelfCutHidden;
    std::unique_ptr<vital::Value> valLowShelfGainHidden;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VitalReverbAudioProcessor)
};