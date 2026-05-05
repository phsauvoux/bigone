#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class PhSynthOneAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit PhSynthOneAudioProcessorEditor(PhSynthOneAudioProcessor&);
    ~PhSynthOneAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    class LookAndFeel;

    PhSynthOneAudioProcessor& audioProcessor;
    std::unique_ptr<LookAndFeel> lookAndFeel;

    juce::ComboBox soundType;
    juce::Slider attack, decay, sustain, release, cutoff, resonance, reverb, delay, pitch, volume;
    juce::Label titleLabel, subtitleLabel;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    std::unique_ptr<ComboAttachment> soundTypeAttachment;
    std::vector<std::unique_ptr<SliderAttachment>> sliderAttachments;
    std::vector<juce::Slider*> sliders;

    void configureSlider(juce::Slider& slider, const juce::String& suffix = {});
    void configureLabel(juce::Label& label, const juce::String& text, float fontSize, juce::Justification justification);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PhSynthOneAudioProcessorEditor)
};
