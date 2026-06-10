#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "OmniColors.h"

static juce::Colour stemColour (const juce::String& id)
{
    if (id == "drums")  return OmniColors::stemDrums;
    if (id == "bass")   return OmniColors::stemBass;
    if (id == "melody") return OmniColors::stemMelody;
    if (id == "vocals") return OmniColors::stemVocals;
    return OmniColors::stemOther;
}

OmnisoniqProcessor::OmnisoniqProcessor()
    : AudioProcessor (BusesProperties()
                      .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      juce::Thread ("OmniLoadThread")
{
    mFormatManager.registerBasicFormats();
}

OmnisoniqProcessor::~OmnisoniqProcessor()
{
    mCancelRequested = true;
    stopThread (3000);
    mStemSeparator.cancel();
}

bool OmnisoniqProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo() &&
        layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono())
        return false;
    return true;
}

void OmnisoniqProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    mSampleRate = sampleRate;
    mBlockSize  = samplesPerBlock;
}

void OmnisoniqProcessor::releaseResources() {}

void OmnisoniqProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

    if (!state.playing.load()) return;

    bool hasStemsLoaded = false;
    {
        juce::ScopedLock sl (mAudioLock);
        for (auto* s : state.stems)
            if (s->loaded) { hasStemsLoaded = true; break; }
    }

    int outCh    = buffer.getNumChannels();
    int numSamps = buffer.getNumSamples();

    long long readPos = mReadPos.load();

    // ── Play back stems (or master) ──
    {
        juce::ScopedLock sl (mAudioLock);

        auto playFrom = [&] (const juce::AudioBuffer<float>& src, float gain)
        {
            int srcCh  = src.getNumChannels();
            int srcLen = src.getNumSamples();
            if (srcLen == 0) return;

            for (int i = 0; i < numSamps; ++i)
            {
                long long p = readPos + i;
                if (p >= srcLen) break;
                for (int c = 0; c < outCh; ++c)
                {
                    int sc = juce::jmin (c, srcCh - 1);
                    buffer.addSample (c, i, src.getSample (sc, (int)p) * gain);
                }
            }
        };

        bool anySolo = false;
        for (auto* s : state.stems)
            if (s->soloed) { anySolo = true; break; }

        if (hasStemsLoaded)
        {
            for (auto* s : state.stems)
            {
                if (!s->loaded) continue;
                bool audible = anySolo ? s->soloed : !s->muted;
                if (audible) playFrom (s->audio, s->volume);
            }
        }
        else if (state.masterLoaded)
        {
            playFrom (state.masterAudio, 1.0f);
        }
    }

    // ── Advance playhead ──
    long long totalSamples = state.masterLoaded ? (long long)state.masterAudio.getNumSamples() : 0;
    if (hasStemsLoaded && !state.stems.isEmpty() && state.stems[0]->loaded)
        totalSamples = (long long)state.stems[0]->audio.getNumSamples();

    if (totalSamples > 0)
    {
        readPos += numSamps;

        if (state.looping.load())
        {
            long long loopStartSamp = (long long)(state.loopStart * totalSamples);
            long long loopEndSamp   = (long long)(state.loopEnd   * totalSamples);
            if (readPos >= loopEndSamp) readPos = loopStartSamp;
        }
        else if (readPos >= totalSamples)
        {
            readPos = totalSamples - 1;
            state.playing.store (false);
        }

        mReadPos.store (readPos);
        state.playPos.store ((float)readPos / (float)totalSamples);
    }

    // ── Host sync ──
    if (state.dawSync)
    {
        if (auto* ph = getPlayHead())
        {
            if (auto pos = ph->getPosition())
            {
                if (pos->getIsPlaying() && !state.playing.load())
                    state.playing.store (true);
                else if (!pos->getIsPlaying() && state.playing.load())
                    state.playing.store (false);
            }
        }
    }
}

