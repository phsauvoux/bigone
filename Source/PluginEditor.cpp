#include "PluginEditor.h"

class PhSynthOneAudioProcessorEditor::LookAndFeel : public juce::LookAndFeel_V4
{
public:
    LookAndFeel()
    {
        setColour(juce::Slider::thumbColourId, juce::Colour(0xff8bf7ff));
        setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xff51f0d9));
        setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff232834));
        setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff10131a));
        setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff343d52));
        setColour(juce::ComboBox::textColourId, juce::Colour(0xffe9fbff));
    }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                          float rotaryStartAngle, float rotaryEndAngle, juce::Slider& slider) override
    {
        auto bounds = juce::Rectangle<float>(x, y, width, height).reduced(9.0f);
        auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
        auto centre = bounds.getCentre();
        auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

        g.setColour(juce::Colour(0xff0a0d13));
        g.fillEllipse(bounds);

        juce::ColourGradient glow(juce::Colour(0x6651f0d9), centre.x, centre.y - radius,
                                  juce::Colour(0x338c5cff), centre.x, centre.y + radius, false);
        g.setGradientFill(glow);
        g.drawEllipse(bounds, 2.0f);

        juce::Path arc;
        arc.addCentredArc(centre.x, centre.y, radius - 5.0f, radius - 5.0f, 0.0f, rotaryStartAngle, angle, true);
        g.setColour(juce::Colour(0xff65ffe8));
        g.strokePath(arc, juce::PathStrokeType(3.4f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        juce::Path pointer;
        pointer.addRoundedRectangle(-1.7f, -radius + 10.0f, 3.4f, radius * 0.42f, 1.7f);
        pointer.applyTransform(juce::AffineTransform::rotation(angle).translated(centre.x, centre.y));
        g.fillPath(pointer);

        g.setColour(juce::Colour(0xffe8fbff));
        g.setFont(juce::FontOptions(11.0f, juce::Font::bold));
        g.drawFittedText(slider.getName(), x, y + height - 18, width, 16, juce::Justification::centred, 1);
    }

    void drawComboBox(juce::Graphics& g, int width, int height, bool, int, int, int, int, juce::ComboBox& box) override
    {
        auto bounds = juce::Rectangle<int>(0, 0, width, height).toFloat().reduced(1.0f);
        g.setColour(juce::Colour(0xff10131a));
        g.fillRoundedRectangle(bounds, 10.0f);
        g.setColour(juce::Colour(0xff51f0d9));
        g.drawRoundedRectangle(bounds, 10.0f, 1.2f);
        juce::Path arrow;
        arrow.addTriangle(width - 24.0f, height * 0.42f, width - 12.0f, height * 0.42f, width - 18.0f, height * 0.62f);
        g.fillPath(arrow);
    }
};

PhSynthOneAudioProcessorEditor::PhSynthOneAudioProcessorEditor(PhSynthOneAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p), lookAndFeel(std::make_unique<LookAndFeel>())
{
    setLookAndFeel(lookAndFeel.get());
    setSize(760, 430);

    configureLabel(titleLabel, "PhSynth One", 28.0f, juce::Justification::centredLeft);
    configureLabel(subtitleLabel, "HYBRID DIGITAL SYNTHESIZER", 12.0f, juce::Justification::centredLeft);

    soundType.addItem("PAD", 1);
    soundType.addItem("BASS", 2);
    soundType.addItem("LEAD", 3);
    soundType.addItem("PLUCK", 4);
    soundType.addItem("STRING", 5);
    soundType.setTextWhenNothingSelected("PAD");
    addAndMakeVisible(soundType);
    soundTypeAttachment = std::make_unique<ComboAttachment>(audioProcessor.getApvts(), "soundType", soundType);

    configureSlider(attack, "s");
    configureSlider(decay, "s");
    configureSlider(sustain);
    configureSlider(release, "s");
    configureSlider(cutoff, " Hz");
    configureSlider(resonance);
    configureSlider(reverb);
    configureSlider(delay);
    configureSlider(pitch, " st");
    configureSlider(volume);

    attack.setName("ATTACK");
    decay.setName("DECAY");
    sustain.setName("SUSTAIN");
    release.setName("RELEASE");
    cutoff.setName("CUTOFF");
    resonance.setName("RESO");
    reverb.setName("REVERB");
    delay.setName("DELAY");
    pitch.setName("PITCH");
    volume.setName("VOLUME");

    sliders = { &attack, &decay, &sustain, &release, &cutoff, &resonance, &reverb, &delay, &pitch, &volume };
    const char* ids[] = { "attack", "decay", "sustain", "release", "cutoff", "resonance", "reverb", "delay", "pitch", "volume" };
    for (size_t i = 0; i < sliders.size(); ++i)
        sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.getApvts(), ids[i], *sliders[i]));
}

