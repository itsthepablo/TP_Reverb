#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class VitalReverbAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    VitalReverbAudioProcessorEditor(VitalReverbAudioProcessor&);
    ~VitalReverbAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    VitalReverbAudioProcessor& audioProcessor;

    // Componentes de Interfaz para los 10 parámetros
    juce::Slider slLowCut, slHighCut, slCutoff, slGain, slChorAmt;
    juce::Slider slChorFreq, slDelay, slSize, slMix, slTime;

    juce::Label lbLowCut, lbHighCut, lbCutoff, lbGain, lbChorAmt;
    juce::Label lbChorFreq, lbDelay, lbSize, lbMix, lbTime;

    // Attachments
    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<Attachment> attLowCut, attHighCut, attCutoff, attGain, attChorAmt;
    std::unique_ptr<Attachment> attChorFreq, attDelay, attSize, attMix, attTime;

    void setupSlider(juce::Slider& slider, juce::Label& label, const juce::String& text);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VitalReverbAudioProcessorEditor)
};