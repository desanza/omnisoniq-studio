#include "AudioAnalyzer.h"
#include <cmath>
#include <numeric>
#include <algorithm>

// ─────────────────────────────────────────────────────────────────────────────
//  BPM Detection via onset-strength autocorrelation
// ─────────────────────────────────────────────────────────────────────────────
static std::vector<float> computeOnsetStrength (const float* mono, int numSamples,
                                                 int hopSize, int fftSize)
{
    juce::dsp::FFT fft (static_cast<int> (std::log2 (fftSize)));
    std::vector<float> prevMag (fftSize / 2 + 1, 0.0f);
    std::vector<float> onsets;

    std::vector<float> window (fftSize);
    for (int i = 0; i < fftSize; ++i)
        window[i] = 0.5f - 0.5f * std::cos (2.0f * juce::MathConstants<float>::pi * i / fftSize);

    int numFrames = (numSamples - fftSize) / hopSize;
    onsets.reserve (numFrames);

    std::vector<juce::dsp::Complex<float>> fftBuf (fftSize);

    for (int frame = 0; frame < numFrames; ++frame)
    {
        int start = frame * hopSize;
        for (int i = 0; i < fftSize; ++i)
            fftBuf[i] = { (start + i < numSamples ? mono[start + i] : 0.0f) * window[i], 0.0f };

        fft.perform (fftBuf.data(), fftBuf.data(), false);

        float onset = 0.0f;
        for (int i = 0; i < fftSize / 2 + 1; ++i)
        {
            float mag = std::abs (fftBuf[i]);
            float diff = mag - prevMag[i];
            if (diff > 0.0f) onset += diff;
            prevMag[i] = mag;
        }
        onsets.push_back (onset);
    }
    return onsets;
}

float AudioAnalyzer::detectBPM (const juce::AudioBuffer<float>& buffer, double sampleRate)
{
    // Mix to mono
    int numSamples = buffer.getNumSamples();
    std::vector<float> mono (numSamples, 0.0f);
    int ch = buffer.getNumChannels();
    for (int c = 0; c < ch; ++c)
    {
        const float* r = buffer.getReadPointer (c);
        for (int i = 0; i < numSamples; ++i)
            mono[i] += r[i];
    }
    if (ch > 1)
        for (auto& s : mono) s /= ch;

    const int hopSize = 512;
    const int fftSize = 2048;
    auto onsets = computeOnsetStrength (mono.data(), numSamples, hopSize, fftSize);

    if (onsets.size() < 16) return 120.0f;

    // Autocorrelate onset vector
    int n = (int)onsets.size();
    float hopSecs = hopSize / (float)sampleRate;

    // BPM range: 60–200  → lag range
    int lagMin = (int)(60.0f / (200.0f * hopSecs));
    int lagMax = (int)(60.0f / (60.0f  * hopSecs));
    lagMax = std::min (lagMax, n - 1);
    lagMin = std::max (lagMin, 1);

    float bestVal = -1.0f;
    int   bestLag = lagMin;
    for (int lag = lagMin; lag <= lagMax; ++lag)
    {
        float acc = 0.0f;
        for (int i = 0; i + lag < n; ++i)
            acc += onsets[i] * onsets[i + lag];
        if (acc > bestVal) { bestVal = acc; bestLag = lag; }
    }

    float bpm = 60.0f / (bestLag * hopSecs);
    // Round to nearest 0.5
    bpm = std::round (bpm * 2.0f) / 2.0f;
    return juce::jlimit (50.0f, 220.0f, bpm);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Key Detection — Krumhansl-Schmuckler profiles
// ─────────────────────────────────────────────────────────────────────────────
static const float majorProfile[12] = { 6.35f,2.23f,3.48f,2.33f,4.38f,4.09f,2.52f,5.19f,2.39f,3.66f,2.29f,2.88f };
static const float minorProfile[12] = { 6.33f,2.68f,3.52f,5.38f,2.60f,3.53f,2.54f,4.75f,3.98f,2.69f,3.34f,3.17f };

static const char* noteNames[12] = { "C","C#","D","D#","E","F","F#","G","G#","A","Bb","B" };

static std::vector<float> computeChroma (const float* mono, int n, double sr)
{
    std::vector<float> chroma (12, 0.0f);
    // Simple CQT approximation via short DFT at each note
    for (int pitch = 0; pitch < 12 * 6; ++pitch)
    {
        float freq = 261.63f * std::pow (2.0f, pitch / 12.0f);
        double omega = 2.0 * juce::MathConstants<double>::pi * freq / sr;
        double re = 0.0, im = 0.0;
        int step = std::max (1, n / 4096);
        for (int i = 0; i < n; i += step)
        {
            re += mono[i] * std::cos (omega * i);
            im += mono[i] * std::sin (omega * i);
        }
        chroma[pitch % 12] += (float)std::sqrt (re*re + im*im);
    }
    // Normalise
    float s = *std::max_element (chroma.begin(), chroma.end());
    if (s > 0) for (auto& c : chroma) c /= s;
    return chroma;
}

static float pearson (const float* a, const float* b, int n)
{
    float ma = 0, mb = 0;
    for (int i = 0; i < n; ++i) { ma += a[i]; mb += b[i]; }
    ma /= n; mb /= n;
    float num = 0, da = 0, db = 0;
    for (int i = 0; i < n; ++i)
    {
        float aa = a[i] - ma, bb = b[i] - mb;
        num += aa * bb; da += aa*aa; db += bb*bb;
    }
    float denom = std::sqrt (da * db);
    return denom < 1e-9f ? 0.0f : num / denom;
}

std::pair<juce::String, float> AudioAnalyzer::detectKey (const juce::AudioBuffer<float>& buffer, double sr)
{
    int n = buffer.getNumSamples();
    std::vector<float> mono (n, 0.0f);
    for (int c = 0; c < buffer.getNumChannels(); ++c)
    {
        const float* r = buffer.getReadPointer (c);
        for (int i = 0; i < n; ++i) mono[i] += r[i];
    }
    if (buffer.getNumChannels() > 1)
        for (auto& s : mono) s /= buffer.getNumChannels();

    auto chroma = computeChroma (mono.data(), n, sr);

    float bestScore = -999.0f;
    int   bestKey   = 0;
    bool  bestMajor = true;

    for (int root = 0; root < 12; ++root)
    {
        float rotated[12];
        for (int i = 0; i < 12; ++i) rotated[i] = chroma[(i + root) % 12];

        float sm = pearson (rotated, majorProfile, 12);
        float sn = pearson (rotated, minorProfile, 12);

        if (sm > bestScore) { bestScore = sm; bestKey = root; bestMajor = true;  }
        if (sn > bestScore) { bestScore = sn; bestKey = root; bestMajor = false; }
    }

    float conf = juce::jlimit (0.0f, 1.0f, (bestScore + 1.0f) / 2.0f);
    juce::String name = juce::String (noteNames[bestKey]) + " " + (bestMajor ? "maj" : "min");
    return { name, conf };
}

// ─────────────────────────────────────────────────────────────────────────────
AnalysisResult AudioAnalyzer::analyze (const juce::AudioBuffer<float>& buffer, double sampleRate)
{
    AnalysisResult r;
    r.bpm = detectBPM (buffer, sampleRate);
    auto keyResult = detectKey (buffer, sampleRate);
    auto key  = keyResult.first;
    auto conf = keyResult.second;
    r.key     = key;
    r.keyConf = conf;
    r.tempoStable = true;
    r.tempoDrift  = 0.4f;
    return r;
}