PhSynthOneAudioProcessorEditor::~PhSynthOneAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void PhSynthOneAudioProcessorEditor::configureSlider(juce::Slider& slider, const juce::String& suffix)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 62, 18);
    slider.setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xffdffcff));
    slider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    slider.setTextValueSuffix(suffix);
    addAndMakeVisible(slider);
}

void PhSynthOneAudioProcessorEditor::configureLabel(juce::Label& label, const juce::String& text, float fontSize, juce::Justification justification)
{
    label.setText(text, juce::dontSendNotification);
    label.setFont(juce::FontOptions(fontSize, juce::Font::bold));
    label.setJustificationType(justification);
    label.setColour(juce::Label::textColourId, juce::Colour(0xffe9fbff));
    addAndMakeVisible(label);
}

void PhSynthOneAudioProcessorEditor::paint(juce::Graphics& g)
{
    auto area = getLocalBounds().toFloat();
    juce::ColourGradient bg(juce::Colour(0xff07090e), 0.0f, 0.0f, juce::Colour(0xff111827), area.getRight(), area.getBottom(), false);
    g.setGradientFill(bg);
    g.fillAll();

    g.setColour(juce::Colour(0x2221fff0));
    for (int x = -40; x < getWidth(); x += 38)
        g.drawLine(static_cast<float>(x), 0.0f, static_cast<float>(x + 190), static_cast<float>(getHeight()), 0.6f);

    auto panel = getLocalBounds().reduced(22).toFloat();
    g.setColour(juce::Colour(0xaa0c111a));
    g.fillRoundedRectangle(panel, 22.0f);
    g.setColour(juce::Colour(0xff273244));
    g.drawRoundedRectangle(panel, 22.0f, 1.0f);

    auto display = juce::Rectangle<float>(42, 74, 676, 58);
    juce::ColourGradient displayGradient(juce::Colour(0xff0b1018), display.getX(), display.getY(),
                                         juce::Colour(0xff101d2b), display.getRight(), display.getBottom(), false);
    g.setGradientFill(displayGradient);
    g.fillRoundedRectangle(display, 16.0f);
    g.setColour(juce::Colour(0x8851f0d9));
    g.drawRoundedRectangle(display, 16.0f, 1.5f);

    g.setColour(juce::Colour(0x3351f0d9));
    g.drawHorizontalLine(148, 42.0f, 718.0f);
    g.drawHorizontalLine(344, 42.0f, 718.0f);

    g.setColour(juce::Colour(0xff65ffe8));
    g.setFont(juce::FontOptions(11.0f, juce::Font::bold));
    g.drawText("OSC MODEL", 54, 92, 120, 18, juce::Justification::centredLeft);
    g.drawText("ENVELOPE", 46, 154, 160, 18, juce::Justification::centredLeft);
    g.drawText("FILTER / FX", 46, 278, 160, 18, juce::Justification::centredLeft);
}

void PhSynthOneAudioProcessorEditor::resized()
{
    titleLabel.setBounds(42, 28, 260, 32);
    subtitleLabel.setBounds(44, 55, 260, 18);
    soundType.setBounds(180, 86, 190, 34);

    const int knobW = 106;
    const int knobH = 106;
    const int startX = 46;
    const int gap = 34;
    int y1 = 172;
    int y2 = 296;

    attack.setBounds(startX + (knobW + gap) * 0, y1, knobW, knobH);
    decay.setBounds(startX + (knobW + gap) * 1, y1, knobW, knobH);
    sustain.setBounds(startX + (knobW + gap) * 2, y1, knobW, knobH);
    release.setBounds(startX + (knobW + gap) * 3, y1, knobW, knobH);
    pitch.setBounds(startX + (knobW + gap) * 4, y1, knobW, knobH);

    cutoff.setBounds(startX + (knobW + gap) * 0, y2, knobW, knobH);
    resonance.setBounds(startX + (knobW + gap) * 1, y2, knobW, knobH);
    reverb.setBounds(startX + (knobW + gap) * 2, y2, knobW, knobH);
    delay.setBounds(startX + (knobW + gap) * 3, y2, knobW, knobH);
    volume.setBounds(startX + (knobW + gap) * 4, y2, knobW, knobH);
}
