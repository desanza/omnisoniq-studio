#include "WorkspacePanel.h"

WorkspacePanel::WorkspacePanel (OmnisoniqProcessor& proc) : mProc (proc)
{
    addAndMakeVisible (mMasterWave);
    addAndMakeVisible (mScrollView);

    mScrollView.setScrollBarsShown (true, false);
    mScrollView.setViewedComponent (&mScrollContent, false);

    mMasterWave.setShowGrid (true);
    mMasterWave.onSeek = [this] (float n) { mProc.seekTo (n); };

    startTimerHz (30);
}

WorkspacePanel::~WorkspacePanel() { stopTimer(); mScrollView.setViewedComponent (nullptr, false); }

void WorkspacePanel::resized()
{
    auto b = getLocalBounds().reduced (16);

    // Master track: 90px tall
    if (mProc.state.clipStatus != ClipStatus::Empty)
    {
        mMasterWave.setBounds (b.removeFromTop (90));
        b.removeFromTop (8);
    }

    mScrollView.setBounds (b);

    // Layout stem lanes
    int stemH = 78;
    int totalH = mStemLanes.size() * (stemH + 8);
    mScrollContent.setBounds (0, 0, b.getWidth(), juce::jmax (totalH, b.getHeight()));
    int y = 0;
    for (auto* lane : mStemLanes)
    {
        lane->setBounds (0, y, mScrollContent.getWidth(), stemH);
        y += stemH + 8;
    }
}

void WorkspacePanel::paint (juce::Graphics& g)
{
    g.fillAll (OmniColors::bgShell);

    auto& st = mProc.state;

    if (st.clipStatus == ClipStatus::Empty)
    {
        paintEmptyState (g);
        return;
    }

    if (st.clipStatus == ClipStatus::Loading)
    {
        paintProgressState (g, "Loading " + st.clipTitle + "…", st.loadProgress);
        return;
    }

    if (st.clipStatus == ClipStatus::Analyzing)
    {
        paintProgressState (g, "Analyzing — detecting tempo & key…", st.analyzeProgress);
        return;
    }

    if (st.clipStatus == ClipStatus::Separating)
    {
        paintProgressState (g, "Separating stems — " + juce::String ((int)(st.separateProgress*100)) + "%",
                            st.separateProgress);
        return;
    }

    // Draw master label
    auto b = getLocalBounds().reduced (16);
    auto headerRow = b.removeFromTop (22).withWidth (200);
    g.setFont (juce::Font (11.5f, juce::Font::bold));
    g.setColour (OmniColors::textSecond);
    g.drawText (st.clipTitle, headerRow, juce::Justification::centredLeft);
}

void WorkspacePanel::paintEmptyState (juce::Graphics& g)
{
    auto b = getLocalBounds().reduced (20);

    juce::Colour borderCol = mDragOver ? OmniColors::accent : OmniColors::borderDim;
    juce::Colour bgCol     = mDragOver ? OmniColors::accent.withAlpha (0.07f) : juce::Colours::transparentBlack;

    g.setColour (bgCol);
    g.fillRoundedRectangle (b.toFloat(), 14.0f);
    g.setColour (borderCol);

    // Dashed border
    juce::Path border;
    border.addRoundedRectangle (b.toFloat(), 14.0f);
    juce::PathStrokeType stroke (1.5f);
    stroke.createDashedStroke (border, border, { 8, 6 }, 2);
    g.fillPath (border);

    // Icon
    auto icon = b.withSizeKeepingCentre (64, 64).withCentreY (b.getCentreY() - 30);
    g.setColour (mDragOver ? OmniColors::accent : OmniColors::textDim);
    g.fillRoundedRectangle (icon.toFloat(), 14.0f);

    // Waveform symbol inside icon
    g.setColour (juce::Colours::white.withAlpha (0.7f));
    float ix = icon.getCentreX();
    float iy = icon.getCentreY();
    float bars[] = { 6, 12, 20, 14, 8 };
    for (int i = 0; i < 5; ++i)
    {
        float x = ix - 12 + i * 6.0f;
        float h = bars[i];
        g.fillRect (x, iy - h / 2, 4.0f, h);
    }

    g.setFont (juce::Font (15.0f, juce::Font::bold));
    g.setColour (OmniColors::textSecond);
    g.drawText (mDragOver ? "Drop to import" : "Your workspace is empty",
                b.withTop (icon.getBottom() + 16).withHeight (22), juce::Justification::centred);

    g.setFont (juce::Font (12.5f));
    g.setColour (OmniColors::textTertiary);
    g.drawText ("Drag an audio file here or use the browser panel",
                b.withTop (icon.getBottom() + 42).withHeight (20), juce::Justification::centred, true);
}

