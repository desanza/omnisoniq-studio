#pragma once
#include <JuceHeader.h>
#include "../OmniColors.h"
#include "../PluginProcessor.h"

struct FileEntry
{
    juce::File file;
    bool       favourite { false };
    bool       loaded    { false };
};

class BrowserPanel : public juce::Component,
                     public juce::FileDragAndDropTarget,
                     public juce::DragAndDropContainer
{
public:
    explicit BrowserPanel (OmnisoniqProcessor& proc);

    void paint   (juce::Graphics& g) override;
    void resized () override;

    // FileDragAndDropTarget
    bool isInterestedInFileDrag (const juce::StringArray& files) override;
    void filesDropped (const juce::StringArray& files, int /*x*/, int /*y*/) override;

    void updateFromState ();

private:
    void openFileBrowser();
    void loadFile (const juce::File& f);
    void paintFileRow (juce::Graphics& g, const FileEntry& entry, juce::Rectangle<int> row, bool hover, bool odd);

    OmnisoniqProcessor&         mProc;

    juce::TextEditor            mSearch;
    juce::TextButton            mBrowseBtn { "Browse Files" };

    juce::Array<FileEntry>      mEntries;
    juce::Array<FileEntry>      mFiltered;
    int                         mHoveredRow { -1 };

    // Simple list rendered via paint
    class FileList : public juce::Component, public juce::ScrollBar::Listener
    {
    public:
        FileList (BrowserPanel& owner);
        void paint (juce::Graphics& g) override;
        void resized() override;
        void mouseMove  (const juce::MouseEvent& e) override;
        void mouseDown  (const juce::MouseEvent& e) override;
        void mouseExit  (const juce::MouseEvent& e) override;
        void scrollBarMoved (juce::ScrollBar* sb, double newRange) override;

        BrowserPanel&    mOwner;
        juce::ScrollBar  mScrollBar { true };
        int              mScrollPos { 0 };
        int              mHovered   { -1 };
        static constexpr int kRowH = 56;
    } mList;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BrowserPanel)
};
