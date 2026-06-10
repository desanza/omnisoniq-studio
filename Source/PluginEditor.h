#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "OmniLookAndFeel.h"
#include "Components/TopBar.h"
#include "Components/BrowserPanel.h"
#include "Components/WorkspacePanel.h"
#include "Components/AnalysisPanel.h"
#include "Components/TransportBar.h"

class OmnisoniqEditor : public juce::AudioProcessorEditor,
                        public juce::ChangeListener
{
public:
    explicit OmnisoniqEditor (OmnisoniqProcessor&);
    ~OmnisoniqEditor() override;

    void paint   (juce::Graphics&) override;
    void resized () override;

    void changeListenerCallback (juce::ChangeBroadcaster*) override;

private:
    OmnisoniqProcessor& mProc;
    OmniLookAndFeel     mLookAndFeel;

    TopBar         mTopBar;
    BrowserPanel   mBrowser;
    WorkspacePanel mWorkspace;
    AnalysisPanel  mAnalysis;
    TransportBar   mTransport;

    // Toast notification
    juce::String  mToastTitle, mToastSub;
    bool          mToastVisible { false };
    juce::Time    mToastShownAt;

    void showToast (const juce::String& title, const juce::String& sub);
    void paintToast (juce::Graphics& g);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OmnisoniqEditor)
};
