#pragma once
#include <JuceHeader.h>
#include "OmniColors.h"

class OmniLookAndFeel : public juce::LookAndFeel_V4
{
public:
    OmniLookAndFeel();

    void drawButtonBackground (juce::Graphics& g, juce::Button& button,
                               const juce::Colour& backgroundColour,
                               bool isHighlighted, bool isDown) override;

    void drawButtonText (juce::Graphics& g, juce::TextButton& button,
                         bool isHighlighted, bool isDown) override;

    void drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPos, float minSliderPos, float maxSliderPos,
                           juce::Slider::SliderStyle style, juce::Slider& slider) override;

    void drawScrollbar (juce::Graphics& g, juce::ScrollBar& scrollbar,
                        int x, int y, int width, int height,
                        bool isScrollbarVertical, int thumbStartPosition,
                        int thumbSize, bool isMouseOver, bool isMouseDown) override;

    juce::Font getTextButtonFont (juce::TextButton&, int buttonHeight) override;

    void drawLabel (juce::Graphics& g, juce::Label& label) override;
};
