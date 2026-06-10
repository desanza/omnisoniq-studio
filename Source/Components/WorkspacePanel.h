#pragma once
#include <JuceHeader.h>
#include "../OmniColors.h"
#include "../PluginProcessor.h"
#include "WaveformComponent.h"
#include "StemLane.h"

class WorkspacePanel : public juce::Component,
                       public juce::FileDragAndDropTarget,
                       private juce::Timer
{
public:
    explicit WorkspacePanel (OmnisoniqProcessor& proc);
    ~WorkspacePanel() override;

    void paint   (juce::Graphics& g) override;
    void resized () override;
    void updateFromState ();

    bool isInterestedInFileDrag (const juce::StringArray& files) override;
    void filesDropped (const juce::StringArray& files, int, int) override;
    void fileDragEnter (const juce::StringArray&, int, int) override;
    void fileDragExit  (const juce::StringArray&) override;

private:
    void timerCallback() override;
    void rebuildStemLanes();
    void paintEmptyState (juce::Graphics& g);
    void paintProgressState (juce::Graphics& g, const juce::String& msg, float pct);

    OmnisoniqProcessor&         mProc;
    WaveformComponent           mMasterWave;
    juce::OwnedArray<StemLane>  mStemLanes;
    bool                        mDragOver { false };

    juce::Viewport              mScrollView;
    juce::Component             mScrollContent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WorkspacePanel)
};
