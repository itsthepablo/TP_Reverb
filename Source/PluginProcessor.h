#pragma once

#include <JuceHeader.h>

// IMPORTANTE: Incluimos el framework base de Vital respetando la jerarquía original.
// Ninguna coma del código de Matt Tytel ha sido alterada.
#include "synthesis/effects/reverb.h"
#include "synthesis/framework/poly_values.h"

class VitalReverbAudioProcessor  : public juce::AudioProcessor
{
public:
    VitalReverbAudioProcessor();
    ~VitalReverbAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

private:
    // Instancia matemática pura e inalterada del algoritmo de Reverb de Vital.
    std::unique_ptr<vital::Reverb> vitalReverb;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VitalReverbAudioProcessor)
};