#include "PluginProcessor.h"

VitalReverbAudioProcessor::VitalReverbAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    )
#endif
{
    // Instanciamos el módulo de Reverb de Vital
    vitalReverb = std::make_unique<vital::Reverb>();
}

VitalReverbAudioProcessor::~VitalReverbAudioProcessor()
{
}

const juce::String VitalReverbAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

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
        vitalReverb->reset(vital::constants::kFullMask);
    }
}

void VitalReverbAudioProcessor::releaseResources()
{
}

bool VitalReverbAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    if (layouts.getMainInputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return true;
}

void VitalReverbAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    int numSamples = buffer.getNumSamples();
    float* leftChannel = buffer.getWritePointer(0);
    float* rightChannel = buffer.getWritePointer(1);

    // ====================================================================================
    // PUENTE SIMD CORREGIDO: Usando kSize y el operador nativo [] de poly_values.h
    // ====================================================================================
    constexpr int vectorSize = vital::poly_float::kSize;

    for (int i = 0; i < numSamples; i += vectorSize)
    {
        int remaining = std::min(vectorSize, numSamples - i);

        alignas(16) float tempLeft[vectorSize] = { 0 };
        alignas(16) float tempRight[vectorSize] = { 0 };

        for (int v = 0; v < remaining; ++v)
        {
            tempLeft[v] = leftChannel[i + v];
            tempRight[v] = rightChannel[i + v];
        }

        // Empaquetamos a SIMD usando la instrucción nativa de carga (load) de Vital
        vital::poly_float polyInLeft = vital::poly_float(vital::poly_float::load(tempLeft));
        vital::poly_float polyInRight = vital::poly_float(vital::poly_float::load(tempRight));

        // Inyectamos a los pines del Reverb
        vitalReverb->input(vital::Reverb::kAudio)->source->buffer[0] = polyInLeft;
        vitalReverb->input(vital::Reverb::kAudio)->source->buffer[1] = polyInRight;

        // Procesamos 1 ciclo de Vector
        vitalReverb->process(1);

        // Extraemos los resultados
        vital::poly_float polyOutLeft = vitalReverb->output(vital::Reverb::kAudio)->buffer[0];
        vital::poly_float polyOutRight = vitalReverb->output(vital::Reverb::kAudio)->buffer[1];

        // Desempaquetamos a la pista de audio de JUCE
        for (int v = 0; v < remaining; ++v)
        {
            leftChannel[i + v] = polyOutLeft[v];
            rightChannel[i + v] = polyOutRight[v];
        }
    }
}

juce::AudioProcessorEditor* VitalReverbAudioProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor(*this);
}

bool VitalReverbAudioProcessor::hasEditor() const
{
    return true;
}

void VitalReverbAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
}

void VitalReverbAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new VitalReverbAudioProcessor();
}