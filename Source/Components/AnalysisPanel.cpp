#include "AnalysisPanel.h"

AnalysisPanel::AnalysisPanel (OmnisoniqProcessor& proc) : mProc (proc)
{
    for (auto* b : { &mSeparateBtn, &mFastBtn, &mBalancedBtn, &mStudioBtn,
                     &mExportFullBtn, &mExportStems, &mExportAcca,
                     &mFmtWav, &mFmtMp3 })
        addAndMakeVisible (b);

    mSeparateBtn.onClick = [this] { mProc.separate (mCurrentModel); };

    mFastBtn    .setClickingTogglesState (true);
    mBalancedBtn.setClickingTogglesState (true);
    mStudioBtn  .setClickingTogglesState (true);
    mBalancedBtn.setToggleState (true, juce::dontSendNotification);

    auto modelBtn = [this] (juce::TextButton* b, const juce::String& model) {
        b->onClick = [this, b, model] {
            mCurrentModel = model;
            for (auto* x : { &mFastBtn, &mBalancedBtn, &mStudioBtn })
                x->setToggleState (x == b, juce::dontSendNotification);
        };
    };
    modelBtn (&mFastBtn,     "fast");
    modelBtn (&mBalancedBtn, "balanced");
    modelBtn (&mStudioBtn,   "studio");

    mFmtWav.setClickingTogglesState (true);
    mFmtMp3.setClickingTogglesState (true);
    mFmtWav.setToggleState (true, juce::dontSendNotification);

    mFmtWav.onClick = [this] { mCurrentFmt = "wav"; mFmtMp3.setToggleState (false, juce::dontSendNotification); };
    mFmtMp3.onClick = [this] { mCurrentFmt = "mp3"; mFmtWav.setToggleState (false, juce::dontSendNotification); };

    mExportStems  .onClick = [this] { doExport ("stems",    mCurrentFmt); };
    mExportFullBtn.onClick = [this] { doExport ("full",     mCurrentFmt); };
    mExportAcca   .onClick = [this] { doExport ("acapella", mCurrentFmt); };
}

void AnalysisPanel::doExport (const juce::String& scope, const juce::String& fmt)
{
    auto chooser = std::make_shared<juce::FileChooser> (
        "Choose export folder",
        juce::File::getSpecialLocation (juce::File::userDesktopDirectory));

    chooser->launchAsync (
        juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectDirectories,
        [this, scope, fmt, chooser] (const juce::FileChooser& fc) {
            auto dir = fc.getResult();
            if (!dir.exists()) return;
            if (scope == "stems")
                mProc.exportStems (dir, fmt);
            else
            {
                auto outFile = dir.getChildFile (mProc.state.clipTitle + "_" + scope + "." + fmt);
                mProc.exportMix (outFile, fmt);
            }
            if (onExport) onExport (scope, fmt);
        });
}

void AnalysisPanel::resized()
{
    auto b = getLocalBounds().reduced (14, 12);

    // Model buttons at top
    auto modelRow = b.removeFromTop (30);
    int w = modelRow.getWidth() / 3;
    mFastBtn    .setBounds (modelRow.removeFromLeft (w).reduced (2));
    mBalancedBtn.setBounds (modelRow.removeFromLeft (w).reduced (2));
    mStudioBtn  .setBounds (modelRow.reduced (2));

    b.removeFromTop (8);
    mSeparateBtn.setBounds (b.removeFromTop (36));
    b.removeFromTop (14);

    // Format
    auto fmtRow = b.removeFromTop (30);
    mFmtWav.setBounds (fmtRow.removeFromLeft (fmtRow.getWidth() / 2).reduced (2));
    mFmtMp3.setBounds (fmtRow.reduced (2));

    b.removeFromTop (8);
    mExportFullBtn.setBounds (b.removeFromTop (32));
    b.removeFromTop (4);
    mExportStems  .setBounds (b.removeFromTop (32));
    b.removeFromTop (4);
    mExportAcca   .setBounds (b.removeFromTop (32));
}

void AnalysisPanel::paintStatCard (juce::Graphics& g, juce::Rectangle<int> r,
                                    const juce::String& label, const juce::String& value,
                                    const juce::String& sub, juce::Colour accentCol)
{
    g.setColour (OmniColors::bgSurface);
    g.fillRoundedRectangle (r.toFloat(), 10.0f);
    g.setColour (OmniColors::borderLight);
    g.drawRoundedRectangle (r.toFloat(), 10.0f, 1.0f);

    g.setFont (juce::Font (9.5f, juce::Font::bold));
    g.setColour (OmniColors::textTertiary);
    g.drawText (label.toUpperCase(), r.reduced (12, 10).removeFromTop (14), juce::Justification::left);

    g.setFont (juce::Font (juce::Font::getDefaultMonospacedFontName(), 28.0f, juce::Font::bold));
    g.setColour (OmniColors::textPrimary);
    auto numArea = r.reduced (12, 0).withTop (r.getY() + 26).withHeight (34);
    g.drawText (value, numArea, juce::Justification::left);

    if (sub.isNotEmpty())
    {
        g.setFont (juce::Font (10.0f));
        g.setColour (accentCol);
        g.drawText (sub, r.reduced (12, 0).withBottom (r.getBottom() - 8).withTop (r.getBottom() - 22),
                    juce::Justification::left);
    }
}

void AnalysisPanel::paintProgressBar (juce::Graphics& g, juce::Rectangle<int> r,
                                       float pct, juce::Colour col)
{
    g.setColour (OmniColors::borderLight);
    g.fillRoundedRectangle (r.toFloat(), 3.0f);
    g.setColour (col);
    auto filled = r.withWidth ((int)(r.getWidth() * pct));
    g.fillRoundedRectangle (filled.toFloat(), 3.0f);
}

