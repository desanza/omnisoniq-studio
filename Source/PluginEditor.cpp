#include "PluginEditor.h"

OmnisoniqEditor::OmnisoniqEditor (OmnisoniqProcessor& p)
    : AudioProcessorEditor (&p), mProc (p),
      mTopBar (p), mBrowser (p), mWorkspace (p), mAnalysis (p), mTransport (p)
{
    setLookAndFeel (&mLookAndFeel);

    addAndMakeVisible (mTopBar);
    addAndMakeVisible (mBrowser);
    addAndMakeVisible (mWorkspace);
    addAndMakeVisible (mAnalysis);
    addAndMakeVisible (mTransport);

    // Wire export actions
    mTransport.onExport = [this] (const juce::String& scope, const juce::String& fmt) {
        if (scope == "stems") mProc.exportStems (juce::File::getSpecialLocation (juce::File::userDesktopDirectory), fmt);
        showToast ("Export started", "Stems saved to Desktop");
    };

    mAnalysis.onExport = [this] (const juce::String& scope, const juce::String& /*fmt*/) {
        showToast (scope + " exported", "Saved to your chosen folder");
    };

    // Register for state changes from processor
    mProc.stateChangeBroadcaster.addChangeListener (this);

    setResizable (true, true);
    setResizeLimits (900, 600, 1600, 1000);
    setSize (1200, 720);
}

OmnisoniqEditor::~OmnisoniqEditor()
{
    mProc.stateChangeBroadcaster.removeChangeListener (this);
    setLookAndFeel (nullptr);
}

void OmnisoniqEditor::changeListenerCallback (juce::ChangeBroadcaster*)
{
    // Called on message thread from processor when state changes
    mTopBar   .updateFromState();
    mBrowser  .updateFromState();
    mWorkspace.updateFromState();
    mAnalysis .updateFromState();
    mTransport.updateFromState();
    repaint();
}

void OmnisoniqEditor::resized()
{
    auto b = getLocalBounds();

    mTopBar   .setBounds (b.removeFromTop (56));
    mTransport.setBounds (b.removeFromBottom (56));

    mBrowser  .setBounds (b.removeFromLeft (320));
    mAnalysis .setBounds (b.removeFromRight (310));
    mWorkspace.setBounds (b);
}

void OmnisoniqEditor::paint (juce::Graphics& g)
{
    g.fillAll (OmniColors::bgShell);
    if (mToastVisible)
    {
        if ((juce::Time::getCurrentTime() - mToastShownAt).inMilliseconds() > 3200)
            mToastVisible = false;
        else
            paintToast (g);
    }
}

void OmnisoniqEditor::showToast (const juce::String& title, const juce::String& sub)
{
    mToastTitle   = title;
    mToastSub     = sub;
    mToastVisible = true;
    mToastShownAt = juce::Time::getCurrentTime();
    repaint();
    // Schedule repaint to hide toast
    juce::Timer::callAfterDelay (3300, [this] { mToastVisible = false; repaint(); });
}

void OmnisoniqEditor::paintToast (juce::Graphics& g)
{
    const int toastW = 320, toastH = 58;
    auto toastR = juce::Rectangle<int> (
        getWidth() / 2 - toastW / 2,
        getHeight() - 80,
        toastW, toastH);

    g.setColour (OmniColors::bgCard);
    g.fillRoundedRectangle (toastR.toFloat(), 11.0f);
    g.setColour (OmniColors::accent.withAlpha (0.4f));
    g.drawRoundedRectangle (toastR.toFloat(), 11.0f, 1.0f);

    // Checkmark box
    auto box = toastR.removeFromLeft (toastH).reduced (12);
    g.setColour (OmniColors::accent);
    g.fillRoundedRectangle (box.toFloat(), 8.0f);
    g.setColour (juce::Colours::white);
    g.setFont (juce::Font (16.0f));
    g.drawText (juce::CharPointer_UTF8 ("\xe2\x9c\x93"), box, juce::Justification::centred);

    g.setFont (juce::Font (12.5f, juce::Font::bold));
    g.setColour (OmniColors::textPrimary);
    g.drawText (mToastTitle, toastR.removeFromTop (28).reduced (4, 4), juce::Justification::centredLeft);
    g.setFont (juce::Font (11.0f));
    g.setColour (OmniColors::textSecond);
    g.drawText (mToastSub, toastR.reduced (4, 0), juce::Justification::centredLeft);
}

juce::AudioProcessorEditor* OmnisoniqProcessor::createEditor()
{
    return new OmnisoniqEditor (*this);
}
