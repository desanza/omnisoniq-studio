#include "TopBar.h"

TopBar::TopBar (OmnisoniqProcessor& proc) : mProc (proc)
{
    addAndMakeVisible (mDawSyncBtn);
    addAndMakeVisible (mStageLabel);

    mDawSyncBtn.setClickingTogglesState (true);
    mDawSyncBtn.setToggleState (proc.state.dawSync, juce::dontSendNotification);
    mDawSyncBtn.onClick = [this] {
        mProc.state.dawSync = mDawSyncBtn.getToggleState();
    };

    mStageLabel.setFont (juce::Font (11.0f, juce::Font::bold));
    mStageLabel.setColour (juce::Label::textColourId, OmniColors::textSecond);
    mStageLabel.setJustificationType (juce::Justification::centred);
}

void TopBar::resized()
{
    auto b = getLocalBounds().reduced (12, 0);
    paintLogo (juce::Graphics (juce::Image()), b.removeFromLeft (160)); // just layout

    mDawSyncBtn.setBounds (b.removeFromRight (90).withSizeKeepingCentre (90, 28));
    mStageLabel .setBounds (b);
}

void TopBar::paintLogo (juce::Graphics& g, juce::Rectangle<int> /*area*/) { (void)g; }

void TopBar::paint (juce::Graphics& g)
{
    g.fillAll (OmniColors::bgTopBar);
    g.setColour (OmniColors::border);
    g.drawHorizontalLine (getHeight() - 1, 0.0f, (float)getWidth());

    auto b = getLocalBounds();

    // ── Logo ──
    auto logoArea = b.removeFromLeft (160).reduced (12, 0);
    {
        // Icon box
        juce::Rectangle<float> iconBox { (float)logoArea.getX(), (float)(logoArea.getCentreY() - 11), 22.0f, 22.0f };
        g.setColour (OmniColors::accent);
        g.fillRoundedRectangle (iconBox, 5.0f);

        // Waveform bars inside icon
        g.setColour (juce::Colours::white);
        float bx = iconBox.getX() + 3;
        float by = iconBox.getCentreY();
        float heights[] = { 4, 8, 12, 8, 4 };
        for (int i = 0; i < 5; ++i)
        {
            float bh = heights[i];
            g.fillRect (bx + i * 3.5f, by - bh / 2.0f, 2.0f, bh);
        }

        // Name
        g.setFont (juce::Font (14.0f, juce::Font::bold));
        g.setColour (OmniColors::textPrimary);
        g.drawText ("Omnisoniq", (int)(iconBox.getRight() + 8), logoArea.getY(),
                    100, logoArea.getHeight(), juce::Justification::centredLeft);

        g.setFont (juce::Font (9.0f, juce::Font::bold));
        g.setColour (OmniColors::textTertiary);
        g.drawText ("STUDIO", (int)(iconBox.getRight() + 8 + 72), logoArea.getY(),
                    50, logoArea.getHeight(), juce::Justification::centredLeft);
    }

    // ── Stage steps (centre) ──
    int stage = 0;
    switch (mProc.state.clipStatus)
    {
        case ClipStatus::Empty:     stage = 0; break;
        case ClipStatus::Loading:
        case ClipStatus::Analyzing: stage = 1; break;
        case ClipStatus::Analyzed:  stage = 1; break;
        case ClipStatus::Separating:stage = 2; break;
        case ClipStatus::Separated: stage = 3; break;
    }

    static const char* stages[] = { "Browse", "Analyze", "Separate", "Export" };
    auto centre = getLocalBounds();
    int stepW = 90, stepsTotal = 4 * stepW + 3 * 14;
    int stepsX = centre.getCentreX() - stepsTotal / 2;
    int stepsY = centre.getCentreY() - 13;

    for (int i = 0; i < 4; ++i)
    {
        bool active = (i == stage);
        bool done   = (i < stage);

        int sx = stepsX + i * (stepW + 14);

        if (active)
        {
            g.setColour (OmniColors::accent.withAlpha (0.16f));
            g.fillRoundedRectangle ((float)sx, (float)stepsY, (float)stepW, 26.0f, 7.0f);
        }

        // Bullet
        juce::Rectangle<float> bullet { (float)(sx + 8), (float)(stepsY + 4), 18.0f, 18.0f };
        g.setColour (active ? OmniColors::accent : (done ? OmniColors::accent.withAlpha (0.35f) : OmniColors::textDim));
        g.fillEllipse (bullet);

        g.setFont (juce::Font (9.0f, juce::Font::bold));
        g.setColour (active || done ? juce::Colours::white : OmniColors::textTertiary);
        g.drawText (done ? "✓" : juce::String (i + 1), bullet.toNearestInt(), juce::Justification::centred);

        // Label
        g.setFont (juce::Font (11.0f, juce::Font::bold));
        g.setColour (active ? OmniColors::textPrimary : (done ? OmniColors::textSecond : OmniColors::textTertiary));
        g.drawText (stages[i], sx + 30, stepsY, stepW - 30, 26, juce::Justification::centredLeft);

        // Arrow
        if (i < 3)
        {
            int ax = sx + stepW + 3;
            g.setColour (OmniColors::textDim);
            g.fillRect (ax, stepsY + 12, 8, 1);
        }
    }
}

void TopBar::updateFromState()
{
    mDawSyncBtn.setToggleState (mProc.state.dawSync, juce::dontSendNotification);
    repaint();
}
