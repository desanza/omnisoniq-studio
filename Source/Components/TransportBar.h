#pragma once
#include <JuceHeader.h>
#include "../OmniColors.h"
#include "../PluginProcessor.h"

class TransportBar : public juce::Component, private juce::Timer
{
public:
    explicit TransportBar (OmnisoniqProcessor& proc);
    ~TransportBar() override;

    void paint   (juce::Graphics& g) override;
    void resized () override;
    void updateFromState ();

    std::function<void(const juce::String&, const juce::String&)> onExport;

private:
    void timerCallback() override;

    OmnisoniqProcessor& mProc;

    juce::TextButton mToStartBtn, mPlayBtn, mStopBtn, mLoopBtn;
    juce::Label      mPosLabel, mBpmLabel;
    juce::Slider     mZoomSlider;
    juce::TextButton mExportBtn;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TransportBar)
};
