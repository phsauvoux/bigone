#pragma once

#include <JuceHeader.h>

class PhSynthOneAudioProcessor : public juce::AudioProcessor
{
public:
    PhSynthOneAudioProcessor();
    ~PhSynthOneAudioProcessor() override;

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

    juce::AudioProcessorValueTreeState& getApvts() { return apvts; }
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

private:
    struct Voice
    {
        bool active = false;
        int note = -1;
        double phase1 = 0.0;
        double phase2 = 0.0;
        double velocity = 0.0;
        double level = 0.0;
        double releaseStart = 0.0;
        enum Stage { off, attack, decay, sustain, release } stage = off;
        juce::dsp::StateVariableTPTFilter<float> filter;

        void start(int midiNote, float vel, double sr);
        void stop();
        float render(double sr, float soundType, float attack, float decay, float sustain, float release,
                     float cutoff, float resonance, float pitch);
    };

    std::array<Voice, 16> voices;
    juce::AudioProcessorValueTreeState apvts;
    juce::dsp::Reverb reverb;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delay { 96000 };
    double currentSampleRate = 44100.0;

    std::atomic<float>* soundTypeParam = nullptr;
    std::atomic<float>* attackParam = nullptr;
    std::atomic<float>* decayParam = nullptr;
    std::atomic<float>* sustainParam = nullptr;
    std::atomic<float>* releaseParam = nullptr;
    std::atomic<float>* cutoffParam = nullptr;
    std::atomic<float>* resonanceParam = nullptr;
    std::atomic<float>* reverbParam = nullptr;
    std::atomic<float>* delayParam = nullptr;
    std::atomic<float>* pitchParam = nullptr;
    std::atomic<float>* volumeParam = nullptr;

    Voice& findVoice();
    void updateParameterPointers();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PhSynthOneAudioProcessor)
};
