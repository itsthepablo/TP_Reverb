#include "PluginProcessor.h"
#include "PluginEditor.h"

// Conversión estricta de Hercios a Nota MIDI para el motor de Vital
inline float hzToMidi(float hz) {
    return 12.0f * std::log2(std::max(hz, 0.001f) / 440.0f) + 69.0f;
}

VitalReverbAudioProcessor::VitalReverbAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    ),
    apvts(*this, nullptr, "Parameters", createParameterLayout())
#else
    : apvts(*this, nullptr, "Parameters", createParameterLayout())
#endif
{
    vitalReverb = std::make_unique<vital::Reverb>();

    // Inicializamos los nodos
    valAudioIn = std::make_unique<vital::Value>();
    valLowCut = std::make_unique<vital::Value>();
    valHighCut = std::make_unique<vital::Value>();
    valCutoff = std::make_unique<vital::Value>();
    valGain = std::make_unique<vital::Value>();
    valChorAmt = std::make_unique<vital::Value>();
    valChorFreq = std::make_unique<vital::Value>();
    valDelay = std::make_unique<vital::Value>();
    valSize = std::make_unique<vital::Value>();
    valMix = std::make_unique<vital::Value>();
    valTime = std::make_unique<vital::Value>();

    valWidthHidden = std::make_unique<vital::Value>();
    valLowShelfCutHidden = std::make_unique<vital::Value>();
    valLowShelfGainHidden = std::make_unique<vital::Value>();

    // Conectamos al ruteo de Vital
    vitalReverb->plug(valAudioIn.get(), vital::Reverb::kAudio);

    // Mapeo de filtros Pre-Reverb
    vitalReverb->plug(valLowCut.get(), vital::Reverb::kPreLowCutoff);
    vitalReverb->plug(valHighCut.get(), vital::Reverb::kPreHighCutoff);

    // Mapeo de Damping (High Shelf Post-Reverb)
    vitalReverb->plug(valCutoff.get(), vital::Reverb::kHighCutoff);
    vitalReverb->plug(valGain.get(), vital::Reverb::kHighGain);

    // Mapeo de Efectos de Modulación y Tiempo
    vitalReverb->plug(valChorAmt.get(), vital::Reverb::kChorusAmount);
    vitalReverb->plug(valChorFreq.get(), vital::Reverb::kChorusFrequency);
    vitalReverb->plug(valDelay.get(), vital::Reverb::kDelay);
    vitalReverb->plug(valSize.get(), vital::Reverb::kSize);
    vitalReverb->plug(valMix.get(), vital::Reverb::kWet);
    vitalReverb->plug(valTime.get(), vital::Reverb::kDecayTime);

    // Pines ocultos para mantener estabilidad matemática idéntica a Vital
    vitalReverb->plug(valWidthHidden.get(), vital::Reverb::kStereoWidth);
    vitalReverb->plug(valLowShelfCutHidden.get(), vital::Reverb::kLowCutoff);
    vitalReverb->plug(valLowShelfGainHidden.get(), vital::Reverb::kLowGain);
}

VitalReverbAudioProcessor::~VitalReverbAudioProcessor()
{
}

