#pragma once
#include <JuceHeader.h>
#include <functional>

struct StemInfo
{
    juce::String id;
    juce::String name;
    juce::File   file;   // written by Demucs
};

class StemSeparator : private juce::Thread
{
public:
    using ProgressFn  = std::function<void(float)>;      // 0..1
    using CompleteFn  = std::function<void(bool, juce::Array<StemInfo>)>;

    StemSeparator();
    ~StemSeparator() override;

    // Start separation in background. pythonExe = "python" or full path.
    void separate (const juce::File& inputFile,
                   const juce::File& outputDir,
                   const juce::String& pythonExe,
                   ProgressFn onProgress,
                   CompleteFn onComplete);

    void cancel();

    // Path to the bundled python script
    static juce::File getScriptPath();

private:
    void run() override;

    juce::File        mInput, mOutputDir;
    juce::String      mPythonExe;
    ProgressFn        mProgress;
    CompleteFn        mComplete;
    bool              mCancelled { false };
};
