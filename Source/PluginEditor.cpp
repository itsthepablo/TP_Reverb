#include "PluginProcessor.h"
#include "PluginEditor.h"

VitalReverbAudioProcessorEditor::VitalReverbAudioProcessorEditor(VitalReverbAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    // Fila 1
    setupSlider(slTime, lbTime, "TIME");
    setupSlider(slSize, lbSize, "SIZE");
    setupSlider(slDelay, lbDelay, "DELAY");
    setupSlider(slMix, lbMix, "MIX");
    setupSlider(slGain, lbGain, "GAIN");

    // Fila 2
    setupSlider(slLowCut, lbLowCut, "LOW CUT");
    setupSlider(slHighCut, lbHighCut, "HIGH CUT");
    setupSlider(slCutoff, lbCutoff, "CUTTOFF");
    setupSlider(slChorFreq, lbChorFreq, "CHOR FREQ");
    setupSlider(slChorAmt, lbChorAmt, "CHOR AMT");

    // Conexión segura al APVTS
    attTime = std::make_unique<Attachment>(audioProcessor.apvts, "time", slTime);
    attSize = std::make_unique<Attachment>(audioProcessor.apvts, "size", slSize);
    attDelay = std::make_unique<Attachment>(audioProcessor.apvts, "delay", slDelay);
    attMix = std::make_unique<Attachment>(audioProcessor.apvts, "mix", slMix);
    attGain = std::make_unique<Attachment>(audioProcessor.apvts, "gain", slGain);

    attLowCut = std::make_unique<Attachment>(audioProcessor.apvts, "low_cut", slLowCut);
    attHighCut = std::make_unique<Attachment>(audioProcessor.apvts, "high_cut", slHighCut);
    attCutoff = std::make_unique<Attachment>(audioProcessor.apvts, "cutoff", slCutoff);
    attChorFreq = std::make_unique<Attachment>(audioProcessor.apvts, "chor_freq", slChorFreq);
    attChorAmt = std::make_unique<Attachment>(audioProcessor.apvts, "chor_amt", slChorAmt);

    setSize(600, 300); // Tamaño adaptado para 2 filas de 5 controles
}

VitalReverbAudioProcessorEditor::~VitalReverbAudioProcessorEditor()
{
}

void VitalReverbAudioProcessorEditor::setupSlider(juce::Slider& slider, juce::Label& label, const juce::String& text)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    addAndMakeVisible(slider);

    label.setText(text, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.attachToComponent(&slider, false);
    addAndMakeVisible(label);
}

void VitalReverbAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour::fromRGB(30, 32, 35));
    g.setColour(juce::Colours::white);
    g.setFont(24.0f);
    g.drawFittedText("VITAL REVERB", getLocalBounds().removeFromTop(40), juce::Justification::centred, 1);
}

void VitalReverbAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(20);
    area.removeFromTop(40); // Espacio para el título

    auto halfHeight = area.getHeight() / 2;
    auto row1 = area.removeFromTop(halfHeight);
    auto row2 = area;

    // Fila 1: 5 controles
    int width = row1.getWidth() / 5;
    slTime.setBounds(row1.removeFromLeft(width).reduced(10));
    slSize.setBounds(row1.removeFromLeft(width).reduced(10));
    slDelay.setBounds(row1.removeFromLeft(width).reduced(10));
    slMix.setBounds(row1.removeFromLeft(width).reduced(10));
    slGain.setBounds(row1.removeFromLeft(width).reduced(10));

    // Fila 2: 5 controles
    slLowCut.setBounds(row2.removeFromLeft(width).reduced(10));
    slHighCut.setBounds(row2.removeFromLeft(width).reduced(10));
    slCutoff.setBounds(row2.removeFromLeft(width).reduced(10));
    slChorFreq.setBounds(row2.removeFromLeft(width).reduced(10));
    slChorAmt.setBounds(row2.removeFromLeft(width).reduced(10));
}