// ─── Load file ────────────────────────────────────────────────────────────
void OmnisoniqProcessor::loadFile (const juce::File& f)
{
    stopThread (2000);
    mPendingFile     = f;
    mCancelRequested = false;
    mLoadRequested   = true;

    state.clipStatus = ClipStatus::Loading;
    state.clipTitle  = f.getFileNameWithoutExtension();
    state.currentFile= f;
    state.masterLoaded = false;
    state.stems.clear();
    state.playPos.store (0.0f);
    state.playing.store (false);
    mReadPos.store (0);
    notifyStateChanged();

    startThread();
}

void OmnisoniqProcessor::run()
{
    if (!mLoadRequested) return;
    mLoadRequested = false;

    // 1. Load audio into masterAudio
    std::unique_ptr<juce::AudioFormatReader> reader (
        mFormatManager.createReaderFor (mPendingFile));

    if (!reader)
    {
        juce::MessageManager::callAsync ([this] {
            state.clipStatus = ClipStatus::Empty;
            notifyStateChanged();
        });
        return;
    }

    double sr         = reader->sampleRate;
    long long numSamp = reader->lengthInSamples;
    int       numCh   = (int)juce::jmin ((int)reader->numChannels, 2);

    juce::AudioBuffer<float> loaded (numCh, (int)numSamp);
    reader->read (&loaded, 0, (int)numSamp, 0, true, true);

    if (threadShouldExit()) return;

    {
        juce::ScopedLock sl (mAudioLock);
        state.masterAudio = std::move (loaded);
        state.masterSampleRate = sr;
        state.masterLoaded = true;
        state.masterDuration = numSamp / (float)sr;
    }

    juce::MessageManager::callAsync ([this] {
        state.clipStatus = ClipStatus::Analyzing;
        state.analyzeProgress = 0.0f;
        notifyStateChanged();
    });

    // 2. Analyze BPM + key
    AnalysisResult result;
    {
        juce::ScopedLock sl (mAudioLock);
        result = AudioAnalyzer::analyze (state.masterAudio, sr);
    }

    if (threadShouldExit()) return;

    juce::MessageManager::callAsync ([this, result] {
        state.analysis   = result;
        state.clipStatus = ClipStatus::Analyzed;
        notifyStateChanged();
    });
}

// ─── Stem separation ─────────────────────────────────────────────────────
void OmnisoniqProcessor::separate (const juce::String& /*model*/)
{
    if (state.clipStatus != ClipStatus::Analyzed && state.clipStatus != ClipStatus::Separated) return;

    state.clipStatus = ClipStatus::Separating;
    state.separateProgress = 0.0f;
    state.stems.clear();
    notifyStateChanged();

    auto outDir = juce::File::getSpecialLocation (juce::File::tempDirectory)
                    .getChildFile ("OmnisoniqStems")
                    .getChildFile (state.currentFile.getFileNameWithoutExtension());

    mStemSeparator.separate (
        state.currentFile,
        outDir,
        pythonExe,
        [this] (float p) {
            state.separateProgress = p;
            notifyStateChanged();
        },
        [this] (bool ok, juce::Array<StemInfo> infos) {
            if (!ok)
            {
                state.clipStatus = ClipStatus::Analyzed;
                notifyStateChanged();
                return;
            }

            // Load each stem WAV
            for (auto& info : infos)
            {
                auto* s   = new StemState();
                s->id     = info.id;
                s->name   = info.name;
                s->colour = stemColour (info.id);
                s->file   = info.file;

                std::unique_ptr<juce::AudioFormatReader> r (
                    mFormatManager.createReaderFor (info.file));
                if (r)
                {
                    juce::AudioBuffer<float> buf ((int)juce::jmin ((int)r->numChannels, 2),
                                                   (int)r->lengthInSamples);
                    r->read (&buf, 0, (int)r->lengthInSamples, 0, true, true);
                    {
                        juce::ScopedLock sl (mAudioLock);
                        s->audio  = std::move (buf);
                        s->loaded = true;
                    }
                }
                state.stems.add (s);
            }

            mReadPos.store (0);
            state.playPos.store (0.0f);
            state.clipStatus = ClipStatus::Separated;
            notifyStateChanged();
        });
}

