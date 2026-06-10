#pragma once
#include <JuceHeader.h>

struct AnalysisResult
{
    float bpm     = 0.0f;
    juce::String key;
    float keyConf = 0.0f;   // 0..1
    bool  tempoStable = true;
    float tempoDrift  = 0.4f;
};

class AudioAnalyzer
{
public:
    // Runs synchronously; call from a background thread.
    static AnalysisResult analyze (const juce::AudioBuffer<float>& buffer, double sampleRate);

private:
    static float detectBPM (const juce::AudioBuffer<float>& buffer, double sampleRate);
    static std::pair<juce::String, float> detectKey (const juce::AudioBuffer<float>& buffer, double sampleRate);
};
