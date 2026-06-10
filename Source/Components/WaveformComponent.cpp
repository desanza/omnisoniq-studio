#include "WaveformComponent.h"

WaveformComponent::WaveformComponent()
{
    setOpaque (false);
}

void WaveformComponent::setAudio (const juce::AudioBuffer<float>& buf, double /*sampleRate*/)
{
    rebuildPeaks (buf);
    repaint();
}

void WaveformComponent::clearAudio()
{
    mPeaks.clear();
    repaint();
}

void WaveformComponent::rebuildPeaks (const juce::AudioBuffer<float>& buf)
{
    const int kColumns = 300;
    mPeaks.resize (kColumns);

    int numSamples = buf.getNumSamples();
    if (numSamples == 0) { mPeaks.assign (kColumns, 0.0f); return; }

    int samplesPerCol = juce::jmax (1, numSamples / kColumns);
    int numCh = buf.getNumChannels();

    for (int col = 0; col < kColumns; ++col)
    {
        int start = col * samplesPerCol;
        int end   = juce::jmin (start + samplesPerCol, numSamples);
        float peak = 0.0f;
        for (int c = 0; c < numCh; ++c)
        {
            const float* r = buf.getReadPointer (c);
            for (int i = start; i < end; ++i)
                peak = juce::jmax (peak, std::abs (r[i]));
        }
        mPeaks[col] = juce::jlimit (0.0f, 1.0f, peak);
    }
}

void WaveformComponent::setPeaks (const std::vector<float>& peaks)
{
    mPeaks = peaks;
    repaint();
}

void WaveformComponent::setPlayPos (float norm) { mPlayPos = norm; repaint(); }
void WaveformComponent::setColour  (juce::Colour c) { mColour = c; repaint(); }
void WaveformComponent::setDim     (bool d) { mDim = d; repaint(); }
void WaveformComponent::setShowGrid(bool s) { mShowGrid = s; repaint(); }
void WaveformComponent::setBars    (int b)  { mBars = b; repaint(); }

void WaveformComponent::setLoopRegion (bool enabled, float start, float end)
{
    mLooping   = enabled;
    mLoopStart = start;
    mLoopEnd   = end;
    repaint();
}

void WaveformComponent::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    float w = bounds.getWidth();
    float h = bounds.getHeight();
    float mid = h / 2.0f;

    // Loop region background
    if (mLooping)
    {
        auto loopR = bounds.withLeft  (mLoopStart * w)
                           .withRight (mLoopEnd   * w);
        g.setColour (OmniColors::accent.withAlpha (0.08f));
        g.fillRect (loopR);
    }

    // Beat grid
    if (mShowGrid && mBars > 0)
    {
        g.setColour (juce::Colour (0xff262b38));
        for (int b = 1; b < mBars; ++b)
        {
            float x = w * b / mBars;
            g.drawVerticalLine ((int)x, 0.0f, h);
        }
    }

    // Waveform
    if (!mPeaks.empty())
    {
        int n = (int)mPeaks.size();
        int playCol = (int)(mPlayPos * n);

        juce::Path pathFull, pathPlayed;
        float step = w / n;

        for (int i = 0; i < n; ++i)
        {
            float amp  = mPeaks[i] * 0.92f;
            float x    = i * step;
            float topY = mid - amp * mid;
            float botY = mid + amp * mid;
            auto  bar  = juce::Rectangle<float> (x, topY, juce::jmax (1.0f, step - 0.5f), botY - topY);

            if (i < playCol)
                pathPlayed.addRectangle (bar);
            else
                pathFull.addRectangle (bar);
        }

        float alpha = mDim ? 0.3f : 1.0f;

        // Unplayed portion
        g.setColour (mColour.withAlpha (0.35f * alpha));
        g.fillPath (pathFull);

        // Played portion (brighter)
        g.setColour (mColour.withAlpha (0.85f * alpha));
        g.fillPath (pathPlayed);
    }
    else
    {
        // Empty placeholder
        g.setColour (OmniColors::textDim.withAlpha (0.5f));
        for (int i = 0; i < 60; ++i)
        {
            float x = i * w / 60.0f;
            float amp = 0.15f;
            g.fillRect (x, mid - amp * mid, w / 65.0f, amp * mid * 2.0f);
        }
    }

    // Playhead
    if (mPlayPos > 0.0f)
    {
        float px = mPlayPos * w;
        g.setColour (juce::Colours::white.withAlpha (0.9f));
        g.fillRect (px - 0.75f, 0.0f, 1.5f, h);
        // Head dot
        g.fillEllipse (px - 4.0f, -2.0f, 8.0f, 8.0f);
    }

    // Loop handles
    if (mLooping)
    {
        g.setColour (OmniColors::accent.withAlpha (0.7f));
        float ls = mLoopStart * w;
        float le = mLoopEnd   * w;
        g.fillRect (ls - 1.0f, 0.0f, 2.0f, h);
        g.fillRect (le - 1.0f, 0.0f, 2.0f, h);
    }
}

void WaveformComponent::seekFromEvent (const juce::MouseEvent& e)
{
    float norm = juce::jlimit (0.0f, 1.0f, (float)e.x / (float)getWidth());
    if (onSeek) onSeek (norm);
}

void WaveformComponent::mouseDown (const juce::MouseEvent& e) { seekFromEvent (e); }
void WaveformComponent::mouseDrag (const juce::MouseEvent& e) { seekFromEvent (e); }