void OmnisoniqProcessor::cancelLoad() { mCancelRequested = true; stopThread (2000); }

// ─── Transport ────────────────────────────────────────────────────────────
void OmnisoniqProcessor::play()  { state.playing.store (true);  }
void OmnisoniqProcessor::pause() { state.playing.store (false); }
void OmnisoniqProcessor::stop()
{
    state.playing.store (false);
    long long totalSamples = state.masterLoaded ? (long long)state.masterAudio.getNumSamples() : 0;
    float startPos = state.looping.load() ? state.loopStart : 0.0f;
    mReadPos.store ((long long)(startPos * totalSamples));
    state.playPos.store (startPos);
}

void OmnisoniqProcessor::seekTo (float norm)
{
    long long totalSamples = state.masterLoaded ? (long long)state.masterAudio.getNumSamples() : 1;
    if (!state.stems.isEmpty() && state.stems[0]->loaded)
        totalSamples = (long long)state.stems[0]->audio.getNumSamples();
    long long p = (long long)(juce::jlimit (0.0f, 1.0f, norm) * totalSamples);
    mReadPos.store (p);
    state.playPos.store (norm);
}

void OmnisoniqProcessor::setLoop (bool on) { state.looping.store (on); }

void OmnisoniqProcessor::setStemVolume (int i, float v)
{
    if (juce::isPositiveAndBelow (i, state.stems.size()))
        state.stems[i]->volume = v;
}
void OmnisoniqProcessor::setStemMute (int i, bool m)
{
    if (juce::isPositiveAndBelow (i, state.stems.size()))
        state.stems[i]->muted = m;
}
void OmnisoniqProcessor::setStemSolo (int i, bool s)
{
    if (juce::isPositiveAndBelow (i, state.stems.size()))
        state.stems[i]->soloed = s;
}

// ─── Export ───────────────────────────────────────────────────────────────
void OmnisoniqProcessor::exportStems (const juce::File& outputDir, const juce::String& format)
{
    outputDir.createDirectory();
    juce::WavAudioFormat wavFmt;

    for (auto* s : state.stems)
    {
        if (!s->loaded) continue;
        auto outFile = outputDir.getChildFile (s->name + "." + format.toLowerCase());
        auto* fos = outFile.createOutputStream().release();
        if (!fos) continue;

        std::unique_ptr<juce::AudioFormatWriter> writer (
            wavFmt.createWriterFor (fos,
                                    state.masterSampleRate,
                                    (unsigned int)s->audio.getNumChannels(),
                                    24, {}, 0));
        if (writer)
            writer->writeFromAudioSampleBuffer (s->audio, 0, s->audio.getNumSamples());
    }
}

void OmnisoniqProcessor::exportMix (const juce::File& outFile, const juce::String& /*format*/)
{
    juce::ScopedLock sl (mAudioLock);
    if (!state.masterLoaded) return;
    juce::WavAudioFormat wavFmt;
    auto* fosRaw = outFile.createOutputStream().release();
    if (!fosRaw) return;
    std::unique_ptr<juce::AudioFormatWriter> writer (
        wavFmt.createWriterFor (fosRaw,
                                state.masterSampleRate,
                                (unsigned int)state.masterAudio.getNumChannels(),
                                24, {}, 0));
    if (writer)
        writer->writeFromAudioSampleBuffer (state.masterAudio, 0, state.masterAudio.getNumSamples());
}

// ─── State persistence ───────────────────────────────────────────────────
void OmnisoniqProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::XmlElement xml ("OmnisoniqState");
    xml.setAttribute ("pythonExe", pythonExe);
    xml.setAttribute ("dawSync",   state.dawSync);
    copyXmlToBinary (xml, destData);
}

void OmnisoniqProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
    {
        pythonExe  = xml->getStringAttribute ("pythonExe", "python");
        state.dawSync = xml->getBoolAttribute ("dawSync", true);
    }
}

void OmnisoniqProcessor::notifyStateChanged()
{
    stateChangeBroadcaster.sendChangeMessage();
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new OmnisoniqProcessor();
}
