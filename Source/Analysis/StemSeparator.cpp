#include "StemSeparator.h"

static const juce::StringArray STEM_ORDER { "drums", "bass", "melody", "vocals", "other" };

StemSeparator::StemSeparator() : juce::Thread ("StemSeparator") {}
StemSeparator::~StemSeparator() { cancel(); }

void StemSeparator::separate (const juce::File& inputFile,
                               const juce::File& outputDir,
                               const juce::String& pythonExe,
                               ProgressFn onProgress,
                               CompleteFn onComplete)
{
    cancel();
    mInput      = inputFile;
    mOutputDir  = outputDir;
    mPythonExe  = pythonExe;
    mProgress   = std::move (onProgress);
    mComplete   = std::move (onComplete);
    mCancelled  = false;
    startThread();
}

void StemSeparator::cancel()
{
    mCancelled = true;
    stopThread (5000);
}

juce::File StemSeparator::getScriptPath()
{
    // Look for separate.py next to the plugin / executable
    auto appDir = juce::File::getSpecialLocation (juce::File::currentApplicationFile)
                                                    .getParentDirectory();
    auto script = appDir.getChildFile ("python").getChildFile ("separate.py");
    if (script.existsAsFile()) return script;
    // Fallback: next to exe
    return appDir.getChildFile ("separate.py");
}

void StemSeparator::run()
{
    auto script = getScriptPath();
    if (!script.existsAsFile())
    {
        juce::MessageManager::callAsync ([c = mComplete] { c (false, {}); });
        return;
    }

    mOutputDir.createDirectory();

    juce::ChildProcess proc;
    juce::StringArray args;
    args.add (mPythonExe);
    args.add (script.getFullPathName());
    args.add (mInput.getFullPathName());
    args.add (mOutputDir.getFullPathName());

    if (!proc.start (args, juce::ChildProcess::wantStdOut | juce::ChildProcess::wantStdErr))
    {
        juce::MessageManager::callAsync ([c = mComplete] { c (false, {}); });
        return;
    }

    // Read stdout line by line for progress updates (format: "PROGRESS 0.45")
    juce::String buffer;
    while (!mCancelled && proc.isRunning())
    {
        juce::String chunk = proc.readAllProcessOutput();
        if (chunk.isNotEmpty())
        {
            buffer += chunk;
            while (buffer.contains ("\n"))
            {
                auto line = buffer.upToFirstOccurrenceOf ("\n", false, false).trim();
                buffer    = buffer.fromFirstOccurrenceOf ("\n", false, false);

                if (line.startsWith ("PROGRESS"))
                {
                    float pct = line.fromFirstOccurrenceOf (" ", false, false).getFloatValue();
                    if (mProgress)
                    {
                        auto cb = mProgress;
                        juce::MessageManager::callAsync ([cb, pct] { cb (pct); });
                    }
                }
            }
        }
        juce::Thread::sleep (100);
    }

    if (mCancelled) { proc.kill(); return; }
    proc.waitForProcessToFinish (60000);

    // Collect output files
    // Demucs writes: <outputDir>/htdemucs/<stem_name>/<inputFile_noExt>.wav
    juce::Array<StemInfo> stems;
    juce::StringArray stemNames { "drums", "bass", "other", "vocals" }; // demucs 4-stem default

    // Try to find output directory created by demucs
    juce::File demucsOut;
    juce::Array<juce::File> subDirs;
    mOutputDir.findChildFiles (subDirs, juce::File::findDirectories, false);

    if (subDirs.isEmpty())
        demucsOut = mOutputDir;
    else
        demucsOut = subDirs[0]; // usually htdemucs

    juce::String baseName = mInput.getFileNameWithoutExtension();

    juce::StringArray displayNames { "Drums", "Bass", "Other", "Vocals" };
    for (int i = 0; i < stemNames.size(); ++i)
    {
        auto stemDir  = demucsOut.getChildFile (stemNames[i]);
        auto wavFile  = stemDir.getChildFile (baseName + ".wav");
        if (!wavFile.existsAsFile())
        {
            // Also try mp3
            wavFile = stemDir.getChildFile (baseName + ".mp3");
        }
        if (wavFile.existsAsFile())
        {
            StemInfo info;
            info.id   = stemNames[i];
            info.name = displayNames[i];
            info.file = wavFile;
            stems.add (info);
        }
    }

    bool success = !stems.isEmpty();
    if (mComplete)
    {
        auto cb = mComplete;
        juce::MessageManager::callAsync ([cb, success, stems] { cb (success, stems); });
    }
}