void WorkspacePanel::paintProgressState (juce::Graphics& g, const juce::String& msg, float pct)
{
    auto b = getLocalBounds().reduced (20);

    // Waveform (dimmed)
    if (mProc.state.masterLoaded)
    {
        auto waveR = b.withTop (b.getY() + 60).withHeight (90).reduced (0);
        mMasterWave.setBounds (waveR);
        mMasterWave.setVisible (true);
    }

    g.setFont (juce::Font (12.5f, juce::Font::bold));
    g.setColour (OmniColors::accent);
    g.drawText (msg, b.withHeight (40).withY (b.getY() + 170), juce::Justification::centred);

    // Progress bar
    auto barR = b.withTop (b.getY() + 215).withHeight (6).reduced (60, 0);
    g.setColour (OmniColors::borderLight);
    g.fillRoundedRectangle (barR.toFloat(), 3.0f);
    g.setColour (OmniColors::accent);
    g.fillRoundedRectangle (barR.withWidth ((int)(barR.getWidth() * pct)).toFloat(), 3.0f);
}

void WorkspacePanel::rebuildStemLanes()
{
    // Remove lanes from scroll content
    for (auto* lane : mStemLanes)
        mScrollContent.removeChildComponent (lane);
    mStemLanes.clear();

    for (int i = 0; i < mProc.state.stems.size(); ++i)
    {
        auto* lane = mStemLanes.add (new StemLane (mProc, i));
        mScrollContent.addAndMakeVisible (lane);
        lane->update();
    }

    resized();
}

void WorkspacePanel::updateFromState()
{
    auto& st = mProc.state;
    bool hasMaster = st.masterLoaded;

    mMasterWave.setVisible (hasMaster &&
        (st.clipStatus == ClipStatus::Analyzed ||
         st.clipStatus == ClipStatus::Separated));

    if (hasMaster && st.clipStatus == ClipStatus::Analyzed)
        mMasterWave.setAudio (st.masterAudio, st.masterSampleRate);

    // Rebuild stem lanes when separation completes
    if (st.clipStatus == ClipStatus::Separated && mStemLanes.size() != st.stems.size())
        rebuildStemLanes();

    for (auto* lane : mStemLanes)
        lane->update();

    mMasterWave.setLoopRegion (st.looping.load(), st.loopStart, st.loopEnd);

    resized();
    repaint();
}

void WorkspacePanel::timerCallback()
{
    float pos = mProc.state.playPos.load();
    mMasterWave.setPlayPos (pos);
    // StemLanes have their own timers
}

bool WorkspacePanel::isInterestedInFileDrag (const juce::StringArray&) { return true; }
void WorkspacePanel::fileDragEnter (const juce::StringArray&, int, int) { mDragOver = true; repaint(); }
void WorkspacePanel::fileDragExit  (const juce::StringArray&)           { mDragOver = false; repaint(); }

void WorkspacePanel::filesDropped (const juce::StringArray& files, int, int)
{
    mDragOver = false;
    if (!files.isEmpty())
        mProc.loadFile (juce::File (files[0]));
    repaint();
}