juce::AudioProcessorValueTreeState::ParameterLayout VitalReverbAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Fila 1 (Filtros con Skew 0.3 para curva logarítmica exacta de Vital)
    params.push_back(std::make_unique<juce::AudioParameterFloat>("low_cut", "LOW CUT", juce::NormalisableRange<float>(10.0f, 2000.0f, 1.0f, 0.3f), 20.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("high_cut", "HIGH CUT", juce::NormalisableRange<float>(500.0f, 20000.0f, 1.0f, 0.3f), 20000.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("cutoff", "CUTTOFF", juce::NormalisableRange<float>(100.0f, 20000.0f, 1.0f, 0.3f), 5000.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("gain", "GAIN", juce::NormalisableRange<float>(-24.0f, 12.0f, 0.1f), -6.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("chor_amt", "CHOR AMT", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.0f));

    // Fila 2
    params.push_back(std::make_unique<juce::AudioParameterFloat>("chor_freq", "CHOR FREQ", juce::NormalisableRange<float>(0.1f, 20.0f, 0.01f, 0.4f), 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("delay", "DELAY", juce::NormalisableRange<float>(0.0f, 1.5f, 0.001f), 0.0f)); // Delay en segundos
    params.push_back(std::make_unique<juce::AudioParameterFloat>("size", "SIZE", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("mix", "MIX", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.3f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("time", "TIME", juce::NormalisableRange<float>(0.1f, 32.0f, 0.01f, 0.3f), 2.0f));

    return { params.begin(), params.end() };
}

const juce::String VitalReverbAudioProcessor::getName() const { return JucePlugin_Name; }
bool VitalReverbAudioProcessor::acceptsMidi() const { return false; }
bool VitalReverbAudioProcessor::producesMidi() const { return false; }
bool VitalReverbAudioProcessor::isMidiEffect() const { return false; }
double VitalReverbAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int VitalReverbAudioProcessor::getNumPrograms() { return 1; }
int VitalReverbAudioProcessor::getCurrentProgram() { return 0; }
void VitalReverbAudioProcessor::setCurrentProgram(int index) {}
const juce::String VitalReverbAudioProcessor::getProgramName(int index) { return {}; }
void VitalReverbAudioProcessor::changeProgramName(int index, const juce::String& newName) {}

void VitalReverbAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    if (vitalReverb != nullptr)
    {
        vitalReverb->setSampleRate(static_cast<int>(sampleRate));
        vitalReverb->reset(vital::constants::kFullMask);
    }
}

void VitalReverbAudioProcessor::releaseResources() {}

bool VitalReverbAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo()) return false;
    if (layouts.getMainInputChannelSet() != juce::AudioChannelSet::stereo()) return false;
    return true;
}

void VitalReverbAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // 1. Valores lineales (Rangos nativos procesados directamente)
    valGain->set(apvts.getRawParameterValue("gain")->load());
    valChorAmt->set(apvts.getRawParameterValue("chor_amt")->load());
    valChorFreq->set(apvts.getRawParameterValue("chor_freq")->load());
    valDelay->set(apvts.getRawParameterValue("delay")->load());
    valSize->set(apvts.getRawParameterValue("size")->load());
    valMix->set(apvts.getRawParameterValue("mix")->load());
    valTime->set(apvts.getRawParameterValue("time")->load());

    // 2. Valores logarítmicos (Conversión de Hz a Midi Notes requerida por el FDN de Vital)
    valLowCut->set(hzToMidi(apvts.getRawParameterValue("low_cut")->load()));
    valHighCut->set(hzToMidi(apvts.getRawParameterValue("high_cut")->load()));
    valCutoff->set(hzToMidi(apvts.getRawParameterValue("cutoff")->load()));

    // 3. Valores fijos ocultos para que Vital rutee estéreo puro y damping natural
    valWidthHidden->set(1.0f); // 100% estéreo
    valLowShelfCutHidden->set(hzToMidi(100.0f)); // Shelf de graves neutral en 100Hz
    valLowShelfGainHidden->set(0.0f); // 0dB de absorción en graves

    int numSamples = buffer.getNumSamples();
    float* leftChannel = buffer.getWritePointer(0);
    float* rightChannel = buffer.getWritePointer(1);

    for (int i = 0; i < numSamples; ++i)
    {
        vital::poly_float frame(leftChannel[i], rightChannel[i], 0.0f, 0.0f);
        valAudioIn->set(frame);

        vitalReverb->process(1);

        vital::poly_float out_frame = vitalReverb->output(0)->buffer[0];

        leftChannel[i] = out_frame[0];
        rightChannel[i] = out_frame[1];
    }
}

juce::AudioProcessorEditor* VitalReverbAudioProcessor::createEditor() { return new VitalReverbAudioProcessorEditor(*this); }
bool VitalReverbAudioProcessor::hasEditor() const { return true; }

void VitalReverbAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void VitalReverbAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr)
        if (xmlState->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new VitalReverbAudioProcessor(); }