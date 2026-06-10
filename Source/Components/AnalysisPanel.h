#pragma once
#include <JuceHeader.h>
#include "../OmniColors.h"
#include "../PluginProcessor.h"

class AnalysisPanel : public juce::Component
{
public:
    explicit AnalysisPanel (OmnisoniqProcessor& proc);

    void paint   (juce::Graphics& g) override;
    void resized () override;
    void updateFromState ();

    std::function<void(const juce::String&, const juce::String&)> onExport;

private:
    void paintStatCard  (juce::Graphics& g, juce::Rectangle<int> r,
                         const juce::String& label, const juce::String& value,
                         const juce::String& sub, juce::Colour accent);
    void paintProgressBar (juce::Graphics& g, juce::Rectangle<int> r, float pct, juce::Colour col);
    void doExport       (const juce::String& scope, const juce::String& fmt);

    OmnisoniqProcessor& mProc;

    // Separation controls
    juce::TextButton mSeparateBtn { "One-Click Separate Stems" };
    juce::TextButton mFastBtn     { "Fast" };
    juce::TextButton mBalancedBtn { "Balanced" };
    juce::TextButton mStudioBtn   { "Studio HQ" };

    // Export controls
    juce::TextButton mExportFullBtn { "Export Full Mix" };
    juce::TextButton mExportStems   { "Export All Stems" };
    juce::TextButton mExportAcca    { "Export Acapella" };
    juce::TextButton mFmtWav        { "WAV 24-bit" };
    juce::TextButton mFmtMp3        { "MP3 320" };

    juce::String mCurrentModel { "balanced" };
    juce::String mCurrentFmt   { "wav" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnalysisPanel)
};
