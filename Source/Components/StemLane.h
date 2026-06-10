#pragma once
#include <JuceHeader.h>
#include "../OmniColors.h"
#include "WaveformComponent.h"
#include "../PluginProcessor.h"

class StemLane : public juce::Component, private juce::Timer
{
public:
    StemLane (OmnisoniqProcessor& proc, int stemIndex);
    ~StemLane() override;

    void resized() override;
    void paint   (juce::Graphics& g) override;
    void update  ();   // call when state changes

private:
    void timerCallback() override;

    OmnisoniqProcessor& mProc;
    int                 mIdx;

    juce::TextButton    mSoloBtn  { "S" };
    juce::TextButton    mMuteBtn  { "M" };
    juce::Slider        mVolSlider;
    WaveformComponent   mWaveform;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StemLane)
};
