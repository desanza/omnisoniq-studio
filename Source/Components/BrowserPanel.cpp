#include "BrowserPanel.h"

static const juce::StringArray SUPPORTED_EXTS { ".wav", ".mp3", ".flac", ".aiff", ".aif", ".ogg", ".m4a" };

static bool isAudioFile (const juce::File& f)
{
    return SUPPORTED_EXTS.contains (f.getFileExtension().toLowerCase());
}

// ─── FileList ─────────────────────────────────────────────────────────────
BrowserPanel::FileList::FileList (BrowserPanel& owner) : mOwner (owner)
{
    addAndMakeVisible (mScrollBar);
    mScrollBar.addListener (this);
    setMouseCursor (juce::MouseCursor::PointingHandCursor);
}

void BrowserPanel::FileList::resized()
{
    mScrollBar.setBounds (getWidth() - 8, 0, 6, getHeight());
    mScrollBar.setRangeLimits (0.0, juce::jmax (0, (int)mOwner.mFiltered.size() * kRowH - getHeight()));
    mScrollBar.setCurrentRange (mScrollPos, getHeight());
}

void BrowserPanel::FileList::paint (juce::Graphics& g)
{
    g.fillAll (OmniColors::bgPanel);
    auto& entries = mOwner.mFiltered;
    if (entries.isEmpty())
    {
        g.setColour (OmniColors::textTertiary);
        g.setFont (12.0f);
        g.drawText ("No audio files.\nClick \"Browse Files\" to add.", getLocalBounds().reduced (20),
                    juce::Justification::centred, true);
        return;
    }

    int y0 = -mScrollPos;
    for (int i = 0; i < entries.size(); ++i)
    {
        auto row = getLocalBounds().withY (y0 + i * kRowH).withHeight (kRowH);
        if (row.getBottom() < 0 || row.getY() > getHeight()) continue;
        bool hover = (i == mHovered);
        mOwner.paintFileRow (g, entries[i], row, hover, i % 2 == 0);
    }
}

void BrowserPanel::FileList::mouseMove (const juce::MouseEvent& e)
{
    int row = (e.y + mScrollPos) / kRowH;
    if (row != mHovered) { mHovered = row; repaint(); }
}

void BrowserPanel::FileList::mouseDown (const juce::MouseEvent& e)
{
    int row = (e.y + mScrollPos) / kRowH;
    if (juce::isPositiveAndBelow (row, mOwner.mFiltered.size()))
        mOwner.loadFile (mOwner.mFiltered[row].file);
}

void BrowserPanel::FileList::mouseExit (const juce::MouseEvent&)
{
    mHovered = -1; repaint();
}

void BrowserPanel::FileList::scrollBarMoved (juce::ScrollBar* /*sb*/, double newRange)
{
    mScrollPos = (int)newRange; repaint();
}

// ─── BrowserPanel ─────────────────────────────────────────────────────────
BrowserPanel::BrowserPanel (OmnisoniqProcessor& proc)
    : mProc (proc), mList (*this)
{
    addAndMakeVisible (mSearch);
    addAndMakeVisible (mBrowseBtn);
    addAndMakeVisible (mList);

    mSearch.setTextToShowWhenEmpty ("Search files…", OmniColors::textTertiary);
    mSearch.setColour (juce::TextEditor::backgroundColourId, OmniColors::bgSurface);
    mSearch.setColour (juce::TextEditor::outlineColourId,    OmniColors::border);
    mSearch.setColour (juce::TextEditor::textColourId,       OmniColors::textPrimary);
    mSearch.setFont (12.5f);
    mSearch.onTextChange = [this] {
        juce::String q = mSearch.getText().toLowerCase();
        mFiltered.clear();
        for (auto& e : mEntries)
            if (q.isEmpty() || e.file.getFileName().toLowerCase().contains (q))
                mFiltered.add (e);
        mList.mScrollPos = 0;
        mList.resized();
        mList.repaint();
    };

    mBrowseBtn.onClick = [this] { openFileBrowser(); };
}

void BrowserPanel::resized()
{
    auto b = getLocalBounds();

    // Top bar
    auto top = b.removeFromTop (52).reduced (10, 8);
    mSearch   .setBounds (top.removeFromTop (32));
    top.removeFromTop (4);

    auto btns = b.removeFromTop (36).reduced (10, 4);
    mBrowseBtn.setBounds (btns);

    b.removeFromTop (1);
    mList.setBounds (b);
}

