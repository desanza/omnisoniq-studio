#include "OmniLookAndFeel.h"

OmniLookAndFeel::OmniLookAndFeel()
{
    setColour (juce::ResizableWindow::backgroundColourId, OmniColors::bgShell);
    setColour (juce::TextButton::buttonColourId,          OmniColors::bgCard);
    setColour (juce::TextButton::buttonOnColourId,        OmniColors::accent);
    setColour (juce::TextButton::textColourOffId,         OmniColors::textSecond);
    setColour (juce::TextButton::textColourOnId,          juce::Colours::white);
    setColour (juce::Label::textColourId,                 OmniColors::textPrimary);
    setColour (juce::Slider::backgroundColourId,          OmniColors::bgSurface);
    setColour (juce::Slider::thumbColourId,               OmniColors::accent);
    setColour (juce::Slider::trackColourId,               OmniColors::accent);
    setColour (juce::ScrollBar::thumbColourId,            OmniColors::textDim);
}

void OmniLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& button,
                                            const juce::Colour& /*backgroundColour*/,
                                            bool isHighlighted, bool isDown)
{
    auto bounds = button.getLocalBounds().toFloat().reduced (0.5f);
    bool on     = button.getToggleState();

    juce::Colour fill;
    if (on)
        fill = OmniColors::accent.withAlpha (0.18f);
    else if (isDown)
        fill = OmniColors::bgSurface.brighter (0.1f);
    else if (isHighlighted)
        fill = OmniColors::bgSurface;
    else
        fill = OmniColors::bgCard;

    g.setColour (fill);
    g.fillRoundedRectangle (bounds, 7.0f);

    juce::Colour strokeCol = on ? OmniColors::accent.withAlpha (0.45f) : OmniColors::border;
    g.setColour (strokeCol);
    g.drawRoundedRectangle (bounds, 7.0f, 1.0f);
}

void OmniLookAndFeel::drawButtonText (juce::Graphics& g, juce::TextButton& button,
                                      bool /*isHighlighted*/, bool /*isDown*/)
{
    juce::Colour col = button.getToggleState() ? OmniColors::accent : OmniColors::textSecond;
    g.setColour (col);
    g.setFont (getTextButtonFont (button, button.getHeight()));
    g.drawText (button.getButtonText(), button.getLocalBounds(), juce::Justification::centred, true);
}

void OmniLookAndFeel::drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                                        float sliderPos, float /*minSliderPos*/, float /*maxSliderPos*/,
                                        juce::Slider::SliderStyle style, juce::Slider& slider)
{
    if (style == juce::Slider::LinearHorizontal)
    {
        auto trackY   = y + height / 2;
        auto trackH   = 3;
        auto trackRect = juce::Rectangle<float> ((float)x, (float)(trackY - trackH/2),
                                                  (float)width, (float)trackH);
        g.setColour (OmniColors::borderLight);
        g.fillRoundedRectangle (trackRect, 1.5f);

        auto filled = juce::Rectangle<float> ((float)x, (float)(trackY - trackH/2),
                                               sliderPos - x, (float)trackH);
        juce::Colour accent = slider.findColour (juce::Slider::trackColourId);
        g.setColour (accent);
        g.fillRoundedRectangle (filled, 1.5f);

        // Thumb
        float thumbX = sliderPos;
        g.setColour (juce::Colours::white);
        g.fillEllipse (thumbX - 5.0f, (float)(trackY - 5), 10.0f, 10.0f);
    }
    else
    {
        LookAndFeel_V4::drawLinearSlider (g, x, y, width, height, sliderPos,
                                          0, 0, style, slider);
    }
}

void OmniLookAndFeel::drawScrollbar (juce::Graphics& g, juce::ScrollBar& /*scrollbar*/,
                                     int x, int y, int width, int height,
                                     bool isScrollbarVertical, int thumbStartPosition,
                                     int thumbSize, bool isMouseOver, bool /*isMouseDown*/)
{
    juce::Rectangle<int> thumb;
    if (isScrollbarVertical)
        thumb = { x + 2, thumbStartPosition, width - 4, thumbSize };
    else
        thumb = { thumbStartPosition, y + 2, thumbSize, height - 4 };

    g.setColour (isMouseOver ? OmniColors::textTertiary : OmniColors::textDim);
    g.fillRoundedRectangle (thumb.toFloat(), 3.0f);
}

juce::Font OmniLookAndFeel::getTextButtonFont (juce::TextButton&, int /*buttonHeight*/)
{
    return juce::Font (12.0f, juce::Font::bold);
}

void OmniLookAndFeel::drawLabel (juce::Graphics& g, juce::Label& label)
{
    g.setColour (label.findColour (juce::Label::textColourId));
    g.setFont (label.getFont());
    g.drawText (label.getText(), label.getLocalBounds().reduced (2, 0),
                label.getJustificationType(), true);
}
