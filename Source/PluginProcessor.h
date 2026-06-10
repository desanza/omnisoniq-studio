#pragma once
#include <JuceHeader.h>
#include "Analysis/AudioAnalyzer.h"
#include "Analysis/StemSeparator.h"

// ─── Stem state (GUI thread) ───────────────────────────────────────────────
struct StemState
{
    juce::String id, name;
    juce::Colour colour;
    float        volume  { 0.8f };
    bool         muted   { false };
    bool         soloed  { false };
    juce::File   file;
    // Loaded audio
    juce::AudioBuffer<float> audio;
    bool loaded { false };
};

// ─── Clip status ──────────────────────────────────────────────────────────
enum class ClipStatus { Empty, Loading, Analyzing, Analyzed, Separating, Separated };

// ─── Shared engine state accessible by editor ──────────────────────────────
struct OmniState
{
    ClipStatus   clipStatus  { ClipStatus::Empty };
    juce::File   currentFile;
    juce::String clipTitle;
    AnalysisResult analysis;
    float        loadProgress    { 0.0f };
    float        analyzeProgress { 0.0f };
    float        separateProgress{ 0.0f };
    juce::OwnedArray<StemState> stems;

    // Transport
    std::atomic<float> playPos { 0.0f };  // 0..1
    std::atomic<bool>  playing { false };
    std::atomic<bool>  looping { false };
    float loopStart { 0.25f };
    float loopEnd   { 0.75f };
    float masterDuration { 0.0f }; // seconds
    float zoom { 1.0f };

    // Master audio (full mix)
    juce::AudioBuffer<float> masterAudio;
    bool masterLoaded { false };
    double masterSampleRate { 44100.0 };

    bool dawSync { true };
};

// ─── Plugin Processor ────────────────────────────────────────────────────
class OmnisoniqProcessor : public juce::AudioProcessor,
                           private juce::Thread
{
public:
    OmnisoniqProcessor();
    ~OmnisoniqProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Omnisoniq Studio"; }
    bool   acceptsMidi()  const override { return false; }
    bool   producesMidi() const override { return false; }
    bool   isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int  getNumPrograms()    override { return 1; }
    int  getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return "Default"; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    // ── Public API called from editor ──
    void loadFile   (const juce::File& f);
    void separate   (const juce::String& model);
    void cancelLoad ();
    void exportStems (const juce::File& outputDir, const juce::String& format);
    void exportMix   (const juce::File& outputFile, const juce::String& format);

    // Transport
    void play();
    void pause();
    void stop();
    void seekTo (float normalised); // 0..1
    void setLoop (bool on);
    void setStemVolume (int stemIdx, float vol);
    void setStemMute   (int stemIdx, bool muted);
    void setStemSolo   (int stemIdx, bool solo);

    // Thread-safe state snapshot for GUI
    OmniState state;

    juce::ChangeBroadcaster stateChangeBroadcaster;
    void notifyStateChanged();

    juce::String pythonExe { "python" };

private:
    void run() override; // background load/analyze thread

    double mSampleRate  { 44100.0 };
    int    mBlockSize   { 512 };

    // Audio playback read position (sample index) — accessed only on audio thread
    std::atomic<long long> mReadPos { 0 };

    juce::AudioFormatManager mFormatManager;
    StemSeparator            mStemSeparator;

    juce::File               mPendingFile;
    std::atomic<bool>        mLoadRequested { false };
    std::atomic<bool>        mCancelRequested { false };

    juce::CriticalSection    mAudioLock;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OmnisoniqProcessor)
};
