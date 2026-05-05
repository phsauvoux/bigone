#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
    constexpr float twoPi = juce::MathConstants<float>::twoPi;

    float sine(float phase) { return std::sin(phase); }
    float saw(float phase) { return (phase / juce::MathConstants<double>::twoPi) * 2.0f - 1.0f; }
    float square(float phase) { return phase < juce::MathConstants<double>::pi ? 1.0f : -1.0f; }
    float tri(float phase) { return std::asin(std::sin(phase)) * (2.0f / juce::MathConstants<float>::pi); }

    float blend(float a, float b, float amount) { return a + (b - a) * juce::jlimit(0.0f, 1.0f, amount); }
}

PhSynthOneAudioProcessor::PhSynthOneAudioProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "PARAMETERS", createParameterLayout())
{
    updateParameterPointers();
}

PhSynthOneAudioProcessor::~PhSynthOneAudioProcessor() = default;

juce::AudioProcessorValueTreeState::ParameterLayout PhSynthOneAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterChoice>("soundType", "Sound Type",
        juce::StringArray { "Pad", "Bass", "Lead", "Pluck", "String" }, 0));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("attack", "Attack", juce::NormalisableRange<float>(0.001f, 4.0f, 0.001f, 0.35f), 0.35f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("decay", "Decay", juce::NormalisableRange<float>(0.01f, 3.0f, 0.001f, 0.45f), 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("sustain", "Sustain", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.72f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("release", "Release", juce::NormalisableRange<float>(0.01f, 6.0f, 0.001f, 0.45f), 1.2f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("cutoff", "Cutoff", juce::NormalisableRange<float>(80.0f, 18000.0f, 1.0f, 0.32f), 7200.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("resonance", "Resonance", juce::NormalisableRange<float>(0.1f, 1.0f, 0.001f), 0.25f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("reverb", "Reverb", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.28f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("delay", "Delay", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.18f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("pitch", "Pitch", juce::NormalisableRange<float>(-24.0f, 24.0f, 1.0f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("volume", "Volume", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.78f));

    return { params.begin(), params.end() };
}

void PhSynthOneAudioProcessor::updateParameterPointers()
{
    soundTypeParam = apvts.getRawParameterValue("soundType");
    attackParam = apvts.getRawParameterValue("attack");
    decayParam = apvts.getRawParameterValue("decay");
    sustainParam = apvts.getRawParameterValue("sustain");
    releaseParam = apvts.getRawParameterValue("release");
    cutoffParam = apvts.getRawParameterValue("cutoff");
    resonanceParam = apvts.getRawParameterValue("resonance");
    reverbParam = apvts.getRawParameterValue("reverb");
    delayParam = apvts.getRawParameterValue("delay");
    pitchParam = apvts.getRawParameterValue("pitch");
    volumeParam = apvts.getRawParameterValue("volume");
}

void PhSynthOneAudioProcessor::Voice::start(int midiNote, float vel, double)
{
    active = true;
    note = midiNote;
    velocity = vel;
    level = 0.0;
    releaseStart = 0.0;
    stage = attack;
}

void PhSynthOneAudioProcessor::Voice::stop()
{
    if (active && stage != off)
    {
        releaseStart = level;
        stage = release;
    }
}

float PhSynthOneAudioProcessor::Voice::render(double sr, float soundType, float attackTime, float decayTime, float sustainLevel,
                                              float releaseTime, float cutoff, float resonance, float pitch)
{
    if (!active) return 0.0f;

    const float noteOffset = soundType == 1.0f ? -12.0f : 0.0f;
    const double freq = juce::MidiMessage::getMidiNoteInHertz(note + pitch + noteOffset);
    const double detune = soundType == 0.0f ? 1.006 : soundType == 4.0f ? 1.004 : 1.002;
    const double inc1 = juce::MathConstants<double>::twoPi * freq / sr;
    const double inc2 = juce::MathConstants<double>::twoPi * freq * detune / sr;

    phase1 += inc1;
    phase2 += inc2;
    if (phase1 >= juce::MathConstants<double>::twoPi) phase1 -= juce::MathConstants<double>::twoPi;
    if (phase2 >= juce::MathConstants<double>::twoPi) phase2 -= juce::MathConstants<double>::twoPi;

    if (stage == attack)
    {
        level += 1.0 / juce::jmax(1.0, attackTime * sr);
        if (level >= 1.0) { level = 1.0; stage = decay; }
    }
    else if (stage == decay)
    {
        level -= (1.0 - sustainLevel) / juce::jmax(1.0, decayTime * sr);
        if (level <= sustainLevel) { level = sustainLevel; stage = sustain; }
    }
    else if (stage == sustain)
    {
        level = sustainLevel;
    }
    else if (stage == release)
    {
        level -= releaseStart / juce::jmax(1.0, releaseTime * sr);
        if (level <= 0.0001) { level = 0.0; active = false; stage = off; return 0.0f; }
    }

    float p1 = static_cast<float>(phase1);
    float p2 = static_cast<float>(phase2);
    float raw = 0.0f;

    if (soundType == 0.0f)
        raw = 0.46f * saw(p1) + 0.38f * saw(p2) + 0.16f * sine(p1 * 0.5f);
    else if (soundType == 1.0f)
        raw = 0.62f * square(p1) + 0.25f * sine(p1) + 0.13f * saw(p2);
    else if (soundType == 2.0f)
        raw = 0.54f * saw(p1) + 0.30f * square(p2) + 0.16f * sine(p1 * 2.0f);
    else if (soundType == 3.0f)
        raw = 0.78f * tri(p1) + 0.22f * saw(p2);
    else
        raw = 0.58f * tri(p1) + 0.32f * sine(p2) + 0.10f * saw(p1);

    filter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
    filter.setCutoffFrequency(juce::jlimit(60.0f, 19000.0f, cutoff));
    filter.setResonance(juce::jlimit(0.1f, 1.0f, resonance));

    return filter.processSample(0, raw) * static_cast<float>(level * velocity) * 0.28f;
}

PhSynthOneAudioProcessor::Voice& PhSynthOneAudioProcessor::findVoice()
{
    for (auto& voice : voices)
        if (!voice.active)
            return voice;
    return voices.front();
}

void PhSynthOneAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    juce::dsp::ProcessSpec spec { sampleRate, static_cast<juce::uint32>(samplesPerBlock), 2 };
    reverb.prepare(spec);
    delay.prepare(spec);
    delay.setMaximumDelayInSamples(static_cast<int>(sampleRate * 2.0));

    for (auto& voice : voices)
    {
        voice.filter.prepare({ sampleRate, static_cast<juce::uint32>(samplesPerBlock), 1 });
        voice.filter.reset();
    }
}

void PhSynthOneAudioProcessor::releaseResources() {}

bool PhSynthOneAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::mono()
        || layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void PhSynthOneAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

    for (const auto metadata : midiMessages)
    {
        auto msg = metadata.getMessage();
        if (msg.isNoteOn())
            findVoice().start(msg.getNoteNumber(), msg.getFloatVelocity(), currentSampleRate);
        else if (msg.isNoteOff())
            for (auto& voice : voices)
                if (voice.active && voice.note == msg.getNoteNumber()) voice.stop();
    }

    const auto type = soundTypeParam->load();
    const auto attack = attackParam->load();
    const auto decayValue = decayParam->load();
    const auto sustain = sustainParam->load();
    const auto release = releaseParam->load();
    const auto cutoffBase = cutoffParam->load();
    const auto resonance = resonanceParam->load();
    const auto pitch = pitchParam->load();
    const auto volume = volumeParam->load();
    const auto reverbMix = reverbParam->load();
    const auto delayMix = delayParam->load();

    auto cutoff = cutoffBase;
    if (type == 1.0f) cutoff = juce::jmin(cutoffBase, 2400.0f);
    if (type == 3.0f) cutoff = juce::jmin(cutoffBase, 5200.0f);

    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        float value = 0.0f;
        for (auto& voice : voices)
            value += voice.render(currentSampleRate, type, attack, decayValue, sustain, release, cutoff, resonance, pitch);

        value = std::tanh(value * (type == 1.0f ? 1.65f : 1.1f)) * volume;

        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
            buffer.setSample(channel, sample, value);
    }

    if (delayMix > 0.001f)
    {
        const int delaySamples = static_cast<int>(currentSampleRate * 0.375);
        delay.setDelay(delaySamples);
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
            {
                const auto dry = buffer.getSample(channel, sample);
                const auto wet = delay.popSample(channel);
                delay.pushSample(channel, dry + wet * 0.42f);
                buffer.setSample(channel, sample, blend(dry, wet, delayMix * 0.55f));
            }
        }
    }

    juce::dsp::Reverb::Parameters rv;
    rv.roomSize = 0.68f;
    rv.damping = 0.45f;
    rv.width = 0.95f;
    rv.wetLevel = reverbMix * 0.34f;
    rv.dryLevel = 1.0f - reverbMix * 0.18f;
    reverb.setParameters(rv);
    juce::dsp::AudioBlock<float> block(buffer);
    reverb.process(juce::dsp::ProcessContextReplacing<float>(block));
}

juce::AudioProcessorEditor* PhSynthOneAudioProcessor::createEditor() { return new PhSynthOneAudioProcessorEditor(*this); }
bool PhSynthOneAudioProcessor::hasEditor() const { return true; }
const juce::String PhSynthOneAudioProcessor::getName() const { return JucePlugin_Name; }
bool PhSynthOneAudioProcessor::acceptsMidi() const { return true; }
bool PhSynthOneAudioProcessor::producesMidi() const { return false; }
bool PhSynthOneAudioProcessor::isMidiEffect() const { return false; }
double PhSynthOneAudioProcessor::getTailLengthSeconds() const { return 4.0; }
int PhSynthOneAudioProcessor::getNumPrograms() { return 1; }
int PhSynthOneAudioProcessor::getCurrentProgram() { return 0; }
void PhSynthOneAudioProcessor::setCurrentProgram(int) {}
const juce::String PhSynthOneAudioProcessor::getProgramName(int) { return {}; }
void PhSynthOneAudioProcessor::changeProgramName(int, const juce::String&) {}

void PhSynthOneAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void PhSynthOneAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState && xmlState->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PhSynthOneAudioProcessor();
}
