#pragma once
#include <JuceHeader.h>
#include "../OmniColors.h"
#include "../PluginProcessor.h"

class TopBar : public juce::Component
{
public:
    explicit TopBar (OmnisoniqProcessor& proc);

    void paint   (juce::Graphics& g) override;
    void resized () override;
    void updateFromState ();

private:
    OmnisoniqProcessor& mProc;

    juce::TextButton mDawSyncBtn { "DAW Sync" };
    juce::Label      mStageLabel;

    void paintLogo (juce::Graphics& g, juce::Rectangle<int> area);
    void paintSteps (juce::Graphics& g, juce::Rectangle<int> area, int currentStage);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TopBar)
};
