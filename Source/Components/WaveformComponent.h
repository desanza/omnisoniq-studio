#pragma once
#include <JuceHeader.h>
#include "../OmniColors.h"

// Renders a waveform from a pre-computed peaks array.
// Peaks array holds amplitude 0..1 per column.
class WaveformComponent : public juce::Component
{
public:
    WaveformComponent();

    void setAudio    (const juce::AudioBuffer<float>& buf, double sampleRate);
    void clearAudio  ();
    void setPlayPos  (float norm);   // 0..1
    void setLoopRegion (bool enabled, float start, float end);
    void setColour   (juce::Colour c);
    void setDim      (bool d);       // for muted stems
    void setShowGrid (bool s);
    void setBars     (int bars);

    // Fast path: supply raw peaks directly (for stems that may already be computed)
    void setPeaks (const std::vector<float>& peaks);

    void paint (juce::Graphics& g) override;
    void mouseDown (const juce::MouseEvent& e) override;
    void mouseDrag (const juce::MouseEvent& e) override;

    std::function<void(float)> onSeek;  // called with normalised pos

private:
    std::vector<float> mPeaks;
    juce::Colour       mColour  { OmniColors::accent };
    float              mPlayPos { 0.0f };
    bool               mLooping { false };
    float              mLoopStart{ 0.25f }, mLoopEnd { 0.75f };
    bool               mDim     { false };
    bool               mShowGrid{ true };
    int                mBars    { 4 };

    void rebuildPeaks (const juce::AudioBuffer<float>& buf);
    void seekFromEvent (const juce::MouseEvent& e);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaveformComponent)
};
