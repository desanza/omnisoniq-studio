#include "TransportBar.h"

TransportBar::TransportBar (OmnisoniqProcessor& proc)
    : mProc (proc),
      mToStartBtn ("|<"), mPlayBtn ("PLAY"), mStopBtn ("■"), mLoopBtn ("↺"),
      mExportBtn  ("Export")
{
    for (auto* b : { &mToStartBtn, &mPlayBtn, &mStopBtn, &mLoopBtn, &mExportBtn })
        addAndMakeVisible (b);

    addAndMakeVisible (mPosLabel);
    addAndMakeVisible (mBpmLabel);
    addAndMakeVisible (mZoomSlider);

    mToStartBtn.onClick = [this] { mProc.stop(); };
    mPlayBtn.onClick    = [this] {
        if (mProc.state.playing.load()) mProc.pause(); else mProc.play();
    };
    mStopBtn.onClick    = [this] { mProc.stop(); };
    mLoopBtn.setClickingTogglesState (true);
    mLoopBtn.onClick    = [this] { mProc.setLoop (mLoopBtn.getToggleState()); };

    mExportBtn.onClick  = [this] {
        if (onExport) onExport ("stems", "wav");
    };

    mPosLabel.setFont (juce::Font (juce::Font::getDefaultMonospacedFontName(), 13.0f, juce::Font::plain));
    mPosLabel.setColour (juce::Label::textColourId, OmniColors::textPrimary);
    mPosLabel.setText ("0:00 / 0:00", juce::dontSendNotification);

    mBpmLabel.setFont (juce::Font (juce::Font::getDefaultMonospacedFontName(), 13.0f, juce::Font::bold));
    mBpmLabel.setColour (juce::Label::textColourId, OmniColors::textPrimary);
    mBpmLabel.setText ("— BPM", juce::dontSendNotification);

    mZoomSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    mZoomSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    mZoomSlider.setRange (1.0, 4.0, 0.5);
    mZoomSlider.setValue (1.0);

    startTimerHz (20);
}

TransportBar::~TransportBar() { stopTimer(); }

void TransportBar::resized()
{
    auto b = getLocalBounds().reduced (8, 6);

    mToStartBtn.setBounds (b.removeFromLeft (28).reduced (2));
    mPlayBtn   .setBounds (b.removeFromLeft (42).reduced (2));
    mStopBtn   .setBounds (b.removeFromLeft (28).reduced (2));
    mLoopBtn   .setBounds (b.removeFromLeft (28).reduced (2));

    b.removeFromLeft (12);
    mPosLabel  .setBounds (b.removeFromLeft (100));
    b.removeFromLeft (8);
    mBpmLabel  .setBounds (b.removeFromLeft (80));

    mExportBtn .setBounds (b.removeFromRight (80).reduced (2));
    b.removeFromRight (8);
    mZoomSlider.setBounds (b.removeFromRight (90));
    b.removeFromRight (4);
    // spacer in b is fine
}

void TransportBar::paint (juce::Graphics& g)
{
    g.fillAll (OmniColors::bgTopBar);
    g.setColour (OmniColors::border);
    g.drawHorizontalLine (0, 0.0f, (float)getWidth());
}

void TransportBar::updateFromState()
{
    bool playing = mProc.state.playing.load();
    mPlayBtn.setButtonText (playing ? "PAUSE" : "PLAY");
    mLoopBtn.setToggleState (mProc.state.looping.load(), juce::dontSendNotification);

    bool canExport = (mProc.state.clipStatus == ClipStatus::Separated);
    mExportBtn.setEnabled (canExport);

    // BPM
    if (mProc.state.clipStatus != ClipStatus::Empty)
    {
        float bpm = mProc.state.analysis.bpm;
        mBpmLabel.setText (bpm > 0 ? juce::String (bpm, 1) + " BPM" : "— BPM",
                           juce::dontSendNotification);
    }
    else
    {
        mBpmLabel.setText ("— BPM", juce::dontSendNotification);
    }
}

void TransportBar::timerCallback()
{
    float pos = mProc.state.playPos.load();
    float dur = mProc.state.masterDuration;

    auto fmt = [&] (float frac) -> juce::String {
        float t = frac * dur;
        int m = (int)(t / 60);
        int s = (int)(t) % 60;
        return juce::String (m) + ":" + juce::String (s).paddedLeft ('0', 2);
    };

    mPosLabel.setText (fmt (pos) + " / " + fmt (1.0f), juce::dontSendNotification);
}