void BrowserPanel::paint (juce::Graphics& g)
{
    g.fillAll (OmniColors::bgPanel);
    g.setColour (OmniColors::border);
    g.drawVerticalLine (getWidth() - 1, 0.0f, (float)getHeight());
}

void BrowserPanel::openFileBrowser()
{
    auto chooser = std::make_shared<juce::FileChooser> (
        "Select audio files",
        juce::File::getSpecialLocation (juce::File::userMusicDirectory),
        "*.wav;*.mp3;*.flac;*.aiff;*.aif;*.ogg;*.m4a");

    chooser->launchAsync (
        juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectMultipleItems |
        juce::FileBrowserComponent::canSelectFiles,
        [this, chooser] (const juce::FileChooser& fc) {
            for (auto& f : fc.getResults())
            {
                if (!isAudioFile (f)) continue;
                bool dup = false;
                for (auto& e : mEntries) if (e.file == f) { dup = true; break; }
                if (!dup) mEntries.add ({ f, false, false });
            }
            // Rebuild filtered
            mFiltered = mEntries;
            mList.resized();
            mList.repaint();
        });
}

void BrowserPanel::loadFile (const juce::File& f)
{
    // Mark loaded state
    for (auto& e : mEntries) e.loaded = (e.file == f);
    for (auto& e : mFiltered) e.loaded = (e.file == f);
    mList.repaint();

    mProc.loadFile (f);
}

void BrowserPanel::paintFileRow (juce::Graphics& g, const FileEntry& entry,
                                  juce::Rectangle<int> row, bool hover, bool /*odd*/)
{
    auto b = row.toFloat().reduced (6, 3);

    // Background
    if (entry.loaded)
    {
        g.setColour (OmniColors::accent.withAlpha (0.13f));
        g.fillRoundedRectangle (b, 8.0f);
        g.setColour (OmniColors::accent.withAlpha (0.32f));
        g.drawRoundedRectangle (b, 8.0f, 1.0f);
    }
    else if (hover)
    {
        g.setColour (juce::Colour (0x10ffffff));
        g.fillRoundedRectangle (b, 8.0f);
    }

    // Colour swatch based on extension
    juce::Colour swatch = OmniColors::accent;
    auto ext = entry.file.getFileExtension().toLowerCase();
    if (ext == ".wav")  swatch = OmniColors::stemMelody;
    if (ext == ".mp3")  swatch = OmniColors::stemVocals;
    if (ext == ".flac") swatch = OmniColors::stemBass;
    if (ext == ".aiff" || ext == ".aif") swatch = OmniColors::stemDrums;

    g.setColour (swatch);
    g.fillRoundedRectangle (b.removeFromLeft (4).reduced (0, 6), 2.0f);
    b.removeFromLeft (8);

    // Filename
    g.setFont (juce::Font (12.0f, juce::Font::bold));
    g.setColour (OmniColors::textPrimary);
    auto nameArea = b.removeFromTop (18);
    g.drawText (entry.file.getFileNameWithoutExtension(), nameArea.toNearestInt(),
                juce::Justification::centredLeft, true);

    // Sub info
    g.setFont (juce::Font (10.5f));
    g.setColour (OmniColors::textTertiary);
    auto size = entry.file.getSize();
    juce::String sub = ext.toUpperCase().removeCharacters (".") + "  ·  "
                       + juce::File::descriptionOfSizeInBytes (size);
    g.drawText (sub, b.toNearestInt(), juce::Justification::centredLeft, true);
}

bool BrowserPanel::isInterestedInFileDrag (const juce::StringArray& files)
{
    for (auto& f : files)
        if (isAudioFile (juce::File (f))) return true;
    return false;
}

void BrowserPanel::filesDropped (const juce::StringArray& files, int, int)
{
    for (auto& fs : files)
    {
        juce::File f (fs);
        if (!isAudioFile (f)) continue;
        bool dup = false;
        for (auto& e : mEntries) if (e.file == f) { dup = true; break; }
        if (!dup) mEntries.add ({ f, false, false });
    }
    mFiltered = mEntries;
    mList.resized();
    mList.repaint();
    if (!files.isEmpty())
        loadFile (juce::File (files[0]));
}

void BrowserPanel::updateFromState()
{
    // Sync loaded markers
    for (auto& e : mEntries)  e.loaded = (e.file == mProc.state.currentFile);
    for (auto& e : mFiltered) e.loaded = (e.file == mProc.state.currentFile);
    mList.repaint();
}