void AnalysisPanel::paint (juce::Graphics& g)
{
    g.fillAll (OmniColors::bgPanel);
    g.setColour (OmniColors::border);
    g.drawVerticalLine (0, 0.0f, (float)getHeight());

    auto b = getLocalBounds();

    // Header
    auto header = b.removeFromTop (44);
    g.setColour (OmniColors::border);
    g.drawHorizontalLine (header.getBottom(), 0.0f, (float)getWidth());
    g.setFont (juce::Font (12.5f, juce::Font::bold));
    g.setColour (OmniColors::textPrimary);
    g.drawText ("Analysis", header.reduced (14, 0), juce::Justification::centredLeft);

    auto& st = mProc.state;

    if (st.clipStatus == ClipStatus::Empty)
    {
        g.setColour (OmniColors::textTertiary);
        g.setFont (12.0f);
        g.drawText ("Import a file to see\nBPM, key, and stem tools.", b.reduced (20),
                    juce::Justification::centred, true);
        return;
    }

    auto inner = b.reduced (14, 10);

    // ── Loading/Analyzing progress ──
    if (st.clipStatus == ClipStatus::Loading || st.clipStatus == ClipStatus::Analyzing)
    {
        g.setFont (juce::Font (12.0f, juce::Font::bold));
        g.setColour (OmniColors::accent);
        auto msg = (st.clipStatus == ClipStatus::Loading) ? "Loading audio…" : "Analyzing…";
        g.drawText (msg, inner.removeFromTop (30), juce::Justification::centred);
        paintProgressBar (g, inner.removeFromTop (6),
                          st.clipStatus == ClipStatus::Loading ? st.loadProgress : st.analyzeProgress,
                          OmniColors::accent);
        return;
    }

    // ── Stats ──
    {
        auto row = inner.removeFromTop (90);
        auto left  = row.removeFromLeft (row.getWidth() / 2 - 4);
        auto right = row;

        paintStatCard (g, left,  "Tempo", juce::String ((int)st.analysis.bpm), "BPM · Locked", OmniColors::accent);
        paintStatCard (g, right, "Key",
                       st.analysis.key.upToFirstOccurrenceOf (" ", false, false),
                       st.analysis.key.fromFirstOccurrenceOf (" ", false, false),
                       OmniColors::success);
    }
    inner.removeFromTop (10);

    // ── Key confidence ──
    {
        g.setFont (juce::Font (10.0f));
        g.setColour (OmniColors::textTertiary);
        g.drawText ("Key confidence", inner.removeFromTop (16), juce::Justification::left);
        auto bar = inner.removeFromTop (6);
        paintProgressBar (g, bar, st.analysis.keyConf, OmniColors::success);
        inner.removeFromTop (14);
    }

    // ── Separating progress ──
    if (st.clipStatus == ClipStatus::Separating)
    {
        g.setFont (juce::Font (12.0f, juce::Font::bold));
        g.setColour (OmniColors::accent);
        auto pct = (int)(st.separateProgress * 100);
        g.drawText ("Separating stems — " + juce::String (pct) + "%",
                    inner.removeFromTop (24), juce::Justification::left);
        paintProgressBar (g, inner.removeFromTop (6), st.separateProgress, OmniColors::accent);
        return;
    }

    // ── Separated stem list ──
    if (st.clipStatus == ClipStatus::Separated && !st.stems.isEmpty())
    {
        g.setFont (juce::Font (10.5f, juce::Font::bold));
        g.setColour (OmniColors::textSecond);
        g.drawText ("STEMS READY", inner.removeFromTop (18), juce::Justification::left);

        for (auto* s : st.stems)
        {
            auto row = inner.removeFromTop (32).reduced (0, 2);
            g.setColour (OmniColors::bgSurface);
            g.fillRoundedRectangle (row.toFloat(), 7.0f);

            // Colour dot
            g.setColour (s->colour);
            g.fillEllipse (row.getX() + 9.0f, row.getCentreY() - 4.0f, 8.0f, 8.0f);

            g.setFont (juce::Font (11.5f));
            g.setColour (OmniColors::textPrimary);
            g.drawText (s->name, row.withLeft (row.getX() + 24), juce::Justification::centredLeft);

            juce::String statusStr = s->muted ? "Muted" : (s->soloed ? "Solo" : "Active");
            g.setFont (juce::Font (10.0f));
            g.setColour (OmniColors::textTertiary);
            g.drawText (statusStr, row.withRight (row.getRight() - 8), juce::Justification::centredRight);
        }
        inner.removeFromTop (8);
    }

    // The rest (controls) are rendered via child components
}

void AnalysisPanel::updateFromState()
{
    auto& st = mProc.state;
    bool hasAudio    = (st.clipStatus != ClipStatus::Empty && st.clipStatus != ClipStatus::Loading);
    bool canSeparate = (st.clipStatus == ClipStatus::Analyzed || st.clipStatus == ClipStatus::Separated);
    bool canExport   = (st.clipStatus == ClipStatus::Separated);

    mSeparateBtn.setEnabled (canSeparate);
    mFastBtn    .setEnabled (canSeparate);
    mBalancedBtn.setEnabled (canSeparate);
    mStudioBtn  .setEnabled (canSeparate);
    mExportFullBtn.setEnabled (hasAudio);
    mExportStems  .setEnabled (canExport);
    mExportAcca   .setEnabled (canExport);

    repaint();
}
